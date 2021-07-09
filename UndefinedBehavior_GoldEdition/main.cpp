
#include <iostream>
#include <string>

#include "kelbon_functional_traits.hpp"
#include "kelbon_memory_block.hpp"

// ������ �������� ������ � ����������� ���������� () � protected ����, ����������� � ������������ ����� �� ��������� ���������� � �������, ��-��.
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

// todo - ���������, ������� ������ ������� ���� ��������� ������, ������������ � ����(��������� ������ ������ 1 ��������� �� ��������� � ��������/������ ������, ���������� ���������
// � �� ��� �������� ���� ����� ���������, �������� ��������� ������ / ���������� ����� �� �����.

// todo ������ is_method is_functor/lambda � �.�.(��� �������� � ��������� ������, � �� ������), ����� ����� is const/volatile/noexcept(���)
// ��� ���� & && ������������� ����� �������, ���������� ����������� �� ��������� ������������� (this)
// todo ����� ��� � ��� �������� ���������?
// ����� ��� �������� ���� tuple �� ������ ����� �� �������� �� ���������� ��� ������, �� ������ ����� �� ��������� ������ � ������ �������

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