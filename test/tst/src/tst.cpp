#include <cppy/object.hpp>
#include <cppy/tuple.hpp>
#include <iostream>
PyObject* basic_test(PyObject *m, PyObject *args_)
{
	cppy::Tuple<> args(args_);
	for (int i=0; i<args.size(); ++i)
	{
		std::cout << i << ": " << args[i].str() << std::endl;
	}

	std::cout << args.str() << std::endl;
	Py_RETURN_NONE;
}

PyMODINIT_FUNC PyInit_testmodule()
{
	static PyMethodDef methods[] = {
		{ "basic_test", basic_test, METH_VARARGS, "print arguments." },
		{}
	};

	static struct PyModuleDef mdef = {
		PyModuleDef_HEAD_INIT,
		"testtmodule",
		"",
		-1,
		methods
	};
	PyObject *m;
	return PyModule_Create(&mdef);
}
