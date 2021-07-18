
#ifndef TEST_KELBON_BASE_HPP
#define TEST_KELBON_BASE_HPP

#include <list>
#include <memory>
#include <iostream>

#include "kelbon_concepts_functional.hpp"
#include "kelbon_template_base_class.hpp"

namespace kelbon::test {
	
	// ничего не принимает, возвращает bool, обычный тестик
	using base_test = ::kelbon::base_action<void>;
	using test_task = act_wrapper<base_action, void(*)()>;

	class test_failed : public std::exception {
	public:
		test_failed(const char* const message) noexcept : std::exception(message) {}
	};

	class test_room {
	private:
		std::list<std::unique_ptr<base_test>> tests;
	public:
		template<typename T> requires callable<T>
		void AddTest(T&& task) {
			tests.emplace_back(std::make_unique<act_wrapper<::kelbon::base_action, T>>(std::forward<T>(task)));
		}
		test_room() = default;

		void StartTesting() noexcept(false) {
			for (auto& test : tests) {
				try {
					(*test)();
				}
				catch (test_failed& error) {
					std::cerr << "Test failed, additional info:\t" << error.what() << std::endl;
				}
				catch (...) {
					std::cerr << "Unexpected exception, something bad here" << std::endl;
					throw;
				}
			}
		}
	};

} // namespace kelbon::test

#endif // !TEST_KELBON_BASE_HPP
