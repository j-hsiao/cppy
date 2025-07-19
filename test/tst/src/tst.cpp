#include <cppy/object.hpp>
#include <cppy/int.hpp>
#include <cppy/string.hpp>
#include <cppy/tuple.hpp>
#include <cppy/list.hpp>
//#include <cppy/float.hpp>
//#include <cppy/method.hpp>
//#include <cppy/util.hpp>

#include <iostream>

PyObject* basic_test(PyObject *m, PyObject *args_)
{
	{
		std::cout << "------------------------------" << std::endl;
		std::cout << "Running basic test as generic borrowed reference object." << std::endl;
		cppy::Object<> args(args_);
		std::cout << "  size: " << args.size() << std::endl;
		std::cout << "  str : " << args.str() << std::endl;
		std::cout << "  repr: " << args.repr() << std::endl;
	}

	{
		std::cout << "------------------------------" << std::endl;
		std::cout << "Running basic test as generic owned reference object." << std::endl;
		cppy::Object<> borrowed(args_);
		cppy::Object<PyObject> owned(borrowed);
		std::cout << "  size: " << owned.size() << std::endl;
		std::cout << "  str : " << owned.str() << std::endl;
		std::cout << "  repr: " << owned.repr() << std::endl;
	}

	{
		std::cout << "------------------------------" << std::endl;
		std::cout << "indexing const generic borrowed reference." << std::endl;
		const cppy::Object<> args(args_);
		for (int i=0; i<args.size(); ++i)
		{ std::cout << "    " << i << ": " << args[i]->str() << std::endl; }
	}


	{
		std::cout << "------------------------------" << std::endl;
		std::cout << "indexing non-const generic borrowed reference." << std::endl;
		cppy::Object<> args(args_);
		for (int i=0; i<args.size(); ++i)
		{ std::cout << "    " << i << ": " << args[i]->str() << std::endl; }
	}

	{
		std::cout << "------------------------------" << std::endl;
		std::cout << "using a borrowed Tuple" << std::endl;
		cppy::TupleRef args(args_);
		for (int i=0; i<args.size(); ++i)
		{ std::cout << "    " << i << ": " << args[i]->str() << std::endl; }
	}

	{
		cppy::TupleRef args(args_);
		if (args.size() >= 3) {
			cppy::Tuple tmp(
				args[0]->obj,
				args[1]->obj,
				args[2]->obj
			);
			std::cout << tmp.str() << std::endl;
		}
		else {
			cppy::Tuple tmp(1, 2, 3);
			std::cout << tmp.str() << std::endl;
		}
	}

	{
		cppy::TupleRef args(args_);
		if (args.check()) {
			std::cout << args.slice(1,args.size_()).str() << std::endl;
		}
	}

	{
		cppy::List lst("hello", 72);

		if (lst.str().string() != "['hello', 72]") { Py_RETURN_FALSE; }
		if (lst.tuple().str().string() != "('hello', 72)") { Py_RETURN_FALSE; }
		lst.extend(lst.tuple());
		if (lst.str().string() != "['hello', 72, 'hello', 72]") { Py_RETURN_FALSE; }
		lst.clear();
		if (lst.str().string() != "[]") { Py_RETURN_FALSE; }
	}
	Py_RETURN_TRUE;
}

//PyObject* tupassign_test(PyObject *m, PyObject *args_)
//{
//	cppy::Tuple<true> ret(
//		1, "hello world!", 1.5, 3.25f, 5,
//		cppy::Tuple<true>("hello", 42));

//	return ret.ret();
//}

//PyObject* call_test(PyObject *m, PyObject *args)
//{
//	int result = cppy::callcpp(
//		[](int a, int b){ return a + b; },
//		args);
//	return cppy::Object<int,true>(result).ret();
//}

//PyObject* call_test2(PyObject *m, PyObject *args_)
//{
//	cppy::Tuple<> args(args_);
//	return cppy::callpy(args[0]().obj, 1, 2);
//}

PyMODINIT_FUNC PyInit_testmodule()
{
	static PyMethodDef methods[] = {
		{ "basic_test", basic_test, METH_VARARGS, "print arguments." },
		//{ "tupassign_test", tupassign_test, METH_VARARGS, "return a tup." },
		//{ "call_test", call_test, METH_VARARGS, "Call a c++ lambda using python arguments." },
		//{ "call_test2", call_test2, METH_VARARGS, "Call a Python func using c++ arguments (1, 2)." },
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
