
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <algorithm>
#include <tuple>

// действительно move, обнул€ет мувнутую пам€ть
inline void realmemmove(void* dest, void* srs, size_t count) {
	std::memcpy(dest, srs, count);
	std::memset(srs, 0, count);
}
class base_remember_destructor {
public:
	virtual void Destroy(void* ptr) const noexcept = 0;
};
// запоминает, если Condition == true и не запоминает, если условие не выполн€етс€
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
	template<size_t other_max_size>
	memory_block(memory_block<other_max_size>&& other) noexcept(enable_if_v<other_max_size <= max_size>) : destructor(other.destructor) {
		std::copy(other.data, other.data + other_max_size, data);
		other.destructor = nullptr;
	}
	// специфика tuple в том, что он складывает элементы в обратном пор€дке, так что если положить в этот класс <int,double>, а потом читать <T,X,int,double>, то всЄ будет хорошо,
	// а если читать как <int,double,X,T>, то всЄ плохо(значени€ int double уедут)
	template<typename ... Types>
	const std::tuple<Types...>& GetDataAs() const {
		static_assert(sizeof(std::tuple<Types...>) <= max_size);
		// bit_cast не подходит, потому что он не работает дл€ объектов с нетривиальными копи конструкторами
		// копировать или даже создавать std::tuple<Types...> нельз€, т.к. может не оказатьс€ конструкторов копировани€/дефолтных
		// остаЄтс€ лишь один вариант - возвращать ссылку на свои данные. » при этом запретить их мен€ть
		return *(reinterpret_cast<std::tuple<Types...>*>(const_cast<char*>(data)));
	}
	~memory_block() {
		if (destructor == nullptr) {
			return;
		}
		// т.к. у мен€ void*, а дл€ корретного выбора из vtable нужен указатель на базовый класс, то реинтерпретирую указатель на указатель...
		reinterpret_cast<base_remember_destructor*>(&destructor)->Destroy(data);
	}
};

#endif // !KELBON_MEMORY_BLOCK_HPP