#include <cppy/tuple.hpp>
#include <cppy/int.hpp>
#include <cppy/object.hpp>
#include <cppy/tuple.hpp>
#include <cppy/float.hpp>

#include <cppy/util.hpp>
#include <iostream>

PyObject* basic_test(PyObject *m, PyObject *args_)
{
	std::cout << "Running basic test as generic object." << std::endl;
	cppy::Object<> args(args_);
	std::cout << "  str : " << args.str() << std::endl;
	std::cout << "  repr: " << args.repr() << std::endl;
	std::cout << "  size: " << args.size() << std::endl;
	for (int i=0; i<args.size(); ++i)
	{
		std::cout << "    " << i << ": " << args[i]().str() << std::endl;
	}
	Py_RETURN_NONE;
}

PyObject* tupassign_test(PyObject *m, PyObject *args_)
{
	cppy::Tuple<true> ret(5);
	ret(0) = 1;
	ret(1) = "hello world!";
	ret(2) = 1.5;
	ret(3) = 3.25f;
	ret(4) = 5;

	return ret.ret();
}

PyObject* call_test(PyObject *m, PyObject *args)
{
	int result = cppy::call(
		[](int a, int b){ return a + b; },
		args);
	return cppy::Object<int,true>(result).ret();
}



PyMODINIT_FUNC PyInit_testmodule()
{
	static PyMethodDef methods[] = {
		{ "basic_test", basic_test, METH_VARARGS, "print arguments." },
		{ "tupassign_test", tupassign_test, METH_VARARGS, "return a tup." },
		{ "call_test", call_test, METH_VARARGS, "add 2 ints." },
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
