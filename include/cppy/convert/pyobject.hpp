//Conversion to PyObject*
#ifndef CPPY_CONVERT_PYOBJECT_HPP
#define CPPY_CONVERT_PYOBJECT_HPP

#include <cppy/errors.hpp>
#include <cppy/util.hpp>

#include <type_traits>
#include <utility>
#include <iostream>
namespace cppy {
	//keep PyObject* alive until used.
	template<class T>
	struct ObjectWrap {
		T keepalive;
		operator PyObject*() const { return keepalive.obj; }
	};

	template<template<class> class Object>
	struct PyObjectConverter {
		private:
			template<class IsObject, int dummy=0> struct Converter {
				template<class T>
				decltype(Converter<decltype(is<Object>(*std::declval<T&&>()))>{}(std::declval<T&&>()))
				ObjectWrap<Object<typename std::decay<T>::type>> operator()(T &&t) const
				{ return Converter<decltype(is<Object>(*std::declval<T&&>()))>{}(std::forward<T>(t)); }

				template<class StarOpIsObject, int dummy=0>
				struct Converter {
					template<class T>
					ObjectWrap<Object<typename std::decay<T>::type>> operator()(T &&t) const
					{ return ObjectWrap<Object<typename std::decay<T>::type>>{std::forward<T>(t)}; }
				};

				template<int dummy> struct Converter<std::true_type, dummy> {
					template<class T>
					ObjectWrap<decltype(*declval<T&&>())> operator()(T &&t) const
					{ return ObjectWrap<decltype(*declval<T&&>())>{*t}; }
				};
			};

			template<int dummy> struct Converter<std::true_type, dummy> {
				template<class T> PyObject* operator()(const Object<T&> &o) const { return o.obj; }

				template<class T> PyObject* operator()(Object<T&> &&o) const { return o.obj; }
				template<class T>
				ObjectWrap<Object<T>> operator()(Object<T> &&o) const { return ObjectWrap<Object<T>>{std::move(o)}; }
			};

		public:
		PyObject* operator()(PyObject* p) const { return p; }

		template<class T>
		decltype(Converter<decltype(is<Object>(std::declval<T&&>()))>{}(std::declval<T&&>()))
		operator()(T &&t) const
		{ return Converter<decltype(is<Object>(std::forward<T>(t)))>{}(std::forward<T>(t)); }
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

			template<class IsObject, int dummy=0> struct Converter {
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

				template<class T> PyObject* operator()(Object<T> &&owned) const
				{ return owned.ret(); }

				//Borrowed refence has no actual reference to steal so incref.
				template<class T> PyObject* operator()(Object<T&> &&borrowed) const {
					Py_INCREF(borrowed.obj);
					return borrowed.obj;
				}
			};
	};

}
#endif//CPPY_CONVERT_PYOBJECT_HPP
