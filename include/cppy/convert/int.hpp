#ifndef CPPY_CONVERT_INT_HPP
#define CPPY_CONVERT_INT_HPP

#include <cppy/int.hpp>
#include <cppy/util.hpp>
#include <type_traits>
namespace cppy {
	template<template<class> class Object>
	struct IntConverter {
		Py_ssize_t operator()(PyObject *obj) const
		{ return Object<int&>(obj); }

		//no need to use checkthrow() because conversion to int
		//would error or not
		template<class T>
		Py_ssize_t operator()(const Object<T> &obj) const
		{ return Object<int&>(obj.obj); }
		Py_ssize_t operator()(const Object<int&> &obj) const
		{ return obj; }

		private:
			struct ConvertIntegral {
				template<class T>
				Py_ssize_t operator()(T &&t) const { return static_cast<Py_ssize_t>(t); }
			};

			struct ConvertProxy {
				template<class T>
				Py_ssize_t operator()(T &&t) const { return Object<int&>(t->obj); }
			};
		public:
		template<class T>
		Py_ssize_t operator()(T &&t) const
		{
			return typename std::conditional<
				decltype(star_is<Object>(std::forward<T>(t)))::value,
				ConvertProxy,
				ConvertIntegral
			>::type{}(std::forward<T>(t));
		}
	};

	template<template<class> class Object>
	struct IndexConverter: IntConverter<Object> {
		using IntConverter<Object>::operator();
		int operator()(double) const = delete;
		int operator()(float) const = delete;
	};
}
#endif//CPPY_CONVERT_INT_HPP
