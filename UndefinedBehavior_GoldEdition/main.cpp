
#include <iostream>
#include <string>
#include <memory>

#include "test_kelbon_concepts.hpp"
#include "test_kelbon_memory_block.hpp"
#include "test_kelbon_act_wrapper.hpp"
#include "test_kelbon_traits.hpp"
#include "test_kelbon_action.hpp"

#include "kelbon_type_traits_functional.hpp"
#include "kelbon_memory_block.hpp"
#include "kelbon_type_traits_numeric.hpp"

#include "kelbon_tuple.hpp"

#include "kelbon_template_base_class.hpp"

#include "kelbon_action.hpp"

// todo - слушатель, который просто передаёт инфу слушающим дальше, подключенным к нему(позволяет делать только 1 указатель на слушателя в объектах/потоке физики, передавать сообщение
// и он уже выбирает кому нужно сообщение, например слушатели физики / конкретных каких то вещей.

using namespace kelbon::test;

// todo - cmake for this project
// todo - проверить на gcc

// todo - сделать так, чтобы в act_wrapper можно было записывать любой callable, ведущий себя также. Напирмер лямбда int(float )это то же самое, что и функция int(float)
// для этого нужно хранить memory_block, в котором хранить любой callble, написать оператор=, принимающий любой 1 callble с такими же return и args, 
// а ещё запоминать как Destroy как вызывать функцию(Call), то есть нужно запомнить функцию принимающую memory_block по ссылке(константной)
//  и возвращающую указатель на функцию, которую нужно вызывать... Сигнатура которой известна. Внутри запоминающей функции очевидно будет block.GetDataAs<Actor>()



int testF(int x) {
	return x * 2;
}
int testM(int x) {
	return x * 2;
}
struct Fctor {

	int operator()(int v) const noexcept {
		return v * 2;
	}
	Fctor() = default;
	//Fctor(const Fctor&) {
	//	std::cout << "copy\n";
	//}
	Fctor(Fctor&&) noexcept {
		std::cout << "move\n";
	}
	Fctor& operator=(Fctor&&) = default;
	int method1(int v) const volatile noexcept {
		return v * 2;
	}
	static int method(int v) noexcept {
		return v * 2;
	}
};

struct s {
	int v;
	s() = default;
	s(int v) :v(v) {}
	s& operator=(const s&) = default;
	int method(float fv) const volatile & noexcept {
		std::cout << "10";
		return v + fv;
	}
	void operator()(int,float) const {

	}
};
#include <functional>
int main() {

	::kelbon::tuple t(Fctor{});
	::kelbon::tuple<Fctor> tt;// (std::move(t));
	tt = std::move(t);
	//tt = std::move(t);
	Fctor fct;
	volatile float fv = 3.4f;
	// todo - для методов сделать перегрузку конструкторов/операторов/call
	::kelbon::action<int(s*, float)> act = &s::method;
	s value1(15);
	s value2(16);
	auto rv1 = act(&value1, 3.14f);
	auto rv2 = act(&value2, 10.f);
	rv1 = act(&value1, 3.14f);
	::kelbon::action<int(s*, float)> act1;
	act1 = act.Clone();
	rv1 = act1(&value2, -2);
	//auto bindedf = std::bind(act, &value1, std::placeholders::_1);
	//rv1 = bindedf(15);
	::kelbon::action<int(int)> FF1([&fv](int x) mutable -> int { fv += 1; return x * 2 + fv; });
	// todo дополнить тесты мемори блока тестами Clone
	// TODO - убрать static_asserts заменить их на requires closure
	// TODO - перевести всё на модули
	// TODO CallByMemory разобраться получше

	// Модули - можно ли экспортировать только одну специализацию???
	// todo - попробовать запустить code cleanup (на другой ветке гита)
	//auto m1 = FF1.Clone();
	//FF1(10);
	//FF1(10);
	//int check_value = m1(10);
	//int check_value1 = FF1(10);
	//FF1 = &testF;
	//FF1 = testF;
	//FF1 = [](int v) -> int {return 2 * v; };
	//FF1 = &Fctor::method;
	//FF1 = Fctor::method;
	//FF1 = [&fv](int v) -> int {
		//std::cout << "Mda";
		//return 150 + fv + v; };

	//FF1 = Fctor{}; // todo - и тут
	//FF1 = fct;
	//FF1 = std::move(fct); // todo - разобраться где он тут зовёт копи конструктор
	//FF1 = std::move(FF1);
	//auto FF2 = std::move(FF1);
	//FF1 = std::move(FF2);
	//TODO TODO TESTS TESTS!!!!!
	
	//auto l = FF1(150);
	TestsForActWrapper();
	TestsForConcepts();
	TestsForMemoryBlock();
	TestsForTraits();
	TestsForAction();

	return 0;
}