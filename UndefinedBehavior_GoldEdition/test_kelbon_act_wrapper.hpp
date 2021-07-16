
#ifndef TEST_KELBON_ACT_WRAPPER_HPP
#define TEST_KELBON_ACT_WRAPPER_HPP

#include <memory>
#include <list>

#include "test_kelbon_base.hpp"
#include "kelbon_template_base_class.hpp"

namespace kelbon::test {

	void act_wrapper_lambdas_test() {
		// протестировать для всех возможных штук + возможность полиморфного использования
		act_wrapper wrap1([](int iv, float fv) { return iv + fv; });
		auto wrap2 = wrap_action([](int iv, float fv) { return iv + fv; });

		if (wrap1(10, 20) != 30.f || wrap2(10, 20) != 30.f) {
			throw test_failed("act_wrapper do not work for lambda without capture");
		}
		int iv = 10;
		float fv = 10.f;

		act_wrapper wrap3([iv, &fv](int mul, float = 0.f) {return (iv + fv) * mul; });
		auto wrap4 = wrap_action([iv, &fv](int mul) {return (iv + fv) * mul; });
		if (wrap3(3,0.f) != 60.f || wrap4(3) != 60.f) {
			throw test_failed("act_wrapped do not work for lambda with capture");
		}
		std::list<std::unique_ptr<base_action<float, int, float>>> test_list;

		auto m = act_wrapper(wrap3);
		test_list.emplace_back(new act_wrapper(wrap1));
		test_list.emplace_back(new act_wrapper([](int iv, float fv) { return iv + fv; }));
		test_list.emplace_back(new act_wrapper(std::move(wrap3)));
		test_list.emplace_back(new act_wrapper([iv, &fv](int mul, float) {return (iv + fv) * mul; }));
	}

	void TestsForActWrapper() {
		test_room tester;
		
		tester.AddTest(&act_wrapper_lambdas_test);

		tester.StartTesting();
	}

} // namespace kelbon::test

#endif // !TEST_KELBON_ACT_WRAPPER_HPP

