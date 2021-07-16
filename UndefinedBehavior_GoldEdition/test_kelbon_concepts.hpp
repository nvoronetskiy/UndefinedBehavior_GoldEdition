
#ifndef TEST_KELBON_CONCEPTS_HPP
#define TEST_KELBON_CONCEPTS_HPP

#include "test_kelbon_base.hpp"
#include "kelbon_concepts_base.hpp"

namespace kelbon::test {

	void numeric_test() {
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

	void functor_test() {
		struct test_class {
			void operator()() {}
		};
		struct no_functor {};

		constexpr bool test_value = functor<test_class> && !functor<no_functor>;

		static_assert(test_value, "functor concept fails test");
	}

	void TestsForConcepts() {
		test_room tester;

		tester.AddTest(&numeric_test);
		tester.AddTest(&functor_test);

		tester.StartTesting();
	}
	
} // namespace kelbon::test

#endif // !TEST_KELBON_CONCEPTS_HPP
