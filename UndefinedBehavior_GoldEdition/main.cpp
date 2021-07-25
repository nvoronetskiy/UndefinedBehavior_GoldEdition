
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
		//std::cout << "copy\n";
	//}
	Fctor(Fctor&&) {
		std::cout << "move\n";
	}
	int method1(int v) const volatile noexcept {
		return v * 2;
	}
	static int method(int v) noexcept {
		return v * 2;
	}
};
int main() {
	Fctor fct;
	volatile float fv = 3.4f;

	::kelbon::action<int(int)> FF1([&fv](int x) mutable -> int { fv += 1; return x * 2 + fv; });
	// todo - заменить копи конструктор на clone функцию, которая может быть недоступна, если некопируемое лежит(избежание throw в конструкторе неочевидного)
	// todo дополнить тесты мемори блока тестами Clone
	// TODO - убрать static_asserts заменить их на requires closure
	// TODO - перевести всё на модули
	// TODO CallByMemory разобраться получше

	// todo - попробовать запустить code cleanup (на другой ветке гита)
	auto m1 = FF1.Clone();
	FF1(10);
	FF1(10);
	int check_value = m1(10);
	int check_value1 = FF1(10);
	FF1 = &testF;
	FF1 = testF;
	FF1 = [](int v) -> int {return 2 * v; };
	FF1 = &Fctor::method;
	FF1 = Fctor::method;
	FF1 = [&fv](int v) -> int {
		std::cout << "Mda";
		return 150 + fv + v; };

	//FF1 = Fctor{}; // todo - и тут
	//FF1 = fct;
	//FF1 = std::move(fct); // todo - разобраться где он тут зовёт копи конструктор
	FF1 = std::move(FF1);
	auto FF2 = std::move(FF1);
	FF1 = std::move(FF2);
	//TODO TODO TESTS TESTS!!!!!
	
	auto l = FF1(150);
	TestsForActWrapper();
	TestsForConcepts();
	TestsForMemoryBlock();
	TestsForTraits();
	TestsForAction();

	return 0;
}