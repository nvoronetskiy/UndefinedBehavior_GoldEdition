
#include <iostream>
#include <string>

#include "kelbon_type_traits_functional.hpp"
#include "kelbon_memory_block.hpp"
#include "kelbon_type_traits_numeric.hpp"

#include "kelbon_tuple.hpp"

#include "kelbon_template_base_class.hpp"

// аналог функции возвращающей значение на компайл тайме - шаблонна€ переменна€(не нужны никакие дополнительные скобки, просто get< ... >, то есть замена () на <>
// аналог функции, провер€ющей что-то - концепты
// функци€, возвращающа€ тип - using

//template<...>
//... WrapAction()

// todo - слушатель, который просто передаЄт инфу слушающим дальше, подключенным к нему(позвол€ет делать только 1 указатель на слушател€ в объектах/потоке физики, передавать сообщение
// и он уже выбирает кому нужно сообщение, например слушатели физики / конкретных каких то вещей.

// todo трейты is_method is_functor/lambda и т.д.(или концепты в некоторых местах, а не трейты), также можно is const/volatile/noexcept(лол)
// ещЄ есть & && квалификаторы после функции, помогающие перегружать по контексту использовани€ (this)
// todo может ещЄ и дл€ шаблонов заху€рить?
// можно ещЄ написать свой tuple во первых чтобы не зависеть от реализаций его внутри, во вторых чтобы он укладывал данные в нужном пор€дке

struct test {
	test() {
		std::cout << "default" << std::endl;
	}
	test(int v) : v(v) {
		std::cout << "standard" << std::endl;
	}
	test(const test& other) {
		std::cout << "copy" << std::endl;
	}
	test(test&& other) noexcept {
		std::cout << "move" << std::endl;
	}
	int func(float value) const {
		return value * v;
	}
	test& operator=(const test&) {
		std::cout << "operator=COPY" << std::endl;
		return *this;
	}
	test& operator=(test&&) noexcept {
		std::cout << "operator=MOVE" << std::endl;
		return *this;
	}
	~test() {
		std::cout << "destroyed" << std::endl;
	}
	int v;
};
constexpr int func1(int value) noexcept {
	return value * 2;
}
void func() {
	//return func1();
}
// TODO - написать тесты в виде static asserts в конце каждого файла про type_traits и т.д., оборачива€ их в on_debug
using namespace kelbon;
int main() {

	func();
	int x1 = 5;
	constexpr float x2 = 10;
	auto wrap1 = wrap_action([](int value) {return value; });
	auto xtest1 = wrap1(5);
	auto wrap2 = wrap_action(func1);
	auto xtest2 = wrap2(x2);

	test test_value(5);
	//(test_value.*xf)(5);
	auto wrap3 = wrap_action(&test::func);
	auto xtest3 = wrap3(&test_value, 20.f);

	using help_lambda = decltype([]() { return get_function_signature(func); });
	constexpr bool h = function<decltype(&help_lambda::operator())>;
	constexpr bool h1 = method<decltype(&help_lambda::operator())>;
	constexpr bool h2 = function<decltype(&func)>;
	signature<decltype(&func)>::result_type;

	// TODO - придумать что то с версионностью моего namespace, мб inline namespace использовать как-то

	//std::tuple<int, double, float, std::string> stp(5, 3., 3.f, std::string("33"));
	//auto x = tp.get<3>();
	//tp.get<3>() = "no idea what to write";

	// TODO - дл€ tuple написать begin/end /iterator, специализации std::tuple_element std::tuple_size, deduction guides и т.д.

	
	// TODO - вз€тие значени€ по номеру из последовательности значений(сейчас вроде неоч работает) и sequence доделать(внутри полезные штуки объ€вить, вз€тие по номеру, кол-во...)
	//function_info<realmemmove>::parameter_list::get_element<2> ann = 6;
	//std::cout << ann << std::endl;
	//using t = decltype([](int) { return false; });
	//function_info<t{}>::result_type sprigan = true;
	//using capt = decltype([&sprigan](int) {return false; });
	// todo - reverse parameter pack type)))
	//signature<decltype(&capt::operator())>::result_type rt;
	//int ival = 5;
	//float fval = 3.14f;
	//std::string s = "All right";
	//tuple<int, double, std::string, char> t;
	//memory_block<140> value(std::tuple(t, s));

	//const auto& [str, fv] = value.GetDataAs<tuple<int, double>, std::string>();

	//std::cout << str << '\t' << fv << '\t' << std::endl;

	return 0;
}