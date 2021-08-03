
#ifndef KELBON_TYPE_TRAITS_VARIADIC_HPP
#define KELBON_TYPE_TRAITS_VARIADIC_HPP

#include "kelbon_type_traits_base.hpp"

namespace kelbon {

	// TEMPLATE TYPE_LIST
	namespace detail {
		template<size_t, size_t, typename ...>
		struct type_of_element_helper;

		template<size_t index, size_t current_index>
		struct type_of_element_helper<index, current_index> {
			using type = false_type;
		};
		template<size_t index, size_t current_index, typename First, typename ... Types>
		struct type_of_element_helper<index, current_index, First, Types...> {
			using type = conditional_t<index == current_index, First, typename type_of_element_helper<index, current_index + 1, Types...>::type>;
		};
	} // namespace detail

	template<size_t index, typename ... Types>
	struct type_of_element {
		static_assert(index < sizeof...(Types) || sizeof...(Types) == 0, "Index >= arguments count");
		using type = typename detail::type_of_element_helper<index, 0, Types...>::type;
	};

	template<typename ... Types>
	struct type_list {
		static constexpr size_t count_of_arguments = sizeof...(Types);
		template<size_t index>
		using get_element = typename type_of_element<index, Types...>::type;
	};

	// TRAIT MERGE_TYPE_LISTS
	template<typename...>
	struct merge_type_lists;

	// объединение списка параметров двух шаблонов(только типы)
	template<template<typename...> typename T, template<typename...> typename U, typename ... A, typename ... B>
	struct merge_type_lists<T<A...>, U<B...>> {
		using type = type_list<A..., B...>;
	};

	template<template<typename...> typename T, template<typename...> typename U, typename ... A, typename ... B, typename ... Types>
	struct merge_type_lists<T<A...>, U<B...>, Types...> {
		using type = typename merge_type_lists<type_list<A..., B...>, Types...>::type;
	};

	template<typename ... Types>
	using merge_type_lists_t = typename merge_type_lists<Types...>::type;

} // namespace kelbon

#endif // !KELBON_TYPE_TRAITS_VARIADIC_HPP
