

#ifndef KELBON_CONCEPTS_FUNCTIONAL_HPP
#define KELBON_CONCEPTS_FUNCTIONAL_HPP

namespace kelbon {
	// CONCEPT like_functor - вспомогательный концепт, проверяющий наличие оператора ()
	// do not works for template operator() in class
	template<typename T> // todo - убрать &, это для исправления бага ide
	concept like_functor = requires { &T::operator(); };

	// todo - concept callable
} // namespace kelbon

#endif // !KELBON_CONCEPTS_FUNCTIONAL_HPP

