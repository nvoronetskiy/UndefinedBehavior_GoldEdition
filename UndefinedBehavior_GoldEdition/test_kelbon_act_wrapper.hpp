
#ifndef TEST_KELBON_ACT_WRAPPER_HPP
#define TEST_KELBON_ACT_WRAPPER_HPP

#include <memory>
#include <list>

#include "test_kelbon_base.hpp"
#include "kelbon_template_base_class.hpp"

namespace kelbon::test {

	void ActWrapperLambdasTest() {
		// протестировать для всех возможных штук + возможность полиморфного использования
		act_wrapper wrap1([](int iv, float fv) { return iv + fv; });
		auto wrap2 = WrapAction([](int iv, float fv) { return iv + fv; });

		if (wrap1(10, 20) != 30.f || wrap2(10, 20) != 30.f) {
			throw test_failed("act_wrapper do not work for lambda without capture");
		}
		int iv = 10;
		float fv = 10.f;

		act_wrapper wrap3([iv, &fv](int mul, float = 0.f) {return (iv + fv) * mul; });
		auto wrap4 = WrapAction([iv, &fv](int mul) {return (iv + fv) * mul; });
		if (wrap3(3,0.f) != 60.f || wrap4(3) != 60.f) {
			throw test_failed("act_wrapped do not work for lambda with capture");
		}
		std::list<std::unique_ptr<base_action<float, int, float>>> test_list;

		auto m = act_wrapper(wrap3);
		test_list.emplace_back(new act_wrapper(wrap1));
		test_list.emplace_back(new act_wrapper([](int iv, float fv) { return iv + fv; }));
		test_list.emplace_back(new act_wrapper(std::move(wrap3)));
		test_list.emplace_back(new act_wrapper([iv, &fv](int mul, float) {return (iv + fv) * mul; }));

		int result = 0;
		for (auto& func : test_list) {
			result += static_cast<float>((*func)(2, 3.f)); // 5 + 5 + 40 + 40
		}
		if (result != 90) {
			throw test_failed("act_wrapper calls bad");
		}
	}

	void ActWrapperFunctorsTest() {
		struct functor {
			functor() = default;
			size_t operator()(int value) {
				return static_cast<size_t>(value) * 2ull;
			}
		};
		act_wrapper wrap1(functor{});
		auto wrap2 = WrapAction(functor{});
		int x = 10;
		auto m = [x]() mutable noexcept  {
			return x + 2;
		};
		act_wrapper wrap3(std::move(m));
		if (wrap1(10) != 20 || wrap2(10) != 20 || wrap3() != 12) {
			throw test_failed("act_wrapper bad working with functors");
		}
	}

	bool TestFunc1(char v) {
		return true;
	}
	using TestUsing = bool(*)(char);
	bool TestFunc2(TestUsing fn) {
		return fn('c');
	}
	void ActWrapperFunctionsTest() {
		act_wrapper wrap1(&TestFunc1);
		auto wrap2 = WrapAction(&TestFunc2);

		// without reference/pointer
		auto wrap3 = WrapAction(TestFunc1);
		act_wrapper wrap4(TestFunc2);
		wrap3('\0');
		wrap4(TestFunc1);

		std::list<std::unique_ptr<base_action<bool(char)>>> test_list;
		test_list.emplace_back(new act_wrapper(wrap1));
		if ((*test_list.front())('c') != true) {
			throw test_failed("act_wrapper failed with functions + unique_ptr");
		}
		if (!wrap1('c') || !(wrap2(TestFunc1))) {
			throw test_failed("act_wrapper error with functions");
		}
	}

	void ActWrapperMethodsTest() {
		struct test_me {
			int method(int f) {
				return f * 2;
			}
		};
		act_wrapper wrap1(&test_me::method);
		auto wrap2 = WrapAction(&test_me::method);
		test_me value;
		if (wrap1(&value, 10) != 20 || wrap2(&value, 10) != 20) {
			throw test_failed("act_wrapper with method fails");
		}
		std::list<std::unique_ptr<base_action<int(test_me*, int)>>> test_list;
		test_list.emplace_back(new act_wrapper(wrap2));
		if ((*test_list.front())(&value, 5) != 10) {
			throw test_failed("act_wrapper method and unique ptr combo fails");
		}
	}

	void TestsForActWrapper() {
		test_room tester;
		
		tester.AddTest(ActWrapperLambdasTest);
		tester.AddTest(ActWrapperFunctorsTest);
		tester.AddTest(ActWrapperFunctionsTest);
		tester.AddTest(ActWrapperMethodsTest);

		tester.StartTesting();
	}

} // namespace kelbon::test

#endif // !TEST_KELBON_ACT_WRAPPER_HPP

