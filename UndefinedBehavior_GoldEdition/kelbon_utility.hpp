
#ifndef KELBON_UTILITY_HPP
#define KELBON_UTILITY_HPP

namespace kelbon {

	// TEMPLATE FUNCTION sizeof_pack
	template<typename ... Types>
	[[nodiscard]] consteval size_t sizeof_pack() noexcept {
		return (sizeof(Types) + ...);
	}

	// applies first thing to all pack, very usefull thing in noexcept/just variadic checks... (Compilers works bad with fold expressions)
	template<template<typename...> typename Check, typename ... Types>
	[[nodiscard]] consteval bool all_in_pack() noexcept {
		return (Check<Types>::value && ...);
	}

	template<template<typename...> typename Check, typename ... Types>
	[[nodiscard]] consteval bool atleast_one_in_pack() noexcept {
		return (Check<Types>::value || ...);
	}

	// TEMPLATE FUNCTION always_false FOR STATIC ASSERTS (for example)
	[[nodiscard]] consteval inline bool always_false(auto) noexcept { return false; }
	template<typename T>
	[[nodiscard]] consteval inline bool always_false() noexcept { return false; }
} // namespace kelbon

#endif // !KELBON_UTILITY_HPP


