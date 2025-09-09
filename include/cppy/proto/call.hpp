#ifndef CPPY_PROTO_CALL_HPP
#define CPPY_PROTO_CALL_HPP
#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/convert/pyobject.hpp>
namespace cppy {

	template<class Base> struct PythonCallable;

	struct Tuple_;
	struct Dict_;


	template<class T, template<class>class Object>
	struct PythonCallable<Object<T>>:
		CheckThrow<PythonCallable<Object<T>>>
	{
		using Base = Object<T&>;

		PythonCallable<Base>&& sequence() && { return std::move(*this); }
		PythonCallable<Base>& sequence() & { return *this; }
		const PythonCallable<Base>& sequence() const& { return *this; }

		bool check() const
		{ return PyCallable_Check(static_cast<const Base*>(this)->obj); }

		Object<PyObject> operator()() const
		{
#			if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 9
				PyObject *ptr = PyObject_CallNoArgs(static_cast<const Base*>(this)->obj);
#			else
				PyObject *ptr = PyObject_CallObject(static_cast<const Base*>(this)->obj, NULL);
#			endif
			if (ptr) { return ptr; }
			else { throw PythonError(); }
		}

		template<class...Args>
		Object<PyObject> operator()(Args&&...args) const; // define in cppy/tuple.hpp

		//call directly with the argtuple and/or kwargs dict
		Object<PyObject> call(const Object<Tuple_&>&) const;
		Object<PyObject> call(const Object<Tuple_&>&, const Object<Dict_&>&) const;
		Object<PyObject> call(const Object<Dict_&>&) const;



	};
}
#endif//CPPY_PROTO_CALL_HPP
