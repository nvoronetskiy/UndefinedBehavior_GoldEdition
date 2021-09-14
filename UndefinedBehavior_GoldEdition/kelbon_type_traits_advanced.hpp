
#ifndef KELBON_TYPE_TRAITS_ADVANCED_HPP
#define KELBON_TYPE_TRAITS_ADVANCED_HPP

#include "kelbon_concepts_base.hpp"
#include "kelbon_utility.hpp"
#include "kelbon_type_traits_variadic.hpp"

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

	template<auto What, typename Where>
	struct push_back;
	template<typename T, typename U, U What, T ... Values>
	struct push_back<What, value_list<size_t, Values...>> {
		using type = value_list<size_t, Values..., static_cast<T>(What)>;
	};

	// TRAIT make_value_sequence
	namespace detail {
		template<typename T, T...>
		struct value_sequence;
		template<typename T, T count, T start_value, T current_index, T step, T ... Values>
		struct value_sequence<T, count, start_value, current_index, step, Values...> {
			using type = typename value_sequence<T, count, start_value, current_index + 1, step, Values..., static_cast<T>(start_value + step * sizeof...(Values))> ::type;
		};
		template<typename T, T count, T start_value, T step, T ... Values> // specialization for end of resursion because count == index
		struct value_sequence<T, count, start_value, count, step, Values...> {
			using type = value_list<T, Values...>;
		};
	} // namespace detail

	template<typename T, size_t Count, T StartValue = static_cast<T>(0), T Step = static_cast<T>(1)>
	using make_value_list = typename detail::value_sequence<T, Count, StartValue, static_cast<T>(0), Step>::type;

	// TRAIT reverse_type_list
	namespace detail {
		template<typename ...>
		struct reverse_type_list_helper;

		template<typename ... Types, int ... Indexes>
		struct reverse_type_list_helper<value_list<int, Indexes...>, Types...> {
		private:
			using help_type = type_list<Types...>;
		public:
			using type = type_list<typename help_type::template get_element<Indexes>...>;
		};
	} // namespace detail

	template<typename ... Types>
	struct reverse_type_list {
		using type = typename detail::reverse_type_list_helper<make_value_list<int, sizeof...(Types), (int)(sizeof...(Types) - 1), -1>, Types...>::type;
	};
	template<typename ... Types>
	struct reverse_type_list<type_list<Types...>> {
		using type = typename reverse_type_list<Types...>::type;
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
	struct is_explicit_default_constructible : ::std::bool_constant<is_explicit_constructible_v<T>> {};
	template<typename T>
	struct is_explicit_copy_constructible : ::std::bool_constant<is_explicit_copy_constructible_v<T>> {};
	template<typename T>
	struct is_explicit_move_constructible : ::std::bool_constant<is_explicit_move_constructible_v<T>> {};

	// TRAIT generate_unique_type
	template<auto V = []() {}>
	using generate_unique_type = decltype(V);

	// TRAIT is_instance_of
	template<template<typename...> typename Template, typename TypeToCheck>
    struct is_instance_of {
    private:
        template<typename>
        struct check : ::std::false_type {};
        template<typename ... Args>
        struct check<Template<Args...>> : ::std::true_type {};
    public:
        static constexpr inline bool value = check<decay_t<TypeToCheck>>::value;
    };

	template<template<typename...> typename Template, typename TypeToCheck>
	constexpr inline bool is_instance_of_v = is_instance_of<Template, TypeToCheck>::value;

	// TRAIT add_effect ( do not works for lambdas with capture because they are not convertible to function pointer)
	// todo (may be) Effect accepting Args.. and returning tuple<Args...> so its can modify accepted paramters(some filter...)
	template<auto V, auto Effect, typename RetType, typename ... Args>
	constexpr auto wrap_help(RetType(*F)(Args...)) {
		return
			[](Args... args) -> RetType {
			Effect();
			return V(args...);
		};
	}

	namespace func {
		template<auto F, auto Effect>
		requires (count_of_arguments<decltype(Effect)> == 0)
		constexpr decltype(+F) add_effect = +wrap_help<F, Effect>(+F);
	}

} // namespace kelbon

#endif // !KELBON_TYPE_TRAITS_ADVANCED_HPP

