//Conversion to PyObject*
#ifndef CPPY_CONVERT_PYOBJECT_HPP
#define CPPY_CONVERT_PYOBJECT_HPP

#include <cppy/errors.hpp>
#include <cppy/util.hpp>

#include <type_traits>
#include <utility>
#include <iostream>
namespace cppy {
	//keep PyObject* alive (Owned Object rvalue ref, prevent the decref)
	template<class T>
	struct ObjectWrap {
		T keepalive;
		operator PyObject*() const { return keepalive.obj; }
	};

	template<template<class> class Object>
	struct PyObjectConverter {
		private:
			//generic
			struct ConvertGeneric {
				template<class T>
				ObjectWrap<Object<typename std::decay<T>::type>> operator()(T &&o) const
				{ return {std::forward<T>(o)}; }
			};
			//owned
			struct ConvertOwned {
				template<class T>
				ObjectWrap<Object<T>> operator()(Object<T> &&o) const { return {std::move(o)}; }
				template<class T>
				ObjectWrap<Object<T>> operator()(const Object<T> &&o) const { return {std::move(o)}; }

				template<class T>
				PyObject* operator()(const Object<T> &o) const { return o.obj; }
			};
			//borrowed
			struct ConvertBorrowed {
				template<class T>
				PyObject* operator()(T &&o) const { return o.obj; }
			};

			template<class T>
			using ConvertObject = typename std::conditional<
				decltype(owned<Object>(std::declval<T&&>()))::value,
				ConvertOwned, ConvertBorrowed >::type;

			struct ConvertProxy {
				template<class T>
				decltype(ConvertObject<decltype(*std::declval<T&&>())>{}(*std::declval<T&&>()))
				operator()(T &&o) const {
					return ConvertObject<decltype(*o)>{}(*o);
				}
			};

			template<class T>
			using Converter = typename std::conditional<
				decltype(is<Object>(std::declval<T&&>()))::value,
				ConvertObject<T&&>,
				typename std::conditional<
					decltype(star_is<Object>(std::declval<T&&>()))::value,
					ConvertProxy, ConvertGeneric >::type
			>::type;
		public:
		PyObject* operator()(PyObject* p) const { return p; }

		template<class T>
		decltype(Converter<T>{}(std::declval<T&&>())) operator()(T &&t) const
		{ return Converter<T>{}(std::forward<T>(t)); }
	};

	//Steals reference from input if owned non-const rvalue.
	//Otherwise incref if applicable
	template<template<class> class Object>
	struct StealConverter {
		private:
			//If Borrowed, then must incref
			//If const owned, then must incref
			//If non-const owned, then steal whether rvalue or lvalue

			struct ConvertGeneric {
				//Generic object, use Object to convert to PyObject*
				template<class T> PyObject* operator()(T &&t) const
				{ return Object<typename std::decay<T&&>::type>(std::forward<T>(t)).ret(); }
			};

			struct ConvertObject {
				//const so cannot steal
				template<class T> PyObject* operator()(const Object<T> &o) const {
					Py_INCREF(o.obj);
					return o.obj;
				}
				template<class T> PyObject* operator()(Object<T&> &&o) const {
					Py_INCREF(o.obj);
					return o;
				}

				//Owned and can steal
				template<class T> PyObject* operator()(Object<T> &&o) const
				{ return o.ret(); }
			};

			struct ConvertProxy {
				template<class T> PyObject* operator()(T &&o) const
				{ return ConvertObject{}(*o); }
			};

			template<class T>
			using Converter = typename std::conditional<
				decltype(is<Object>(std::declval<T&&>()))::value,
				ConvertObject,
				typename std::conditional<
					decltype(star_is<Object>(std::declval<T&&>()))::value,
					ConvertProxy, ConvertGeneric
				>::type
			>::type;

		public:

		PyObject* operator()(PyObject* p) const {
			//Incref to ensure there is a ref that can be stolen.
			Py_INCREF(p);
			return p;
		}

		template<class T> PyObject* operator()(T &&t) const
		{ return Converter<T>{}(std::forward<T>(t)); }

	};

}
#endif//CPPY_CONVERT_PYOBJECT_HPP
