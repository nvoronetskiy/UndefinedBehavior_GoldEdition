
#ifndef KELBON_MEMORY_BLOCK_HPP
#define KELBON_MEMORY_BLOCK_HPP

#include <algorithm> // std::copy, memcpy, memset + traits

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
	};

	// false false case
	template<typename T, bool IsNotTriviallyDestructible, bool IsCopyConstructible, bool IsMoveConstructible>
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
			if constexpr (IsCopyConstructible) {
				new (dest) T(*(reinterpret_cast<const T*>(srs)));
			}
			else {
				// nothing
			}
		}

		[[nodiscard]] constexpr inline bool is_trivially_destructible() const noexcept override {
			return !IsNotTriviallyDestructible;
		}
		[[nodiscard]] constexpr inline bool is_move_constructible() const noexcept override {
			return IsMoveConstructible;
		}
		[[nodiscard]] constexpr inline bool is_copy_constructible() const noexcept override {
			return IsCopyConstructible;
		}
	};

	// ѕередава€ данные на хранение в memory_block<max_size>(далее - memory_block) ¬ы: 
	// 1. ѕонимаете, что переданное значение больше нельз€ использовать, оно побайтово обнул€етс€
	// 2. ѕодтверждаете, что в деструкторах типов передаваемых значений нет переходов по ссылкам/разыменовани€ указателей €вл€ющихс€ пол€ми класса, нет арифметики удал€емых указателей
	// 3. —оглашаетесь, что больше ими не владеете и передаЄте право на владение и об€зательство на удаление на принимающую сторону(memory_block)
	template<size_t max_size, template<typename...> typename TupleType = ::kelbon::tuple>
	class memory_block {
	private:
		char  data[max_size]; // пам€ть под хранение любых входных данных
		void* memory;     // всЄ что лежит в классе-запоминателе деструктора - указатель на таблицу виртуальных функций

		void Clear() noexcept {
			if (memory == nullptr) [[unlikely]] {
				return;
			}
			// т.к. у мен€ void*, а дл€ корретного выбора из vtable нужен указатель на базовый класс, то реинтерпретирую указатель на указатель...
			reinterpret_cast<base_remember_type_info*>(&memory)->Destroy(data);
			memory = nullptr;
		}
		const base_remember_type_info* const GetRTTI() const noexcept {
			return reinterpret_cast<const base_remember_type_info* const>(&memory);
		}
	public:
		constexpr memory_block() noexcept : data{ 0 }, memory(nullptr) {}

		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		memory_block(TupleType<Types...>&& value) noexcept {
			using Tuple = TupleType<Types...>;
			static_assert(std::is_copy_constructible_v<Tuple> || (std::is_move_constructible_v<Tuple> && std::is_nothrow_move_constructible_v<Tuple>),
				"Object is non copyble and non-movable, its useless to store it here");
			//realmemmove(data, &value, sizeof(Tuple));
			// по сути здесь происходит запоминание деструктора, пр€мо в значении указател€ € конструирую класс(т.к. он состоит из всего одного указател€ на vtable)
			new(&memory)
				remember_type_info<Tuple,
				!std::is_trivially_destructible_v<Tuple>,
				std::is_copy_constructible_v<Tuple>,
				std::is_move_constructible_v<Tuple>>{};

			if constexpr (std::is_move_constructible_v<Tuple>) {
				GetRTTI()->Move(&value, data);
			}
			else { // here Tuple is copible, because of static_assert here
				GetRTTI()->Copy(&value, data);
			}
		}

		template<typename ... Types>
		requires (sizeof(TupleType<std::remove_reference_t<Types>...>) <= max_size)
		memory_block(Types&& ... args)
			noexcept(std::is_nothrow_constructible_v<TupleType<std::remove_reference_t<Types>...>, Types...>)
			: memory_block(TupleType<std::remove_reference_t<Types>...>(std::forward<Types>(args)...))
		{}

		// not noexcept, if value non-movable and throw constructible
		template<size_t other_max_size> requires(other_max_size <= max_size)
		memory_block(memory_block<other_max_size, TupleType>&& other) : memory(other.memory) {
			// other is empty block
			if (other.memory == nullptr) [[unlikely]] {
				return;
			}
			const auto RTTI = other.GetRTTI();
			if (RTTI->is_move_constructible()) [[likely]] {
				RTTI->Move(other.data, data);
			}
			else { // here we know RTTI.copy constructible == true, because of check in other constructor(static_assert)
				RTTI->Copy(other.data, data);
			}
		}
		// not noexcept, if value non-movable and throw constructible
		template<size_t other_max_size> requires(other_max_size <= max_size)
		memory_block& operator=(memory_block<other_max_size, TupleType>&& other) {
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
				RTTI->Copy(other.data, data);
			}
			return *this;
		}

		// may throw double_free_possible if no avalible copy constructor for stored value
		[[nodiscard]] memory_block Clone() const {
			if (!CanBeCopied()) [[unlikely]] {
				throw double_free_possible("no copy constructor avalible for stored value (kelbon::memory_block::Clone)");
			}
			memory_block clone;
			clone.memory = memory;
			GetRTTI()->Copy(data, clone.data);
			return clone;
		}
		
		[[nodiscard]] bool IsTriviallyDestructibleNow() const noexcept {
			if (memory == nullptr) [[unlikely]] {
				return true;
			}
			return GetRTTI()->is_trivially_destructible();
		}
		[[nodiscard]] bool CanBeMoved() const noexcept {
			if (memory == nullptr) [[unlikely]] {
				return true;
			}
			return GetRTTI()->is_move_constructible();
		}
		// check it before using Clone method if you want to not catch exception
		[[nodiscard]] bool CanBeCopied() const noexcept {
			if (memory == nullptr) [[unlikely]] {
				return true;
			}
			return GetRTTI()->is_copy_constructible();
		}

		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] const TupleType<Types...>& GetDataAs() const noexcept {
			// bit_cast не подходит, потому что он не работает дл€ объектов с нетривиальными копи конструкторами
			// копировать или даже создавать std::tuple<Types...> нельз€, т.к. может не оказатьс€ конструкторов копировани€/дефолтных
			// остаЄтс€ лишь один вариант - возвращать ссылку на свои данные. » при этом запретить их мен€ть
			return *(reinterpret_cast<const TupleType<Types...>* const>(data));
		}
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] TupleType<Types...>& GetDataAs() noexcept {
			return *(reinterpret_cast<TupleType<Types...>*>(const_cast<char*>(data)));
		}
		// same as GetDataAs, but with checking if right types you trying to get, if not - exception thrown
		template<typename ... Types> requires(sizeof(TupleType<Types...>) <= max_size)
		[[nodiscard]] const TupleType<Types...>& SafeGetDataAs() const {
			auto check_value =
				remember_type_info<TupleType<Types...>,
				!std::is_trivially_destructible_v<TupleType<Types...>>,
				std::is_copy_constructible_v<TupleType<Types...>>,
				std::is_move_constructible_v<TupleType<Types...>>>{};

			if ((*(reinterpret_cast<void**>(&check_value))) != memory) {
				throw bad_memory_block_access(
					"Types you're trying to get do not match types stored here."
					"If that's exactly what you need, use GetDataAs method");
			}
			return GetDataAs<Types...>();
		}
		~memory_block() {
			Clear();
		}
	};

	template<template<typename...> typename TupleType, typename ... Types>
	memory_block(TupleType<Types...>&&)->memory_block<sizeof_pack<TupleType<Types...>>(), TupleType>;

	// BUG IN COMPILER(parser) - bad working with pack expansion/sizeof/tetrary operator
	// in deduction guides/noexcept expressions/template arguments in specializations/when inheriting
	template<typename ... Types>
	memory_block(Types&& ...)->memory_block<sizeof_pack<::kelbon::tuple<std::remove_reference_t<Types>...>>(), ::kelbon::tuple>;

} // namespace kelbon

#endif // !KELBON_MEMORY_BLOCK_HPP