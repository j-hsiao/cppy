#include <cppy/tuple.hpp>
#include <cppy/int.hpp>
#include <cppy/object.hpp>

#include <iostream>
PyObject* basic_test(PyObject *m, PyObject *args_)
{
	cppy::Object<> args(args_);
	for (int i=0; i<args.size(); ++i)
	{
		std::cout << i << ": " << args[i].str() << std::endl;
		//std::cout << i << ": " << args[cppy::Object<int,true>(i).obj].str() << std::endl;
	}
	std::cout << args.str() << std::endl;
	Py_RETURN_NONE;
}

PyObject* tuparg_test(PyObject *m, PyObject *args_)
{
	cppy::Tuple<> args(args_);
	for (int i=0; i<args.size(); ++i)
	{
		std::cout << i << ": " << args[i].str() << std::endl;
		//std::cout << i << ": " << args[cppy::Object<int,true>(i).obj].str() << std::endl;
	}
	std::cout << args.str() << std::endl;
	Py_RETURN_NONE;
}


//PyObject* call_test(PyObject *m, PyObject *args)
//{
//	int result = cppy::call(
//		[](int a, int b){ return a + b; },
//		args);
//	return cppy::Object<int,true>(result).ret();
//}



PyMODINIT_FUNC PyInit_testmodule()
{
	static PyMethodDef methods[] = {
		{ "basic_test", basic_test, METH_VARARGS, "print arguments." },
		{ "tuparg_test", tuparg_test, METH_VARARGS, "print arguments." },
		//{ "call_test", call_test, METH_VARARGS, "add 2 ints." },
		{}
	};

	static struct PyModuleDef mdef = {
		PyModuleDef_HEAD_INIT,
		"testtmodule",
		"",
		-1,
		methods
	};
	return PyModule_Create(&mdef);
}
