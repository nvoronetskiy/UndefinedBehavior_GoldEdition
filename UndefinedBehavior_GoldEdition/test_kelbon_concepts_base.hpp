
#ifndef TEST_KELBON_CONCEPTS_BASE_HPP
#define TEST_KELBON_CONCEPTS_BASE_HPP
#if _DEBUG

#include "kelbon_preprocessor.hpp"
#include "kelbon_concepts_base.hpp"

namespace kelbon {
	// ������ ����� ��� ������ ���� ���� std::list std::unique_ptr<template_base_class> ������� � ���� ����������� �������(��������()), �����������. 
	// TEST_START ����� �������� ���������� �������, ������ �������� ��� ��������� �����, ������ ���� ��� ���������� ����������(��������� �� �����) � std::list
	// ������. � ����� ����� ������� ������� ����������� ���������� ���������� ����� � ������������ ���������() � ������� �� �����������.
	// ������������ ����� ��� ���� ��� �� ��������� base ����� � 
	TEST(same_as) {
		//static_assert(!same_as<int, int>);
	}
	

} // namespace kelbon


#endif // _DEBUG
#endif // !TEST_KELBON_CONCEPTS_BASE_HPP
