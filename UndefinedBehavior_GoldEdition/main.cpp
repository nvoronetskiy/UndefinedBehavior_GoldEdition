
#include <iostream>
#include <tuple>
#include <memory>
#include <string>

#if _DEBUG
#define on_debug(expression) expression
#else
#define on_debug(expression)
#endif
// ������������� move, �������� �������� ������ ������
inline void realmemmove(void* dest, void* srs, size_t count) {
	std::memcpy(dest, srs, count);
	std::memset(srs, 0, count);
}

class remember_destructor_helper {
public:
	virtual void Destroy(void* ptr) = 0;
};
template<typename T>
class remember_destructor : public remember_destructor_helper {
	void Destroy(void* ptr) override {
		(reinterpret_cast<T*>(ptr))->~T();
	}
};

// contains one unit of any struct/class ��� ������������ �������������( ������� ������, ���� ������. ��! �������� ���������� � ����� ������ ���������)
template<size_t Max_size>
class any_data {
private:
	char data[Max_size];
	void* destructor; // [sizeof(void*)] ; // lol, �� ��� ����� � ������ - ��������� �� ������� ����������� �������
	bool trivially_destructible;    // �� ����� ���� �������� �� ����������, �.�. � ������� ��� ��� ��������������� � ����
public:
	// ��������� �����������, ����� ��������� ������������ ������ ������� � �������� ������������� � �.�.
	// 1. �������� �������� ������ �� rvalue, ����� ���������� ������������ �� ������� ���� �����
	// 2. ������� �������� ���� � ����� � ����� ������� �� � value, ������������ ������� �������� � �������� ����,
	// ��� ���� ������ ��������������, ��� � ����������� ��� ������ ������� delete ptr, �� ���� ��� delete ptr+1, ��������
	// ��� ������� �� � �������� �� ����� ������ � ������� ���������
	template<typename ... Types>
	any_data(std::tuple<Types...>&& value) : on_debug(data{}) {
		static_assert(sizeof(std::tuple<Types...>) <= Max_size);

		realmemmove(&data[0], &value, sizeof(std::tuple<Types...>));
		if constexpr (std::is_trivially_destructible_v<std::tuple<Types...>>) {
			trivially_destructible = true;
		}
		else {
			trivially_destructible = false;
			// �� ���� ����� ���������� ����������� �����������, ����� � �������� ��������� � ���� �����(�.�. �� ������� �� ����� ������ ��������� �� vtable)
			new(&destructor) remember_destructor<std::tuple<Types...>>{};
		}
	}

	// todo - move ����������� ��� ���� �������, ���� �������. � ������ ����������� ������, �������� �����.

	// ������� ���������� �� ��������, ����� ������ �� ��� ��� ����, ��� ����������� ������ ������������ ���� ��� � ����.
	// ������ const_cast ���������� ������ ������ �������� ���� ������
	template<typename ... Types>
	const std::tuple<Types...>& GetDataAs() const {
		static_assert(sizeof(std::tuple<Types...>) <= Max_size);
		// bit_cast �� ��������, ������ ��� �� �� �������� ��� �������� � �������������� ���� ��������������
		// ���������� ��� ���� ��������� std::tuple<Types...> ������, �.�. ����� �� ��������� ������������� �����������/���������
		// ������� ���� ���� ��� - ���������� ������ �� ���� ������. � ��� ���� ��������� �� ������
		return *(reinterpret_cast<std::tuple<Types...>*>(const_cast<char*>(&data[0])));
	}

	~any_data() {
		if (trivially_destructible) {
			return;
		} // not trivially destructible
		// �.�. � ���� void*, � ��� ���������� ������ �� vtable ����� ��������� �� ������� �����, �� ��������������� ��������� �� ���������...
		reinterpret_cast<remember_destructor_helper*>(&destructor)->Destroy(&data[0]);
	}
};

class testclass {
private:
	std::string s;
public:
	testclass(std::string&& s) : s(s) {}
	~testclass() {
		std::cout << " im here" << std::endl;
	}
};

int main() {
	int ival = 5;
	float fval = 3.14f;

	any_data<48> value(std::tuple<testclass, int>{ std::string("4324"), fval});
	auto& [a, b] = value.GetDataAs<std::string, int>();
	std::cout << a << '\t' << b;
}