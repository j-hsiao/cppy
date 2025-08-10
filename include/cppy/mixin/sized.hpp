#ifndef CPPY_MIXIN_SIZED_HPP
#define CPPY_MIXIN_SIZED_HPP

#include <cppy/errors.hpp>

namespace cppy {
	// The unchecked seems to be usually a macro.
	// Cannot pass a macro as a template parameter so...
	template<
		class Derived,
		Py_ssize_t (*checked)(PyObject*),
		Py_ssize_t invalid=-1>
	struct Sized {
		Py_ssize_t size() const {
			Py_ssize_t ret = checked(static_cast<const Derived&>(*this).obj);
			if (ret == invalid) { throw PyError(); }
			return ret;
		}
	};

}
#endif//CPPY_MIXIN_SIZED_HPP
