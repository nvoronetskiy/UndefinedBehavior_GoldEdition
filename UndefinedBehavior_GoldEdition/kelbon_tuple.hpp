
#ifndef KELBON_TUPLE_HPP
#define KELBON_TUPLE_HPP

#include <tuple> // structure binding specializtions
#include <type_traits>
#include <compare>

#include "kelbon_type_traits_advanced.hpp"

namespace kelbon {

	// behavior of this T wrapper should be as similar to T as possible
	template<typename T, size_t index>
	struct value_in_tuple {
		constexpr explicit(is_explicit_default_constructible_v<T>) value_in_tuple()
			noexcept(std::is_nothrow_default_constructible_v<T>)
			requires(std::is_default_constructible_v<T>) = default;

		constexpr explicit(is_explicit_copy_constructible_v<T>) value_in_tuple(const T& value)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires(std::is_copy_constructible_v<T>)
			: value(value)
		{}

		constexpr explicit(is_explicit_move_constructible_v<T>) value_in_tuple(T&& value)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			: value(std::move(value))
		{}

		template<size_t other_index>
		constexpr explicit(is_explicit_copy_constructible_v<T>) value_in_tuple(const value_in_tuple<T, other_index>& other)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires(std::is_copy_constructible_v<T>)
			: value(other.value)
		{}

		template<size_t other_index>
		constexpr explicit(is_explicit_move_constructible_v<T>) value_in_tuple(value_in_tuple<T, other_index>&& other)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			: value(std::move(other.value))
		{}

		template<size_t other_index>
		constexpr value_in_tuple& operator=(const value_in_tuple<T, other_index>& other)
			noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires(std::is_copy_assignable_v<T>) {
			value = other.value;
		}

		constexpr value_in_tuple& operator=(const T& other_value)
			noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires(std::is_copy_assignable_v<T>) {
			value = other_value;
		}

		template<size_t other_index>
		constexpr value_in_tuple& operator=(value_in_tuple<T, other_index>&& other)
			noexcept(std::is_nothrow_move_assignable_v<T>) {
			value = std::move(other.value);
		}

		constexpr value_in_tuple& operator=(T&& other_value)
			noexcept(std::is_nothrow_move_assignable_v<T>) {
			value = std::move(other_value);
		}

		constexpr ~value_in_tuple() = default;

		template<size_t other_index>
		constexpr auto operator <=>(const value_in_tuple<T, other_index>& other) const noexcept {
			if (value == other.value) {
				return std::strong_ordering::equivalent;
			}
			else {
				if (value < other.value) {
					return std::strong_ordering::less;
				}
				else {
					return std::strong_ordering::greater;
				}
			}
		}

		T value;
	};

	template<bool, typename...>
	struct tuple_base;

	constexpr decltype(auto) if_not_ref_move(auto&& v) noexcept {
		if constexpr (std::is_lvalue_reference_v<decltype(v)>) {
			return v;
		}
		else {
			return std::move(v);
		}
	}

	// COPY CONSTRUCTOR AND BUG IN MSVC (requires and other shit cant delete copy constructor) FORCE ME INTO CREATE TWO SPECIALIZTIONS OF TUPLE HERE... GOD FORGIVE ME

	// избегание неправильного раскрытия паков через дополнительную прослойку наследования
	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<false, value_list<size_t, Indexes...>, Types...> : value_in_tuple<Types, Indexes>... {
	protected:
		using type_array = type_list<Types...>; // запоминание типов для взятия значений по индексу/типу

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr explicit(atleast_one_in_pack<is_explicit_default_constructible, Types...>) tuple_base()
			noexcept(all_in_pack<std::is_nothrow_default_constructible, Types...>)
			: value_in_tuple<Types, Indexes>()...
		{}

		// Bug of MSVC workaround
		//template<typename FakeArgument = tuple_base<value_list<size_t, Indexes...>, Types...>, typename = std::enable_if< copy cttbl...>.
		//constexpr tuple_base(const FakeArgument& other)
		//	noexcept(((std::is_nothrow_copy_constructible_v<Types>) && ...))
		//	: value_in_tuple<Types, Indexes>(other.template get<Indexes>())...
		//{}

		//constexpr tuple_base(const tuple_base& other)
			//noexcept(((std::is_nothrow_copy_constructible_v<Types>) && ...))
			//requires(((std::is_copy_constructible_v<Types>) && ...)) // чтобы один удалённый конструктор копирования не ломал компиляцию
			//: value_in_tuple<Types, Indexes>(other.template get<Indexes>())...
		//{}

		constexpr explicit(atleast_one_in_pack<is_explicit_move_constructible, Types...>) tuple_base(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_constructible, Types...>)
			: value_in_tuple<Types, Indexes>(std::move(other.template get<Indexes>()))...
		{}
		// избегание перекрытия других конструкторов, в том числе мув/копи, т.к. видимо из-за requires он выигрывает перегрузку у мув конструктора
		template<typename ... Args>
		requires(sizeof...(Args) == sizeof...(Types))
		constexpr tuple_base(Args&& ... args)
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
		
		constexpr tuple_base& operator=(const tuple_base& other)
			noexcept(all_in_pack<std::is_nothrow_copy_assignable, Types...>)
			requires(all_in_pack<std::is_copy_assignable, Types...>) {
			((this->template get<Indexes>() = other.template get<Indexes>()), ...);
			return *this;
		}
		constexpr tuple_base& operator=(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_assignable, Types...>) {

			((this->template get<Indexes>() = std::move(other.template get<Indexes>())), ...);
			return *this;
		}
	};

	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<true, value_list<size_t, Indexes...>, Types...> : value_in_tuple<Types, Indexes>... {
	protected:
		using type_array = type_list<Types...>; // запоминание типов для взятия значений по индексу/типу

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr explicit(atleast_one_in_pack<is_explicit_default_constructible, Types...>) tuple_base()
			noexcept(all_in_pack<std::is_nothrow_default_constructible, Types...>)
			: value_in_tuple<Types, Indexes>()...
		{}

		constexpr explicit(atleast_one_in_pack<is_explicit_copy_constructible, Types...>) tuple_base(const tuple_base& other)
			noexcept(all_in_pack<std::is_nothrow_copy_constructible, Types...>)
			: value_in_tuple<Types, Indexes>(other.template get<Indexes>())...
		{}

		constexpr explicit(atleast_one_in_pack<is_explicit_move_constructible, Types...>) tuple_base(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_constructible, Types...>)
			: value_in_tuple<Types, Indexes>(if_not_ref_move(other.template get<Indexes>()))...
		{}
		// избегание перекрытия других конструкторов, в том числе мув/копи, т.к. видимо из-за requires он выигрывает перегрузку у мув конструктора
		template<typename ... Args>
		requires(sizeof...(Args) == sizeof...(Types))
		constexpr tuple_base(Args&& ... args)
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
		
		constexpr tuple_base& operator=(const tuple_base& other)
			noexcept(all_in_pack<std::is_nothrow_copy_assignable, Types...>)
			requires(all_in_pack<std::is_copy_assignable, Types...>) {
			((this->template get<Indexes>() = other.template get<Indexes>()), ...);
			return *this;
		}
		constexpr tuple_base& operator=(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_assignable, Types...>) {

			((this->template get<Indexes>() = std::move(other.template get<Indexes>())), ...);
			return *this;
		}
	};

	// использую тут "технологии" множественного наследования с последовательностью чисел
	template<typename ... Types>
	class tuple : public tuple_base<all_in_pack<std::is_copy_constructible, Types...>, make_value_list<size_t, sizeof...(Types)>, Types...> {
	protected:
		// тут просто передача полномочий на всё tuple_base, потому что там раскрыт пак индексов позволяющий красиво всё писать
		using base_t = tuple_base<all_in_pack<std::is_copy_constructible, Types...>, make_value_list<size_t, sizeof...(Types)>, Types...>;
	public:
		using base_t::base_t;
	};

	// TEMPLATE FUNCTION tuple_cat
	/*
	template<typename...>
	struct tuple_cat_helper;

	template<typename ... Types1, typename ... Types2, size_t ... V1, size_t ... V2>
	struct tuple_cat_helper<tuple<Types1...>, tuple<Types2...>, value_list<size_t, V1...>, value_list<size_t, V2...>> {
		static constexpr auto cat(tuple<Types1...>&& tpl1, tuple<Types2...>&& tpl2) noexcept(false) { // todo - noexcept
			return tuple<Types1..., Types2...>(std::forward<tuple<Types1...>>(tpl1).template get<V1>()..., std::forward<tuple<Types1...>>(tpl2).template get<V2>()...);
		}
	};

	template<typename ... Types1, typename ... Types2>
	constexpr auto tuple_cat(tuple<Types1...>&& tpl1, tuple<Types2...>&& tpl2) // todo - noexcept
		noexcept(false) {

		return tuple_cat_helper<tuple<Types1...>, tuple<Types2...>,
			make_value_list<size_t, sizeof...(Types1)>,
			make_value_list<size_t, sizeof...(Types2)>>::cat(std::forward<tuple<Types1...>>(tpl1), std::forward<tuple<Types2...>>(tpl2));
	}

	template<typename ... Types1, typename ... Types2, typename ... Tuples>
	constexpr auto tuple_cat(tuple<Types1...>&& tpl1, tuple<Types2...>&& tpl2, Tuples&& ... tuples) // todo - noexcept
		noexcept(false)
		->insert_type_list_t<::kelbon::tuple, merge_type_lists_t<Tuples...>> {
		//using type_array = merge_type_lists_t<Tuples...>;
		//using big_tuple_t = insert_type_list_t<::kelbon::tuple, type_array>;
		// for iterating throught index_array
		//using indexes = make_value_list<size_t, type_array::count_of_arguments>;
		//using tuple_length_array = value_list<size_t, extract_type_list_t<Tuples>::count_of_arguments ...>;
		//using index_array = merge_value_lists_t<make_value_list<size_t, extract_type_list_t<Tuples>::count_of_arguments> ...>;

		return tuple_cat
		(tuple_cat_helper<tuple<Types1...>, tuple<Types2...>,
			make_value_list<size_t, sizeof...(Types1)>,
			make_value_list<size_t, sizeof...(Types2)>>::cat(std::forward<tuple<Types1...>>(tpl1), std::forward<tuple<Types2...>>(tpl2)),
			std::forward<Tuples>(tpl1)...);
	}*/
	template<size_t i>
	struct increment {
		static constexpr int value = i + 1;
	};

	template<typename ... Tuples, size_t ... TupleIndexes, size_t ... IndexesInTuple>
	constexpr auto tuple_cat_helper2(value_list<size_t, TupleIndexes...>, value_list<size_t, IndexesInTuple...>, tuple<Tuples...>&& megatuple) {
		using type_array = merge_type_lists_t<std::decay_t<Tuples>...>;
		using big_tuple_t = insert_type_list_t<tuple, type_array>;
		// ещё проблема, && не достать, т.к. & в get всегда её перебьёт...
		return big_tuple_t(std::forward<tuple<Tuples...>>(megatuple).template get<TupleIndexes>().template get<IndexesInTuple>()...); // todo - add forward here
	}
	template<typename ... Tuples, size_t ... Values, size_t ... Indexes>
	constexpr decltype(auto) tuple_cat_helper1(value_list<size_t, Values...>, value_list<size_t, Indexes...>, tuple<Tuples...>&& megatuple) {
		using tuple_indexes = merge_value_lists_t<make_value_list<size_t, extract_type_list_t<decay_t<Tuples>>::count_of_arguments, Values, 0ull>...>; // TRICKY))))))

		return tuple_cat_helper2(tuple_indexes{}, value_list<size_t, Indexes...>{}, std::forward<tuple<Tuples...>>(megatuple));
	}

	template<typename ... Tuples>
	constexpr auto tuple_cat(Tuples&& ... tuples) // todo - noexcept
		noexcept(false) {
		using index_array = merge_value_lists_t<make_value_list<size_t, extract_type_list_t<decay_t<Tuples>>::count_of_arguments>...>;
		// ТОЧНО!!! Нужно просто сделать value sequence где будут длина тупла индексов 0, дальше длина второго тупла индексов 1 и т.д.!! И можно будет раскрыть по
		// двум одновременно!
		return tuple_cat_helper1(make_value_list<size_t, sizeof...(Tuples)>{}, index_array{}, tuple<Tuples&&...>(std::forward<Tuples>(tuples)...));
	}

	// deduction guide
	// чтобы этот гайд не перекрывал возможность move конструктора тут enable_if
	template<typename First, typename ... Types>
	tuple(First&&, Types&& ...)->tuple<
		std::enable_if_t<
		!std::same_as<decay_t<First>, tuple<std::remove_reference_t<First>, std::remove_reference_t<Types>...>>,
		std::remove_reference_t<First>>,
		std::remove_reference_t<Types>...
		>;

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
