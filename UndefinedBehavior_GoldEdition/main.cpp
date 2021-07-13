
#include <iostream>
#include <string>

#include "kelbon_type_traits_functional.hpp"
#include "kelbon_memory_block.hpp"
#include "kelbon_type_traits_numeric.hpp"

#include "kelbon_tuple.hpp"

#include "kelbon_template_base_class.hpp"

// ������ ������� ������������ �������� �� ������� ����� - ��������� ����������(�� ����� ������� �������������� ������, ������ get< ... >, �� ���� ������ () �� <>
// ������ �������, ����������� ���-�� - ��������
// �������, ������������ ��� - using

//template<...>
//... WrapAction()

// todo - ���������, ������� ������ ������� ���� ��������� ������, ������������ � ����(��������� ������ ������ 1 ��������� �� ��������� � ��������/������ ������, ���������� ���������
// � �� ��� �������� ���� ����� ���������, �������� ��������� ������ / ���������� ����� �� �����.

// todo ������ is_method is_functor/lambda � �.�.(��� �������� � ��������� ������, � �� ������), ����� ����� is const/volatile/noexcept(���)
// ��� ���� & && ������������� ����� �������, ���������� ����������� �� ��������� ������������� (this)
// todo ����� ��� � ��� �������� ���������?
// ����� ��� �������� ���� tuple �� ������ ����� �� �������� �� ���������� ��� ������, �� ������ ����� �� ��������� ������ � ������ �������

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
// TODO - �������� ����� � ���� static asserts � ����� ������� ����� ��� type_traits � �.�., ���������� �� � on_debug
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

	// TODO - ��������� ��� �� � ������������� ����� namespace, �� inline namespace ������������ ���-��

	//std::tuple<int, double, float, std::string> stp(5, 3., 3.f, std::string("33"));
	//auto x = tp.get<3>();
	//tp.get<3>() = "no idea what to write";

	// TODO - ��� tuple �������� begin/end /iterator, ������������� std::tuple_element std::tuple_size, deduction guides � �.�.

	
	// TODO - ������ �������� �� ������ �� ������������������ ��������(������ ����� ���� ��������) � sequence ��������(������ �������� ����� ��������, ������ �� ������, ���-��...)
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