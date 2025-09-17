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

	//Default constructible instance
	template<class T, bool DefaultConstructible=default_constructible<T>::value>
	struct Inst {
		static T inst() { return T(); }
		template<class V>
		static void set_inst(V&&){}
	};
	//must have copy (like a lambda)
	template<class T, false> struct Inst {
		static AlignedBuffer<T> buf;
		static T& inst() { return buf.ref(); }
		template<class V>
		static void set_inst(V&&v) { buf = std::forward<V>(v); }
	};
	template<class T, false> AlignedBuffer<T> Inst<T,false>::buf;

	template<class T> struct Wrapper
	{
		static Inst<T> instance;

		typedef function_signature<T> signature;
		typedef typename signature::return_type return_type;

		//Get the wrapping PyCFunction pointer.
		static PyCFunction get(const T &functor)
		{
			instance.set_inst(functor);
			return reinterpret_cast<PyCFunction>(call<T>);
		}

		//METH_NOARGS
		template<class V=T, typename enable<signature::arguments_type::count == 0>::type=true>
		static PyObject* call(PyObject *self, PyObject *args)
		{ return catchcall(buf.ref()); }

		//METH_O
		template<class V=T, typename enable<signature::arguments_type::count == 1>::type=true>
		static PyObject* call(PyObject *self, PyObject *args)
		{
			Object<typename signature::arguments_type::template get<0>::type, false> arg(args);
			if (args.check()) { return catchcall(buf.ref(), arg); }
			else {
				TypeError("Type mismatch.");
				return NULL;
			}
		}
		//METH_ARGS
		template<class V=T, typename enable<(signature::arguments_type::count > 1)>::type=true>
		static PyObject* call(PyObject *mod, PyObject *args)
		{
			Tuple<false> tup(args);
			return catchcall(CPPCaller{}, buf.ref(), tup);
		}
	};

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
