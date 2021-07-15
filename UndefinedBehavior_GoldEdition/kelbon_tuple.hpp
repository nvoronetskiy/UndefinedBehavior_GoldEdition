
#ifndef KELBON_TUPLE_HPP
#define KELBON_TUPLE_HPP

#include <tuple> // structure binding specializtions
#include <type_traits>

#include "kelbon_type_traits_numeric.hpp"

// todo - специализация memory_block для последовательности типов, которая в себя будет включать просто memory_block от размера, подходящего для типов
// и там сделать deduction guide для конструктора от списка типов, чтобы писать просто значения, вместо tuple{....}

namespace kelbon {

	template<typename T, size_t index>
	struct value_in_tuple {
		constexpr value_in_tuple()
			noexcept(std::is_nothrow_default_constructible_v<T>)
			: value()
		{}
		// опять проверка, чтобы не перекрывало move конструктор(хотя казалось бы и не должно...)
		template<typename U> requires (!std::same_as<U, value_in_tuple<T, index>>)
		constexpr value_in_tuple(U&& v)
			noexcept(std::is_nothrow_constructible_v<T, U>)
			: value(std::forward<U>(v))
		{}
		constexpr value_in_tuple(const value_in_tuple& other)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
			: value(other.value)
		{}
		constexpr value_in_tuple(value_in_tuple&& other)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			: value(std::move(other.value))
		{}
		constexpr value_in_tuple& operator=(const value_in_tuple& other)
			noexcept(std::is_nothrow_copy_assignable_v<T>) {

			value = other.value;
			return *this;
		}
		constexpr value_in_tuple& operator=(value_in_tuple&& other)
			noexcept(std::is_nothrow_move_assignable_v<T>) {

			value = std::move(other.value);
			return *this;
		}

		constexpr ~value_in_tuple() = default;
		// есть вероятность, что здесь можно добавить operator spaceship <=>
		T value;
	};

	template<typename...>
	struct tuple_base;

	// избегание неправильного раскрытия паков через дополнительную прослойку наследования
	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<value_list<size_t, Indexes...>, Types...> : value_in_tuple<Types, Indexes>... {
	protected:
		using type_array = type_list<Types...>; // запоминание типов для взятия значений по индексу/типу

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr tuple_base()
			noexcept(((std::is_nothrow_default_constructible_v<Types>) && ...))
			: value_in_tuple<Types, Indexes>()...
		{}

		// казалось бы, что проверять второй раз здесь не нужно(requires), но если не проверить,
		// то этот конструктор "закрывает" собой move конструктор
		template<typename ... Args> requires (((std::constructible_from<Types, Args>) && ...))
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

		constexpr tuple_base(const tuple_base& other)
			noexcept(((std::is_nothrow_copy_constructible_v<Types>) && ...))
			: value_in_tuple<Types, Indexes>(other.get<Indexes>())...
		{}
		constexpr tuple_base(tuple_base&& other)
			noexcept(((std::is_nothrow_move_constructible_v<Types>) && ...))
			: value_in_tuple<Types, Indexes>(std::move(other.get<Indexes>()))...
		{}
		constexpr tuple_base& operator=(const tuple_base& other)
			noexcept(((std::is_nothrow_copy_constructible_v<Types>) && ...)) {

			((this->template get<Indexes>() = other.template get<Indexes>()), ...);
			return *this;
		}
		constexpr tuple_base& operator=(tuple_base&& other)
			noexcept(((std::is_nothrow_move_constructible_v<Types>) && ...)) {

			((this->template get<Indexes>() = std::move(other.template get<Indexes>)), ...);
			return *this;
		}
	};

	// использую тут "технологии" множественного наследования с последовательностью чисел
	template<typename ... Types>
	class tuple : public tuple_base<make_value_list<size_t, sizeof...(Types)>, Types...> {
	protected:
		// тут просто передача полномочий на всё tuple_base, потому что там раскрыт пак индексов позволяющий красиво всё писать
		using base_t = tuple_base<make_value_list<size_t, sizeof...(Types)>, Types...>;
	public:
		constexpr tuple()
			noexcept(std::is_nothrow_default_constructible_v<base_t>)
			: base_t() {
			static_assert(((std::is_default_constructible_v<Types>) && ...));
		}
		template<typename ... Args> requires (((std::constructible_from<Types, Args>) && ...))
		constexpr tuple(Args&& ... args)
			noexcept(std::is_nothrow_constructible_v<base_t, Args...>)
			: base_t(std::forward<Args>(args)...)
		{}

		constexpr tuple(const tuple& other)
			noexcept(std::is_nothrow_copy_constructible_v<base_t>) : base_t(other)
		{}
		constexpr tuple(tuple&& other)
			noexcept(std::is_nothrow_move_constructible_v<base_t>) : base_t(std::move(other))
		{}

		constexpr ~tuple() = default;
	};
	
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
