
#ifndef KELBON_TYPE_TRAITS_NUMERIC_HPP
#define KELBON_TYPE_TRAITS_NUMERIC_HPP

#include "kelbon_concepts_base.hpp"

// TRAIT VALUE_LIST
namespace detail {
	template<int index, int current_index, int First, int ... Values>
	struct value_of_element_helper {
		static constexpr int value = index == current_index ? First : value_of_element_helper<index, current_index + 1, Values...>::value;
	};
} // namespace detail
template<size_t index, int ... Values>
struct value_of_element {
	static_assert(index < sizeof...(Values) || sizeof...(Values) == 0, "Index >= arguments count");
	static constexpr int value = detail::value_of_element_helper<index, 0, Values...>::value;
};
template<typename T, T ... Values>
struct value_list {
	static constexpr size_t count_of_arguments = sizeof...(Values);
	template<size_t index> // TODO - исправить всё с интов на T
	static constexpr T argument_value = value_of_element<index, Values...>::value;
};

// TRAIT INTEGER SEQUENCE
template<numeric T, T...>
struct value_sequence;

template<numeric T, T count, T start_value, T current_index, T ... Values>
struct value_sequence<T, count, start_value, current_index, Values...> {
	using type = typename value_sequence<T, count, start_value, current_index + 1, Values..., start_value + sizeof...(Values)>::type;
};
template<numeric T, T count, T start_value, T ... Values> // specialization for end of resursion because count == index
struct value_sequence<T, count, start_value, count, Values...> {
	using type = value_list<T, Values...>;
};

template<auto ... Args>
struct sequence{};

template<typename...>
struct extract_value_sequence;

template<typename T, template<typename T, T... args> typename U, T ... args>
struct extract_value_sequence<U<T, args...>> {
	using type = sequence<args...>;
};

// 10.07.2021 - работает на clang ( для положительных чисел, иначе приводит почему то к size_t и громадные числа получаются), на msvc не работает, но clang не умеет в лямбды в 
// невычислимых контекстах на данный момент( а мне это нужно щас)))
template<numeric auto Count, numeric auto StartValue = decltype(Count){0}>
using make_value_sequence = typename extract_value_sequence<value_sequence<decltype(Count), Count, StartValue, decltype(Count){0}>::type>::type;

#endif // !KELBON_TYPE_TRAITS_NUMERIC_HPP

