

#ifndef KELBON_CONCEPTS_FUNCTIONAL_HPP
#define KELBON_CONCEPTS_FUNCTIONAL_HPP

// CONCEPT FUNCTOR
template<typename T> // todo - ������ &, ��� ��� ����������� ���� ide
concept functor = requires { &T::operator(); };

// todo - concept function

#endif // !KELBON_CONCEPTS_FUNCTIONAL_HPP

