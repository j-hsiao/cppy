#include <cppy/dict.hpp>
#include <cppy/list.hpp>
#include <cppy/tuple.hpp>
#include <cppy/string.hpp>
#include <cppy/float.hpp>
#include <cppy/int.hpp>
#include <cppy/object.hpp>
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

#define CHECK(...) \
	{ \
		iout << "  " #__VA_ARGS__ ": "; \
		if (!(__VA_ARGS__)) { \
			iout << "fail" << std::endl; \
			Py_RETURN_FALSE; \
		} \
		else { iout << "pass" << std::endl; } \
	}

//Conversion, It seems that conversion operators
//are "ambiguous" regardless of what woud be seemingly best
//ex: comparison to an int can be int == double, int == int, int == float,
//etc if those conversion operators are defined.  As a result,
//probably only a single implicit conversion should be allowed.
PyObject* test_convert(PyObject *m, PyObject *args_) {
	{
		cppy::Object<float> floatfromdouble(3.14);
		cppy::Object<float> floatfromfloat(32.4f);
		cppy::Object<float> floatfromint(3);

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
		     << "list creation" << std::endl;
		cppy::List lst("hello", 72);
		CHECK(lst.str().string() == "['hello', 72]")
		CHECK(lst.size() == 2)
		CHECK(lst.size_() == 2)
		iout << "------------------------------" << std::endl
		     << "list to tuple" << std::endl;
		CHECK(lst.tuple().str().string() == "('hello', 72)")
		CHECK(lst.tuple().size() == 2)
		iout << "------------------------------" << std::endl
		     << "extend" << std::endl;
		lst.extend(lst.tuple());
		CHECK(lst.str().string() == "['hello', 72, 'hello', 72]")
		CHECK(lst.size() == 4)
		CHECK(lst.size_() == 4)
		iout << "------------------------------" << std::endl
		     << "slice" << std::endl;
		CHECK(lst.slice(1,3).size() == 2)
		CHECK(lst.slice(1,3).size_() == 2)
		CHECK(lst[1]->obj == lst.slice(1,3)[0]->obj)
		CHECK(lst[2]->obj == lst.slice(1,3)[1]->obj)
		CHECK(lst.getitem(1).obj == lst.slice(1,3)[0]->obj)
		CHECK(lst.getitem(2).obj == lst.slice(1,3)[1]->obj)
		iout << "------------------------------" << std::endl
		     << "set item" << std::endl;
		lst.setitem(0, "hello");
		lst[1] = 3.1415926;
		CHECK(lst.size() == 4)
		CHECK(lst.size_() == 4)
		iout << lst[0] << std::endl
		     << lst[1] << std::endl;
		CHECK(lst[0]->repr().string() == "'hello'")
		CHECK(lst[1]->as<float&>() == 3.1415926)

		iout << "------------------------------" << std::endl
		     << "set items" << std::endl
		     << lst << std::endl;
		lst.setitems<2>(1,2);
		iout << lst << std::endl;
		CHECK(lst.size() == 4)
		CHECK(lst.size_() == 4)
		try {
			CHECK(lst[2]->as<int&>() == 1)
			CHECK(lst[3]->as<int&>() == 2)
		}
		catch (cppy::Error &e) { return NULL; }

		lst.setitems(0, "1", "2");
		CHECK(lst.size() == 4)
		CHECK(lst.size_() == 4)
		CHECK(lst[0]->as<const char*&>().string() == "1")
		CHECK(lst[1]->as<const char*&>().string() == "2")
		iout << lst << std::endl;

		iout << "------------------------------" << std::endl
		     << "clear" << std::endl;
		lst.clear();
		CHECK(lst.str().string() == "[]")
		CHECK(lst.size() == 0)
		CHECK(lst.size_() == 0)
	}
	{
		cppy::TupleRef args(args_);
		if (args.size() >= 2) {
			iout << "------------------------------" << std::endl
			     << "append Object<>" << std::endl;
			cppy::List lst;
			CHECK(lst.size() == 0)
			lst.append(args.getitem(0));
			CHECK(lst.size() == 1)
			CHECK(lst[0]->obj == args[0]->obj && lst[0]->obj)
			lst.append(args[0]);
			CHECK(lst.size() == 2)
			CHECK(lst[1]->obj == args[0]->obj && lst[1]->obj)

			iout << "------------------------------" << std::endl
			     << "insert Object<>" << std::endl;
			lst.insert(0, args.getitem(1));
			CHECK(lst.size() == 3)
			CHECK(lst[0]->obj == args[1]->obj && lst[0]->obj)

			lst.insert(0, args[1]);
			CHECK(lst.size() == 4)
			CHECK(lst[0]->obj == args[1]->obj && lst[0]->obj)

			iout << "------------------------------" << std::endl
			     << "append int" << std::endl;
			lst.append(1234);
			CHECK(lst.size() == 5)
			CHECK(lst[lst.size()-1]->as<int&>() == 1234)
			iout << "------------------------------" << std::endl
			     << "insert float" << std::endl;
			lst.insert(1, 4321.1234);
			CHECK(lst.size() == 6)
			CHECK(lst[1]->as<float&>() == 4321.1234)
		}
	}
	Py_RETURN_TRUE;
}


PyObject* test_object(PyObject *m, PyObject *args_) {
	{
		iout << "------------------------------" << std::endl
		     << "Running basic test as generic borrowed reference object." << std::endl;
		cppy::Object<> args(args_);
		CHECK(args.size() == 5)
		CHECK(args.str() == "([slice(10, 30, None)], (3.14, 101), 'hello', None, MyThing(32))")
		CHECK(args.repr() == "([slice(10, 30, None)], (3.14, 101), 'hello', None, MyThing(32))")
		CHECK(static_cast<bool>(args.size()) == static_cast<bool>(args))
	}

	{
		iout << "------------------------------" << std::endl
		     << "Running basic test as generic owned reference object." << std::endl;
		cppy::Object<> borrowed(args_);
		cppy::Object<PyObject> owned(borrowed);
		CHECK(owned.size() == 5)
		CHECK(owned.str() == "([slice(10, 30, None)], (3.14, 101), 'hello', None, MyThing(32))")
		CHECK(owned.repr() == "([slice(10, 30, None)], (3.14, 101), 'hello', None, MyThing(32))")
		CHECK(static_cast<bool>(owned.size()) == static_cast<bool>(owned))
	}

	{
		iout << "------------------------------" << std::endl
		     << "indexing const generic borrowed reference." << std::endl;
		const cppy::ObjRef args(args_);
		CHECK(args[0]->str() == "[slice(10, 30, None)]")
		CHECK(args[0]->repr() == "[slice(10, 30, None)]")
		CHECK(!args[0]->is_none())
		CHECK(args[1]->str()  == "(3.14, 101)")
		CHECK(args[1]->repr() == "(3.14, 101)")
		CHECK(!args[1]->is_none())
		CHECK(args[2]->str()  == "hello")
		CHECK(args[2]->repr() == "'hello'")
		CHECK(!args[2]->is_none())
		CHECK(args[3]->str()  == "None")
		CHECK(args[3]->repr() == "None")
		CHECK(args[3]->is_none())
		CHECK(args[4]->str()  == "MyThing(32)")
		CHECK(args[4]->repr() == "MyThing(32)")
		CHECK(!args[4]->is_none())

		CHECK(args.getitem(0).str() == "[slice(10, 30, None)]")
		CHECK(args.getitem(0).repr() == "[slice(10, 30, None)]")
		CHECK(!args.getitem(0).is_none())
		CHECK(args.getitem(1).str()  == "(3.14, 101)")
		CHECK(args.getitem(1).repr() == "(3.14, 101)")
		CHECK(!args.getitem(1).is_none())
		CHECK(args.getitem(2).str()  == "hello")
		CHECK(args.getitem(2).repr() == "'hello'")
		CHECK(!args.getitem(2).is_none())
		CHECK(args.getitem(3).str()  == "None")
		CHECK(args.getitem(3).repr() == "None")
		CHECK(args.getitem(3).is_none())
		CHECK(args.getitem(4).str()  == "MyThing(32)")
		CHECK(args.getitem(4).repr() == "MyThing(32)")
		CHECK(!args.getitem(4).is_none())

		iout << "------------------------------" << std::endl
		     << "getattr" << std::endl;

		CHECK(args[4]->attr("value").as<int&>() == 32)
	}

	{
		iout << "------------------------------" << std::endl
		     << "generic setitem" << std::endl;
		cppy::List lst(1,2,3);
		CHECK(lst.size() == 3)
		cppy::Object<> obj(lst);
		obj[0] = 32;
		CHECK(obj.str() == "[32, 2, 3]")
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
		iout << *dct[1] << std::endl;
		iout << dct.getitem(1) << std::endl;
		iout << dct[1] << std::endl;
		auto result = dct.getitem(69, nullptr);
		if (result.obj) { Py_RETURN_FALSE; }
	}
	catch (cppy::Error&) {
		return NULL;
	}
	try {
		*dct[0xFFFFFFFFFFFFFFFF];
		Py_RETURN_FALSE;
	}
	catch (cppy::Error &e) { e.clear(); }





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
