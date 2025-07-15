#ifndef CPPY_CONVERT_PYOBJECT_HPP
#define CPPY_CONVERT_PYOBJECT_HPP

#include <cppy/errors.hpp>
#include <utility>
namespace cppy {


	template<template<class> class Object>
	struct PyObjectConverter {

		PyObject* operator()(PyObject* p) const { return p; }

		template<class T>
		struct Wrap {
			Object<T> keepalive;
			operator PyObject*() const { return keepalive.obj; }
		};

		template<class T> PyObject* operator()(const Object<T> &o) const
		{ return o.obj; }
		template<class T> PyObject* operator()(Object<T> &o) const
		{ return o.obj; }
		template<class T> Wrap<T> operator()(Object<T> &&o) const
		{ return std::move(o); }
		template<class T> Wrap<T> operator()(const Object<T> &&o) const
		{ return std::move(o); }

		template<class T> Wrap<typename std::remove_reference<T>::type> operator()(T &&t) const
		{ return Wrap<typename std::remove_reference<T>::type>{std::forward<T>(t)}; }
	};
}
#endif//CPPY_CONVERT_PYOBJECT_HPP
