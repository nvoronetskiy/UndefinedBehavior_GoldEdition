
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <algorithm>

namespace kelbon {

	// ������������� move, �������� �������� ������
	inline void realmemmove(void* dest, void* srs, size_t count) noexcept {
		std::memcpy(dest, srs, count);
		std::memset(srs, 0, count);
	}
	class base_remember_destructor {
	public:
		virtual void Destroy(void* ptr) const noexcept = 0;
	};
	// ����������, ���� Condition == true � �� ����������, ���� ������� �� �����������
	template<typename T, bool Condition>
	class remember_destructor : public base_remember_destructor {
		void Destroy(void* ptr) const noexcept override { }
	};
	template<typename T>
	class remember_destructor<T, true> : public base_remember_destructor {
		void Destroy(void* ptr) const noexcept override {
			(reinterpret_cast<T*>(ptr))->~T();
		}
	};

	class bad_memory_block_access : public ::std::exception {
	public:
		bad_memory_block_access(const char* message) : ::std::exception(message) {}
	};

	template<typename...>
	class tuple;
	// ��������� ������ �� �������� � memory_block<max_size>(����� - memory_block) ��: 
	// 1. ���������, ��� ���������� �������� ������ ������ ������������, ��� ��������� ����������
	// 2. �������������, ��� � ������������ ����� ������������ �������� ��� ��������� �� �������/������������� ���������� ���������� ������ ������, ��� ���������� ��������� ����������
	// 3. ������������, ��� ������ ��� �� �������� � �������� ����� �� �������� � ������������� �� �������� �� ����������� �������(memory_block)
	template<size_t max_size, template<typename...> typename TupleType = tuple>
	class memory_block {
	private:
		char  data[max_size]; // ������ ��� �������� ����� ������� ������
		void* destructor;     // �� ��� ����� � ������-������������ ����������� - ��������� �� ������� ����������� �������

		void Clear() noexcept {
			if (destructor == nullptr) {
				return;
			}
			// �.�. � ���� void*, � ��� ���������� ������ �� vtable ����� ��������� �� ������� �����, �� ��������������� ��������� �� ���������...
			reinterpret_cast<base_remember_destructor*>(&destructor)->Destroy(data);
		}
	public:
		template<typename ... Types>
		memory_block(TupleType<Types...>&& value) noexcept {
			static_assert(sizeof(TupleType<Types...>) <= max_size);

			realmemmove(data, &value, sizeof(TupleType<Types...>));
			// �� ���� ����� ���������� ����������� �����������, ����� � �������� ��������� � ����������� �����(�.�. �� ������� �� ����� ������ ��������� �� vtable)
			new(&destructor) remember_destructor<TupleType<Types...>, !std::is_trivially_destructible_v<TupleType<Types...>>>{};
		}
		template<typename ... Types>
		memory_block(Types&& ... args) : memory_block(TupleType<std::remove_reference_t<Types>...>(std::forward<Types>(args)...)) {}

		template<size_t other_max_size>
		memory_block(memory_block<other_max_size>&& other) noexcept : destructor(other.destructor) {
			static_assert(other_max_size <= max_size);

			std::copy(other.data, other.data + other_max_size, data);
			other.destructor = nullptr;
		}
		template<size_t other_max_size>
		memory_block& operator=(memory_block<other_max_size, TupleType>&& other) noexcept {
			Clear();
			destructor = other.destructor;
			std::copy(other.data, other.data + other_max_size, data);
			other.destructor = nullptr;
		}
		
		// ��������� tuple � ���, ��� �� ���������� �������� � �������� �������, ��� ��� ���� �������� � ���� ����� <int,double>, � ����� ������ <T,X,int,double>, �� �� ����� ������,
		// � ���� ������ ��� <int,double,X,T>, �� �� �����(�������� int double �����)
		template<typename ... Types>
		[[nodiscard]] const TupleType<Types...>& GetDataAs() const noexcept {
			static_assert(sizeof(TupleType<Types...>) <= max_size);
			// bit_cast �� ��������, ������ ��� �� �� �������� ��� �������� � �������������� ���� ��������������
			// ���������� ��� ���� ��������� std::tuple<Types...> ������, �.�. ����� �� ��������� ������������� �����������/���������
			// ������� ���� ���� ������� - ���������� ������ �� ���� ������. � ��� ���� ��������� �� ������
			return *(reinterpret_cast<TupleType<Types...>*>(const_cast<char*>(data)));
		}
		// ����� �� GetDataAs, ������ � ��������� �� ������������ ������(�� ���� ���� ����� �� ��, ��� ��������, �� �������� ����������)
		template<typename ... Types>
		[[nodiscard]] const TupleType<Types...>& SafeGetDataAs() const {
			static_assert(sizeof(TupleType<Types...>) <= max_size);
			auto check_value = remember_destructor<TupleType<Types...>, !std::is_trivially_destructible_v<TupleType<Types...>>>{};
			if ((*(reinterpret_cast<void**>(&check_value))) != destructor) {
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
	memory_block(TupleType<Types...>&&)->memory_block<sizeof(TupleType<Types..., void*>), TupleType>;
	// TODO - check �� ����������� �� ������ ���� ������
	template<typename ... Types>
	memory_block(Types&& ...)->memory_block<sizeof(::kelbon::tuple<std::remove_reference_t<Types>..., void*>), ::kelbon::tuple>;

} // namespace kelbon

#endif // !KELBON_MEMORY_BLOCK_HPP