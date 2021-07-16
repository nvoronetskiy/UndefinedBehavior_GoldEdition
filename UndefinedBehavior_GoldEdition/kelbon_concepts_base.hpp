
#ifndef KELBON_CONCEPTS_BASE_HPP
#define KELBON_CONCEPTS_BASE_HPP

#include "kelbon_type_traits_functional.hpp"
#include <concepts>

namespace kelbon {
	// for situatuions like requires { { expression } -> exist }; 
	template<typename T>
	concept exist = true;

	// CONCEPT NUMERIC
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

	namespace func {
		// CONCEPT returns (checking result type of the function)
		template<typename Func, typename ResultType>
		concept returns = function<Func> && requires {
			{ declval<typename signature<Func>::result_type>() }->std::same_as<ResultType>;
		};

		// CONCEPT accepts
		template<typename Func, typename ... Types>
		concept accepts = function<Func> && requires {
			{ declval<typename signature<Func>::result_type>() }->std::same_as<type_list<Types...>>;
		};

	} // namespace func
} // namespace kelbon

#endif // !KELBON_CONCEPTS_BASE_HPP
