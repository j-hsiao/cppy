#ifndef CPPY_CONVERT_INT_HPP
#define CPPY_CONVERT_INT_HPP

#include <cppy/int.hpp>
#include <type_traits>
namespace cppy {
	template<template<class> class Object>
	struct IntConverter {
		Py_ssize_t operator()(PyObject *obj) const
		{ return Object<int&>(obj).checkthrow(); }

		template<class T>
		Py_ssize_t operator()(const Object<T> &obj) const
		{ return Object<int&>(obj.obj).checkthrow(); }

		Py_ssize_t operator()(const Object<int&> &obj) const
		{ return obj; }

		template<class T>
		Py_ssize_t operator()(T &&t) const
		{ return static_cast<Py_ssize_t>(t); }
	};
}
#endif//CPPY_CONVERT_INT_HPP
