//base clase for iterators
#ifndef CPPY_PROTO_ITERATOR_HPP
#define CPPY_PROTO_ITERATOR_HPP
#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>

namespace cppy {
	template<template<class>class Object>
	struct Iterator {
		PyObject *pyit;
		Object<PyObject> obj;

		Iterator():
			pyit(nullptr),
			obj()
		{}

		Iterator(PyObject *pyit):
			pyit(pyit),
			obj(PyIter_Next(pyit))
		{}

		Object<PyObject>& operator*() { return obj; }

		bool operator==(const Iterator &it)
		{ return it.obj.obj == obj.obj; }
		bool operator!=(const Iterator &it)
		{ return it.obj.obj != obj.obj; }

		Iterator& operator++() {
			obj = Object<PyObject>(PyIter_Next(pyit));
			if (!obj.obj && PyErr_Occurred()) { throw PythonError(); }
			return *this;
		}
	};


	template<class Base> struct PythonIterator;
	template<class Base> struct PythonAIterator;

	template<class T, template<class>class Object> struct PythonIterator<Object<T&>>:
		CheckThrow<PythonIterator<Object<T&>>>
	{
		using Base = Object<T&>;

		PythonIterator<Base>&& iter() && { return std::move(*this); }
		PythonIterator<Base>& iter() & { return *this; }
		const PythonIterator<Base>& iter() const& { return *this; }

		bool check() const
		{ return PyIter_Check(static_cast<const Base*>(this)->obj); }

		Object<PyObject> next() {
			if (PyObject *ret = PyIter_Next(static_cast<Base*>(this)->obj))
			{ return Object<PyObject>(ret); }
			else if (PyErr_Occurred()) { throw PythonError(); }
			else { return Object<PyObject>(nullptr); }
		}
	};

	template<class T, template<class>class Object> struct PythonAIterator<Object<T&>>:
		CheckThrow<PythonAIterator<Object<T&>>>
	{
		using Base = Object<T&>;

		PythonAIterator<Base>&& aiter() && { return std::move(*this); }
		PythonAIterator<Base>& aiter() & { return *this; }
		const PythonAIterator<Base>& aiter() const& { return *this; }

		bool check() const
		{ return PythonAIter_Check(static_cast<const Base*>(this)->obj); }
	};
}
#endif//CPPY_PROTO_ITERATOR_HPP
