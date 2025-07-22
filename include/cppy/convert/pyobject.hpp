//Conversion to PyObject*
#ifndef CPPY_CONVERT_PYOBJECT_HPP
#define CPPY_CONVERT_PYOBJECT_HPP

#include <cppy/errors.hpp>
#include <cppy/util.hpp>

#include <type_traits>
#include <utility>
namespace cppy {
	template<class T>
	struct ObjectWrap {
		T keepalive;
		operator PyObject*() const { return keepalive.obj; }
	};

	template<template<class> class Object>
	struct PyObjectConverter {

		PyObject* operator()(PyObject* p) const { return p; }


		template<class T> PyObject* operator()(const Object<T> &o) const { return o.obj; }
		template<class T> PyObject* operator()(Object<T> &o) const { return o.obj; }
		template<class T> ObjectWrap<Object<T>> operator()(Object<T> &&o) const { return std::move(o); }
		template<class T> ObjectWrap<Object<T>> operator()(const Object<T> &&o) const { return std::move(o); }

		template<class T> ObjectWrap<Object<typename std::decay<T>::type>> operator()(T &&t) const
		{ return ObjectWrap<Object<typename std::decay<T>::type>>{std::forward<T>(t)}; }
	};

	//Steals reference from input.
	template<template<class> class Object>
	struct StealConverter {
		PyObject* operator()(PyObject* p) const {
			//give a ref to steal
			Py_INCREF(p);
			return p;
		}

		template<class T> PyObject* operator()(T &&t) const
		{ return Converter<decltype(is<Object>(std::forward<T>(t)))>{}(std::forward<T>(t)); }

		private:
			//If Borrowed, then must incref
			//If const owned, then must incref
			//If non-const owned, then steal whether rvalue or lvalue

			template<class IsObject, int dummy=0> struct Converter;

			template<int dummy>
			struct Converter<std::false_type, dummy> {
				//Generic object, use Object to convert to PyObject*
				template<class T> PyObject* operator()(T &&t) const
				{ return Object<typename std::decay<T&&>::type>(std::forward<T>(t)).ret(); }
			};

			template<int dummy>
			struct Converter<std::true_type, dummy> {
				template<class T>
				PyObject* operator()(const Object<T> &owned) const {
					Py_INCREF(owned.obj);
					return owned.obj;
				}
				template<class T> PyObject* operator()(Object<T> &&owned) const { return owned.ret(); }

				template<class T> PyObject* operator()(const Object<T&> &borrowed) const {
					Py_INCREF(borrowed.obj);
					return borrowed.obj;
				}
			};
	};

}
#endif//CPPY_CONVERT_PYOBJECT_HPP
