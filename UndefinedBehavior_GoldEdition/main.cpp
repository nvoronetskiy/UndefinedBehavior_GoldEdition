
#include <iostream>
#include <string>
#include <memory>

#include "test_kelbon_concepts.hpp"
#include "test_kelbon_memory_block.hpp"
#include "test_kelbon_act_wrapper.hpp"

#include "kelbon_type_traits_functional.hpp"
#include "kelbon_memory_block.hpp"
#include "kelbon_type_traits_numeric.hpp"

#include "kelbon_tuple.hpp"

#include "kelbon_template_base_class.hpp"

// todo - ���������, ������� ������ ������� ���� ��������� ������, ������������ � ����(��������� ������ ������ 1 ��������� �� ��������� � ��������/������ ������, ���������� ���������
// � �� ��� �������� ���� ����� ���������, �������� ��������� ������ / ���������� ����� �� �����.

using namespace kelbon::test;

// todo - cmake for this project
// todo - ��������� �� gcc

int main() {

	TestsForActWrapper();
	TestsForConcepts();
	TestsForMemoryBlock();

	return 0;
}