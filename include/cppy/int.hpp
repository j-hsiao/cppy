#ifndef CPPY_INT_HPP
#define CPPY_INT_HPP

#include "cppy/object.hpp"
#include "cppy/mixin.hpp"

namespace cppy
{
	template<class Type> struct IntConvert;
	template<> struct IntConvert<long>
	{
		static long toc(PyObject *obj) { return PyLong_AsLong(obj); }
		static constexpr long badc() { return -1l; }
	};
	template<> struct IntConvert<int>
	{
#if PY_MAJOR_VERSION >=3 && PY_MINOR_VERSION >= 13
		static int toc(PyObject *obj) { return PyLong_AsInt(obj); }
#else
		static int toc(PyObject *obj) { return PyLong_AsLong(obj); }
#endif
		static constexpr int badc() { return -1; }
	};
	template<> struct IntConvert<long long>
	{
		static long long toc(PyObject *obj) { return PyLong_AsLongLong(obj); }
		static constexpr long long  badc() { return -1ll; }
	};
	//template<> struct IntConvert<Py_ssize_t>
	//{
	//	static Py_ssize_t toc(PyObject *obj) { return PyLong_AsSsize_t(obj); }
	//	static constexpr Py_ssize_t badc() { return -1; }
	//};
	template<> struct IntConvert<unsigned long>
	{
		static unsigned long toc(PyObject *obj) { return PyLong_AsUnsignedLong(obj); }
		static constexpr unsigned long  badc() { return -1ul; }
	};
	//template<> struct IntConvert<std::size_t>
	//{
	//	static std::size_t toc(PyObject *obj) { return PyLong_AsSize_t(obj); }
	//	static constexpr std::size_t badc() { return v == std::numeric_limits<std::size_t>::max()-1; }
	//};
	template<> struct IntConvert<unsigned long long>
	{
		static unsigned long long toc(PyObject *obj) { return PyLong_AsUnsignedLongLong(obj); }
		static constexpr unsigned long long badc() { return -1ull; }
	};
	template<> struct IntConvert<void*>
	{
		static void* toc(PyObject *obj) { return PyLong_AsVoidPtr(obj); }
		static constexpr void* badc() { return NULL; }
	};

	template<> struct Object<int&>: Object<PyObject&>, CheckThrow<int&>, Convertible<int&, IntConvert>
	{
		using Object<PyObject&>::Object;

		bool check() const { return PyLong_Check(this->obj); }
		static constexpr const char* name() { return "int"; }
	};


	template<>
	struct Object<int>: Object<PyObject, int>
	{
		using Base = Object<PyObject, int>;
		using Base::Base;

		// Create a new int.
		Object(int val): Base(success(PyLong_FromLong(val)), false) {}
		Object(long val): Base(success(PyLong_FromLong(val)), false) {}
		Object(unsigned int val): Base(success(PyLong_FromUnsignedLong(val)), false) {}
		Object(unsigned long val): Base(success(PyLong_FromUnsignedLong(val)), false) {}
		//typedef/alias results in repeated definitions.
		//Object(Py_ssize_t val): Base(success(PyLong_FromSSize_t(val)), false) {}
		//Object(std::size_t val): Base(success(PyLong_FromSize_t(val)), false) {}
		Object(long long val): Base(success(PyLong_FromLongLong(val)), false) {}
		Object(unsigned long long val): Base(success(PyLong_FromUnsignedLongLong(val)), false) {}
		Object(double val): Base(success(PyLong_FromDouble(val)), false) {}
		Object(const char *str, int base=0): Base(success(PyLong_FromString(str, NULL, base)), false) {}
		Object(void *ptr): Base(success(PyLong_FromVoidPtr(ptr)), false) {}
	};

#define MAKE_CPPY_INT_TYPE(tp) \
	template<> struct Object<tp&>: Object<int&> { using Object<int&>::Object; }; \
	template<> struct Object<tp>: Object<int> { using Object<int>::Object; }


MAKE_CPPY_INT_TYPE(unsigned int);
MAKE_CPPY_INT_TYPE(short);
MAKE_CPPY_INT_TYPE(unsigned short);
MAKE_CPPY_INT_TYPE(long);
MAKE_CPPY_INT_TYPE(unsigned long);
MAKE_CPPY_INT_TYPE(long long);
MAKE_CPPY_INT_TYPE(unsigned long long);
MAKE_CPPY_INT_TYPE(void*);

#undef MAKE_CPPY_INT_TYPE
}
#endif//CPPY_INT_HPP
