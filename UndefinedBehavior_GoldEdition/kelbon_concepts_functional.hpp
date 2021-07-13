

#ifndef KELBON_CONCEPTS_FUNCTIONAL_HPP
#define KELBON_CONCEPTS_FUNCTIONAL_HPP

namespace kelbon {
	// CONCEPT FUNCTOR
	template<typename T> // todo - убрать &, это для исправления бага ide
	concept functor = requires { &T::operator(); };

	// todo - concept function
} // namespace kelbon

#endif // !KELBON_CONCEPTS_FUNCTIONAL_HPP

