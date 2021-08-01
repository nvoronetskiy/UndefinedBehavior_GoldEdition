
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <algorithm> // std::copy, memcpy, memset + traits

#include "kelbon_tuple.hpp"

namespace kelbon {

	class bad_memory_block_access : public ::std::exception {
	public:
		bad_memory_block_access(const char* message) : ::std::exception(message) {}
	};

	class double_free_possible : public ::std::exception {
		using ::std::exception::exception;
	};

	// ������������� move, �������� �������� ������
	inline void realmemmove(void* dest, void* srs, size_t count) noexcept {
		if (dest == srs) {
			return;
		}
		std::memcpy(dest, srs, count);
		std::memset(srs, 0, count);
	}

	struct base_remember_type_info {
		virtual void Destroy(void* ptr) const noexcept = 0;
		virtual bool is_trivially_destructible() const noexcept = 0;
		virtual void Copy(const void* srs, void* dest) const = 0;
		virtual bool is_copy_constructible() const noexcept = 0;
	};

	// false false case
	template<typename T, bool IsNotTriviallyDestructible, bool IsCopyConstructible>
	struct remember_type_info final
		: base_remember_type_info {

		void Destroy(void* ptr) const noexcept override {
			// nothing
		}
		bool is_trivially_destructible() const noexcept override {
			return true;
		}
		void Copy(const void* srs, void* dest) const noexcept override  {
			// nothing
		}
		bool is_copy_constructible() const noexcept override {
			return false;
		}
	};
	// true false case
	template<typename T>
	struct remember_type_info<T, true, false> final
		: base_remember_type_info {

		void Destroy(void* ptr) const noexcept override {
			(reinterpret_cast<T*>(ptr))->~T();
		}
		bool is_trivially_destructible() const noexcept override {
			return false;
		}
		void Copy(const void* srs, void* dest) const noexcept override {
			// nothing
		}
		bool is_copy_constructible() const noexcept override {
			return false;
		}
	};

	// true true case
	template<typename T>
	struct remember_type_info<T, true, true> final
		: base_remember_type_info {

		void Destroy(void* ptr) const noexcept(std::is_nothrow_destructible_v<T>) override {
			(reinterpret_cast<T*>(ptr))->~T();
		}
		bool is_trivially_destructible() const noexcept override {
			return false;
		}
		void Copy(const void* srs, void* dest) const noexcept(std::is_nothrow_copy_constructible_v<T>) override {
			new (dest) T(*(reinterpret_cast<const T*>(srs)));
		}
		bool is_copy_constructible() const noexcept override {
			return true;
		}
	};

	// false true case
	template<typename T>
	class remember_type_info<T, false, true> final
		: base_remember_type_info {

		void Destroy(void* ptr) const noexcept override {
			// nothing
		}
		bool is_trivially_destructible() const noexcept override {
			return true;
		}
		void Copy(const void* srs, void* dest) const noexcept(std::is_nothrow_copy_constructible_v<T>) override {
			new (dest) T(*(reinterpret_cast<const T*>(srs)));
		}
		bool is_copy_constructible() const noexcept override {
			return true;
		}
	};

	// ��������� ������ �� �������� � memory_block<max_size>(����� - memory_block) ��: 
	// 1. ���������, ��� ���������� �������� ������ ������ ������������, ��� ��������� ����������
	// 2. �������������, ��� � ������������ ����� ������������ �������� ��� ��������� �� �������/������������� ���������� ���������� ������ ������, ��� ���������� ��������� ����������
	// 3. ������������, ��� ������ ��� �� �������� � �������� ����� �� �������� � ������������� �� �������� �� ����������� �������(memory_block)
	template<size_t max_size, template<typename...> typename TupleType = ::kelbon::tuple>
	class memory_block {
	private:
		char  data[max_size]; // ������ ��� �������� ����� ������� ������
		void* memory;     // �� ��� ����� � ������-������������ ����������� - ��������� �� ������� ����������� �������

		void Clear() noexcept {
			if (memory == nullptr) {
				return;
			}
			// �.�. � ���� void*, � ��� ���������� ������ �� vtable ����� ��������� �� ������� �����, �� ��������������� ��������� �� ���������...
			reinterpret_cast<base_remember_type_info*>(&memory)->Destroy(data);
		}
		const base_remember_type_info* const GetRTTI() const noexcept {
			return reinterpret_cast<const base_remember_type_info* const>(&memory);
		}
	public:
		constexpr memory_block() noexcept : data{ 0 }, memory(nullptr) {}

		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
			memory_block(TupleType<Types...>&& value) noexcept {
			realmemmove(data, &value, sizeof(TupleType<Types...>));
			// �� ���� ����� ���������� ����������� �����������, ����� � �������� ��������� � ����������� �����(�.�. �� ������� �� ����� ������ ��������� �� vtable)
			new(&memory)
				remember_type_info<TupleType<Types...>,
				!std::is_trivially_destructible_v<TupleType<Types...>>,
				std::is_copy_constructible_v<TupleType<Types...>>>{};
		}

		template<typename ... Types>
		requires (sizeof(TupleType<std::remove_reference_t<Types>...>) <= max_size)
			memory_block(Types&& ... args)
			noexcept(std::is_nothrow_constructible_v<TupleType<std::remove_reference_t<Types>...>, Types...>)
			: memory_block(TupleType<std::remove_reference_t<Types>...>(std::forward<Types>(args)...))
		{}

		template<size_t other_max_size> requires(other_max_size <= max_size)
			memory_block(memory_block<other_max_size, TupleType>&& other) noexcept : memory(other.memory) {
			std::copy(other.data, other.data + other_max_size, data);
			other.memory = nullptr;
		}
		template<size_t other_max_size> requires(other_max_size <= max_size)
		memory_block& operator=(memory_block<other_max_size, TupleType>&& other) noexcept {
			if (&other == this) {
				return *this;
			}
			Clear();
			memory = other.memory;
			std::copy(other.data, other.data + other_max_size, data);
			other.memory = nullptr;
			return *this;
		}

		// may throw double_free_possible if no avalible copy constructor for stored value
		memory_block Clone() const {
			if (!GetRTTI()->is_copy_constructible()) {
				throw double_free_possible("no copy constructor avalible for stored value (kelbon::memory_block::Clone)");
			}
			memory_block clone;
			clone.memory = memory;
			GetRTTI()->Copy(data, clone.data);
			return clone;
		}
		
		bool IsTriviallyDestructibleStored() const noexcept {
			if (memory == nullptr) {
				return true;
			}
			return GetRTTI()->is_trivially_destructible();
		}
		// ��������, � ���� ��������� ����� �� ������� ������ ������������, ����� �� �������� ������ ����������
		bool IsCopybleStored() const noexcept {
			if (memory == nullptr) {
				return false;
			}
			return GetRTTI()->is_copy_constructible();
		}
		// ��������� tuple � ���, ��� �� ���������� �������� � �������� �������, ��� ��� ���� �������� � ���� ����� <int,double>, � ����� ������ <T,X,int,double>, �� �� ����� ������,
		// � ���� ������ ��� <int,double,X,T>, �� �� �����(�������� int double �����)
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] const TupleType<Types...>& GetDataAs() const noexcept {
			// bit_cast �� ��������, ������ ��� �� �� �������� ��� �������� � �������������� ���� ��������������
			// ���������� ��� ���� ��������� std::tuple<Types...> ������, �.�. ����� �� ��������� ������������� �����������/���������
			// ������� ���� ���� ������� - ���������� ������ �� ���� ������. � ��� ���� ��������� �� ������
			return *(reinterpret_cast<const TupleType<Types...>* const>(data));
		}
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] TupleType<Types...>& GetDataAs() noexcept {
			return *(reinterpret_cast<TupleType<Types...>*>(const_cast<char*>(data)));
		}
		// ����� �� GetDataAs, ������ � ��������� �� ������������ ������(�� ���� ���� ����� �� ��, ��� ��������, �� �������� ����������)
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] const TupleType<Types...>& SafeGetDataAs() const {
			auto check_value =
				remember_type_info<TupleType<Types...>,
				!std::is_trivially_destructible_v<TupleType<Types...>>,
				std::is_copy_constructible_v<TupleType<Types...>>>{};

			if ((*(reinterpret_cast<void**>(&check_value))) != memory) {
				throw bad_memory_block_access(
					"Types you're trying to get do not match types stored here."
					"If that's exactly what you need, use GetDataAs method");
			}
			// bit_cast �� ��������, ������ ��� �� �� �������� ��� �������� � �������������� ���� ��������������
			// ���������� ��� ���� ��������� std::tuple<Types...> ������, �.�. ����� �� ��������� ������������� �����������/���������
			// ������� ���� ���� ������� - ���������� ������ �� ���� ������. � ��� ���� ��������� �� ������
			return GetDataAs<Types...>();
		}
		~memory_block() {
			Clear();
		}
	};

	template<template<typename...> typename TupleType, typename ... Types>
	memory_block(TupleType<Types...>&&)->memory_block<sizeof(TupleType<Types...>), TupleType>;
	// TODO - check �� ����������� �� ������ ���� ������
	template<typename ... Types>
	memory_block(Types&& ...)->memory_block<sizeof(::kelbon::tuple<std::remove_reference_t<Types>...>), ::kelbon::tuple>;

} // namespace kelbon

#endif // !KELBON_MEMORY_BLOCK_HPP