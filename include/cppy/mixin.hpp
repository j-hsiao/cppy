#ifndef CPPY_MIXIN_HPP
#define CPPY_MIXIN_HPP

#include "cppy/object.hpp"
#include <utility>

namespace cppy
{
	//Mix in to add checkthrow methods.
	//The Derived class should define a bool check() const
	//method.
	template<class DerivedRaw>
	struct CheckThrow
	{
		using Derived = Object<DerivedRaw>;

		Derived& derived() { return static_cast<Derived&>(*this); }
		const Derived& derived() const { return static_cast<Derived&>(*this); }

		const Derived& checkthrow() const&
		{
			derived().throwifnot(derived().check(), derived().name());
			return derived();
		}
		Derived& checkthrow() &
		{
			derived().throwifnot(derived().check(), derived().name());
			return derived();
		}

		Derived&& checkthrow() &&
		{
			derived().throwifnot(derived().check(), derived().name());
			return std::move(derived());
		}
	};

	// Mixin for derived objects that can be converted
	// to some c type.
	template<class DerivedRaw, template<class> class Converter>
	struct Convertible
	{
		using Derived = Object<DerivedRaw>;

		template<class T>
		T to() const {
			T val = Converter<T>::toc(static_cast<const Derived*>(this)->obj);
			if (val == Converter<T>::badc() && PyErr_Occurred() != NULL)
			{ throw PyError(); }
			return val;
		}
		template<class T> operator T() const { return to<T>(); }
	};
}
#endif//CPPY_MIXIN_HPP
