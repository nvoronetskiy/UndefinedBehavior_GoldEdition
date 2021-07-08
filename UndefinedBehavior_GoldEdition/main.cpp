
#include <iostream>
#include <algorithm>
#include <tuple>
#include <string>

#if _DEBUG
#define on_debug(expression) expression
#else
#define on_debug(expression)
#endif
// ������������� move, �������� �������� ������
inline void realmemmove(void* dest, void* srs, size_t count) {
	std::memcpy(dest, srs, count);
	std::memset(srs, 0, count);
}
class base_remember_destructor {
public:
	virtual void Destroy(void* ptr) const = 0;
};
// ����������, ���� Condition == true � �� ����������, ���� ������� �� �����������
template<typename T, bool Condition>
class remember_destructor : public base_remember_destructor {
	void Destroy(void* ptr) const override { std::cout << "TRIVIAL"; }
};
template<typename T>
class remember_destructor<T, true> : public base_remember_destructor {
	void Destroy(void* ptr) const override {
		std::cout << "NONTRIVIAL";
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
	memory_block(memory_block&& other) noexcept : destructor(other.destructor) {
		std::copy(other.data, other.data + max_size, data);
		other.destructor = nullptr;
	}
	template<typename ... Types>
	const std::tuple<Types...>& GetDataAs() const {
		static_assert(sizeof(std::tuple<Types...>) <= max_size);
		// bit_cast �� ��������, ������ ��� �� �� �������� ��� �������� � �������������� ���� ��������������
		// ���������� ��� ���� ��������� std::tuple<Types...> ������, �.�. ����� �� ��������� ������������� �����������/���������
		// ������� ���� ���� ������� - ���������� ������ �� ���� ������. � ��� ���� ��������� �� ������
		return *(reinterpret_cast<std::tuple<Types...>*>(const_cast<char*>(data)));
	}
	~memory_block() {
		// �.�. � ���� void*, � ��� ���������� ������ �� vtable ����� ��������� �� ������� �����, �� ��������������� ��������� �� ���������...
		reinterpret_cast<base_remember_destructor*>(&destructor)->Destroy(data);
	}
};

class testclass {
private:
	std::string s;
public:
	testclass(const testclass& other) : s(other.s) {}
	testclass(const std::string& s) : s(s) {}
	testclass(std::string&& s) : s(s) {}
	testclass(testclass&& other) noexcept : s(std::move(other.s)) {}
	~testclass() {
		std::cout << s << " im here" << std::endl;
	}
};

int main() {
	int ival = 5;
	float fval = 3.14f;
	std::string s = "All right";
	using namespace std::literals;

	memory_block<126> value(std::tuple("���"s, fval, ival));
	auto& [str, fv, iv] = value.GetDataAs<int, float, int>();
	std::cout << str << '\t' << fv << '\t' << iv << std::endl;
	return 0;
}