
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
		constexpr value_in_tuple() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

		constexpr value_in_tuple(const T& value)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires(std::is_copy_constructible_v<T>)
			: value(value)
		{}

		constexpr value_in_tuple(T&& value)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			: value(std::move(value))
		{}
		constexpr value_in_tuple(const value_in_tuple&)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires(std::is_copy_constructible_v<T>) = default;

		constexpr value_in_tuple(value_in_tuple&&)
			noexcept(std::is_nothrow_move_constructible_v<T>) = default;

		constexpr value_in_tuple& operator=(const value_in_tuple&)
			noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires(std::is_copy_constructible_v<T>) = default;

		constexpr value_in_tuple& operator=(value_in_tuple&&)
			noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		constexpr ~value_in_tuple() = default;
		// ���� �����������, ��� ����� ����� �������� operator spaceship <=>
		T value;
	};

	template<bool, typename...>
	struct tuple_base;

	// COPY CONSTRUCTOR AND BUG IN MSVC (requires and other shit cant delete copy constructor) FORCE ME INTO CREATE TWO SPECIALIZTIONS OF TUPLE HERE... GOD FORGIVE ME

	// ��������� ������������� ��������� ����� ����� �������������� ��������� ������������
	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<false, value_list<size_t, Indexes...>, Types...> : value_in_tuple<Types, Indexes>... {
	protected:
		using type_array = type_list<Types...>; // ����������� ����� ��� ������ �������� �� �������/����

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr tuple_base()
			noexcept(all_in_pack<std::is_nothrow_default_constructible, Types...>())
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
			//requires(((std::is_copy_constructible_v<Types>) && ...)) // ����� ���� �������� ����������� ����������� �� ����� ����������
			//: value_in_tuple<Types, Indexes>(other.template get<Indexes>())...
		//{}

		constexpr tuple_base(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_constructible, Types...>())
			: value_in_tuple<Types, Indexes>(std::move(other.template get<Indexes>()))...
		{}
		// ��������� ���������� ������ �������������, � ��� ����� ���/����, �.�. ������ ��-�� requires �� ���������� ���������� � ��� ������������
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
			noexcept(all_in_pack<std::is_nothrow_copy_assignable, Types...>())
			requires(all_in_pack<std::is_copy_assignable, Types...>()) {
			((this->template get<Indexes>() = other.template get<Indexes>()), ...);
			return *this;
		}
		constexpr tuple_base& operator=(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_assignable, Types...>()) {

			((this->template get<Indexes>() = std::move(other.template get<Indexes>())), ...);
			return *this;
		}
	};

	template<size_t ... Indexes, typename ... Types>
	struct tuple_base<true, value_list<size_t, Indexes...>, Types...> : value_in_tuple<Types, Indexes>... {
	protected:
		using type_array = type_list<Types...>; // ����������� ����� ��� ������ �������� �� �������/����

		template<size_t index>
		using get_type = typename type_array::template get_element<index>;
	public:
		constexpr tuple_base()
			noexcept(all_in_pack<std::is_nothrow_default_constructible, Types...>())
			: value_in_tuple<Types, Indexes>()...
		{}

		constexpr tuple_base(const tuple_base& other)
			noexcept(all_in_pack<std::is_nothrow_copy_constructible, Types...>())
			: value_in_tuple<Types, Indexes>(other.template get<Indexes>())...
		{}

		constexpr tuple_base(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_constructible, Types...>())
			: value_in_tuple<Types, Indexes>(std::move(other.template get<Indexes>()))...
		{}
		// ��������� ���������� ������ �������������, � ��� ����� ���/����, �.�. ������ ��-�� requires �� ���������� ���������� � ��� ������������
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
			noexcept(all_in_pack<std::is_nothrow_copy_assignable, Types...>())
			requires(all_in_pack<std::is_copy_assignable, Types...>()) {
			((this->template get<Indexes>() = other.template get<Indexes>()), ...);
			return *this;
		}
		constexpr tuple_base& operator=(tuple_base&& other)
			noexcept(all_in_pack<std::is_nothrow_move_assignable, Types...>()) {

			((this->template get<Indexes>() = std::move(other.template get<Indexes>())), ...);
			return *this;
		}
	};

	// ��������� ��� "����������" �������������� ������������ � ������������������� �����
	template<typename ... Types>
	class tuple : public tuple_base<all_in_pack<std::is_copy_constructible, Types...>(), make_value_list<size_t, sizeof...(Types)>, Types...> {
	protected:
		// ��� ������ �������� ���������� �� �� tuple_base, ������ ��� ��� ������� ��� �������� ����������� ������� �� ������
		using base_t = tuple_base<all_in_pack<std::is_copy_constructible, Types...>(), make_value_list<size_t, sizeof...(Types)>, Types...>;
	public:
		using base_t::base_t;
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
