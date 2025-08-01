#include <cppy/object.hpp>
#include <cppy/int.hpp>
#include <cppy/string.hpp>
#include <cppy/tuple.hpp>
#include <cppy/list.hpp>
#include <cppy/float.hpp>
#include <cppy/dict.hpp>
//#include <cppy/method.hpp>
//#include <cppy/util.hpp>

#include <type_traits>
#include <iostream>


struct Indented {
	std::ostream &o;
	const char *indent;
	bool start;

	Indented(std::ostream &o, const char *indent = "    "): o(o), indent(indent), start(true)  {}

	Indented& operator<<(std::ostream& (*funct)(std::ostream&)) {
		o << funct;
		if (funct == std::endl<std::ostream::char_type, std::ostream::traits_type>)
		{ start = true; }
		return *this;
	}

	template<class T>
	Indented& operator<<(T &&t) {
		if (start) {
			o << indent;
			start = false;
		}
		o << t;
		return *this;
	}
};

static Indented iout(std::cout);

PyObject* test_convert(PyObject *m, PyObject *args_) {
	{
		cppy::Object<float> floatfromdouble(3.14);
		cppy::Object<float> floatfromfloat(32.4f);
		cppy::Object<float> floatfromint(3);
#		define CHECK(...) \
		iout << #__VA_ARGS__ ": "; \
		if (!(__VA_ARGS__)) { \
			iout << "fail" << std::endl; \
			Py_RETURN_FALSE; \
		} \
		else { iout << "pass" << std::endl; }

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
		iout << "------------------------------" << std::endl
		     << "list creation" << std::endl
		     << "list" << std::endl;

		cppy::List lst("hello", 72);
		if (lst.str().string() != "['hello', 72]") { Py_RETURN_FALSE; }
		iout << "------------------------------" << std::endl
		     << "list to tuple" << std::endl;
		if (lst.tuple().str().string() != "('hello', 72)") { Py_RETURN_FALSE; }
		iout << "------------------------------" << std::endl
		     << "extend" << std::endl;
		lst.extend(lst.tuple());
		if (lst.str().string() != "['hello', 72, 'hello', 72]") { Py_RETURN_FALSE; }
		iout << "------------------------------" << std::endl
		     << "clear" << std::endl;
		lst.clear();
		if (lst.str().string() != "[]") { Py_RETURN_FALSE; }

	}
	{
		cppy::TupleRef args(args_);
		iout << "------------------------------" << std::endl
		     << "append" << std::endl;
		cppy::List lst;
		if (lst.size() != 0) { Py_RETURN_FALSE; }
		iout << "before append." << std::endl;

		cppy::PyObjectConverter<cppy::Object> cvt;
		PyObject* ptr = cvt(args.getitem(0));
		if (ptr != args.getitem(0).obj) {
			iout << "Convert to ptr (getitem) fail." << std::endl;
			Py_RETURN_FALSE;
		}
		if (ptr != (*args[0]).obj) {
			iout << "Convert to ptr (*[]) fail." << std::endl;
			Py_RETURN_FALSE;
		}
		if (ptr != args[0]->obj) {
			iout << "Convert to ptr ([]->) fail." << std::endl;
			Py_RETURN_FALSE;
		}
		if (ptr != cvt(args[0])) {
			iout << "Convert to ptr converter([]) fail." << std::endl;
			Py_RETURN_FALSE;
		}

		lst.append(args.getitem(0));
		lst.insert(0, args.getitem(0));

		lst.append(args[0]);
		lst.insert(0, args[0]);

		lst.append(0);
		if (lst[lst[lst.size()-1]]->obj != args[0]->obj) {
			iout << "lst[lst[-1]] != lst[0]" << std::endl;
			Py_RETURN_FALSE;
		}
	}

	Py_RETURN_TRUE;
}


PyObject* test_object(PyObject *m, PyObject *args_) {
	{
		iout << "------------------------------" << std::endl
		     << "Running basic test as generic borrowed reference object." << std::endl;
		cppy::Object<> args(args_);
		iout << "  size: " << args.size() << std::endl
		     << "  str : " << args.str() << std::endl
		     << "  repr: " << args.repr() << std::endl;
	}

	{
		iout << "------------------------------" << std::endl
		     << "Running basic test as generic owned reference object." << std::endl;
		cppy::Object<> borrowed(args_);
		cppy::Object<PyObject> owned(borrowed);
		iout << "  size: " << owned.size() << std::endl
		     << "  str : " << owned.str() << std::endl
		     << "  repr: " << owned.repr() << std::endl;
	}

	{
		iout << "------------------------------" << std::endl
		     << "indexing const generic borrowed reference." << std::endl;
		const cppy::Object<> args(args_);
		for (int i=0; i<args.size(); ++i)
		{ iout << "  " << i << ": " << args[i]->str() << std::endl; }
	}


	{
		iout << "------------------------------" << std::endl
		     << "indexing non-const generic borrowed reference." << std::endl;
		cppy::Object<> args(args_);
		for (int i=0; i<args.size(); ++i)
		{ iout << "  " << i << ": " << args[i]->str() << std::endl; }
	}
	Py_RETURN_TRUE;
}

PyObject* test_tuple(PyObject *m, PyObject *args_)
{
	{
		iout << "------------------------------" << std::endl
		     << "using a borrowed Tuple" << std::endl;
		cppy::TupleRef args(args_);
		for (int i=0; i<args.size(); ++i)
		{ iout << "  " << i << ": " << args[i]->str() << std::endl; }
	}

	{
		iout << "------------------------------" << std::endl
		     << "testing tuple creation" << std::endl;
		cppy::TupleRef args(args_);
		if (args.size() >= 3) {
			cppy::Tuple tmp(
				args[0]->obj,
				args[1]->obj,
				args[2]->obj
			);
			iout << "  " << tmp.str() << std::endl;
		}
		else {
			cppy::Tuple tmp(1, 2, 3);
			iout << "  " << tmp.str() << std::endl;
		}
	}

	{
		iout << "------------------------------" << std::endl
		     << "tuple size_+slice" << std::endl;
		cppy::TupleRef args(args_);
		if (args.check()) {
			iout << "  " << args.slice(1,args.size_()).str() << std::endl;
		}
	}

	Py_RETURN_TRUE;
}

PyObject* test_dict(PyObject *m, PyObject *args_) {
	cppy::TupleRef args(args_);
	cppy::DictRef dct(args[0]);
	iout << dct << std::endl;

	try {
		dct.setitem(1, 2);
		iout << dct << std::endl;
		//TODO Why mapping dct[1] causing some kind of error
		//but getitem(1) no error
		//auto thing = dct.getitem(1);
		//iout << thing << std::endl;
		iout << dct.getitem(1).str() << std::endl;
		//iout << dct[1]->str() << std::endl;
	}
	catch (cppy::Error&) {
		return NULL;
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
		{"test_dict", test_dict, METH_VARARGS, "Test dict."},
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
