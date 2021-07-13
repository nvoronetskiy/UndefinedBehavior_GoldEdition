
#ifndef TEST_KELBON_CONCEPTS_BASE_HPP
#define TEST_KELBON_CONCEPTS_BASE_HPP
#if _DEBUG

#include "kelbon_preprocessor.hpp"
#include "kelbon_concepts_base.hpp"

namespace kelbon {
	// короче тесты это должен быть один std::list std::unique_ptr<template_base_class> имеющий у себ€ виртуальную функцию(оператор()), тестирующую. 
	// TEST_START будет означать объ€вление функции, внутри написаны все остальные тесты, каждый тест это добавление экзекутора(указател€ на класс) в std::list
	// тестов. ј потом после вызоыва функции добавл€ющей экзекуторы происходит вызов и виртуального оператора() у каждого из экзекуторов.
	// использовать можно при этом мои же шаблонный base класс и 
	TEST(same_as) {
		//static_assert(!same_as<int, int>);
	}
	

} // namespace kelbon


#endif // _DEBUG
#endif // !TEST_KELBON_CONCEPTS_BASE_HPP
