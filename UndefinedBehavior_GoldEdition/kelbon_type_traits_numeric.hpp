
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

// TRAIT VALUE_SEQUENCE TODO - улучшить (sequence) и перенести в variadic
template<numeric T, T...>
struct value_sequence;

template<numeric T, T count, T start_value, T current_index, T step, T ... Values>
struct value_sequence<T, count, start_value, current_index, step, Values...> {
	using type = typename value_sequence<T, count, start_value, current_index + 1, step, Values..., static_cast<T>(start_value + step * sizeof...(Values))> ::type;
};
template<numeric T, T count, T start_value, T step, T ... Values> // specialization for end of resursion because count == index
struct value_sequence<T, count, start_value, count, step, Values...> {
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

// Нужно явно указывать какого типа ВСЕ ТРИ аргумента. Например, если нужно size_t, то должно быть make_value_sequen<size_t(N), size_t(M), size_t(K)>
// 10.07.2021 - работает на clang ( для положительных чисел, иначе приводит почему то к size_t и громадные числа получаются), на msvc не работает, но clang не умеет в лямбды в 
// невычислимых контекстах на данный момент( а мне это нужно щас)))
// по идее можно заменить типы второго и третьего аргумента на decltype(Count), но пока это не работает(нормально)
template<numeric auto Count, numeric auto StartValue = decltype(Count){0}, numeric auto Step = decltype(Count){1} >
using make_value_sequence = typename extract_value_sequence<value_sequence<decltype(Count), Count, StartValue, decltype(Count){0}, Step>::type>::type;

// TEMPLATE REVERSE_TYPE_LIST
namespace detail {
	template<typename ...>
	struct reverse_type_list_helper;

	template<typename ... Types, int ... Args>
	struct reverse_type_list_helper<sequence<Args...>, Types...> {
		static_assert(sizeof...(Types) == sizeof...(Args), "Incorrect sequence or type list");
		using help_type = type_list<Types...>;
		using type = type_list<typename help_type::template argument_type<Args>...>;
	};
} // namespace detail

template<typename ... Types>
struct reverse_type_list {
	using type = typename detail::reverse_type_list_helper<make_value_sequence<static_cast<int>(sizeof...(Types)), static_cast<int>(sizeof...(Types)) - 1, -1>, Types...>::type;
};

#endif // !KELBON_TYPE_TRAITS_NUMERIC_HPP

