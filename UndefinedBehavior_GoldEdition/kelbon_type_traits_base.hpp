

#ifndef KELBON_TYPE_TRAITS_BASE_HPP
#define KELBON_TYPE_TRAITS_BASE_HPP

// TEMPLATE REMOVE_POINTER
template<typename T>
struct remove_pointer { using type = T; };
template<typename T>
struct remove_pointer<T*> { using type = T; };

template<typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

// TEMPLATE CONDITIONAL
template<bool Condition, typename T, typename U> 
struct conditional { using type = T; };
template<typename T,typename U> 
struct conditional<false, T, U> { using type = U; };

template<bool Condition, typename T, typename U>
using conditional_t = typename conditional<Condition, T, U>::type;

// TEMPLATE CONSTANTS
template<typename T, T Value>
struct constant { static constexpr T value = Value; };
template<bool value>
using bool_constant = constant<bool, value>;
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// TEMPLATE FUNCTION always_false FOR STATIC ASSERTS (for example)
consteval bool always_false(auto) noexcept { return false; }

// TEMPLATE IS_SAME
template<typename T, typename U>
struct is_same { static constexpr bool value = false; };
template<typename T>
struct is_same<T, T> { static constexpr bool value = true; };

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
	using argument_type = typename type_of_element<index, Types...>::type;
};

// TEMPLATE REVERSE_TYPE_LIST
namespace detail {// TODO
	// struct type_list_without_last
	//template<typename First, typename ... Types>
	//struct type_list_without_last {
	//	using type = typename type_list_without_last < First,
	//};
	//template<typename ... Types>
	//struct reverse_type_list_helper {
		//std::make_index_sequence<19>::value_type::
		//using type = typename reverse_type_list_helper<type_of_element<sizeof...(Types) - 1, Types...>, typename type_list_without_last<Types...>::type >::type;
	//};
} // namespace detail

template<typename ... Types>
struct reverse_type_list {
	// TODO
};

// TEMPLATE ADD_RVALUE_REFERENCE
template<typename T>
struct add_rvalue_reference;
template<typename T>
struct add_rvalue_reference      { using type = T&&; };
template<typename T>
struct add_rvalue_reference<T&>  { using type = T&&; };

// TEMPLATE FUNCTION DECLVAL
template<typename T>
add_rvalue_reference<T> declval() noexcept;

// TEMPLATE DECAY
template<typename T>
struct decay { using type = T; };
template<typename T>
struct decay<T*> { using type = typename decay<T>::type; };
template<typename T>
struct decay<T&> { using type = typename decay<T>::type; };
template<typename T>
struct decay<T&&> { using type = typename decay<T>::type; };
template<typename T>
struct decay<const T> { using type = typename decay<T>::type; };
template<typename T>
struct decay<volatile T> { using type = typename decay<T>::type; };

template<typename T>
using decay_t = typename decay<T>::type;

// TEMPLATE ENABLE_IF_VALUE
template<bool Condition>
struct enable_if_value {};
template<>
struct enable_if_value<true> { static constexpr bool value = true; };

template<bool Condition>
constexpr inline bool enable_if_v = enable_if_value<Condition>::value;

#endif // !KELBON_TYPE_TRAITS_BASE_HPP

