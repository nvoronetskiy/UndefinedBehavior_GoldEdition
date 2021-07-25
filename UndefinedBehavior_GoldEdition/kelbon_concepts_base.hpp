
#ifndef KELBON_CONCEPTS_BASE_HPP
#define KELBON_CONCEPTS_BASE_HPP

#include <concepts>

namespace kelbon {
	// for situatuions like requires { { expression } -> exist }; 
	template<typename T>
	concept exist = true;

	// CONCEPT NUMERIC
	namespace detail {
		template<typename T, typename U>
		concept convertible_or_same = std::convertible_to<T, U> || std::same_as<T, U>;
	} // namespace detail

	template<typename T>
	concept numeric = requires(T a, T b) {
		{a + b}->detail::convertible_or_same<T>; // �������� ���� ����� ��� ��� �� ���. ��� char ������� ���� �� ��������� std::same_as, �.�. char + char = int
		{a - b}->detail::convertible_or_same<T>;
	} && requires {
		static_cast<T>(4); // ����� ��������� �� int
	};

} // namespace kelbon

#endif // !KELBON_CONCEPTS_BASE_HPP
