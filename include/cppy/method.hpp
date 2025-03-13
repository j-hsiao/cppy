// Wrap functions/functors/lambdas into python-compatible function pointer.
// struct PyMethodDef
// { "name", funcptr, flags, "docstr" };

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
	//hold arguments
	template<class...Args> struct Arguments {
		static constexpr std::size_t size = sizeof...(Args);

		//Get a type by index
		template<std::size_t idx, class cls=Arguments<Args...>> struct get
		{
			template<class T, class...Remain>
			static Arguments<Remain...> rtp(Arguments<T, Remain...>);

			typedef typename get<idx-1, decltype(rtp(cls{}))>::type type;
		};
		template<class args> struct get<0, args>
		{
			template<class T, class...Remain>
			static T rtp(Arguments<T, Remain...>);
			typedef decltype(rtp(args{})) type;
		};
	};

	//function signature
	template<class T> struct function_signature;
	template<class T> struct function_signature<T&>: function_signature<T> {};
	template<class T> struct function_signature<T&&>: function_signature<T> {};

	//function
	template<class ret, class...Args>
	struct function_signature<ret (Args...)>
	{
		typedef ret return_type;
		typedef Arguments<Args...> arguments_type;
	};

	template<class ret, class...Args>
	struct function_signature<ret (*)(Args...)>: function_signature<ret (Args...)> {};

	//member function
	template<class Functor, class ret, class...Args>
	struct function_signature<ret (Functor::*)(Args...) const>
	{
		typedef ret return_type;
		typedef Arguments<Args...> arguments_type;
	};
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
		typedef typename function_signature<decltype(&Functor::operator())>::arguments_type arguments_type;
	};

	template<
		class Callable, class...Args,
		typename enabled<sizeof...(Args) != function_signature<Callable>::arguments_type::size>::type = true
	>
	typename function_signature<Callable>::return_type call(
		Callable &&callable, const Tuple<> &args, Args&&...converted)
	{
		return call(
			std::forward<Callable>(callable), args,
			std::forward<Args>(converted)...,
			Object<typename function_signature<Callable>::arguments_type::get<sizeof...(Args)>::type>(args[sizeof...(Args)])
		);
	}

	template<
		class Callable, class...Args,
		typename enabled<sizeof...(Args) == function_signature<Callable>::arguments_type::size>::type = true
	>
	typename function_signature<Callable>::return_type call(
		Callable &&callable, const Tuple<> &args, Args&&...converted)
	{
		return callable(std::forward<Args>(converted)...);
	}

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
