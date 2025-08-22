#ifndef CPPY_ERRORS_HPP
#define CPPY_ERRORS_HPP
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <cppy/util.hpp>

#include <stdexcept>
#include <utility>

namespace cppy
{
	struct Error: std::runtime_error {
		template<class...T>
		Error(T&&...t): std::runtime_error(std::forward<T>(t)...) {}

		//Clear error.
		void clear() const { PyErr_Clear(); }
	};

	//Innate python error occurred. After catching, return NULL.
	struct PythonError: Error { PythonError(): Error("") {} };

	//Some error occurred. Need to call PyErr_*
	struct CPPError: Error {
		CPPError(PyObject *tp, const char *msg): Error(msg) { PyErr_SetString(tp, msg); }
	};

#define MAKE_CPPY_PYTHON_ERROR(name) \
	struct name: CPPError \
	{ \
		name(): CPPError(PyExc_ ## name, "") {} \
		name(const char *msg): CPPError(PyExc_ ## name, msg) {} \
	}
	MAKE_CPPY_PYTHON_ERROR(Exception);
	MAKE_CPPY_PYTHON_ERROR(TypeError);
	MAKE_CPPY_PYTHON_ERROR(ValueError);
	MAKE_CPPY_PYTHON_ERROR(IndexError);
	MAKE_CPPY_PYTHON_ERROR(KeyError);

#undef MAKE_CPPY_PYTHON_ERROR

	static inline PyObject* success(PyObject *obj) {
		if (obj) { return obj; }
		throw PythonError();
	}

	void throwifnot(bool success, const char *msg="")
	{ if (not success) { throw TypeError(msg); } }

//	//Call a functor, catching errors.
//	template<class T, class...Args>
//	PyObject* catchcall(T &&functor, Args&&...args) {
//		typedef function_signature<T> signature;
//		try {
//			return Object<typename signature::return_type, true>(
//				std::forward<Args>(args)...).ret();
//		}
//		catch (Error&) {}
//		catch (std::exception&) { CPPError("Unknown Error"); }
//		return NULL;
//	}
}

#endif//CPPY_ERRORS_HPP
