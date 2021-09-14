
#ifndef TEST_KELBON_MEMORY_BLOCK_HPP
#define TEST_KELBON_MEMORY_BLOCK_HPP

#include <string>
#include <sstream>
#include <array>
#include <vector>

#include "kelbon_memory_block.hpp"
#include "test_kelbon_base.hpp"
#include "kelbon_tuple.hpp"

namespace kelbon::test {
	
	inline void MemoryBlockTest() {
		int x = 10;
		std::vector<char> vect(100, 'c');
		std::string str("some string");
		memory_block justin = 5;
		justin = [](int x) { return x * 2; };
		justin = x;
		memory_block<200> block(x, vect, str, 266.f);

		try {
			auto& [a, b, c, d] = block.SafeGetDataAs<int, std::vector<char>, std::string, float>();

			if (a != x || b.size() != 100 || str != "some string" || d != 266.f) {
				throw test_failed("memory block GetDataAs works wrong");
			}
		}
		catch (bad_memory_block_access&) {
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
				auto& [a, b, c, d] = block_copy.SafeGetDataAs<int, std::vector<char>, std::string, float>();

				if (a != x || b.size() != 100 || str != "some string" || d != 266.f) {
					throw test_failed("memory block GetDataAs works wrong after move");
				}
				block = std::move(block_copy);
				auto& [a1, b1, c1, d1] = block.SafeGetDataAs<int, std::vector<char>, std::string, float>();

				if (a1 != x || b1.size() != 100 || str != "some string" || d1 != 266.f) {
					throw test_failed("memory block GetDataAs works wrong after move copy");
				}
			}
			catch (bad_memory_block_access&) {
				throw test_failed("memory block SafeGetDataAs checking false positive after move");
			}
		}
	}

	inline void MemoryBlockDestroyTest() {
		struct delete_checker {
			bool was_deleted = false;
			void operator++(int) {
				was_deleted = true;
			}
		};
		struct destr_test {
			delete_checker* v;
			destr_test(delete_checker& v) : v(&v) {}
			~destr_test() {
				if (v != nullptr) {
					(*v)++;
				}
			}
		};
		delete_checker counter;
		destr_test tester(counter);
		auto ptr = new memory_block<100>(tester);

		delete ptr;
		if (!counter.was_deleted) {
			throw test_failed("memory block dont delete its storage right");
		}
		tester.v = nullptr;
	}

	inline void MemoryBlockCloneTest() {
		auto v = "abcdf";
		memory_block block(150, 10.f, v, 16ull);
		auto block1 = block.Clone();
		try {
			auto& [a, b, c, d] = block1.SafeGetDataAs<int, float, const char*, unsigned long long>();
			if (a != 150 || b != 10.f || c != v || d != 16ull) {
				throw test_failed("memory block bad data after Clone");
			}
		}
		catch (...) {
			throw test_failed("memory_block stores data wrong or false positive throw exception after Clone");
		}
	}

	inline void MemoryBlockSerializationTest() {
		std::stringstream sstrm;

		std::array<float, 280> arr = { 3.f };

		std::vector<std::array<float, 280>> mega(15, arr);
		for (auto& bigvalue : mega) {
			for (auto& value : arr) {
				value = static_cast<float>(rand() % 2000);
			}
		}

		kelbon::memory_block block = { arr, mega, 5, std::string("Hello world"), 'c' };

		block.WriteAs<std::array<float, 280>, std::vector<std::array<float, 280>>, int, std::string, char>(sstrm);

		kelbon::memory_block<1208> read_block; // may be make_block function or something for size calculating

		read_block.ReadAs<std::array<float, 280>, std::vector<std::array<float, 280>>, int, std::string, char>(sstrm);

		auto& [x1, x2, x3, x4, x5] = read_block.GetDataAs<std::array<float, 280>, std::vector<std::array<float, 280>>, int, std::string, char>();
		auto& [x11, x22, x33, x44, x55] = block.GetDataAs<std::array<float, 280>, std::vector<std::array<float, 280>>, int, std::string, char>();

		bool test_value = (x1 == x11) && (x2 == x22) && (x3 == x33) && (x4 == x44) && (x5 == x55);
		if (!test_value) {
			throw test_failed("Memory block serialization reading or writing works bad\n");
		}
	}

	inline void TestsForMemoryBlock() {
		test_room tester;
		tester.AddTest(MemoryBlockTest);
		tester.AddTest(MemoryBlockDestroyTest);
		tester.AddTest(MemoryBlockCloneTest);
		tester.AddTest(MemoryBlockSerializationTest);
		tester.StartTesting();
	}

} // namespace kelbon::test

#endif // !TEST_KELBON_MEMORY_BLOCK_HPP

