#ifndef CPPY_CONVERTER_HPP
#define CPPY_CONVERTER_HPP

#include <cppy/int.hpp>
#include <cppy/float.hpp>
#include <cppy/string.hpp>

namespace cppy
{

	template<class tp, template<bool managed> class Obj>
	struct ObjectConverter
	{
		static tp toc(PyObject *obj) { return Obj<false>(obj); }
		static Obj topy(tp item) { return Obj<true>(item).ret(); }
	};


	template<class tp>
	struct Converter;

	template<> struct Converter<int>: public ObjectConverter<int, Int> {};
	template<> struct Converter<float>: public ObjectConverter<float, Float> {};
	template<> struct Converter<double>: public ObjectConverter<double, Float> {};
	template<> struct Converter<const char*>: public ObjectConverter<const char*, String> {};

}
#endif//CPPY_CONVERTER_HPP
