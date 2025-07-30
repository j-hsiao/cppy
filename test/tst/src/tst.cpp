#include <cppy/object.hpp>
#include <cppy/int.hpp>
#include <cppy/string.hpp>
#include <cppy/tuple.hpp>
#include <cppy/list.hpp>
#include <cppy/float.hpp>
//#include <cppy/method.hpp>
//#include <cppy/util.hpp>

#include <iostream>
PyObject* test_convert(PyObject *m, PyObject *args_) {
	{
		cppy::Object<float> floatfromdouble(3.14);
		cppy::Object<float> floatfromfloat(32.4f);
		cppy::Object<float> floatfromint(3);
#		define CHECK(...) \
		std::cerr << "    " #__VA_ARGS__ ": "; \
		if (!(__VA_ARGS__)) { \
			std::cerr << "fail" << std::endl; \
			Py_RETURN_FALSE; \
		} \
		else { std::cerr << "pass" << std::endl; }

		CHECK(floatfromdouble.to<double>() == 3.14)
		CHECK(floatfromfloat.to<float>() == 32.4f)
		CHECK(floatfromint.to<int>() == 3)
		CHECK(3.14 == floatfromdouble)
		CHECK(32.4f == floatfromfloat)
		CHECK(3 == floatfromint)
	}
	{
		cppy::Object<int> intfromdouble(3.14);
		cppy::Object<int> intfromfloat(4.13f);
		cppy::Object<int> intfromint(2);

		CHECK(intfromdouble.to<double>() == 3.0)
		CHECK(intfromfloat.to<double>() == 4.0)
		CHECK(intfromint.to<double>() == 2.0)
		CHECK(intfromdouble.to<int>() == 3)
		CHECK(intfromfloat.to<int>() == 4)
		CHECK(intfromint.to<int>() == 2)
		CHECK(3 == intfromdouble)
		CHECK(4 == intfromfloat)
		CHECK(2 == intfromint)
		CHECK(3.0 == intfromdouble)
		CHECK(4.0 == intfromfloat)
		CHECK(2.0 == intfromint)
	}
	Py_RETURN_TRUE;
}

PyObject* test_list(PyObject *m, PyObject *args_) {
	{
		std::cout << "    ------------------------------" << std::endl
		          << "    list creation" << std::endl;
		std::cout << "    list" << std::endl;
		cppy::List lst("hello", 72);
		if (lst.str().string() != "['hello', 72]") { Py_RETURN_FALSE; }
		std::cout << "    ------------------------------" << std::endl
		          << "    list to tuple" << std::endl;
		if (lst.tuple().str().string() != "('hello', 72)") { Py_RETURN_FALSE; }
		std::cout << "    ------------------------------" << std::endl
		          << "    extend" << std::endl;
		lst.extend(lst.tuple());
		if (lst.str().string() != "['hello', 72, 'hello', 72]") { Py_RETURN_FALSE; }
		std::cout << "    ------------------------------" << std::endl
		          << "    clear" << std::endl;
		lst.clear();
		if (lst.str().string() != "[]") { Py_RETURN_FALSE; }

	}
	{
		cppy::TupleRef args(args_);
		std::cout << "    ------------------------------" << std::endl
		          << "    append" << std::endl;
		cppy::List lst;
		if (lst.size() != 0) { Py_RETURN_FALSE; }
		std::cout << "    before append." << std::endl;

		cppy::PyObjectConverter<cppy::Object> cvt;
		PyObject* ptr = cvt(args.getitem(0));
		if (ptr != args.getitem(0).obj) {
			std::cout << "    Convert to ptr (getitem) fail." << std::endl;
			Py_RETURN_FALSE;
		}
		if (ptr != (*args[0]).obj) {
			std::cout << "    Convert to ptr (*[]) fail." << std::endl;
			Py_RETURN_FALSE;
		}
		if (ptr != args[0]->obj) {
			std::cout << "    Convert to ptr ([]->) fail." << std::endl;
			Py_RETURN_FALSE;
		}
		if (ptr != cvt(args[0])) {
			std::cout << "    Convert to ptr converter([]) fail." << std::endl;
			Py_RETURN_FALSE;
		}

		lst.append(args.getitem(0));
		lst.insert(0, args.getitem(0));

		lst.append(args[0]);
		lst.insert(0, args[0]);

		lst.append(0);
		if (lst[lst[lst.size()-1]]->obj != args[0]->obj) {
			std::cout << "    lst[lst[-1]] != lst[0]" << std::endl;
			Py_RETURN_FALSE;
		}
	}

	Py_RETURN_TRUE;
}


PyObject* test_object(PyObject *m, PyObject *args_) {
	{
		std::cout << "    ------------------------------" << std::endl
		          << "    Running basic test as generic borrowed reference object." << std::endl;
		cppy::Object<> args(args_);
		std::cout << "      size: " << args.size() << std::endl;
		std::cout << "      str : " << args.str() << std::endl;
		std::cout << "      repr: " << args.repr() << std::endl;
	}

	{
		std::cout << "    ------------------------------" << std::endl
		          << "    Running basic test as generic owned reference object." << std::endl;
		cppy::Object<> borrowed(args_);
		cppy::Object<PyObject> owned(borrowed);
		std::cout << "      size: " << owned.size() << std::endl;
		std::cout << "      str : " << owned.str() << std::endl;
		std::cout << "      repr: " << owned.repr() << std::endl;
	}

	{
		std::cout << "    ------------------------------" << std::endl
		          << "    indexing const generic borrowed reference." << std::endl;
		const cppy::Object<> args(args_);
		for (int i=0; i<args.size(); ++i)
		{ std::cout << "        " << i << ": " << args[i]->str() << std::endl; }
	}


	{
		std::cout << "    ------------------------------" << std::endl
		          << "    indexing non-const generic borrowed reference." << std::endl;
		cppy::Object<> args(args_);
		for (int i=0; i<args.size(); ++i)
		{ std::cout << "        " << i << ": " << args[i]->str() << std::endl; }
	}
	Py_RETURN_TRUE;
}

PyObject* test_tuple(PyObject *m, PyObject *args_)
{
	{
		std::cout << "    ------------------------------" << std::endl
		          << "    using a borrowed Tuple" << std::endl;
		cppy::TupleRef args(args_);
		for (int i=0; i<args.size(); ++i)
		{ std::cout << "        " << i << ": " << args[i]->str() << std::endl; }
	}

	{
		std::cout << "    ------------------------------" << std::endl
		          << "    testing tuple creation" << std::endl;
		cppy::TupleRef args(args_);
		if (args.size() >= 3) {
			cppy::Tuple tmp(
				args[0]->obj,
				args[1]->obj,
				args[2]->obj
			);
			std::cout << "      " << tmp.str() << std::endl;
		}
		else {
			cppy::Tuple tmp(1, 2, 3);
			std::cout << "      " << tmp.str() << std::endl;
		}
	}

	{
		std::cout << "    ------------------------------" << std::endl
		          << "    tuple size_+slice" << std::endl;
		cppy::TupleRef args(args_);
		if (args.check()) {
			std::cout << "      " << args.slice(1,args.size_()).str() << std::endl;
		}
	}

	Py_RETURN_TRUE;
}


PyMODINIT_FUNC PyInit_testmodule()
{
	static PyMethodDef methods[] = {
		{"test_object", test_object, METH_VARARGS, "Test object interface." },
		{"test_tuple", test_tuple, METH_VARARGS, "Test tuples." },
		{"test_convert", test_convert, METH_VARARGS, "test some conversion operators."},
		{"test_list", test_list, METH_VARARGS, "Test list."},
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
