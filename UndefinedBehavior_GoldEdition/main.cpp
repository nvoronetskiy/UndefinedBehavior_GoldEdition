
#include <iostream>
#include <tuple>
#include <memory>
#include <string>

#if _DEBUG
#define on_debug(expression) expression
#else
#define on_debug(expression)
#endif
// действительно move, обнуляет мувнутую память памяти
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

// contains one unit of any struct/class ДЛЯ ОДНОРАЗОВОГО ИСПОЛЬЗОВАНИЯ( положил данные, взял данные. Всё! Вызывает деструктор в конце своего лайфтайма)
template<size_t Max_size>
class any_data {
private:
	char data[Max_size];
	void* destructor; // [sizeof(void*)] ; // lol, всё что лежит в классе - указатель на таблицу виртуальных функций
	bool trivially_destructible;    // не может быть известно на компиляции, т.к. я вызываю это уже непосредственно в коде
public:
	// некоторые ограничения, чтобы корректно обрабатывать всякие объекты с сложными деструкторами и т.д.
	// 1. Принимаю значение только по rvalue, чтобы ограничить пользователя от ломания себе всего
	// 2. Копирую значение себе в буфер и потом обнуляю всё в value, предупреждая двойное удаление и подобные вещи,
	// при этом неявно предполагается, что в деструкторе нет ничего сложнее delete ptr, то есть нет delete ptr+1, например
	// что привело бы к удалению не своей памяти и падению программы
	template<typename ... Types>
	any_data(std::tuple<Types...>&& value) : on_debug(data{}) {
		static_assert(sizeof(std::tuple<Types...>) <= Max_size);

		realmemmove(&data[0], &value, sizeof(std::tuple<Types...>));
		if constexpr (std::is_trivially_destructible_v<std::tuple<Types...>>) {
			trivially_destructible = true;
		}
		else {
			trivially_destructible = false;
			// по сути здесь происходит запоминание деструктора, прямо в значение указателя я пишу класс(т.к. он состоит из всего одного указателя на vtable)
			new(&destructor) remember_destructor<std::tuple<Types...>>{};
		}
	}

	// todo - move конструктор для этой ебанины, копи удалить. И только конструктор кстати, оператор нахуй.

	// получаю информацию по значению, потом удаляю всё что тут было, для корректного вызова деструкторов один раз и проч.
	// короче const_cast полученной отсюда ссылки аукнется тебе больно
	template<typename ... Types>
	const std::tuple<Types...>& GetDataAs() const {
		static_assert(sizeof(std::tuple<Types...>) <= Max_size);
		// bit_cast не подходит, потому что он не работает для объектов с нетривиальными копи конструкторами
		// копировать или даже создавать std::tuple<Types...> нельзя, т.к. может не оказаться конструкторов копирования/дефолтных
		// остаётся лишь один тип - возвращать ссылку на свои данные. И при этом запретить их менять
		return *(reinterpret_cast<std::tuple<Types...>*>(const_cast<char*>(&data[0])));
	}

	~any_data() {
		if (trivially_destructible) {
			return;
		} // not trivially destructible
		// т.к. у меня void*, а для корретного выбора из vtable нужен указатель на базовый класс, то реинтерпретирую указатель на указатель...
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