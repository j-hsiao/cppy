#ifndef CPPY_FLOAT_HPP
#define CPPY_FLOAT_HPP

#include "cppy/object.hpp"
#include "cppy/mixin.hpp"

namespace cppy
{
	template<class T> FloatConvert;
	template<> struct FloatConvert<float>
	{
		static float toc(PyObject *obj) { return PyFloat_AsDouble(obj); }
		static constexpr float badc() { return -1.0f; }
	};
	template<> struct Conversion<double>
	{
		static double toc(PyObject *obj) { return PyFloat_AsDouble(obj); }
		static constexpr double badc() { return -1.0; }
	};

	template<>
	struct Object<float>: Object<>, CheckThrow<float>, Convertible<float, FloatConvert>
	{
		using Object<>::Object;
		bool check() const { return PyFloat_Check(this->obj); }
		static constexpr const char* name() { return "float"; }
	};

	template<>
	struct Object<float, true>: Managed<PyObject*>
	{
		using Managed::Managed;

		Float(const char *val): Managed(success(PyFloat_FromString())) {}
		Float(double val): Managed(success(PyFloat_FromDouble(val))) {}
	};

	//In python, double/float are the same
	template<bool m> struct Object<double, m>: Object<float, m> { using Object<float, m>::Object; };

}
#endif//CPPY_FLOAT_HPP
