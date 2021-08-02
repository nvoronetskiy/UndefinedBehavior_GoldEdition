

#ifndef KELBON_ACTION_HPP
#define KELBON_ACTION_HPP

#include "kelbon_concepts_functional.hpp"
#include "kelbon_memory_block.hpp"

namespace kelbon {

	template<size_t Size, typename ResultType, typename ... ArgTypes>
	struct base_remember_call {
		virtual ResultType CallByMemory(memory_block<Size>& block, ArgTypes ... args) const = 0;
	};

	template<size_t, typename...>
	struct remember_call;

	template<callable Actor, typename ResultType, typename ... ArgTypes, size_t Size>
	struct remember_call<Size, Actor, ResultType, type_list<ArgTypes...>>
		: base_remember_call<Size, ResultType, ArgTypes...> {
		virtual ResultType CallByMemory(memory_block<Size>& block, ArgTypes ... args) const override {
			if constexpr (!functor<Actor>) {
				const auto& [invocable] = block.GetDataAs<Actor>();
				return invocable(args...);
			} // non const, so its mutable lambda or functor with no const operator()
			else if constexpr (!signature<Actor>::is_const) {
				// всё равно будет компилироваться, нужно обернуть в какую то штуку, например always_int<Actor>(...)
				auto& [invocable] = block.GetDataAs<Actor>();
				return invocable(args...);
			}
			else {
				const auto& [invocable] = block.GetDataAs<Actor>();
				return invocable(args...);
			}
		}
	};

	template<method Actor, typename ResultType, typename OwnerPtrType, typename ... ArgTypes, size_t Size>
	struct remember_call<Size, Actor, ResultType, type_list<OwnerPtrType, ArgTypes...>>
		: base_remember_call<Size, ResultType, OwnerPtrType, ArgTypes...> {
		virtual ResultType CallByMemory(memory_block<Size>& block, OwnerPtrType owner_this, ArgTypes ... args) const override {
			const auto& [invocable] = block.GetDataAs<ResultType(std::remove_pointer_t<OwnerPtrType>::*)(ArgTypes...)>();
			return ((*owner_this).*invocable)(args...);
		}
	};

	class empty_function_call : public ::std::exception {
		using ::std::exception::exception;
	};

	template<typename, size_t = 55, typename = void> // last argument - fake(for deduction guide)
	class action;

	template<typename ResultType, typename ... ArgTypes, size_t Size>
	class action<ResultType(ArgTypes...), Size> {
	protected:
		using suitable_func_type = ResultType(ArgTypes...);

		mutable memory_block<Size> memory;
		void* invoker;

		template<callable Actor>
		void RememberHowToCall() noexcept {
			using result_type = typename signature<Actor>::result_type;
			using parameter_list = typename signature<Actor>::parameter_list;
			new(&invoker) remember_call<Size, Actor, result_type, parameter_list>{};
		}

		ResultType Call(ArgTypes... args) const {
			return reinterpret_cast<const base_remember_call<Size, ResultType, ArgTypes...>*>
				(&invoker)->CallByMemory(memory, args...);
		}
	public:
		constexpr action()
			noexcept(std::is_nothrow_default_constructible_v<memory_block<Size>>)
			: memory(), invoker(nullptr)
		{}
		constexpr action(action&& other) noexcept : memory(std::move(other.memory)), invoker(other.invoker) {
			other.invoker = nullptr;
		}
		// in case value is a SomeType& template parameter, so its not callable,
		// its good behavior because memory_block takes control over actor
		template<callable Actor>
		constexpr action(Actor&& actor) noexcept : memory(std::forward<Actor>(actor)) {
			static_assert(func::returns<Actor, ResultType>, "Incorrect result type of the function");
			static_assert(func::accepts<Actor, ArgTypes...>, "Incorrent argument list of the function");
			RememberHowToCall<Actor>();
		}

		// for deduction guide support
		constexpr action(action<ResultType, Size, type_list<ArgTypes...>>&& action_from_deduction_guide)
			noexcept(std::is_nothrow_move_constructible_v<decltype(*this)>)
			: action(static_cast<action&&>(action_from_deduction_guide)) {}
		constexpr action& operator=(action<ResultType, Size, type_list<ArgTypes...>>&& action_from_deduction_guide)
		noexcept(std::is_nothrow_move_assignable_v<decltype(*this)>) {
			return *this = static_cast<action&&>(action_from_deduction_guide);
		}

		// may throw double_free_possible if no avalible copy constructor for stored value
		[[nodiscard]] action Clone() const {
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
			memory = memory_block<Size, ::kelbon::tuple>(std::forward<Actor>(something));
			RememberHowToCall<Actor>();
			return *this;
		}
		template<typename Actor> requires (functor<Actor>&& std::is_copy_constructible_v<Actor>)
		constexpr action& operator=(const Actor& something) noexcept(std::is_nothrow_copy_constructible_v<Actor>) {
			static_assert(func::returns<Actor, ResultType>, "Incorrect result type of the function");
			static_assert(func::accepts<Actor, ArgTypes...>, "Incorrent argument list of the function");
			Actor copy = something;
			memory = memory_block<Size, ::kelbon::tuple>(copy);
			RememberHowToCall<Actor>();
			return *this;
		}
		// for functions/methods and pointers to it
		constexpr action& operator=(suitable_func_type* something) noexcept {
			memory = memory_block<Size, ::kelbon::tuple>(something);
			RememberHowToCall<suitable_func_type*>();
			return *this;
		}
		constexpr action& operator=(action&& other) noexcept {
			if (this == &other) [[unlikely]] {
				return *this;
			}
			invoker = other.invoker;
			memory = std::move(other.memory);
			other.invoker = nullptr;
			return *this;
		}

		
		[[nodiscard]] constexpr bool Empty() const noexcept {
			return invoker == nullptr;
		}

		
		constexpr ResultType operator()(ArgTypes ... args) const {
			if (Empty()) [[unlikely]] {
				throw empty_function_call("You called an empty function, kelbon::action operator()");
			}
			return Call(args...);
		}
	};

	// for deduction guide only
	template<typename ResultType, typename ... ArgTypes, size_t Size>
	class action<ResultType, Size, type_list<ArgTypes...>>
		: public action<ResultType(ArgTypes...), Size> {
	private:
		using base_t = action<ResultType(ArgTypes...), Size>;
	public:
		using base_t::base_t;
	};


	// only for deduction guide ( ? : works bad here)))
	consteval size_t size_helper(size_t sz) noexcept {
		constexpr size_t default_size = 55;
		return sz < default_size ? default_size : sz;
	}

	// Deduction guide
	template<callable Something>
	action(Something&&)->action<typename signature<Something>::result_type, size_helper(sizeof(Something)), typename signature<Something>::parameter_list>;

} // namespace kelbon



#endif // !KELBON_ACTION_HPP

