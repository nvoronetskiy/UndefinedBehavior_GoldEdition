
#include <iostream>
#include <string>

#include "kelbon_type_traits_functional.hpp"
#include "kelbon_memory_block.hpp"
#include "kelbon_type_traits_numeric.hpp"

#include "kelbon_tuple.hpp"

// ������ ������� ������������ �������� �� ������� ����� - ��������� ����������(�� ����� ������� �������������� ������, ������ get< ... >, �� ���� ������ () �� <>
// ������ �������, ����������� ���-�� - ��������
// �������, ������������ ��� - using
/*
// ������ �������� ������ � ����������� ���������� () � protected ����, ����������� � ������������ ����� �� ��������� ���������� � �������, ��-��.
template<typename ResultType, typename ... ArgTypes>
class base_action {	
public:
	using result_type = ResultType;
	template<size_t index> using get_element = typename type_of_element<index, ArgTypes...>::type;

	virtual ~base_action() = 0;
protected:
	virtual result_type operator()(ArgTypes...) = 0;
};

template<typename Function, typename BaseClass>
class wrap_action : public BaseClass {
public:
protected:

};
*/
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
	test(test&& other) {
		std::cout << "move" << std::endl;
	}

	test& operator=(const test&) {
		std::cout << "operator=COPY" << std::endl;
		return *this;
	}
	test& operator=(test&&) {
		std::cout << "operator=MOVE" << std::endl;
		return *this;
	}
	~test() {
		std::cout << "destroyed" << std::endl;
	}
	int v;
};
// TODO - �������� ����� � ���� static asserts � ����� ������� ����� ��� type_traits � �.�., ���������� �� � on_debug
using namespace kelbon;
int main() {

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