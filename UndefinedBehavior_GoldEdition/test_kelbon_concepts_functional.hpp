

#ifndef TEST_KELBON_CONCEPTS_FUNCTIONAL_HPP
#define TEST_KELBON_CONCEPTS_FUNCTIONAL_HPP

#include "kelbon_concepts_functional.hpp"
#include "test_kelbon_base.hpp"

namespace kelbon::test {

	void functor_test() {
		struct test_class {
			void operator()() {}
		};
		struct no_functor {};

		constexpr bool test_value = functor<test_class> && !functor<no_functor>;

		static_assert(test_value, "functor concept fails test");
	}

	void TestsForConceptsFunctional() {
		test_room tester;
		tester.AddTest(&functor_test);

		tester.StartTesting();
	}
} // namespace kelbon::test

#endif // !TEST_KELBON_CONCEPTS_FUNCTIONAL_HPP

