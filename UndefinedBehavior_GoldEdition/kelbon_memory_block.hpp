
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <algorithm>

namespace kelbon {

	// действительно move, обнуляет мувнутую память
	inline void realmemmove(void* dest, void* srs, size_t count) noexcept {
		std::memcpy(dest, srs, count);
		std::memset(srs, 0, count);
	}
	class base_remember_destructor {
	public:
		virtual void Destroy(void* ptr) const noexcept = 0;
	};
	// запоминает, если Condition == true и не запоминает, если условие не выполняется
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
	// Передавая данные на хранение в memory_block<max_size>(далее - memory_block) Вы: 
	// 1. Понимаете, что переданное значение больше нельзя использовать, оно побайтово обнуляется
	// 2. Подтверждаете, что в деструкторах типов передаваемых значений нет переходов по ссылкам/разыменования указателей являющихся полями класса, нет арифметики удаляемых указателей
	// 3. Соглашаетесь, что больше ими не владеете и передаёте право на владение и обязательство на удаление на принимающую сторону(memory_block)
	template<size_t max_size, template<typename...> typename TupleType = tuple>
	class memory_block {
	private:
		char  data[max_size]; // память под хранение любых входных данных
		void* destructor;     // всё что лежит в классе-запоминателе деструктора - указатель на таблицу виртуальных функций

		void Clear() noexcept {
			if (destructor == nullptr) {
				return;
			}
			// т.к. у меня void*, а для корретного выбора из vtable нужен указатель на базовый класс, то реинтерпретирую указатель на указатель...
			reinterpret_cast<base_remember_destructor*>(&destructor)->Destroy(data);
		}
	public:
		template<typename ... Types>
		memory_block(TupleType<Types...>&& value) noexcept {
			static_assert(sizeof(TupleType<Types...>) <= max_size);

			realmemmove(data, &value, sizeof(TupleType<Types...>));
			// по сути здесь происходит запоминание деструктора, прямо в значении указателя я конструирую класс(т.к. он состоит из всего одного указателя на vtable)
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
		
		// специфика tuple в том, что он складывает элементы в обратном порядке, так что если положить в этот класс <int,double>, а потом читать <T,X,int,double>, то всё будет хорошо,
		// а если читать как <int,double,X,T>, то всё плохо(значения int double уедут)
		template<typename ... Types>
		[[nodiscard]] const TupleType<Types...>& GetDataAs() const noexcept {
			static_assert(sizeof(TupleType<Types...>) <= max_size);
			// bit_cast не подходит, потому что он не работает для объектов с нетривиальными копи конструкторами
			// копировать или даже создавать std::tuple<Types...> нельзя, т.к. может не оказаться конструкторов копирования/дефолтных
			// остаётся лишь один вариант - возвращать ссылку на свои данные. И при этом запретить их менять
			return *(reinterpret_cast<TupleType<Types...>*>(const_cast<char*>(data)));
		}
		// такая же GetDataAs, только с проверкой на правильность взятия(то есть если взять не то, что положили, то вылетает исключение)
		template<typename ... Types>
		[[nodiscard]] const TupleType<Types...>& SafeGetDataAs() const {
			static_assert(sizeof(TupleType<Types...>) <= max_size);
			auto check_value = remember_destructor<TupleType<Types...>, !std::is_trivially_destructible_v<TupleType<Types...>>>{};
			if ((*(reinterpret_cast<void**>(&check_value))) != destructor) {
				throw bad_memory_block_access(
					"Types you're trying to get do not match types stored here."
					"If that's exactly what you need, use GetDataAs method");
			}
			// bit_cast не подходит, потому что он не работает для объектов с нетривиальными копи конструкторами
			// копировать или даже создавать std::tuple<Types...> нельзя, т.к. может не оказаться конструкторов копирования/дефолтных
			// остаётся лишь один вариант - возвращать ссылку на свои данные. И при этом запретить их менять
			return GetDataAs<Types...>();
		}
		~memory_block() {
			Clear();
		}
	};

	template<template<typename...> typename TupleType, typename ... Types>
	memory_block(TupleType<Types...>&&)->memory_block<sizeof(TupleType<Types..., void*>), TupleType>;
	// TODO - check не перекрывает ли второй гайд первый
	template<typename ... Types>
	memory_block(Types&& ...)->memory_block<sizeof(::kelbon::tuple<std::remove_reference_t<Types>..., void*>), ::kelbon::tuple>;

} // namespace kelbon

#endif // !KELBON_MEMORY_BLOCK_HPP