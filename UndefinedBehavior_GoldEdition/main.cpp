

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
#include "kelbon_type_traits_advanced.hpp"

#include "kelbon_tuple.hpp"

#include "kelbon_template_base_class.hpp"

#include "kelbon_action.hpp"

using namespace kelbon::test;

// todo - cmake for this project
// todo - проверить на gcc

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
	explicit Fctor() = default;

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
	explicit s(int v) :v(v) {}
	s& operator=(const s&) = default;
	int method(float fv) const volatile & noexcept {
		std::cout << "10";
		return static_cast<int>(static_cast<float>(v) + fv);
	}
	void operator()(int,float) const {

	}
};

int main() {
	constexpr ::kelbon::tuple tpl(3, 4.f, 'c');
	constexpr auto cxtv = tpl.get<2>();
	auto mmmm = tpl.get<2>();
	std::cout << mmmm;
	constexpr ::kelbon::value_in_tuple<int, 0> spaceship1(10);
	constexpr ::kelbon::value_in_tuple<int, 10> spaceship2(20);
	constexpr bool spbv = spaceship1 > spaceship2;

	constexpr bool sss = ::kelbon::is_explicit_constructible_v<s>;
	constexpr bool waw = ::kelbon::is_explicit_constructible_v<Fctor>;
	::kelbon::action myf = [](int v) {return v; };

	Fctor fct;
	volatile float fv = 3.4f;
	kelbon::tuple t(1, 2, 3);
	constexpr auto tuplecatresult = ::kelbon::tuple_cat(tpl, ::kelbon::tuple(1, 2, 3)); // tpl check
	auto tcatres1 = kelbon::tuple_cat(tpl, t, kelbon::tuple('c', 10, 150.f, nullptr));
	// todo -fixed string, которую можно передавать как шаблонный параметр + user defined literal, который её создает из const char*
	// todo - Clang Format
	constexpr bool isfctor = ::kelbon::functor<decltype([]() {})>;
	::kelbon::action<int(s*, float)> act = &s::method;
	s value1(15);
	s value2(16);

	::kelbon::tuple<int, float> tt1(5, 10.f);
	auto ttcpy = tt1;
	constexpr bool bj = std::is_copy_constructible_v<decltype(&s::method)>;
	constexpr bool bjrly = std::is_copy_constructible_v<kelbon::tuple<int(s::*)(float) const volatile& noexcept>>;
	auto rv1 = act(&value1, 3.14f);
	auto rv2 = act(&value2, 10.f);
	rv1 = act(&value1, 3.14f);
	::kelbon::action<int(s*, float)> act1;
	act1 = act.Clone();
	rv1 = act1(&value2, -2);

	::kelbon::action FF1([&fv](int x) mutable -> int { fv += 1; return static_cast<int>(static_cast<float>(x * 2) + fv); });

	// TODO - перевести всё на модули
	// Модули - можно ли экспортировать только одну специализацию???
	// todo - попробовать запустить code cleanup (на другой ветке гита)

	auto m1 = FF1.Clone();
	FF1(10);
	FF1(10);
	int check_value = m1(10);
	int check_value1 = FF1(10);
	FF1 = &testF;
	FF1 = &testF;
	FF1 = [](int v) -> int {return 2 * v; };
	FF1 = &Fctor::method;
	FF1 = &Fctor::method;
	FF1 = [&fv](int v) -> int {
		std::cout << "Mda";
		return static_cast<int>(static_cast<float>(150 + v) + fv); };

	FF1 = Fctor{};
	FF1 = std::move(fct);
	FF1 = std::move(fct);
	FF1 = std::move(FF1);
	auto FF2 = std::move(FF1);
	FF1 = std::move(FF2);

	TestsForActWrapper();
	TestsForConcepts();
	TestsForMemoryBlock();
	TestsForTraits();
	TestsForAction();

	return 0;
}