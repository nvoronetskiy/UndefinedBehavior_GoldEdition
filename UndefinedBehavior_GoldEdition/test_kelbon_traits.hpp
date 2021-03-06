
#ifndef TEST_KELBON_TRAITS_HPP
#define TEST_KELBON_TRAITS_HPP

#include <concepts>

#include "kelbon_type_traits_base.hpp"
#include "kelbon_type_traits_functional.hpp"
#include "kelbon_type_traits_advanced.hpp"
#include "kelbon_type_traits_variadic.hpp"

#include "test_kelbon_base.hpp"

namespace kelbon::test {
	using std::same_as;
	inline void CreateRvalueRefTest() {
		constexpr bool test_value =
			same_as<int&&, create_rvalue_reference_t<int>> &&
			same_as<int&&, create_rvalue_reference_t<int&>> &&
			same_as<int&&, create_rvalue_reference_t<int&&>> &&
			same_as<volatile int&&, create_rvalue_reference_t<int volatile>> &&
			same_as<volatile int&&, create_rvalue_reference_t<volatile int&>> &&
			same_as<volatile int&&, create_rvalue_reference_t<volatile int&&>>;

		static_assert(test_value, "create_rvalue_reference test failed");
	}

	inline void ConditionalTest() {
		constexpr bool test_value =
			same_as<float, conditional_t<false, int, float>> &&
			same_as<int, conditional_t<true, int, float>>;

		static_assert(test_value, "conditional test failed");
	}

	inline void DecayTest() {
		constexpr bool test_value =
			same_as<int, decay_t<const volatile int*****>> &&
			same_as<char, decay_t<char&&>> &&
			same_as<char, decay_t<const char*>> &&
			!same_as<char, decay_t<char8_t>> &&
			same_as<char, decay_t<const volatile char&>> &&
			same_as<char, decay_t<const char&>> &&
			same_as<char, decay_t<const char* const>> &&
			same_as<char, decay_t<const char&&>> &&
			same_as<char, decay_t<const char&>> &&
			same_as<char, decay_t<const volatile char[]>>;

		static_assert(test_value, "decay test failed");
	}

	inline void SignatureTest() {
		using test_func = decltype(DecayTest);
		constexpr bool test_value =
			!signature<test_func>::is_noexcept &&
			same_as<::kelbon::func::parameter_list<test_func>, type_list<>> &&
			same_as<void, ::kelbon::func::result_type<test_func>> &&
			same_as<::kelbon::func::parameter_list<decltype([](float, int) { return true; })>, type_list<float, int>> &&
			// lambdas with capture have no default constructor, so it cant be sended into function_info like template parameter
			same_as<typename signature<decltype([test_value](float) {return test_value; })>::parameter_list, type_list<float>>;

		static_assert(test_value, "signature trait fails test");
	}

	inline void TestsForTraits() {
		test_room tester;

		tester.AddTest(CreateRvalueRefTest);
		tester.AddTest(ConditionalTest);
		tester.AddTest(DecayTest);
		tester.AddTest(SignatureTest);

		tester.StartTesting();
	}

} // namespace kelbon::test

#endif // !TEST_KELBON_TRAITS_HPP

