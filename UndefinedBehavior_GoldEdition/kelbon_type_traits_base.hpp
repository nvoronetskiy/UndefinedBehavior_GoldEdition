

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
	struct create_rvalue_reference;
	template<typename T>
	struct create_rvalue_reference { using type = T&&; };
	template<typename T>
	struct create_rvalue_reference<T&> { using type = T&&; };

	template<typename T>
	using create_rvalue_reference_t = typename create_rvalue_reference<T>::type;

	// TEMPLATE ALIAS ref_to_func_which_returns. Used for concepts as an analogue of declval template 
	template<typename T>
	using ref_to_func_which_returns = create_rvalue_reference_t<T>(*)();

	// TEMPLATE CONDITIONAL
	template<bool Condition, typename T, typename U>
	struct conditional { using type = T; };
	template<typename T, typename U>
	struct conditional<false, T, U> { using type = U; };

	template<bool Condition, typename T, typename U>
	using conditional_t = typename conditional<Condition, T, U>::type;

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
	template<typename T> // const and volatile have same priority, so for decay<const volatile T> ambigious const or volatile specialization it needs
	struct decay<const volatile T> { using type = typename decay<T>::type; };

	template<typename T>
	using decay_t = typename decay<T>::type;

}//namespace kelbon

#endif // !KELBON_TYPE_TRAITS_BASE_HPP

