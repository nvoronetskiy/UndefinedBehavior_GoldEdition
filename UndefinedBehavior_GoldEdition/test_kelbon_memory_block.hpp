
#ifndef TEST_KELBON_MEMORY_BLOCK_HPP
#define TEST_KELBON_MEMORY_BLOCK_HPP

#include "kelbon_memory_block.hpp"
#include "test_kelbon_base.hpp"
#include "kelbon_tuple.hpp"

#include <string>
#include <vector>

namespace kelbon::test {

	void memory_block_constructor_test() {
		int x = 10;
		std::vector<char> vect(100, 'c');
		std::string str("some string");

		memory_block block(x, vect, str, 266.f);

		try {
			auto [a, b, c, d] = block.SafeGetDataAs<int, std::vector<char>, std::string, float>();

			if (a != x || b.size() != 100 || str != "some string" || d != 266.f) {
				throw test_failed("memory block GetDataAs works wrong");
			}
		}
		catch (bad_memory_block_access& error) {
			throw test_failed("memory block SafeGetDataAs checking false positive");
		}
		try {
			auto& [a1, b1, c1, d1] = block.SafeGetDataAs<int&, std::vector<char>, std::string, float>();
			throw test_failed("memory block SafeGetDataAs checking false negative");
		}
		catch (bad_memory_block_access&) {
			// good way
			try {
				auto block_copy = std::move(block);
				auto [a, b, c, d] = block.SafeGetDataAs<int, std::vector<char>, std::string, float>();

				if (a != x || b.size() != 100 || str != "some string" || d != 266.f) {
					throw test_failed("memory block GetDataAs works wrong after move");
				}
			}
			catch (bad_memory_block_access& error) {
				throw test_failed("memory block SafeGetDataAs checking false positive after move");
			}
		}
		
	}

	void TestsForMemoryBlock() {
		test_room tester;
		tester.AddTest(&memory_block_constructor_test);

		tester.StartTesting();
	}
} // namespace kelbon::test

#endif // !TEST_KELBON_MEMORY_BLOCK_HPP

