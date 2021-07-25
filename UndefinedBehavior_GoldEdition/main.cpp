
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

// todo - ���������, ������� ������ ������� ���� ��������� ������, ������������ � ����(��������� ������ ������ 1 ��������� �� ��������� � ��������/������ ������, ���������� ���������
// � �� ��� �������� ���� ����� ���������, �������� ��������� ������ / ���������� ����� �� �����.

using namespace kelbon::test;

// todo - cmake for this project
// todo - ��������� �� gcc

// todo - ������� ���, ����� � act_wrapper ����� ���� ���������� ����� callable, ������� ���� �����. �������� ������ int(float )��� �� �� �����, ��� � ������� int(float)
// ��� ����� ����� ������� memory_block, � ������� ������� ����� callble, �������� ��������=, ����������� ����� 1 callble � ������ �� return � args, 
// � ��� ���������� ��� Destroy ��� �������� �������(Call), �� ���� ����� ��������� ������� ����������� memory_block �� ������(�����������)
//  � ������������ ��������� �� �������, ������� ����� ��������... ��������� ������� ��������. ������ ������������ ������� �������� ����� block.GetDataAs<Actor>()



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
	// todo - �������� ���� ����������� �� clone �������, ������� ����� ���� ����������, ���� ������������ �����(��������� throw � ������������ ������������)
	// todo ��������� ����� ������ ����� ������� Clone
	// TODO - ������ static_asserts �������� �� �� requires closure
	// TODO - ��������� �� �� ������
	// TODO CallByMemory ����������� �������

	// todo - ����������� ��������� code cleanup (�� ������ ����� ����)
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

	//FF1 = Fctor{}; // todo - � ���
	//FF1 = fct;
	//FF1 = std::move(fct); // todo - ����������� ��� �� ��� ���� ���� �����������
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