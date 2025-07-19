#ifndef CPPY_BOOL_HPP
#define CPPY_BOOL_HPP

#include <cppy/object.hpp>
namespace cppy {

	template<> struct Object<bool&>: Borrowed { using Borrowed::Borrowed; };

	template<> struct Object<bool>: Owned<bool> {
		using Owned<bool>::Owned;
		Object(bool b): Owned<bool>(b ?  Py_True : Py_False) { Py_INCREF(obj); }
	};
}
#endif//CPPY_BOOL_HPP
