
#ifndef TEST_KELBON_CONCEPTS_HPP
#define TEST_KELBON_CONCEPTS_HPP

#include "test_kelbon_base.hpp"
#include "kelbon_concepts_base.hpp"

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

	void TestsForConcepts() {
		test_room tester;

		tester.AddTest(&NumericTest);
		tester.AddTest(&FunctorTest);

		tester.StartTesting();
	}
	
} // namespace kelbon::test

#endif // !TEST_KELBON_CONCEPTS_HPP
