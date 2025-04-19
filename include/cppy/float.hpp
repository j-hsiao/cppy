#ifndef CPPY_FLOAT_HPP
#define CPPY_FLOAT_HPP

#include "cppy/object.hpp"
#include "cppy/mixin.hpp"

namespace cppy
{
	template<class T> struct FloatConvert;
	template<> struct FloatConvert<float>
	{
		static float toc(PyObject *obj) { return PyFloat_AsDouble(obj); }
		static constexpr float badc() { return -1.0f; }
	};
	template<> struct FloatConvert<double>
	{
		static double toc(PyObject *obj) { return PyFloat_AsDouble(obj); }
		static constexpr double badc() { return -1.0; }
	};

	template<>
	struct Object<float>: CheckThrow<float>, Convertible<float, FloatConvert>, Make<float>, Object<>
	{
		using Make<float>::Make;
		bool check() const { return PyFloat_Check(this->obj); }
		static constexpr const char* name() { return "float"; }
		using CheckThrow<float>::checkthrow;
	};

	template<>
	struct Object<float, true>: Managed<PyObject*>, Make<float, true>
	{
		using Base = Make<float, true>;
		using Base::Base;

		Object(const char *val): Base(success(PyFloat_FromString(Object<const char*, true>(val).obj))) {}
		Object(float val): Base(success(PyFloat_FromDouble(val))) {}
		Object(double val): Base(success(PyFloat_FromDouble(val))) {}
	};

	//In python, double/float are the same
	template<bool m> struct Object<double, m>: Object<float, m>, Make<double, m>
	{
		using Object<float, m>::Object;
		using Make<double, m>::Make;
	};

}
#endif//CPPY_FLOAT_HPP
