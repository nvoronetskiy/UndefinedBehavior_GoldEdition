
#ifndef KELBON_CONCEPTS_HPP
#define KELBON_CONCEPTS_HPP
template<typename T> struct remove_pointer { using type = T; };
template<typename T> struct remove_pointer<T*> { using type = T; };

template<typename T> using remove_pointer_t = typename remove_pointer<T>::type;

template<typename T>
concept functor = requires { T::operator(); }; // todo - ������ ������ ��� noexcept ��� �� ��������

template<typename T>
concept not_functor = !functor<T>;
// �� ������, ��� ����������(����������), todo - concept function
#endif // !KELBON_CONCEPTS_HPP
