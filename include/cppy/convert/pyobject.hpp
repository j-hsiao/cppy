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
		Object<T> keepalive;
		operator PyObject*() const { return keepalive.obj; }
	};

	template<template<class> class Object>
	struct PyObjectConverter {

		PyObject* operator()(PyObject* p) const { return p; }


		template<class T> PyObject* operator()(const Object<T> &o) const { return o.obj; }
		template<class T> PyObject* operator()(Object<T> &o) const { return o.obj; }
		template<class T> ObjectWrap<T> operator()(Object<T> &&o) const { return std::move(o); }
		template<class T> ObjectWrap<T> operator()(const Object<T> &&o) const { return std::move(o); }

		template<class T> ObjectWrap<typename std::decay<T>::type> operator()(T &&t) const
		{ return ObjectWrap<typename std::decay<T>::type>{std::forward<T>(t)}; }
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
		{ return Converter<typename std::decay<T&&>::type>{}(std::forward<T>(t)); }

		private:
			//If Borrowed, then must incref
			//If const owned, then must incref
			//If non-const owned, then steal whether rvalue or lvalue

			template<class T>
			struct Converter {
				//Generic object, use Object to convert to PyObject*
				template<class V>
				PyObject* operator()(V &&v) const { return Object<T>(std::forward<V>(v)).ret(); }
			};

			template<class T>
			struct Converter<Object<T>> {
				PyObject* operator()(const Object<T&> &owned) const {
					Py_INCREF(owned.obj);
					return owned.obj;
				}
				PyObject* operator()(Object<T&> &owned) const { return owned.obj; }
				PyObject* operator()(Object<T&> &&owned) const { return owned.obj; }
			};

			template<class T>
			struct Converter<Object<T&>> {
				PyObject* operator()(const Object<T&> &borrowed) const {
					Py_INCREF(borrowed.obj);
					return borrowed.obj;
				}
			};
	};

}
#endif//CPPY_CONVERT_PYOBJECT_HPP
