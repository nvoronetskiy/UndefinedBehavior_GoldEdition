
#ifndef TEST_KELBON_BASE_HPP
#define TEST_KELBON_BASE_HPP

#include <list>
#include <memory>
#include <iostream>

#include "kelbon_template_base_class.hpp"
#include "kelbon_memory_block.hpp"

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
		template<typename T> requires function<T>
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

	struct flags {
		constexpr flags() noexcept
			: moved(), copied(), deleted(), default_constructed(), copy_constructed(), move_constructed()
		{}
		char moved : 1;
		char copied : 1;
		char deleted : 1;
		char default_constructed : 1;
		char copy_constructed : 1;
		char move_constructed : 1;
	};
	struct test_struct {
		test_struct() : usefull_data() {
			std::cout << "default" << std::endl;
		}
		test_struct(const test_struct& other) {
			std::cout << "copy" << std::endl;
		}
		test_struct(test_struct&& other) noexcept {
			std::cout << "move" << std::endl;
		}
		test_struct& operator=(const test_struct&) {
			std::cout << "operator=COPY" << std::endl;
			return *this;
		}
		test_struct& operator=(test_struct&&) noexcept {
			std::cout << "operator=MOVE" << std::endl;
			return *this;
		}
		~test_struct() {
			std::cout << "destroyed" << std::endl;
		}
		flags usefull_data;
	};
} // namespace kelbon::test

#endif // !TEST_KELBON_BASE_HPP
