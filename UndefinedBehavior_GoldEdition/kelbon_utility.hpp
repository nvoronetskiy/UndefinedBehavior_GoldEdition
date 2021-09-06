
#ifndef KELBON_UTILITY_HPP
#define KELBON_UTILITY_HPP

#include <type_traits>

namespace kelbon {

	// TEMPLATE FUNCTION sizeof_pack
	template<typename ... Types>
	[[nodiscard]] consteval size_t sizeof_pack() noexcept {
		return (sizeof(Types) + ...);
	}
	template<typename T>
	requires (std::is_enum_v<T>)
	constexpr auto to_underlying(T value) noexcept {
		return static_cast<std::underlying_type_t<T>>(value);
	}

	// applies first thing to all pack, very usefull thing in noexcept/just variadic checks... (Compilers works bad with fold expressions)
	template<template<typename...> typename Check, typename ... Types>
	constexpr bool all_in_pack = (Check<Types>::value && ...);

	template<template<typename...> typename Check, typename ... Types>
	constexpr bool atleast_one_in_pack = (Check<Types>::value || ...);

	// TEMPLATE FUNCTION always_false FOR STATIC ASSERTS (for example)
	[[nodiscard]] consteval inline bool always_false(auto) noexcept { return false; }
	template<typename...>
	[[nodiscard]] consteval inline bool always_false() noexcept { return false; }

} // namespace kelbon

#endif // !KELBON_UTILITY_HPP


