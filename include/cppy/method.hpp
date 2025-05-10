// Wrap c++ callables into a PyMethodDef for calling via python binding.
// Distinction is made between function (standalone function)
// and method (the "self"/"module" argument will be passed as well.)
// struct PyMethodDef
// { "name", funcptr, flags, "docstr" };
//
//
#ifndef CPPY_METHOD_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <cppy/errors.hpp>
#include <cppy/util.hpp>
#include <cppy/object.hpp>
#include <cppy/tuple.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace cppy
{
	//functor: copy constructible or default constructible
	//If neither... then what? impossible I think...
	//
	//                          1 arg: METH_O           N args: METH_VARARGS    kwargs: METH_KEYWORDS
	//copy constructible        arguments_type::count   >1                      call with keywords list
	//                            == 1
	//
	//
	//
	//
	//default constructible
	//
	//
	//

	template<class T, bool DefaultConstructible=default_constructible<T>::value>
	struct Inst {
		static T inst() { return T(); }
		template<class V>
		static void set_inst(V&&){}
	};
	template<class T, false> struct Inst {
		static AlignedBuffer<T> buf;
		static T& inst() { return buf.ref(); }
		template<class V>
		static void set_inst(V&&v) { buf = v; }
	};
	template<class T, false>
	AlignedBuffer<T> Inst<T,false>::buf;

	//Store a copy of the functor as a static member.
	//Mostly for lambdas
	template<class T> struct CopyWrapper
	{
		static AlignedBuffer<T> buf;

		typedef function_signature<T> signature;
		typedef typename signature::return_type return_type;

		//Get the wrapping PyCFunction pointer.
		static PyCFunction get(const T &functor)
		{
			buf = functor;
			if (signature::arguments_type::count == 0)
			{ return call_noargs; }
			else if (signature::arguments_type::count == 1)
			{
				return call_args
			}
			return nullptr;
		}

		//METH_NOARGS
		template<class V=T, typename enable<signature::arguments_type::count == 0>::type=true>
		static PyObject* call(PyObject *self, PyObject *args)
		{ return Object<return_type, true>(buf.ref()()).ret(); }

		//METH_O
		template<class V=T, typename enable<signature::arguments_type::count == 1>::type=true>
		static PyObject* call(PyObject *self, PyObject *args)
		{
			return Object<return_type, true>(buf.ref()(
				Object<typename signature::arguments_type::get<0>::type, false>(args).checkthrow()
				)).ret();
		}
		//METH_ARGS
		template<class V=T, typename enable<(signature::arguments_type::count > 1)>::type=true>
		static PyObject* call(PyObject *mod, PyObject *args)
		{
			Tuple<false> tup(args);
			try
			{
				return Object<return_type, true> (call(buf.ref(), tup)).ret();
			}
			catch(PyError&) { return NULL; }
			catch(Error &e) { return NULL; }
			catch(std::exception &e)
			{
				Exception("Unknown Error.");
				return NULL;
			}
		}
	};

	//Default-constructible functors
	template<class T> struct DefaultWrapper
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

	//default constructible functor
	//template<class T, typename enabled<default_constructible<T>::value>::type=true>
	//PyMethodDef wrap(
	//	const char *name,
	//	T &&func,
	//	const char *doc)
	//{
	//	return PyMethodDef{name, func, METH_VARARGS, doc};
	//}

	//lambda / copy-constructible functor
	template<class T, typename enabled<!default_constructible<T>::value>::type=true>
	PyMethodDef methwrap(
		const char *name,
		const T &func,
		const char *doc)
	{
		if (CopyWrapper<T>::signature::arguments_type::count == 1)
		{
			return PyMethodDef{name, func, METH_O, doc};
		}
		else
		{
			return PyMethodDef{name, func, METH_VARARGS, doc};
		}
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
