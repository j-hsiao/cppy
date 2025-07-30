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

		private:
			struct ConvertIntegral {
				template<class T>
				Py_ssize_t operator()(T &&t) const { return static_cast<Py_ssize_t>(t); }
			};

			struct ConvertObject {
				template<class T>
				Py_ssize_t operator()(const Object<T> &obj) const
				{ return Object<int&>(obj.obj); }
				Py_ssize_t operator()(const Object<int&> &obj) const
				{ return obj; }
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
				decltype(is<Object>(t))::value,
				ConvertObject,
				typename std::conditional<
					decltype(star_is<Object>(std::forward<T>(t)))::value,
					ConvertProxy,
					ConvertIntegral
				>::type
			>::type{}(std::forward<T>(t));
		}
	};

	template<template<class> class Object>
	struct IndexConverter {
		Py_ssize_t operator()(PyObject *obj) const
		{ return Object<int&>(obj).checkthrow(); }

		private:
			struct ConvertIntegral {
				template<class T>
				Py_ssize_t operator()(T &&t) const { return static_cast<Py_ssize_t>(t); }
			};

			struct ConvertObject {
				Py_ssize_t operator()(const Object<PyObject*&> &obj) const
				{ return Object<int&>(obj.obj).checkthrow(); }
				Py_ssize_t operator()(const Object<int&> &obj) const
				{ return obj.checkthrow(); }
			};

			struct ConvertProxy {
				template<class T>
				Py_ssize_t operator()(T &&t) const { return Object<int&>(t->obj).checkthrow(); }
			};
		public:
		template<class T>
		Py_ssize_t operator()(T &&t) const
		{
			return typename std::conditional<
				decltype(is<Object>(t))::value,
				ConvertObject,
				typename std::conditional<
					decltype(star_is<Object>(std::forward<T>(t)))::value,
					ConvertProxy,
					ConvertIntegral
				>::type
			>::type{}(std::forward<T>(t));
		}
	};

}
#endif//CPPY_CONVERT_INT_HPP
