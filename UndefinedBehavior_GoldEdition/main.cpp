
#include <iostream>
#include <string>

#include "kelbon_functional_traits.hpp"
#include "kelbon_memory_block.hpp"

// шаблон базового класса с виртуальным оператором () в protected зоне, принимающим и возвращающим какие то аргументы переданные в шаблоне, да-да.
template<typename ResultType, typename ... ArgTypes>
class base_action {	
public:
	using result_type = ResultType;
	template<size_t index> using argument_type = typename type_of_element<index, ArgTypes...>::type;

	virtual ~base_action() = 0;
protected:
	virtual result_type operator()(ArgTypes...) = 0;
};

template<typename Function, typename BaseClass>
class wrap_action : public BaseClass {
public:
protected:

};
//template<...>
//... WrapAction()

// todo - слушатель, который просто передаЄт инфу слушающим дальше, подключенным к нему(позвол€ет делать только 1 указатель на слушател€ в объектах/потоке физики, передавать сообщение
// и он уже выбирает кому нужно сообщение, например слушатели физики / конкретных каких то вещей.

// todo трейты is_method is_functor/lambda и т.д.(или концепты в некоторых местах, а не трейты), также можно is const/volatile/noexcept(лол)
// ещЄ есть & && квалификаторы после функции, помогающие перегружать по контексту использовани€ (this)
// todo может ещЄ и дл€ шаблонов заху€рить?
// можно ещЄ написать свой tuple во первых чтобы не зависеть от реализаций его внутри, во вторых чтобы он укладывал данные в нужном пор€дке

struct s {
	constexpr size_t operator()(int, double) const {
		return 4;
	}
};
int main() {
	using t = decltype([](int){ return false; });
	function_info<realmemmove>::parameter_list::argument_type<2> ann = 6;
	std::cout << ann << std::endl;

	function_info<t{}>::result_type sprigan = true;
	int ival = 5;
	float fval = 3.14f;
	std::string s = "All right";

	memory_block<126> value(std::tuple(s, fval, ival));
	const auto& [str, fv, iv] = value.GetDataAs<int, float, int>();

	std::cout << str << '\t' << fv << '\t' << iv << std::endl;

	return 0;
}