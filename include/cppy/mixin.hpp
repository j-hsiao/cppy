#ifndef CPPY_MIXIN_HPP
#define CPPY_MIXIN_HPP

#include <cppy/errors.hpp>
#include <cppy/mixin/derived.hpp>

#include <utility>
namespace cppy
{
	template<class T> struct Object;

	//Mix in to add checkthrow methods.
	//The Derived class should define a bool check() const
	//method.
	template<class T>
	struct CheckThrow
	{
		using Derived = Object<T>;
#		define CHECKTHROW(prefix, suffix) \
		prefix Derived suffix checkthrow() prefix suffix { \
			throwifnot(derived().check(), derived().name()); \
			return derived(); \
		}
		CHECKTHROW(const, &)
		CHECKTHROW(, &)
		CHECKTHROW(, &&)
#		undef CHECKTHROW

		private:
			MIXIN_DEFINE_DERIVED
	};

	// Mixin for derived objects that can be converted to some c type.
	// struct Converter<T>:
	// {
	//	 T toc(PyObject*);
	//	 static T bad;
	// }
	// toc converts PyObject* to a T type.  If the result == bad,
	// then check PyErr_Occurred to see if it was really a bad
	// conversion.
	template<class T, template<class> class Converter>
	struct Convertible
	{
		using Derived = Object<T>;

		template<class O>
		O to() const {
			typedef Converter<O> conv;
			O val = conv::toc(static_cast<const Derived*>(this)->obj);
			if (val == conv::badc() && PyErr_Occurred() != NULL) { throw PyError(); }
			return val;
		}
		template<class O> operator O() const { return to<O>(); }
	};

}
#endif//CPPY_MIXIN_HPP
