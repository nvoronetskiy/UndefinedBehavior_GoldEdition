
#ifndef KELBON_TYPE_TRAITS_ADVANCED_HPP
#define KELBON_TYPE_TRAITS_ADVANCED_HPP

#include "kelbon_concepts_base.hpp"
#include "kelbon_utility.hpp"

namespace kelbon {

	// TRAIT value_list
	namespace detail {
		template<typename, size_t, size_t, auto...>
		struct value_of_element_helper;

		template<typename T, size_t index, size_t current_index, T First, T ... Values>
		struct value_of_element_helper<T, index, current_index, First, Values...> {
			static constexpr T value = value_of_element_helper<T, index, current_index + 1ull, Values...>::value;
		};
		template<typename T, size_t index, T First, T ... Values>
		struct value_of_element_helper<T, index, index, First, Values...> {
			static constexpr T value = First;
		};
	} // namespace detail

	template<size_t index, typename T, T ... Values>
	struct value_of_element {
		static_assert(index < sizeof...(Values) || sizeof...(Values) == 0, "Index >= arguments count");
		static constexpr T value = detail::value_of_element_helper<T, index, 0ull, Values...>::value;
	};
	template<typename T, T ... Values>
	struct value_list {
		static constexpr size_t count_of_arguments = sizeof...(Values);
		template<size_t index>
		static constexpr T get_element = value_of_element<index, T, Values...>::value;
	};

	// TRAIT merge_value_lists
	template<typename...>
	struct merge_value_lists;

	template<typename T, T ... A, T ... B>
	struct merge_value_lists<value_list<T, A...>, value_list<T, B...>> {
		using type = value_list<T, A..., B...>;
	};

	template<typename T, T ... A, T ... B, typename ... Types>
	struct merge_value_lists<value_list<T, A...>, value_list<T, B...>, Types...> {
		using type = typename merge_value_lists<value_list<T, A..., B...>, Types...>::type;
	};

	template<typename ... Types>
	using merge_value_lists_t = typename merge_value_lists<Types...>::type;

	// TRAIT make_value_sequence
	namespace detail {
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
	} // namespace detail

	template<auto ... Args>
	struct sequence {
		// чтобы внезапно неправильно не раскрылс€ пак при написании ...
		// при этом пришлось раскрыть пак, т.к. на decltype(Args) компил€тор реагирует невн€тно
		using type = typename type_list<decltype(Args)...>::template get_element<0>;

		static constexpr size_t count_of_arguments = sizeof...(Args);
		template<size_t index>
		static constexpr auto get_element = value_list<type, Args...>::template get_element<index>;
	};

	template<typename...>
	struct extract_value_sequence;

	template<typename T, template<typename X, X... args> typename U, T ... args>
	struct extract_value_sequence<U<T, args...>> {
		using type = sequence<args...>;
	};

	// Ќужно €вно указывать какого типа ¬—≈ “–» аргумента. Ќапример, если нужно size_t, то должно быть make_value_sequen<size_t(N), size_t(M), size_t(K)>
	// 10.07.2021 - работает на clang ( дл€ положительных чисел, иначе приводит почему то к size_t и громадные числа получаютс€), на msvc не работает, но clang не умеет в л€мбды в 
	// невычислимых контекстах на данный момент( а мне это нужно щас)))
	// по идее можно заменить типы второго и третьего аргумента на decltype(Count), но пока это не работает(нормально)
	template<numeric auto Count, numeric auto StartValue = decltype(Count){0}, numeric auto Step = decltype(Count){1} >
	using make_value_sequence = typename extract_value_sequence<typename detail::value_sequence<decltype(Count), Count, StartValue, decltype(Count){0}, Step > ::type > ::type;

	// пока плохо работает на всех компил€торах auto деклараци€ using как make_value_sequence, так что использую пока что такой вариант.
	template<typename T, size_t Count, T StartValue = static_cast<T>(0), T Step = static_cast<T>(1)>
	using make_value_list = typename detail::value_sequence<T, Count, StartValue, static_cast<T>(0), Step>::type;

	// TRAIT reverse_type_list
	namespace detail {
		template<typename ...>
		struct reverse_type_list_helper;

		template<typename ... Types, int ... Args>
		struct reverse_type_list_helper<sequence<Args...>, Types...> {
			static_assert(sizeof...(Types) == sizeof...(Args), "Incorrect sequence or type list");
			using help_type = type_list<Types...>;
			using type = type_list<typename help_type::template get_element<Args>...>;
		};
	} // namespace detail

	template<typename ... Types>
	struct reverse_type_list {
		using type = typename detail::reverse_type_list_helper<make_value_sequence<static_cast<int>(sizeof...(Types)), static_cast<int>(sizeof...(Types)) - 1, -1>, Types...>::type;
	};

	template<typename ... Types>
	using reverse_type_list_t = typename reverse_type_list<Types...>::type;

	// TRAIT is_explicit_constructible
	template<typename T, typename ... Args>
	struct is_explicit_constructible {
	private:
		template<typename Type>
		static true_type check(Type&&);

		template<typename ... Something>
		static consteval auto func(int)->decltype(check<T>({ std::declval<Something>()... }), bool{}) {
			return false;
		}
		template<typename ... Something>
		static consteval bool func(...) {
			return true;
		}
	public:
		static constexpr bool value = func<Args...>(0);
	};

	template<typename T, typename ... Types>
	constexpr inline bool is_explicit_constructible_v = is_explicit_constructible<T, Types...>::value;
	template<typename T>
	constexpr inline bool is_explicit_default_constructible_v = is_explicit_constructible_v<T>;
	template<typename T>
	constexpr inline bool is_explicit_copy_constructible_v = is_explicit_constructible_v<T, const T&>;
	template<typename T>
	constexpr inline bool is_explicit_move_constructible_v = is_explicit_constructible_v<T, T&&>;
	// for all_in_pack or atleast_one_in_pack
	template<typename T>
	struct is_explicit_default_constructible : std::bool_constant<is_explicit_constructible_v<T>> {};
	template<typename T>
	struct is_explicit_copy_constructible : std::bool_constant<is_explicit_copy_constructible_v<T>> {};
	template<typename T>
	struct is_explicit_move_constructible : std::bool_constant<is_explicit_move_constructible_v<T>> {};

} // namespace kelbon

#endif // !KELBON_TYPE_TRAITS_ADVANCED_HPP

