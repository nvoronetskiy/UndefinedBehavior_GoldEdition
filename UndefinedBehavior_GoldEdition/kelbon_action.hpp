

#ifndef KELBON_ACTION_HPP
#define KELBON_ACTION_HPP

#include "kelbon_concepts_functional.hpp"
#include "kelbon_memory_block.hpp"

namespace kelbon {

	constexpr inline size_t default_action_size = 55;

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
			auto& [invocable] = block.GetDataAs<Actor>();
			return invocable(args...);
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

	template<typename, size_t = default_action_size>
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
		constexpr action(action&& other) noexcept
			: memory(std::move(other.memory)), invoker(other.invoker) {
			other.invoker = nullptr;
		}

		// in case value is a SomeType& template parameter, so its not callable,
		// its good behavior because memory_block takes control over actor
		template<callable Actor>
		requires(func::returns<Actor, ResultType>&& func::accepts<Actor, ArgTypes...>)
		constexpr action(Actor&& actor)
			noexcept(std::is_nothrow_constructible_v<memory_block<Size>, decltype(actor)>)
			: memory(std::forward<Actor>(actor)) {
			RememberHowToCall<Actor>();
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
		requires(func::returns<Actor, ResultType>&& func::accepts<Actor, ArgTypes...>)
		constexpr action& operator=(Actor&& something) noexcept {
			memory = memory_block<Size, ::kelbon::tuple>(std::forward<Actor>(something));
			RememberHowToCall<Actor>();
			return *this;
		}
		template<typename Actor>
		requires (functor<Actor>&& std::is_copy_constructible_v<Actor> && func::returns<Actor, ResultType>&& func::accepts<Actor, ArgTypes...>)
		constexpr action& operator=(const Actor& something) noexcept(std::is_nothrow_copy_constructible_v<Actor>) {
			memory = memory_block<Size, ::kelbon::tuple>(something);
			RememberHowToCall<Actor>();
			return *this;
		}

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

		
		[[nodiscard]] constexpr inline bool Empty() const noexcept {
			return invoker == nullptr;
		}
		// use it before Clone method if you want to not catch exception
		[[nodiscard]] constexpr inline bool CanBeCopiedNow() const noexcept {
			return memory.CanBeCopied();
		}
		
		constexpr ResultType operator()(ArgTypes ... args) const {
			if (Empty()) [[unlikely]] {
				throw empty_function_call("You called an empty function, kelbon::action operator()");
			}
			return Call(args...);
		}
	};

	// only for deduction guide ( ? : works bad in it)))
	consteval size_t size_helper(size_t sz) noexcept {
		return sz < default_action_size ? default_action_size : sz;
	}

	// Deduction guide for functions and function pointers
	template<typename ResultType, typename ... ArgTypes>
	action(ResultType(*)(ArgTypes...))->action<ResultType(ArgTypes...), default_action_size>;

	// Deduction guide for functors/lamdas with capture/methods
	template<callable Something, typename ... Types>
	action(Something&&)->action<typename signature<Something>::func_type, size_helper(sizeof(Something))>;

} // namespace kelbon

#endif // !KELBON_ACTION_HPP

