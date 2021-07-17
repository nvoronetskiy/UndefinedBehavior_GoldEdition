

#ifndef KELBON_TYPE_TRAITS_FUNCTIONAL_HPP
#define KELBON_TYPE_TRAITS_FUNCTIONAL_HPP

#include "kelbon_concepts_functional.hpp"
#include "kelbon_type_traits_variadic.hpp"

namespace kelbon {

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
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const>
		: signature< ResultType(OwnerType::*)(ArgumentTypes...)> {
		static constexpr bool is_const = true;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile>
		: signature< ResultType(OwnerType::*)(ArgumentTypes...)> {
		static constexpr bool is_volatile = true;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...)> {
		static constexpr bool is_noexcept = true;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const> {
		static constexpr bool is_noexcept = true;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const noexcept> {
		static constexpr bool is_volatile = true;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const> {
		static constexpr bool is_volatile = true;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile noexcept>
		: signature< ResultType(OwnerType::*)(ArgumentTypes...) volatile> {
		static constexpr bool is_noexcept = true;
	};

	// METHODS (LVALUE ref-qual)

	
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) &>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...)> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const &>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile &>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) & noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const & noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile & noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile &>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile & noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};

	// METHODS (RVALUE ref-qual)

	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...)&&>
	: signature<ResultType(OwnerType::*)(ArgumentTypes...)> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const&&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile&&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) && noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const&& noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile&& noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile&&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile> {
		static constexpr ref_qual ref_qualification = ref_qual::rvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile&& noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile noexcept> {
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
	struct signature<ResultType(*)(ArgumentTypes...) noexcept> {
		using result_type = ResultType;
		using parameter_list = type_list<ArgumentTypes...>;
		static constexpr bool is_noexcept = true;
	};

	template<like_functor T>
	struct signature<T> : signature<decltype(&T::operator())> {};
	
	// takes reference to callable/method/lambda/FUNCTOR and gives its info
	template<typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(&)(ArgumentTypes...)> : signature<ResultType(*)(ArgumentTypes...)> {};
	// for just type of function/ref
	template<typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(ArgumentTypes...)> : signature<ResultType(*)(ArgumentTypes...)> {};

	// по сути ещё элипсы сишные ... , но пошли они нахуй

	consteval auto get_function_signature(auto x) noexcept {
		return signature<decltype(x)>{};
	}
	consteval auto get_function_signature(like_functor auto x) noexcept {
		return signature<decltype(&decltype(x)::operator())>{};
	}

	// CONCEPT callable
	template<typename Applicant>
	concept callable = requires {
		typename signature<Applicant>::result_type;
		typename signature<Applicant>::parameter_list;
		signature<Applicant>::is_noexcept;
	};

	// CONCEPT functor
	template<typename Applicant>
	concept functor = callable<Applicant> && like_functor<Applicant>;

	// CONCEPT function
	template<typename Applicant>
	concept function = !like_functor<Applicant> && callable<Applicant>;

	// CONCEPT method
	// DO NOT WORK FOR STATIC METHODS(they are same as functions) частный случай функции
	template<typename Applicant> // not like_functor because signature see functors as operator() == method
	concept method = !like_functor<Applicant> && callable<Applicant> && requires {
		typename signature<Applicant>::owner_type;
		signature<Applicant>::is_const;
		signature<Applicant>::is_volatile;
		signature<Applicant>::ref_qualification;
	};

	// Need real value, so declval is not working here(not a problem for functors, use {} for initialization
	// or signature template or get_function_signature callable...)
	// lambdas with capture have no default constructor, so you cant use it for them, but can use signature<T>
	template<auto Function>
	using function_info = decltype(get_function_signature(Function));

} // namespace kelbon

#endif // !KELBON_TYPE_TRAITS_FUNCTIONAL_HPP

