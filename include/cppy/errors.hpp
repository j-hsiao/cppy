#ifndef CPPY_ERRORS_HPP
#define CPPY_ERRORS_HPP
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdexcept>

namespace cppy
{
	//Innate python error occurred. After catching, return NULL.
	struct PyError: public std::runtime_error {
		PyError(): std::runtime_error("") {}
	};

	//Some error occurred. Need to call PyErr_*
	struct Error: public std::runtime_error
	{
		PyObject *tp;
		Error(PyObject *tp, const char *msg):
			std::runtime_error(msg),
			tp(tp)
		{
			PyErr_SetString(tp, msg);
		}
	};

#define MAKE_CPPY_PYTHON_ERROR(name) \
	struct name: public Error \
	{ \
		name(): Error(PyExc_ ## name, "") {} \
		name(const char *msg): Error(PyExc_ ## name, msg) {} \
	};

	MAKE_CPPY_PYTHON_ERROR(TypeError);
	MAKE_CPPY_PYTHON_ERROR(ValueError);
	MAKE_CPPY_PYTHON_ERROR(IndexError);

#undef MAKE_CPPY_PYTHON_ERROR
}

#endif//CPPY_ERRORS_HPP
