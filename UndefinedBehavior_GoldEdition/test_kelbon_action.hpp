


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

		std::string operator()(size_t size, char c) volatile & noexcept {
			return std::string(size, c);
		}
	};
	using test_lambda = decltype([](size_t size, char c) -> std::string {
		return std::string(size, c);
	});

	inline void FunctionsTest() {
		testf2 functor;
		int v1 = 10;
		float v2 = 15.f;
		//::kelbon::tuple tt(testf2{});
		::kelbon::tuple tt1(std::move(functor));
		action<std::string(size_t, char)> act1;
		action act2(&testf); // Todo научиться принимать без &
		action act3(&testf);
		action act4(testf2{});
		// забирается владение передаваемой сущностью, так что конструктора на не мув версию нет
		action act5(std::move(functor));
		action act6(test_lambda{});
		action act7([v1, &v2](size_t size, char c) mutable noexcept ->std::string {
			v2 *= 2;
			v2 += v1;
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
		catch (const double_free_possible& msg) {
			// good way
		}
		fs[4] = std::move(act5);
		fs[5] = act6.Clone();
		fs[6] = act7.Clone();

		fs[0] = act7.Clone(); // was empty;
		for (auto& func : fs) {
			if (!func.Empty()) {
				std::string check_result = func(5, 'c');
				if (check_result != "ccccc") {
					throw test_failed("kelbon::action call works bad");
				}
			}
		}
		if (v2 != 90.f) {
			throw test_failed("kelobn::action lambdas with capture works bad");
		}
	}

	inline void TestsForAction() {
		test_room tester;

		tester.AddTest(FunctionsTest);

		tester.StartTesting();
	}
} // namespace kelbon::test

#endif // !TEST_KELBON_ACTION_HPP
