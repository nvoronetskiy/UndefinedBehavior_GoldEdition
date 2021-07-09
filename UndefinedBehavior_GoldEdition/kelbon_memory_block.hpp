
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <algorithm>
#include <tuple>

// ������������� move, �������� �������� ������
inline void realmemmove(void* dest, void* srs, size_t count) {
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

// ��������� ������ �� �������� � memory_block<max_size>(����� - memory_block) ��: 
// 1. ���������, ��� ���������� �������� ������ ������ ������������, ��� ��������� ����������
// 2. �������������, ��� � ������������ ����� ������������ �������� ��� ��������� �� �������/������������� ���������� ���������� ������ ������, ��� ���������� ��������� ����������
// 3. ������������, ��� ������ ��� �� �������� � �������� ����� �� �������� � ������������� �� �������� �� ����������� �������(memory_block)
template<size_t max_size>
class memory_block {
private:
	char  data[max_size]; // ������ ��� �������� ����� ������� ������
	void* destructor;     // �� ��� ����� � ������-������������ ����������� - ��������� �� ������� ����������� �������
public:
	template<typename ... Types>
	explicit memory_block(std::tuple<Types...>&& value) {
		static_assert(sizeof(std::tuple<Types...>) <= max_size);

		realmemmove(data, &value, sizeof(std::tuple<Types...>));
		// �� ���� ����� ���������� ����������� �����������, ����� � �������� ��������� � ����������� �����(�.�. �� ������� �� ����� ������ ��������� �� vtable)
		new(&destructor) remember_destructor<std::tuple<Types...>, !std::is_trivially_destructible_v<std::tuple<Types...>>>{};
	}
	template<size_t other_max_size>
	memory_block(memory_block<other_max_size>&& other) noexcept(enable_if_v<other_max_size <= max_size>) : destructor(other.destructor) {
		std::copy(other.data, other.data + other_max_size, data);
		other.destructor = nullptr;
	}
	// ��������� tuple � ���, ��� �� ���������� �������� � �������� �������, ��� ��� ���� �������� � ���� ����� <int,double>, � ����� ������ <T,X,int,double>, �� �� ����� ������,
	// � ���� ������ ��� <int,double,X,T>, �� �� �����(�������� int double �����)
	template<typename ... Types>
	const std::tuple<Types...>& GetDataAs() const {
		static_assert(sizeof(std::tuple<Types...>) <= max_size);
		// bit_cast �� ��������, ������ ��� �� �� �������� ��� �������� � �������������� ���� ��������������
		// ���������� ��� ���� ��������� std::tuple<Types...> ������, �.�. ����� �� ��������� ������������� �����������/���������
		// ������� ���� ���� ������� - ���������� ������ �� ���� ������. � ��� ���� ��������� �� ������
		return *(reinterpret_cast<std::tuple<Types...>*>(const_cast<char*>(data)));
	}
	~memory_block() {
		if (destructor == nullptr) {
			return;
		}
		// �.�. � ���� void*, � ��� ���������� ������ �� vtable ����� ��������� �� ������� �����, �� ��������������� ��������� �� ���������...
		reinterpret_cast<base_remember_destructor*>(&destructor)->Destroy(data);
	}
};

#endif // !KELBON_MEMORY_BLOCK_HPP