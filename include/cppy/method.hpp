// Wrap functions/functors/lambdas into python-compatible function pointer.
// struct PyMethodDef
// { "name", funcptr, flags, "docstr" };
#ifndef CPPY_METHOD_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <cstddef>
#include <type_traits>
#include <utility>

#include <cppy/util.hpp>
#include <cppy/object.hpp>
#include <cppy/tuple.hpp>

namespace cppy
{
	template<class T> class is_object
	{
		template<class V, bool b>
		static True check(const Object<V,b>&);
		static False check(...);

		public:
			static constexpr bool value = decltype(check(Types<T>::cref()))::value;
	};

	//Store a copy of the functor as a static member.
	//Mostly for lambdas
	template<class T> struct CopyWrapper
	{
		static AlignedBuffer<T> buf;

		typedef function_signature<T> signature;
		typedef typename signature::return_type return_type;

		static PyObject* call_args(PyObject *mod, PyObject *args)
		{
			Tuple<false> tup(args);
			Object<return_type, true> ret(call(, tup))
		}
		static PyObject* call_args(PyObject *mod, PyObject *args)
		static call_kwargs(PyObject *mod, PyObject *args, PyObject *kwargs)
		{
			Py_RETURN_NONE;
		}

		static PyObject* meth_args(PyObject *mod, PyObject *args)
		{
			Py_RETURN_NONE;
		}
		static PyObject* meth_kwargs(PyObject *mod, PyObject *args, PyObject *kwargs)
		{
			Py_RETURN_NONE;
		}
	};

	//Default-constructible functors
	template<class T> struct Defaultrapper
	{
		static PyObject* call_args(PyObject *mod, PyObject *args)
		{
			Tuple<false> tup(args);
		}
		static PyObject* call_args(PyObject *mod, PyObject *args)
		static call_kwargs(PyObject *mod, PyObject *args, PyObject *kwargs)
		{
			Py_RETURN_NONE;
		}

		static PyObject* meth_args(PyObject *mod, PyObject *args)
		{
			Py_RETURN_NONE;
		}
		static PyObject* meth_kwargs(PyObject *mod, PyObject *args, PyObject *kwargs)
		{
			Py_RETURN_NONE;
		}
	};

	template<class T, typename enabled<default_constructible<T>::value>::type=true>
	PyMethodDef wrap(
		const char *name,
		const char *doc)
	{



		return PyMethodDef{name, func, METH_VARARGS, doc};
	}








	////Provide a static function to wrap a function/functor
	//template<class Functor, class Converter>
	//struct functor_wrap
	//{
	//	typedef void PyObject;//


	//	template<class...Args, typename enabled<(sizeof...(Args) <  function_signature<Functor>::arguments_type::size)>::type = true>
	//	static PyObject* call(PyObject *args, Args&&...converted)
	//	{
	//		return call(args, std::forward<Args>(converted)..., converted);
	//	}

	//	static PyObject* call(PyObject *, Args&&...args)
	//	{
	//		Functor func;
	//		//TODO: convert to PyObject*
	//		func(std::forward<Args>(args)...);
	//		return nullptr;
	//	}

	//	static PyObject* wrapped(PyObject *self, PyObject *args)
	//	{
	//		//todo convert each item in args into c++ type
	//		//then pass to func
	//		Functor func;
	//		auto ret = func();
	//		return convert(ret).ret();
	//	}
	//};





	//struct Method
	//{
	//	template<class Func>
	//	Method(
	//		const char *name, Func func,
	//		const char *docstring="",
	//		int callmethod=METH_VARARGS
	//	):
	//		name(name),
	//		docs(docstring),
	//		func(...???),
	//		callmethod(callmethod)
	//	{}

	//	operator PyMethodDef() const
	//	{
	//		return PyMethodDef{
	//			name, (PyCFunction) ??,
	//			METH_...
	//			docs
	//		};
	//	}

	//	const char *name;
	//	const char *docs;
	//	PyCFunction func;
	//	int callmethod;
	//};
}
#endif//CPPY_METHOD_HPP
