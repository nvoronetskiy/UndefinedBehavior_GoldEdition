
#ifndef TEST_KELBON_CONCEPTS_BASE_HPP
#define TEST_KELBON_CONCEPTS_BASE_HPP

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

	void TestsForConceptsBase() {
		test_room tester;
		tester.AddTest(&numeric_test);

		tester.StartTesting();
	}
	
} // namespace kelbon::test

#endif // !TEST_KELBON_CONCEPTS_BASE_HPP
