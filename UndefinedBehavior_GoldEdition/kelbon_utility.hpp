
#ifndef KELBON_UTILITY_HPP
#define KELBON_UTILITY_HPP

namespace kelbon {

	// TEMPLATE FUNCTION sizeof_pack
	template<typename ... Types>
	consteval size_t sizeof_pack() noexcept {
		return (sizeof(Types) + ...);
	}

	// TEMPLATE FUNCTION always_false FOR STATIC ASSERTS (for example)
	consteval inline bool always_false(auto) noexcept { return false; }

} // namespace kelbon

#endif // !KELBON_UTILITY_HPP


