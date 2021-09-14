
#ifndef KELBON_TUPLE_HPP
#define KELBON_TUPLE_HPP

#include <tuple> // structure binding specializtions and compare types for <=>
#include <type_traits>

#include "kelbon_type_traits_base.hpp"
#include "kelbon_type_traits_advanced.hpp"
#include "kelbon_type_traits_variadic.hpp"

namespace kelbon {

	template<typename T, size_t index>
	struct value_in_tuple {
		constexpr auto operator <=>(const value_in_tuple&) const = default;
		T value;
	};

	template<typename...>
	struct tuple_base;

	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<value_list<size_t, Indexes...>, Types...>
		: value_in_tuple<Types, Indexes>... {
	private:
		using type_array = type_list<Types...>;

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr tuple_base() = default;
		constexpr tuple_base(const tuple_base&) = default;
		constexpr tuple_base(tuple_base&&) = default;
		constexpr tuple_base& operator=(const tuple_base&) = default;
		constexpr tuple_base& operator=(tuple_base&&) = default;
		constexpr auto operator<=>(const tuple_base&) const = default;

		template<typename ... Args>
		requires(sizeof...(Args) == sizeof...(Types))
		constexpr explicit tuple_base(Args&& ... args)
			noexcept(((std::is_nothrow_constructible_v<Types, Args>) && ...))
			: value_in_tuple<Types, Indexes>(std::forward<Args>(args))...
		{}

		template<size_t index>
		constexpr const get_type<index>& get() const noexcept {
			return static_cast<const value_in_tuple<get_type<index>, index>* const>(this)->value;
		}
		template<size_t index>
		constexpr get_type<index>& get() noexcept {
			return static_cast<value_in_tuple<get_type<index>, index>*>(this)->value;
		}
	};

	template<typename ... Types>
	struct tuple
		: tuple_base<make_value_list<size_t, sizeof...(Types)>, Types...> {
	private:
		using base_t = tuple_base<make_value_list<size_t, sizeof...(Types)>, Types...>;
	public:
		using base_t::base_t;
	};
	
	// TEMPLATE FUNCTION tuple_cat
	template<typename ... Tuples, size_t ... TupleIndexes, size_t ... IndexesInTuple>
	constexpr auto tuple_cat_helper2(value_list<size_t, TupleIndexes...>, value_list<size_t, IndexesInTuple...>, tuple<Tuples...>&& megatuple)
		noexcept(std::is_nothrow_constructible_v<insert_type_list_t<tuple, merge_type_lists_t<decay_t<Tuples>...>>,
			typename extract_type_list_t<decay_t<typename type_list<Tuples...>::template get_element<TupleIndexes>>>::template get_element<IndexesInTuple>...>) {
		using type_array = merge_type_lists_t<decay_t<Tuples>...>;
		using big_tuple_t = insert_type_list_t<tuple, type_array>;

		return big_tuple_t(std::forward<tuple<Tuples...>>(megatuple).template get<TupleIndexes>().template get<IndexesInTuple>()...);
	}
	template<typename ... Tuples, size_t ... Values, size_t ... Indexes>
	constexpr decltype(auto) tuple_cat_helper1(value_list<size_t, Values...>, value_list<size_t, Indexes...>, tuple<Tuples...>&& megatuple)
		noexcept(noexcept(
			tuple_cat_helper2(
				std::declval<merge_value_lists_t<make_value_list<size_t, extract_type_list_t<decay_t<Tuples>>::count_of_arguments, Values, 0ull>...>>(),
				std::declval<value_list<size_t, Indexes...>>(),
				std::declval<tuple<Tuples...>>()
			))) {
		using tuple_indexes = merge_value_lists_t<make_value_list<size_t, extract_type_list_t<decay_t<Tuples>>::count_of_arguments, Values, 0ull>...>; // TRICKY))))))

		return tuple_cat_helper2(tuple_indexes{}, value_list<size_t, Indexes...>{}, std::forward<tuple<Tuples...>>(megatuple));
	}

	template<typename ... Tuples>
	constexpr auto tuple_cat(Tuples&& ... tuples)
		noexcept(noexcept(
			tuple_cat_helper1(
				std::declval<make_value_list<size_t, sizeof...(Tuples)>>(),
				std::declval<merge_value_lists_t<make_value_list<size_t, extract_type_list_t<decay_t<Tuples>>::count_of_arguments>...>>(),
				std::declval<tuple<Tuples&&...>>())))
		{
		using index_array = merge_value_lists_t<make_value_list<size_t, extract_type_list_t<decay_t<Tuples>>::count_of_arguments>...>;
		return tuple_cat_helper1(make_value_list<size_t, sizeof...(Tuples)>{}, index_array{}, tuple<Tuples&&...>(std::forward<Tuples>(tuples)...));
	}
	
	// deduction guide
	template<typename ... Types>
	tuple(Types&& ...)->tuple<::std::remove_reference_t<Types>...>;

} // namespace kelbon

// structure binding
namespace std {
	template<size_t index, typename ... Types>
	struct tuple_element<index, ::kelbon::tuple<Types...>> {
		using type = typename ::kelbon::type_list<Types...>::template get_element<index>;
	};
	template<typename ... Types>
	struct tuple_size<::kelbon::tuple<Types...>> {
		static constexpr size_t value = sizeof...(Types);
	};
	template<size_t index, typename ... Types>
	constexpr decltype(auto) get(const ::kelbon::tuple<Types...>& tupl) noexcept {
		return tupl.template get<index>();
	}
}

#endif // !KELBON_TUPLE_HPP
