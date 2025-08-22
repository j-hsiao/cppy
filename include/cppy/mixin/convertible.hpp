#ifndef CPPY_MIXIN_CONVERTIBLE_HPP
#define CPPY_MIXIN_CONVERTIBLE_HPP

#include <cppy/errors.hpp>
#include <type_traits>
namespace cppy
{
	// Mixin for derived objects that can be converted to some c type.
	// struct Converter<T>:
	// {
	//	 static T toc(PyObject*);
	//	 static (constexpr) T badc();
	// }
	// toc converts PyObject* to a T type.  If the result == badc(),
	// then check PyErr_Occurred to see if it was really a bad
	// conversion.
	template<class Derived, template<class> class Converter>
	struct Convertible
	{
		template<class O>
		O to() const {
			typedef Converter<O> conv;
			O val = conv::toc(static_cast<const Derived*>(this)->obj);
			if (val == conv::badc() && PyErr_Occurred() != NULL) { throw PythonError(); }
			return val;
		}
		template<class O> explicit operator O() const { return to<O>(); }

		template<class T>
		bool operator==(T &&t) const
		{ return t == to<typename std::decay<T>::type>(); }
	};

	template<class Derived, template<class> class Converter, class Value>
	bool operator==(Value &&v, const Convertible<Derived, Converter> &o)
	{ return o == v; }
}
#endif//CPPY_MIXIN_CONVERTIBLE_HPP
