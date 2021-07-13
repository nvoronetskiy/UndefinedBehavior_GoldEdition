

#ifndef KELBON_TEMPLATE_BASE_CLASS_HPP
#define KELBON_TEMPLATE_BASE_CLASS_HPP

#include <type_traits>

#include "kelbon_type_traits_functional.hpp"

namespace kelbon {

	// шаблон базового класса с виртуальным оператором () в protected зоне, принимающим и возвращающим какие то аргументы переданные в шаблоне, да-да.
	template<typename ResultType, typename ... ArgTypes>
	class base_action {
	public:
		using result_type = ResultType;
		using parameter_list = type_list<ArgTypes...>;

		// ArgTypes is not template parameter here, so && is not universal reference here,
		// and it dont need here, because ArgTypes exactly what function need
		virtual result_type operator()(ArgTypes ...) const = 0;
		virtual ~base_action() = default;
	};
	
	template<typename T>
	struct has_default_constructor {
		constexpr has_default_constructor() noexcept(std::is_nothrow_default_constructible_v<T>) {}
	};
	struct no_default_constructor {};

	template<template<typename...> typename Base, typename...>
	class act_wrapper;

	template<template<typename...> typename Base, function Actor, typename ResultType, typename ... Types>
	class act_wrapper<Base, Actor, ResultType, type_list<Types...>>
		: public Base<ResultType, Types...> {
	private:
		Actor F;
	public:
		// for universal reference here
		template<typename WorkAround>
		constexpr act_wrapper(WorkAround&& actor)
			noexcept(std::is_nothrow_copy_constructible_v<WorkAround>)
			: F(std::forward<WorkAround>(actor))
		{}
		virtual constexpr ResultType operator()(Types ... args) const override {
			return F(std::forward<Types>(args)...);
		}
		virtual constexpr ~act_wrapper() override = default;
	};

	// for methods with explicit THIS pointer in method
	template<template<typename...> typename Base, method Actor, typename ResultType, typename ThisType, typename ... Types>
	class act_wrapper<Base, Actor, ResultType, type_list<ThisType, Types...>>
		: public Base<ResultType, ThisType, Types...> {
	private:
		Actor F;
	public:
		// for universal reference here
		template<typename WorkAround>
		constexpr act_wrapper(WorkAround&& actor)
			noexcept(std::is_nothrow_copy_constructible_v<WorkAround>)
			: F(std::forward<WorkAround>(actor))
		{}
		virtual constexpr ResultType operator()(ThisType owner_this, Types ... args) const override {
			return ((*owner_this).*F)(std::forward<Types>(args)...);
		}
		virtual constexpr ~act_wrapper() override = default;
	};

	template<template<typename...> typename Base, function Function>
	class act_wrapper<Base, Function>
		: public act_wrapper<Base, Function, typename signature<Function>::result_type, typename signature<Function>::parameter_list>,
		std::conditional_t<std::is_default_constructible_v<Function>, has_default_constructor<Function>, no_default_constructor> {
	public:
		// for universal reference here
		template<typename WorkAround>
		constexpr act_wrapper(WorkAround&& actor)
			noexcept(std::is_nothrow_copy_constructible_v<WorkAround>)
			: act_wrapper<Base, Function, typename signature<Function>::result_type, typename signature<Function>::parameter_list>(std::forward<WorkAround>(actor))
		{}
	};

	
	template<template<typename...> typename Base, method Function>
	class act_wrapper<Base, Function>
		: public act_wrapper<Base, Function, typename signature<Function>::result_type,
		merge_type_lists_t<type_list<std::add_pointer_t<typename signature<Function>::owner_type>>, typename signature<Function>::parameter_list>>,
		// inherit with adding type of THIS in the begining of method signature(because its non-static method)
		std::conditional_t<std::is_default_constructible_v<Function>, has_default_constructor<Function>, no_default_constructor> {
	public:
		// for universal reference here
		template<typename WorkAround>
		constexpr act_wrapper(WorkAround&& actor)
			noexcept(std::is_nothrow_copy_constructible_v<WorkAround>)
			: act_wrapper<Base, Function, typename signature<Function>::result_type,
			merge_type_lists_t<type_list<std::add_pointer_t<typename signature<Function>::owner_type>>, typename signature<Function>::parameter_list>>
			(std::forward<WorkAround>(actor))
		{}
	};
	
	template<template<typename...> typename Base = base_action>
	[[nodiscard]] constexpr auto wrap_action(function auto value) noexcept(std::is_nothrow_copy_constructible<decltype(value)>) {
		return act_wrapper<Base, decltype(value)>(std::forward<decltype(value)>(value));
	}
	//template<template<typename...> typename Base = base_action>
	//[[nodiscard]] auto wrap_action(method auto value) noexcept(std::is_nothrow_copy_constructible<decltype(value)>) {
	//	return act_wrapper<Base, decltype(value)>(std::forward<decltype(value)>(value));
	//}

} // namespace kelbon

#endif // !KELBON_TEMPLATE_BASE_CLASS_HPP

