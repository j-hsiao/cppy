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
	//Wrap function into a functor for uniform interface.
	template<class T> struct Wrapper;

	template<class T, class B=bool> struct Exist { typedef B type; };

	template<class T> struct DefaultConstructible
	{
		private:
			struct truetype { static constexpr bool value = true; };
			struct falsetype { static constexpr bool value = false; };
			static T& inst();
			template<class V, typename Exist<decltype(V())>::type=true>
			static truetype check(V&);
			static falsetype check(...);
		public:
			static constexpr bool value =  decltype(check(inst()))::value;
	};

	template<class T, bool b> struct FunctorWrap;

	template<class T> struct FunctorWrap<T, false>
	{
		template<std::size_t Size> struct alignas(T) Buffer { static char buf[sizeof(T)]; };
		static Buffer<sizeof(T)> buf;
	};

	template<class T>
	FunctorWrap<T,false>::Buffer<sizeof(T)> FunctorWrap<T,false>::buf;








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
