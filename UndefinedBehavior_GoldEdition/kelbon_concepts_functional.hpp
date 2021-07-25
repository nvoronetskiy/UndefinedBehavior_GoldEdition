

#ifndef KELBON_CONCEPTS_FUNCTIONAL_HPP
#define KELBON_CONCEPTS_FUNCTIONAL_HPP

#include <concepts>
#include "kelbon_type_traits_functional.hpp"

namespace kelbon {

	// CONCEPT callable
	template<typename Applicant>
	concept callable = requires {
		typename signature<Applicant>::result_type;
		typename signature<Applicant>::parameter_list;
		signature<Applicant>::is_noexcept;
	};

	// CONCEPT functor
	template<typename Applicant>
	concept functor = callable<Applicant> && like_functor<Applicant>;

	// CONCEPT function
	template<typename Applicant>
	concept function = !like_functor<Applicant> && callable<Applicant>;

	// CONCEPT method
	// DO NOT WORK FOR STATIC METHODS(they are same as functions) частный случай функции
	template<typename Applicant> // not like_functor because signature see functors as operator() == method
	concept method = !like_functor<Applicant> && callable<Applicant> && requires {
		typename signature<Applicant>::owner_type;
		signature<Applicant>::is_const;
		signature<Applicant>::is_volatile;
		signature<Applicant>::ref_qualification;
	};

	namespace func {
		// CONCEPT returns (checking result type of the callable)
		template<typename Func, typename ResultType>
		concept returns = callable<Func> && std::same_as<typename signature<Func>::result_type, ResultType>;

		// CONCEPT accepts
		template<typename Func, typename ... Types>
		concept accepts = callable<Func> && std::same_as<typename signature<Func>::parameter_list, type_list<Types...>>;
	} // namespace func

} // namespace kelbon

#endif // !KELBON_CONCEPTS_FUNCTIONAL_HPP

