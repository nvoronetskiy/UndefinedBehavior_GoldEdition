
#ifndef KELBON_UTILITY_HPP
#define KELBON_UTILITY_HPP

namespace kelbon {

	// TEMPLATE FUNCTION sizeof_pack
	template<typename ... Types>
	consteval size_t sizeof_pack() noexcept {
		return (sizeof(Types) + ...);
	}

	// applies first thing to all pack, very usefull thing in noexcept/just variadic checks... (Compilers works bad with fold expressions)
	template<template<typename...> typename Check, typename ... Types>
	consteval bool all_in_pack() noexcept {
		return (Check<Types>::value && ...);
	}

	template<template<typename...> typename Check, typename ... Types>
	consteval bool atleast_one_in_pack() noexcept {
		return (Check<Types>::value || ...);
	}

	// TEMPLATE FUNCTION always_false FOR STATIC ASSERTS (for example)
	consteval inline bool always_false(auto) noexcept { return false; }
	template<typename T>
	consteval inline bool always_false() noexcept { return false; }
} // namespace kelbon

#endif // !KELBON_UTILITY_HPP


