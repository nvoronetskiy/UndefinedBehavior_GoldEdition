

#ifndef KELBON_TYPE_TRAITS_BASE_HPP
#define KELBON_TYPE_TRAITS_BASE_HPP

namespace kelbon {

	// TEMPLATE CONSTANTS
	template<typename T, T Value>
	struct constant { static constexpr T value = Value; };
	template<bool value>
	using bool_constant = constant<bool, value>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	// TEMPLATE ADD_RVALUE_REFERENCE
	template<typename T>
	struct add_rvalue_reference;
	template<typename T>
	struct add_rvalue_reference { using type = T&&; };
	template<typename T>
	struct add_rvalue_reference<T&> { using type = T&&; };

	// TEMPLATE FUNCTION DECLVAL
	template<typename T>
	add_rvalue_reference<T> declval() noexcept;

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
	template<typename T, typename U>
	struct conditional<false, T, U> { using type = U; };

	template<bool Condition, typename T, typename U>
	using conditional_t = typename conditional<Condition, T, U>::type;

	// TEMPLATE IS_SAME
	template<typename T, typename U>
	struct is_same { static constexpr bool value = false; };
	template<typename T>
	struct is_same<T, T> { static constexpr bool value = true; };

	template<typename T, typename U>
	constexpr inline bool is_same_v = is_same<T, U>::value;

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

}//namespace kelbon

#endif // !KELBON_TYPE_TRAITS_BASE_HPP

