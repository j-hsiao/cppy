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
				template<class Actual>
				ObjectWrap<Object<typename std::decay<Actual>::type>> operator()(Actual &&a) const
				{ return {std::forward<Actual>(a)}; }
			};
			//owned
			struct ConvertOwned {
				template<class T>
				ObjectWrap<Object<T>> operator()(Object<T> &&o) const { return {std::move(o)}; }

				template<class T>
				PyObject* operator()(const Object<T> &o) const { return o.obj; }
			};
			//borrowed
			struct ConvertBorrowed {
				template<class Actual>
				PyObject* operator()(Actual &&a) const { return a.obj; }
			};

			template<class Actual>
			struct Converter {
				using type = typename std::conditional<
					decltype(is<Object>(std::declval<Actual&&>()))::value,
					typename std::conditional<
						decltype(owned<Object>(std::declval<Actual&&>()))::value,
						ConvertOwned,
						ConvertBorrowed
						>::type,
					typename std::conditional<
						decltype(star_is<Object>(std::declval<Actual&&>()))::value,
						typename Converter<decltype(*istd::declval<Actual&&>())>::type,
						ConvertGeneric
						>::type
				>::type
			};
		public:
		PyObject* operator()(PyObject* p) const { return p; }

		template<class T>
		decltype(typename Converter<T&&>::type{}(std::declval<T&&>())) operator()(T &&t) const
		{ return Converter<T&&>{}(std::forward<T>(t)); }
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
