// Wrap functions/functors/lambdas into python-compatible function pointer.
// struct PyMethodDef
// { "name", funcptr, flags, "docstr" };

#include <cstddef>
//#define PY_SSIZE_T_CLEAN
//#include <Python.h>

namespace cppy
{
	//hold arguments
	template<class...Args> struct Arguments {
		static constexpr std::size_t size = sizeof...(Args);
	};

	//Get a type by index
	template<class cls, std::size_t idx> struct get
	{
		template<class T, class...Remain>
		static Arguments<Remain...> rtp(Arguments<T, Remain...>);

		typedef typename get<decltype(rtp(cls{})), idx-1>::type type;
	};
	template<class args> struct get<args, 0>
	{
		template<class T, class...Remain>
		static T rtp(Arguments<T, Remain...>);
		typedef decltype(rtp(args{})) type;
	};

	//function signature
	template<class T> struct function_signature;
	//function
	template<class ret, class...Args>
	struct function_signature<ret (Args...)>
	{
		typedef ret return_type;
		typedef Arguments<Args...> arguments_type;
	};
	//member function
	template<class Functor, class ret, class...Args>
	struct function_signature<ret (Functor::*)(Args...)>
	{
		typedef ret return_type;
		typedef Arguments<Args...> arguments_type;
	};
	//functor
	template<class Functor>
	struct function_signature
	{
		typedef typename function_signature<decltype(&Functor::operator())>::return_type return_type;
		typedef typename function_signature<decltype(&Functor::operator())>::argument_types argument_types;
	};

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
