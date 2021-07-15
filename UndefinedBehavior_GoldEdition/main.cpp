
#include <iostream>
#include <string>
#include <memory>

#include "test_kelbon_concepts_base.hpp"
#include "test_kelbon_concepts_functional.hpp"
#include "test_kelbon_memory_block.hpp"

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

struct test1 {
	test1() {
		std::cout << "default" << std::endl;
	}
	test1(int v) : v(v) {
		std::cout << "standard" << std::endl;
	}
	test1(const test1& other) {
		std::cout << "copy" << std::endl;
	}
	test1(test1&& other) noexcept {
		std::cout << "move" << std::endl;
	}
	int func(float value) const {
		return value * v;
	}
	test1& operator=(const test1&) {
		std::cout << "operator=COPY" << std::endl;
		return *this;
	}
	test1& operator=(test1&&) noexcept {
		std::cout << "operator=MOVE" << std::endl;
		return *this;
	}
	~test1() {
		std::cout << "destroyed" << std::endl;
	}
	int v;
};

constexpr int func1(int&& value, float* v, const std::string& s) noexcept {
	return value * 2;
}
void func() {
	//return func1();
}
// TODO - написать тесты в виде static asserts в конце каждого файла про type_traits и т.д., оборачива€ их в on_debug
using namespace kelbon;

int main() {
	constexpr bool j = function<decltype(func)>;
	test::TestsForConceptsBase();
	test::TestsForConceptsFunctional();
	test::TestsForMemoryBlock();

	//tuple mytuple(1, 2., 2.f, std::string("rewrw"));
	// todo - кака€ то хрень с сишными строками типо "wtf man"
	//tuple mmm{ 5,2.,2.f,std::string("432")};
	//int x24 = 420;
	//signature<decltype([&x24]() { x24 += 10; })> m24;
	//auto& [a, b, c, d] = mmm; // todo - пон€ть почему & нужно тут
	//func();
	//int x1 = 5;
	//constexpr float x2 = 10;
	// using my_base_class и т.д. дл€ этого
	//using my_act = base_action<int, int&&, float*, const std::string&>;
	//using my_wrapped_act = decltype(wrap_action(&func1));
	//std::unique_ptr<my_act> myaction(new my_wrapped_act(act_wrapper(&func1)));
	//auto myaction1 = std::make_unique<my_wrapped_act>(&func1);
	//auto m = myaction1->operator()(5, nullptr, std::string("mda"));
	//std::unique_ptr<base_action> wrap1(std::make_unique([](int value) {return value; }));

	//auto xtest1 = wrap1(5);
	//auto wrap2 = wrap_action(&func1);
	//std::string s;

	//act_wrapper wrapme(&func);

	//auto xtest2 = wrap2(5, nullptr, s);
	
	//signature<decltype(decltype(wrapme)::act_wrapper<decltype(wrapme)>)>
	//test1 test_value(5);
	//(test_value.*xf)(5);
	//auto wrap3 = wrap_action(&test1::func);
	//auto xtest3 = wrap3(&test_value, 20.f);

	//using help_lambda = decltype([]() { return get_function_signature(func); });
	//constexpr bool h = function<decltype(&help_lambda::operator())>;
	//constexpr bool h1 = method<decltype(&help_lambda::operator())>;
	//constexpr bool h2 = function<decltype(&func)>;
	//signature<decltype(&main)>::result_type;

	//std::tuple<int, double, float, std::string> stp(5, 3., 3.f, std::string("33"));
	//auto x = tp.get<3>();
	//tp.get<3>() = "no idea what to write";

	
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