


#ifndef TEST_KELBON_ACTION_HPP
#define TEST_KELBON_ACTION_HPP

#include <string>
#include <array>

#include "test_kelbon_base.hpp"
#include "kelbon_action.hpp"

namespace kelbon::test {
	inline std::string testf(size_t size, char c) {
		return std::string(size, c);
	}
	struct testf1 {
		testf1(const testf1&) = delete;
		std::string testm(size_t size, char c) const {
			return std::string(size, c);
		}
		std::string testm(size_t size, char c) const volatile {
			return std::string(size, c);
		}

		std::string operator()(size_t size, char c) const {
			return std::string(size, c);
		}
		
	};
	struct testf2 {
		testf2() = default;
		//testf2(const testf2&) = delete;
		testf2(testf2&&) = default;
		testf2& operator=(testf2&&) = default;

		std::string operator()(size_t size, char c) volatile noexcept {
			return std::string(size, c);
		}
	};
	using test_lambda = decltype([](size_t size, char c) -> std::string {
		return std::string(size, c);
	});
	struct test_ref_overload {
		char operator()(const int& x, double& y) const& {
			y = 200.;
			return static_cast<char>(x);
		}
	};
	inline void RefOverloadTest() {
		test_ref_overload functor;
		action act = functor;
		int x = 5;
		double y = 10;
		char result = act(x, y);
		if (result != static_cast<char>(x) || y != 200.) {
			throw test_failed("kelbon::action for functors with & overloaded operator() dont work");
		}
	}
	struct testfov {
		int operator()(int) {
			return 10;
		}
		int operator()(double ender) {
			return 15;
		}
	};
	inline void TemplateLambdasTest() {
		auto tlambda = [](auto... args) {
			return (args + ...);
		};
		action<int(int, int)> tact = tlambda;
		if (tact(5, 10) != 15) {
			throw test_failed("kelbon::action works bad with template-lambda");
		}
		action<int(double)> act2 = testfov{};
		if (act2(10) != 15) {
			throw test_failed("kelbon::action works bad for overloaded operator() in functors");
		}
	}
	inline void FunctionsTest() {
		testf2 functorv;
		int v1 = 10;
		float v2 = 15.f;
		::kelbon::tuple tt(testf2{});
		::kelbon::tuple tt1(std::move(functorv));
		action<std::string(size_t, char)> act1;
		action act2(testf);
		action act3(&testf);
		action<std::string(size_t, char)> act_template = [](auto...) {return std::string("Hello world"); };
		if (act_template(10, 'c') != "Hello world") {
			throw test_failed("kelbon::action classes with template operator() dont work");
		}
		action<std::string(size_t, char)> act4(testf2{});
		action act5(std::move(functorv));
		action act6(test_lambda{});
		action act7([v1, &v2](size_t size, char c) mutable noexcept ->std::string {
			v2 *= 2;
			v2 += static_cast<float>(v1);
			return std::string(size, c);
		});

		if (!act1.Empty() || act7.Empty()) {
			throw test_failed("kelbon::action method Empty dont work");
		}
		std::array<action<std::string(size_t, char)>, 7> fs;
		fs[0] = std::move(act1);
		fs[1] = act2.Clone();
		fs[2] = act3.Clone();
		try {
			fs[3] = act4.Clone();
			fs[4] = act5.Clone();

			throw test_failed("kelbon::action Clone dont throw exception when needed");
		}
		catch (const double_free_possible&) {
			// good way
		}
		fs[4] = std::move(act5);
		fs[5] = act6.Clone();
		fs[6] = act7.Clone();

		fs[0] = act7.Clone(); // was empty
		for (auto& func : fs) {
			if (!func.Empty()) {
				std::string check_result = func(5, 'c');
				if (check_result != "ccccc") {
					throw test_failed("kelbon::action call works bad");
				}
			}
		}
		if (v2 != 90.f) {
			throw test_failed("kelbon::action lambdas with capture works bad");
		}
	}

	inline void TestsForAction() {
		test_room tester;

		tester.AddTest(FunctionsTest);
		tester.AddTest(RefOverloadTest);
		tester.AddTest(TemplateLambdasTest);

		tester.StartTesting();
	}
} // namespace kelbon::test

#endif // !TEST_KELBON_ACTION_HPP
