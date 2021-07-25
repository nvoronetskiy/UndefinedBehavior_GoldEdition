

#ifndef KELBON_ACTION_HPP
#define KELBON_ACTION_HPP

#include "kelbon_concepts_functional.hpp"
#include "kelbon_memory_block.hpp"
#include <vector>
namespace kelbon {

	template<typename ResultType, typename ... ArgTypes>
	struct base_remember_call {
		virtual ResultType CallByMemory(memory_block<55>& block, ArgTypes ... args) const = 0;
	};

	template<typename...>
	struct remember_call;

	template<callable Actor, typename ResultType, typename ... ArgTypes>
	struct remember_call<Actor, ResultType, type_list<ArgTypes...>>
		: base_remember_call<ResultType, ArgTypes...> {
		virtual ResultType CallByMemory(memory_block<55>& block, ArgTypes ... args) const override {
			if constexpr (!functor<Actor>) {
				const auto& [invocable] = block.GetDataAs<Actor>();
				return invocable(std::forward<std::remove_reference_t<ArgTypes>>(args)...);
			} // non const, so its mutable lambda or functor with no const operator()
			else if constexpr (!signature<Actor>::is_const) {
				// всё равно будет компилироваться, нужно обернуть в какую то штуку, например always_int<Actor>(...)
				auto& [invocable] = block.GetDataAs<Actor>();
				return invocable(std::forward<std::remove_reference_t<ArgTypes>>(args)...);
			}
			else {
				const auto& [invocable] = block.GetDataAs<Actor>();
				return invocable(std::forward<std::remove_reference_t<ArgTypes>>(args)...);
			}
		}
	};

	class empty_function_call : public ::std::exception {
		using ::std::exception::exception;
	};

	template<typename...>
	class action;

	template<typename ResultType, typename ... ArgTypes>
	class action<ResultType(ArgTypes...)> {
	protected:
		using suitable_func_type = ResultType(ArgTypes...);

		mutable memory_block<55> memory; // 64 byte on x64 TODO
		void* invoker;

		template<callable Actor>
		void RememberHowToCall() noexcept {
			using result_type = typename signature<Actor>::result_type;
			using parameter_list = typename signature<Actor>::parameter_list;
			new(&invoker) remember_call<Actor, result_type, parameter_list>{};
		}
		// interesting forward here... ArgTypes may be reference, but forward<T&> ill formed
		ResultType Call(ArgTypes... args) const {
			return reinterpret_cast<const base_remember_call<ResultType, ArgTypes...>*>
				(&invoker)->CallByMemory(memory, std::forward<std::remove_reference_t<ArgTypes>>(args)...);
		}
	public:
		constexpr action()
			noexcept(std::is_nothrow_default_constructible_v<memory_block<55>>)
			: memory(), invoker(nullptr)
		{}
		constexpr action(action&& other) noexcept : memory(std::move(other.memory)), invoker(other.invoker) {
			other.invoker = nullptr;
		}
		// in case value its a SomeType& template parameter, so its not callable,
		// its good behavior because memory_block takes control of the actor
		template<callable Actor>
		constexpr action(Actor&& actor) noexcept : memory(std::forward<Actor>(actor)) {
			static_assert(func::returns<Actor, ResultType>, "Incorrect result type of the function");
			static_assert(func::accepts<Actor, ArgTypes...>, "Incorrent argument list of the function");
			RememberHowToCall<Actor>();
		}

		// may throw double_free_possible if no avalible copy constructor for stored value
		action Clone() const {
			action clone;
			clone.invoker = invoker;
			clone.memory = memory.Clone();
			return clone;
		}

		// for lambdas, functors
		template<callable Actor>
		constexpr action& operator=(Actor&& something) noexcept {
			static_assert(func::returns<Actor, ResultType>, "Incorrect result type of the function");
			static_assert(func::accepts<Actor, ArgTypes...>, "Incorrent argument list of the function");
			memory = memory_block<55, ::kelbon::tuple>(std::forward<Actor>(something));
			RememberHowToCall<Actor>();
			return *this;
		}
		template<typename Actor> requires (functor<Actor>&& std::is_copy_constructible_v<Actor>)
		constexpr action& operator=(const Actor& something) noexcept(std::is_nothrow_copy_constructible_v<Actor>) {
			static_assert(func::returns<Actor, ResultType>, "Incorrect result type of the function");
			static_assert(func::accepts<Actor, ArgTypes...>, "Incorrent argument list of the function");
			Actor copy = something;
			memory = memory_block<55, ::kelbon::tuple>(copy);
			RememberHowToCall<Actor>();
			return *this;
		}
		// for functions/methods and pointers to it
		constexpr action& operator=(suitable_func_type* something) noexcept {
			memory = memory_block<55, ::kelbon::tuple>(something);
			RememberHowToCall<suitable_func_type*>();
			return *this;
		}
		constexpr action& operator=(action&& other) noexcept {
			if (this == &other) {
				return *this;
			}
			invoker = other.invoker;
			memory = std::move(other.memory);
			other.invoker = nullptr;
			return *this;
		}

		
		constexpr bool Empty() const noexcept {
			return invoker == nullptr;
		}

		
		constexpr ResultType operator()(ArgTypes ... args) const {
			if (Empty()) {
				throw empty_function_call("You called an empty function, kelbon::action operator()");
			}
			return Call(std::forward<std::remove_reference_t<ArgTypes>>(args)...);
		}
	};

} // namespace kelbon



#endif // !KELBON_ACTION_HPP

