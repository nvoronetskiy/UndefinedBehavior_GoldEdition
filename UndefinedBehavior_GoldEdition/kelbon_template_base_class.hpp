

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

	template<template<typename...> typename Base, typename...>
	class act_wrapper;

	// for non-methods
	template<template<typename...> typename Base, function Actor, typename ResultType, typename ... Types>
	class act_wrapper<Base, Actor, ResultType, type_list<Types...>>
		: public Base<ResultType, Types...> {
	private:
		Actor F;
	public:
		constexpr act_wrapper(const Actor& actor)
			noexcept(std::is_nothrow_copy_constructible_v<Actor>)
			: F(actor)
		{}
		constexpr act_wrapper(Actor&& actor)
			noexcept(std::is_nothrow_move_constructible_v<Actor>)
			: F(std::move(actor))
		{}

		constexpr act_wrapper(act_wrapper&& other)
			noexcept(std::is_nothrow_move_constructible_v<Actor>)
			: F(std::move(other.F))
		{}
		constexpr act_wrapper(const act_wrapper& other)
			noexcept(std::is_nothrow_copy_constructible_v<Actor>)
			: F(other.F)
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
		constexpr act_wrapper(const Actor& actor)
			noexcept(std::is_nothrow_copy_constructible_v<Actor>)
			: F(actor)
		{}
		constexpr act_wrapper(Actor&& actor)
			noexcept(std::is_nothrow_move_constructible_v<Actor>)
			: F(std::move(actor))
		{}

		constexpr act_wrapper(act_wrapper&& other)
			noexcept(std::is_nothrow_move_constructible_v<Actor>)
			: F(std::move(other.F))
		{}
		constexpr act_wrapper(const act_wrapper& other)
			noexcept(std::is_nothrow_copy_constructible_v<Actor>)
			: F(other.F)
		{}

		// nothing about virtual / override, because user can use non-virtual base class
		virtual constexpr ResultType operator()(ThisType owner_this, Types ... args) const override {
			return ((*owner_this).*F)(std::forward<Types>(args)...);
		}
		virtual constexpr ~act_wrapper() override = default;
	};

	// прослойка FOR FUNCTIONS and FUNCTORS обрабатывающая входящие типы и передающая дальше
	template<template<typename...> typename Base, function Function>
	class act_wrapper<Base, Function>
		: public act_wrapper<Base, Function, typename signature<Function>::result_type, typename signature<Function>::parameter_list> {
	private:
		using base_t = act_wrapper<Base, Function, typename signature<Function>::result_type, typename signature<Function>::parameter_list>;
	public:
		constexpr act_wrapper(const act_wrapper& other)
			noexcept(std::is_nothrow_copy_constructible_v<base_t>)
			: base_t(other)
		{}
		constexpr act_wrapper(act_wrapper&& other)
			noexcept(std::is_nothrow_move_constructible_v<base_t>)
			: base_t(std::move(other))
		{}
		constexpr act_wrapper(const Function& actor)
			noexcept(std::is_nothrow_copy_constructible_v<Function>)
			: base_t(actor)
		{}
		constexpr act_wrapper(Function&& actor)
			noexcept(std::is_nothrow_move_constructible_v<Function>)
			: base_t(std::move(actor))
		{}
	};

	// прослойка FOR METHODS обрабатывающая входящие типы и передающая дальше
	template<template<typename...> typename Base, method Function>
	class act_wrapper<Base, Function>
		: public act_wrapper<Base, Function, typename signature<Function>::result_type,
		merge_type_lists_t<type_list<std::add_pointer_t<typename signature<Function>::owner_type>>, typename signature<Function>::parameter_list>> {
	private:
		using base_t = act_wrapper<Base, Function,
			typename signature<Function>::result_type,
			merge_type_lists_t<
			type_list<std::add_pointer_t<typename signature<Function>::owner_type>>, typename signature<Function>::parameter_list>
		>;
	public:
		constexpr act_wrapper(const act_wrapper& other)
			noexcept(std::is_nothrow_copy_constructible_v<base_t>)
			: base_t(other)
		{}
		constexpr act_wrapper(act_wrapper&& other)
			noexcept(std::is_nothrow_move_constructible_v<base_t>)
			: base_t(std::move(other))
		{}
		constexpr act_wrapper(const Function& actor)
			noexcept(std::is_nothrow_copy_constructible_v<Function>)
			: base_t(actor)
		{}
		constexpr act_wrapper(Function&& actor)
			noexcept(std::is_nothrow_move_constructible_v<Function>)
			: base_t(std::move(actor))
		{}
	};
	
	// deduction guide
	template<function Function>
	act_wrapper(Function&&)->act_wrapper<base_action, Function>;
	// TODO - deduction guide for act_wrapper 

	// returns functor with wrapped function (operator() ) inherit from base class, by default its polymorhic with virutal destructor and operator()
	template<typename T, template<typename...> typename Base = base_action> requires function<T>
	[[nodiscard]] constexpr auto wrap_action(T&& value) noexcept(std::is_nothrow_copy_constructible<T>) {
		return act_wrapper<Base, T>(std::forward<T>(value));
	}

} // namespace kelbon

#endif // !KELBON_TEMPLATE_BASE_CLASS_HPP

