

#ifndef KELBON_FUNCTIONAL_TRAITS_HPP
#define KELBON_FUNCTIONAL_TRAITS_HPP

#include "kelbon_type_traits.hpp"
#include "kelbon_concepts.hpp"

enum class ref_qual : int { none, lvalue, rvalue };

template<typename ...> struct signature;

// METHODS (no ref-qual)

template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...)> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::none;
};
// METHODS (LVALUE ref-qual)

template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) &> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const &> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) & noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const & noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile & noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile &> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile & noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::lvalue;
};
// METHODS (RVALUE ref-qual)

template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) &&> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const &&> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) && noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const && noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = false;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile && noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile &&> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = true;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = false;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};
template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile && noexcept> {
	using owner_type = OwnerType;
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_const = false;
	static constexpr bool is_volatile = true;
	static constexpr bool is_noexcept = true;
	static constexpr ref_qual ref_qualification = ref_qual::rvalue;
};

// JUST FUNCTIONS

template<typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(*)(ArgumentTypes...)> {
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_noexcept = false;
};
template<typename ResultType, typename ... ArgumentTypes>
struct signature<ResultType(*)(ArgumentTypes...) noexcept > {
	using result_type = ResultType;
	using parameter_list = type_list<ArgumentTypes...>;
	static constexpr bool is_noexcept = true;
};
// по сути ещё элипсы сишные ... , но пошли они нахуй

consteval auto get_function_signature(auto x) noexcept {
	return signature<decltype(x)>{};
}
consteval auto get_function_signature(functor auto x) noexcept {
	return signature<decltype(&decltype(x)::operator())>{};
}
// Need real value, so declval is not working here(not a problem for functors, use {} for initialization
// or signature template or get_function_signature function...)
template<auto Function>
using function_info = decltype(get_function_signature(Function));

#endif // !KELBON_FUNCTIONAL_TRAITS_HPP

