
#ifndef KELBON_CONCEPTS_BASE_HPP
#define KELBON_CONCEPTS_BASE_HPP

#include "kelbon_type_traits_base.hpp"

namespace kelbon {
	namespace detail {
		template<typename T, typename U>
		concept same_as_helper = is_same<T, U>::value;
	} // namespace detail

	template<typename T, typename U>
	concept same_as = detail::same_as_helper<T, U> && detail::same_as_helper<U, T>;

	template<typename T>
	concept numeric = requires(T a, T b) {
		{a + b}->same_as<T>; // сложение двух числе даёт тот же тип. Для char неверно, т.к. char + char = int
		{a - b}->same_as<T>;
	}&& requires {
		static_cast<T>(4); // можно создавать от числа
	};
} // namespace kelbon

#endif // !KELBON_CONCEPTS_BASE_HPP
