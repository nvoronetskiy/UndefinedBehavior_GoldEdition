
#ifndef TEST_KELBON_CONCEPTS_HPP
#define TEST_KELBON_CONCEPTS_HPP

#include "test_kelbon_base.hpp"
#include "kelbon_concepts_base.hpp"
#include "kelbon_type_traits_functional.hpp"

namespace kelbon::test {

	struct test_class {
		void operator()() {}
		void method() {}
	};

	void NumericTest() {
		constexpr bool test_value =
			numeric<int> &&
			numeric<unsigned long> &&
			numeric<long long> &&
			numeric<float> &&
			numeric<double> &&
			numeric<long double> &&
			numeric<short> &&
			!numeric<void*> &&
			!numeric<nullptr_t> &&
			!numeric<int&> &&
			!numeric<const char*>;

		static_assert(test_value, "numeric concept fails test");
	}

	void LikeFunctorTest() {
		test_class{}();
		struct no_functor {};

		constexpr bool test_value =
			like_functor<test_class> &&
			!like_functor<no_functor> &&
			!like_functor<decltype(&test_class::operator())>;

		static_assert(test_value, "like_functor concept fails test");
	}

	void CallableTest() {
		constexpr bool test_value =
			callable<decltype(CallableTest)> &&
			callable<decltype(&CallableTest)> &&
			callable<decltype(&test_class::method)> &&
			callable<decltype([](int) -> bool { return false; })> &&
			callable<decltype([&test_value](float) { return test_value; }) > ;

		static_assert(test_value, "callable concept fails test");
	}

	void FunctorTest() {
		struct no_functor {};
		constexpr bool test_value =
			functor<decltype([]() mutable noexcept { return false; }) > &&
			functor<test_class> && !functor<no_functor> &&
			functor<decltype([&test_value](float) { return test_value; }) > &&
			!functor<decltype(&FunctorTest)> &&
			!functor<int> &&
			!functor<decltype(&test_class::operator())>;

		static_assert(test_value, "functor concept failes test");
	}

	void MethodTest() {
		constexpr bool test_value =
			method<decltype(&test_class::method)> &&
			method<decltype(&test_class::operator())> &&
			method<decltype(&test_room::AddTest<decltype(&MethodTest)>)> &&
			!method<decltype([]() {})> &&
			!method<decltype(MethodTest)> &&
			!method<decltype(&MethodTest)> &&
			!method<void*>;

		static_assert(test_value, "method concept fails test");
	}

	void FunctionTest() {
		constexpr bool test_value =
			function<decltype(FunctionTest)> &&
			!function<decltype([]() mutable {}) > &&
			function<decltype(&test_class::method)> && // методы это частный случай функций
			function<void(float, double)> &&
			!function<void>; 

		static_assert(test_value, "method concept fails test");
	}

	void TestsForConcepts() {
		test_room tester;

		tester.AddTest(NumericTest);
		tester.AddTest(LikeFunctorTest);
		tester.AddTest(CallableTest);
		tester.AddTest(FunctorTest);
		tester.AddTest(MethodTest);
		tester.AddTest(FunctionTest);

		tester.StartTesting();
	}
	
} // namespace kelbon::test

#endif // !TEST_KELBON_CONCEPTS_HPP
