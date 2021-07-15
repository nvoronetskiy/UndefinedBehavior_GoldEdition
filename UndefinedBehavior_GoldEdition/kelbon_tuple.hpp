
#ifndef KELBON_TUPLE_HPP
#define KELBON_TUPLE_HPP

#include <tuple> // structure binding specializtions
#include <type_traits>

#include "kelbon_type_traits_numeric.hpp"

// todo - ������������� memory_block ��� ������������������ �����, ������� � ���� ����� �������� ������ memory_block �� �������, ����������� ��� �����
// � ��� ������� deduction guide ��� ������������ �� ������ �����, ����� ������ ������ ��������, ������ tuple{....}

namespace kelbon {

	template<typename T, size_t index>
	struct value_in_tuple {
		constexpr value_in_tuple()
			noexcept(std::is_nothrow_default_constructible_v<T>)
			: value()
		{}
		// ����� ��������, ����� �� ����������� move �����������(���� �������� �� � �� ������...)
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
		// ���� �����������, ��� ����� ����� �������� operator spaceship <=>
		T value;
	};

	template<typename...>
	struct tuple_base;

	// ��������� ������������� ��������� ����� ����� �������������� ��������� ������������
	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<value_list<size_t, Indexes...>, Types...> : value_in_tuple<Types, Indexes>... {
	protected:
		using type_array = type_list<Types...>; // ����������� ����� ��� ������ �������� �� �������/����

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr tuple_base()
			noexcept(((std::is_nothrow_default_constructible_v<Types>) && ...))
			: value_in_tuple<Types, Indexes>()...
		{}

		// �������� ��, ��� ��������� ������ ��� ����� �� �����(requires), �� ���� �� ���������,
		// �� ���� ����������� "���������" ����� move �����������
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

	// ��������� ��� "����������" �������������� ������������ � ������������������� �����
	template<typename ... Types>
	class tuple : public tuple_base<make_value_list<size_t, sizeof...(Types)>, Types...> {
	protected:
		// ��� ������ �������� ���������� �� �� tuple_base, ������ ��� ��� ������� ��� �������� ����������� ������� �� ������
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
	// ����� ���� ���� �� ���������� ����������� move ������������ ��� enable_if
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
