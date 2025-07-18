#ifndef CPPY_MIXIN_CHECKTHROW_HPP
#define CPPY_MIXIN_CHECKTHROW_HPP
#include <cppy/errors.hpp>
namespace cppy {
	//Mix in to add checkthrow methods.
	//The Derived class should define a bool check() const
	//method.
	template<class Derived>
	struct CheckThrow
	{
#		define CHECKTHROW(prefix, suffix) \
		prefix Derived suffix checkthrow() prefix suffix { \
			throwifnot(static_cast<prefix Derived suffix>(*this).check(), static_cast<prefix Derived suffix>(*this).name()); \
			return static_cast<prefix Derived suffix>(*this); \
		}
		CHECKTHROW(const, &)
		CHECKTHROW(, &)
		CHECKTHROW(, &&)
#		undef CHECKTHROW
	};
}
#endif//CPPY_MIXIN_CHECKTHROW_HPP
