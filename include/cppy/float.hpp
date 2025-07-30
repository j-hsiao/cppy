#ifndef CPPY_FLOAT_HPP
#define CPPY_FLOAT_HPP

#include <cppy/object.hpp>
#include <cppy/string.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/mixin/convertible.hpp>

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
	template<class T> struct FloatConvert: FloatConvert<double>{};

	template<> struct Object<float&>:
		CheckThrow<Object<float&>>,
		Convertible<Object<float&>, FloatConvert>,
		Borrowed
	{
		using Borrowed::Borrowed;

		bool check() const { return PyFloat_Check(this->obj); }
		static constexpr const char* name() { return "float"; }

		operator double() const { return to<double>(); }
	};

	template<>
	struct Object<float>: Owned<float> {
		using Base = Owned<float>;
		using Base::Base;

		Object(const char *val): Base(success(PyFloat_FromString(Object<const char*>(val).obj))) {}
		template<class T>
		Object(T&&i): Object(static_cast<double>(i)) {}
		Object(float val): Base(success(PyFloat_FromDouble(val))) {}
		Object(double val): Base(success(PyFloat_FromDouble(val))) {}
	};

	template<> struct Object<double&>: Object<float&>{ using Object<float&>::Object; };
	template<> struct Object<double>: Object<float>{ using Object<float>::Object; };
}
#endif//CPPY_FLOAT_HPP
