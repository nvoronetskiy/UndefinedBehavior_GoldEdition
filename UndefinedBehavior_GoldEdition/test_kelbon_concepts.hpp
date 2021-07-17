
#ifndef TEST_KELBON_CONCEPTS_HPP
#define TEST_KELBON_CONCEPTS_HPP

#include "test_kelbon_base.hpp"
#include "kelbon_concepts_base.hpp"
#include "kelbon_type_traits_functional.hpp"

namespace kelbon::test {

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

	void FunctorTest() {
		struct test_class {
			void operator()() {}
		};
		struct no_functor {};

		constexpr bool test_value = like_functor<test_class> && !like_functor<no_functor>;

		static_assert(test_value, "like_functor concept fails test");
	}

	void CallableTest() {
		struct test_class {
			void method(){}
		};
		constexpr bool test_value =
			callable<decltype(CallableTest)> &&
			callable<decltype(&CallableTest)> &&
			callable<decltype(&test_class::method)> &&
			callable<decltype([](int) -> bool { return false; })> &&
			callable<decltype([&test_value](float) {})>;

		static_assert(test_value, "callable concept fails test");
	}
	void TestsForConcepts() {
		test_room tester;

		tester.AddTest(NumericTest);
		tester.AddTest(FunctorTest);
		tester.AddTest(CallableTest);

		tester.StartTesting();
	}
	
} // namespace kelbon::test

#endif // !TEST_KELBON_CONCEPTS_HPP
