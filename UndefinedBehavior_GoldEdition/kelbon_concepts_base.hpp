
#ifndef KELBON_CONCEPTS_BASE_HPP
#define KELBON_CONCEPTS_BASE_HPP

#include <concepts>

namespace kelbon {
	// CONCEPT exist for situatuions like requires { { expression } -> exist }; 
	template<typename T>
	concept exist = true;

	// CONCEPT numeric
	namespace detail {
		template<typename T, typename U>
		concept convertible_or_same = std::convertible_to<T, U> || std::same_as<T, U>;
	} // namespace detail

	template<typename T>
	concept numeric = requires(T a, T b) {
		{a + b}->detail::convertible_or_same<T>; // сложение двух числе даёт тот же тип. Для char неверно было бы проверять std::same_as, т.к. char + char = int
		{a - b}->detail::convertible_or_same<T>;
	} && requires {
		static_cast<T>(4); // можно создавать от int
	};

	// CONCEPT contextually_convertible_to_bool
	template<typename T>
	concept contextually_convertible_to_bool = requires (T value) {
		value ? 1 : 2;
	};

	//CONCEPT one_of
	template<typename T, typename ... Args>
	concept one_of = sizeof...(Args) > 0 && ((std::same_as<T, Args>) || ...);

	// CONCEPT co_awaitable

	template<typename T>
	concept has_member_operator_co_await = requires (T value) {
		value.operator co_await();
	};
	template<typename T>
	concept has_global_co_await = requires (T value) {
		operator co_await(value);
	};
	template<typename T>
	concept co_awaitable = has_member_operator_co_await<T> || has_global_co_await<T>;

} // namespace kelbon

#endif // !KELBON_CONCEPTS_BASE_HPP
