
#include <iostream>
#include <algorithm>
#include <tuple>
#include <string>

#if _DEBUG
#define on_debug(expression) expression
#else
#define on_debug(expression)
#endif
// действительно move, обнул€ет мувнутую пам€ть
inline void realmemmove(void* dest, void* srs, size_t count) {
	std::memcpy(dest, srs, count);
	std::memset(srs, 0, count);
}
class base_remember_destructor {
public:
	virtual void Destroy(void* ptr) const = 0;
};
// запоминает, если Condition == true и не запоминает, если условие не выполн€етс€
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

// ѕередава€ данные на хранение в memory_block<max_size>(далее - memory_block) ¬ы: 
// 1. ѕонимаете, что переданное значение больше нельз€ использовать, оно побайтово обнул€етс€
// 2. ѕодтверждаете, что в деструкторах типов передаваемых значений нет переходов по ссылкам/разыменовани€ указателей €вл€ющихс€ пол€ми класса, нет арифметики удал€емых указателей
// 3. —оглашаетесь, что больше ими не владеете и передаЄте право на владение и об€зательство на удаление на принимающую сторону(memory_block)
template<size_t max_size>
class memory_block {
private:
	char  data[max_size]; // пам€ть под хранение любых входных данных
	void* destructor;     // всЄ что лежит в классе-запоминателе деструктора - указатель на таблицу виртуальных функций
public:
	template<typename ... Types>
	explicit memory_block(std::tuple<Types...>&& value) {
		static_assert(sizeof(std::tuple<Types...>) <= max_size);

		realmemmove(data, &value, sizeof(std::tuple<Types...>));
		// по сути здесь происходит запоминание деструктора, пр€мо в значении указател€ € конструирую класс(т.к. он состоит из всего одного указател€ на vtable)
		new(&destructor) remember_destructor<std::tuple<Types...>, !std::is_trivially_destructible_v<std::tuple<Types...>>>{};
	}
	memory_block(memory_block&& other) noexcept : destructor(other.destructor) {
		std::copy(other.data, other.data + max_size, data);
		other.destructor = nullptr;
	}
	template<typename ... Types>
	const std::tuple<Types...>& GetDataAs() const {
		static_assert(sizeof(std::tuple<Types...>) <= max_size);
		// bit_cast не подходит, потому что он не работает дл€ объектов с нетривиальными копи конструкторами
		// копировать или даже создавать std::tuple<Types...> нельз€, т.к. может не оказатьс€ конструкторов копировани€/дефолтных
		// остаЄтс€ лишь один вариант - возвращать ссылку на свои данные. » при этом запретить их мен€ть
		return *(reinterpret_cast<std::tuple<Types...>*>(const_cast<char*>(data)));
	}
	~memory_block() {
		// т.к. у мен€ void*, а дл€ корретного выбора из vtable нужен указатель на базовый класс, то реинтерпретирую указатель на указатель...
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

	memory_block<126> value(std::tuple("хех"s, fval, ival));
	auto& [str, fv, iv] = value.GetDataAs<int, float, int>();
	std::cout << str << '\t' << fv << '\t' << iv << std::endl;
	return 0;
}