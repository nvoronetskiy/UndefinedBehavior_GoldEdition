
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <exception>
#include <ranges>
#include <iostream>

#include "kelbon_tuple.hpp"

namespace kelbon {

	class bad_memory_block_access : public ::std::exception {
		using ::std::exception::exception;
	};

	class double_free_possible : public ::std::exception {
		using ::std::exception::exception;
	};

	struct base_remember_type_info {
		virtual constexpr void Destroy(void* ptr) const noexcept = 0;
		virtual constexpr void Move(void* srs, void* dest) const = 0;
		virtual constexpr void Copy(const void* srs, void* dest) const = 0;
		virtual constexpr inline bool is_trivially_destructible() const noexcept = 0;
		virtual constexpr inline bool is_move_constructible() const noexcept = 0;
		virtual constexpr inline bool is_copy_constructible() const noexcept = 0;
		virtual constexpr inline bool is_nothrow_copy_constructible() const noexcept = 0;
	};

	// false false case
	template<typename T>
	requires(std::destructible<T>)
	struct remember_type_info final
		: base_remember_type_info {

		constexpr void Destroy(void* ptr) const noexcept override {
			(reinterpret_cast<T*>(ptr))->~T();	
		}

		constexpr void Move(void* srs, void* dest) const noexcept(std::is_nothrow_move_constructible_v<T>) override {
			if constexpr (std::is_move_constructible_v<T>) {
				new (dest) T(std::move(*(reinterpret_cast<T*>(srs))));
			}
			else {
				// nothing
			}
		}

		constexpr void Copy(const void* srs, void* dest) const noexcept(std::is_nothrow_copy_constructible_v<T>) override {
			if constexpr (std::is_copy_constructible_v<T>) {
				new (dest) T(*(reinterpret_cast<const T*>(srs)));
			}
			else {
				// nothing
			}
		}

		[[nodiscard]] constexpr inline bool is_nothrow_copy_constructible() const noexcept override {
			return std::is_nothrow_copy_constructible_v<T>;
		}
		[[nodiscard]] constexpr inline bool is_trivially_destructible() const noexcept override {
			return std::is_trivially_destructible_v<T>;
		}
		[[nodiscard]] constexpr inline bool is_move_constructible() const noexcept override {
			return std::is_move_constructible_v<T>;
		}
		[[nodiscard]] constexpr inline bool is_copy_constructible() const noexcept override {
			return std::is_copy_constructible_v<T>;
		}
	};

	template<typename T>
	concept serializable_helper = std::is_fundamental_v<T> && !std::is_void_v<T>;

	template<typename T>
	concept serializable_range = std::ranges::range<T> &&
		(serializable_helper<typename T::value_type> || serializable_range<typename T::value_type>);

	template<typename T>
	concept serializable = serializable_helper<T> || serializable_range<T>;

	template<typename Traits, typename T>
	requires (serializable<T>&& serializable_helper<T>)
	std::basic_ostream<char, Traits>& WriteToStream(std::basic_ostream<char, Traits>& ostream, const T& value) {
		if constexpr (std::same_as<T, char>) {
			ostream << static_cast<int>(value) << '\n';
		}
		else {
			ostream << value << '\n';
		}
		return ostream;
	}
	
	template<typename Traits, typename T>
	requires serializable_range<T>
	std::basic_ostream<char, Traits>& WriteToStream(std::basic_ostream<char, Traits>& ostream, const T& range) {
		ostream << range.size() << '\n';
		for(const auto& value : range) {
			WriteToStream(ostream, value);
		}
		return ostream;
	}

	template<typename T, typename Traits>
	T ReadFromStream(std::basic_istream<char, Traits>& istream) {
		if constexpr (std::same_as<T, char>) {
			// for example if ' ' inserted into stream like ' ', then its ignored by reader, its error
			int value;
			istream >> value;
			return static_cast<char>(value);
		}
		else {
			T value;
			istream >> value;
			return value;
		}
	}

	template<typename T>
	concept have_resize = requires(T value) {
		value.resize(10);
	};

	template<typename T, typename Traits>
	requires (serializable_range<T>)
	T ReadFromStream(std::basic_istream<char, Traits>& istream) {
		T range;
		size_t range_size = 0;
		istream >> range_size;

		if constexpr (have_resize<T>) {
			range.resize(range_size);
		}

		for (auto& value : range) {
			value = ReadFromStream<typename T::value_type>(istream);
		}

		return range;
	}

	template<size_t max_size, template<typename...> typename TupleType = ::kelbon::tuple>
	class memory_block {
	private:
		char  data[max_size];
		void* memory; // vtable ptr

		constexpr void Clear() noexcept {
			if (memory == nullptr) [[unlikely]] {
				return;
			}
			// т.к. у меня void*, а для корретного выбора из vtable нужен указатель на базовый класс, то реинтерпретирую указатель на указатель...
			reinterpret_cast<base_remember_type_info*>(&memory)->Destroy(data);
			memory = nullptr;
		}
		constexpr const base_remember_type_info* const GetRTTI() const noexcept {
			return reinterpret_cast<const base_remember_type_info* const>(&memory);
		}

	public:
		constexpr memory_block() noexcept : data{ 0 }, memory(nullptr) {}

		// DO NOT THROW IF COPY CONSTUCTOR THROWING!!!
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		constexpr explicit memory_block(TupleType<Types...>&& value) noexcept : data{ 0 } {
			using Tuple = TupleType<Types...>;
			static_assert(std::is_copy_constructible_v<Tuple> || (std::is_move_constructible_v<Tuple> && std::is_nothrow_move_constructible_v<Tuple>),
				"Object is non copyble and non-movable, its useless to store it here");

			new(&memory) remember_type_info<Tuple>{};

			if constexpr (std::is_move_constructible_v<Tuple>) {
				GetRTTI()->Move(&value, data);
			}
			else { // here Tuple is copyable, because of static_assert here
				if constexpr (std::is_nothrow_copy_constructible_v<Tuple>) {
					GetRTTI()->Copy(&value, data);
				}
				else {
					try {
						GetRTTI()->Copy(&value, data);
					}
					catch (...) {
						// nothing to do, just because i want noexcept ctor
					}
				}
			}
		}

		template<typename ... Types>
		requires (sizeof(TupleType<std::remove_reference_t<Types>...>) <= max_size)
		constexpr memory_block(Types&& ... args)
			noexcept(std::is_nothrow_constructible_v<TupleType<std::remove_reference_t<Types>...>, Types...>)
			: memory_block(TupleType<std::remove_reference_t<Types>...>(std::forward<Types>(args)...))
		{}

		// DO NOT THROW IF COPY CONSTUCTOR THROWING!!!
		template<size_t other_max_size> requires(other_max_size <= max_size)
		constexpr memory_block(memory_block<other_max_size, TupleType>&& other) noexcept : memory(other.memory), data{ 0 } {
			// other is empty block
			if (other.memory == nullptr) [[unlikely]] {
				return;
			}
			const auto RTTI = other.GetRTTI();
			if (RTTI->is_move_constructible()) [[likely]] {
				RTTI->Move(other.data, data);
			}
			else { // here we know RTTI.copy constructible == true, because of check in other constructor(static_assert)
				try {
					RTTI->Copy(other.data, data);
				}
				catch (...) {
					// nothing, just for noexcept
				}
			}
		}

		// DO NOT THROW IF COPY CONSTUCTOR THROWING!!!
		template<size_t other_max_size> requires(other_max_size <= max_size)
		constexpr memory_block& operator=(memory_block<other_max_size, TupleType>&& other) noexcept {
			if (&other == this) [[unlikely]] {
				return *this;
			}
			Clear();
			memory = other.memory;
			// other is empty block
			if (other.memory == nullptr) [[unlikely]] {
				return *this;
			}
			const auto RTTI = other.GetRTTI();
			if (RTTI->is_move_constructible()) [[likely]] {
				RTTI->Move(other.data, data);
			}
			else { // here we know RTTI.copy constructible == true, because of check in other constructor(static_assert)
				try {
					RTTI->Copy(other.data, data);
				}
				catch (...) {
					// nothing, just for noexcept
				}
			}
			return *this;
		}

		// may throw double_free_possible if no avalible copy constructor for stored value
		[[nodiscard]] constexpr memory_block Clone() const {
			if (!CanBeCopied()) [[unlikely]] {
				throw double_free_possible("no copy constructor avalible for stored value (kelbon::memory_block::Clone)");
			}
			memory_block clone;
			clone.memory = memory;
			GetRTTI()->Copy(data, clone.data);
			return clone;
		}
		
		[[nodiscard]] constexpr inline bool IsTriviallyDestructibleNow() const noexcept {
			if (memory == nullptr) [[unlikely]] {
				return true;
			}
			return GetRTTI()->is_trivially_destructible();
		}
		[[nodiscard]] constexpr inline bool CanBeMoved() const noexcept {
			if (memory == nullptr) [[unlikely]] {
				return true;
			}
			return GetRTTI()->is_move_constructible();
		}
		// use it for check before using Clone method if you want to not catch exception
		[[nodiscard]] constexpr inline bool CanBeCopied() const noexcept {
			if (memory == nullptr) [[unlikely]] {
				return true;
			}
			return GetRTTI()->is_copy_constructible();
		}

		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] constexpr const TupleType<Types...>& GetDataAs() const noexcept {
			return *(reinterpret_cast<const TupleType<Types...>* const>(data));
		}
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] constexpr TupleType<Types...>& GetDataAs() noexcept {
			return *(reinterpret_cast<TupleType<Types...>*>(const_cast<char*>(data)));
		}
		// same as GetDataAs, but with checking if right types you trying to get, if not - exception thrown
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] constexpr const TupleType<Types...>& SafeGetDataAs() const {
			auto check_value = remember_type_info<TupleType<Types...>>{};

			if ((*(reinterpret_cast<void**>(&check_value))) != memory) {
				throw bad_memory_block_access(
					"Types you're trying to get do not match types stored here."
					"If that's exactly what you need, use GetDataAs method");
			}
			return GetDataAs<Types...>();
		}
		constexpr ~memory_block() {
			Clear();
		}

		// serialization/deserialization
		template<typename ... Types, typename Traits>
		requires(serializable<Types> && ...)
		std::basic_ostream<char, Traits>& WriteAs(std::basic_ostream<char, Traits>& ostream) const noexcept {
			return WriteAs_helper<Types...>(ostream, make_value_list<size_t, sizeof...(Types)>{});
		}
		template<typename ... Types, typename Traits>
		requires(serializable<Types> && ...)
		std::basic_istream<char, Traits>& ReadAs(std::basic_istream<char, Traits>& istream) noexcept {
			TupleType<Types...> tpl;
			ReadAs_helper<Types...>(istream, tpl, make_value_list<size_t, sizeof...(Types)>{});
			*this = memory_block(std::move(tpl));
			return istream;
		}
	private:
		template<typename ... Types, typename Traits, size_t ... Indexes>
		std::basic_ostream<char, Traits>& WriteAs_helper(std::basic_ostream<char, Traits>& ostream, value_list<size_t, Indexes...>) const noexcept {
			const auto& tuple_to_write = GetDataAs<Types...>();
			((WriteToStream(ostream, std::get<Indexes>(tuple_to_write))), ...);
			return ostream;
		}
		template<typename ... Types, typename Traits, size_t  ... Indexes>
		void ReadAs_helper(std::basic_istream<char, Traits>& istream, TupleType<Types...>& tpl, value_list<size_t, Indexes...>) noexcept {
			((tpl.template get<Indexes>() = ReadFromStream<Types>(istream)), ...);
		}
	};

	memory_block()->memory_block<55>;

	template<template<typename...> typename TupleType, typename ... Types>
	memory_block(TupleType<Types...>&&)->memory_block<sizeof_pack<TupleType<Types...>>(), TupleType>;

	template<typename ... Types>
	memory_block(Types&& ...)->memory_block<sizeof_pack<::kelbon::tuple<std::remove_reference_t<Types>...>>(), ::kelbon::tuple>;

} // namespace kelbon

#endif // !KELBON_MEMORY_BLOCK_HPP