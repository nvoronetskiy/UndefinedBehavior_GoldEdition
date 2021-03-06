

#ifndef KELBON_TYPE_TRAITS_FUNCTIONAL_HPP
#define KELBON_TYPE_TRAITS_FUNCTIONAL_HPP

#include "kelbon_type_traits_variadic.hpp"

namespace kelbon {

	// CONCEPT like_functor - ??????????????? ???????, ??????????? ??????? ????????? ()
	// do not works for template operator() in class
	template<typename T>
	concept like_functor = requires { &T::operator(); };

	enum class ref_qual : int { none, lvalue, rvalue };

	template<typename ...> struct signature;

	// METHODS (no ref-qual)

	// for methods, need explicit this
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...)> {
		using func_type = ResultType(OwnerType*, ArgumentTypes...);
		using owner_type = OwnerType;
		using result_type = ResultType;
		using parameter_list = type_list<OwnerType*, ArgumentTypes...>;
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
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...)&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...)> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) & noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const& noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile& noexcept>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile noexcept> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile&>
		: signature<ResultType(OwnerType::*)(ArgumentTypes...) const volatile> {
		static constexpr ref_qual ref_qualification = ref_qual::lvalue;
	};
	template<typename OwnerType, typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(OwnerType::*)(ArgumentTypes...) volatile& noexcept>
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
		using func_type = ResultType(ArgumentTypes...);
		using result_type = ResultType;
		using parameter_list = type_list<ArgumentTypes...>;
		static constexpr bool is_noexcept = false;
	};
	template<typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(*)(ArgumentTypes...) noexcept> {
		using func_type = ResultType(ArgumentTypes...);
		using result_type = ResultType;
		using parameter_list = type_list<ArgumentTypes...>;
		static constexpr bool is_noexcept = true;
	};

	namespace detail {
		template<typename T>
		struct remove_first_arg;

		template<template<typename...> typename T, typename First, typename ... Types>
		struct remove_first_arg<T<First, Types...>> {
			using type = T<Types...>;
		};

		template<typename T>
		using remove_first_arg_t = typename remove_first_arg<T>::type;

	} // namespace detail

	// helper for functor
	template<typename...>
	struct func_type_helper;
	template<typename ResultType, typename ... ArgTypes>
	struct func_type_helper<ResultType, type_list<ArgTypes...>> {
		using type = ResultType(ArgTypes...);
	};

	template<like_functor T>
	struct signature<T> {
	private:
		using base_t = signature<decltype(&T::operator())>;
	public:
		using owner_type = T;
		using result_type = typename base_t::result_type;
		using parameter_list = typename detail::remove_first_arg_t<typename base_t::parameter_list>;
		static constexpr bool is_const = base_t::is_const;
		static constexpr bool is_volatile = base_t::is_volatile;
		static constexpr bool is_noexcept = base_t::is_noexcept;
		static constexpr ref_qual ref_qualification = base_t::ref_qualification;
		using func_type = typename func_type_helper<result_type, parameter_list>::type;
	};

	// takes reference to callable/method/lambda/FUNCTOR and gives its info
	template<typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(&)(ArgumentTypes...)> : signature<ResultType(*)(ArgumentTypes...)> {};
	// for just type of function/ref
	template<typename ResultType, typename ... ArgumentTypes>
	struct signature<ResultType(ArgumentTypes...)> : signature<ResultType(*)(ArgumentTypes...)> {};

	// ?? ???? ??? ?????? ?????? ... , ?? ????? ??? ?????

	namespace func {
		template<typename F>
		using result_type = typename signature<F>::result_type;

		template<typename F>
		using parameter_list = typename signature<F>::parameter_list;

		template<typename F>
		constexpr inline size_t count_of_arguments = parameter_list<F>::count_of_arguments;

	} // namespace func

} // namespace kelbon

#endif // !KELBON_TYPE_TRAITS_FUNCTIONAL_HPP

