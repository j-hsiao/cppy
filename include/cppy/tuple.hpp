#ifndef CPPY_TUPLE_HPP
#define CPPY_TUPLE_HPP

#include "cppy/object.hpp"
#include "cppy/int.hpp"
#include "cppy/mixin.hpp"
#include "cppy/util.hpp"
#include <iostream>

#include <utility>

namespace cppy
{
	struct Tuple_{};

	template<bool m=false>
	using Tuple = Object<Tuple_,m>;

	template<> struct Object<Tuple_, false>: CheckThrow<Tuple_>, Make<Tuple_>, Object<>
	{
		using Make<Tuple_>::Make;
		using CheckThrow<Tuple_>::checkthrow;
		bool check() const { return PyTuple_Check(obj); }
		static constexpr const char* name() { return "Tuple"; }

		Py_ssize_t size() const {
			Py_ssize_t ret = PyTuple_Size(obj);
			if (ret == -1) { throw PyError(); }
			return ret;
		}
		// Tuples return a borrowed reference
		Object<> operator[](Py_ssize_t pos) const
		{ return Object<>(success(PyTuple_GetItem(obj, pos))); }

		Object<> operator[](PyObject *pos) const
		{ return (*this)[static_cast<Py_ssize_t>(Object<int>(pos).checkthrow())]; }

		// Represent an item at particular index of a tuple.
		struct TupleItem
		{
			Object<Tuple_, false> &tup;
			Py_ssize_t idx;

			template<class T=PyObject*>
			Object<T,false> obj() {
				Object<T, false> ret(success(PyTuple_GetItem(tup.obj, idx)));
				return ret;
			}

			template<class T>
			operator Object<T, false>() { return tup[idx]; }

			//NOTE: this steals a reference
			TupleItem& operator=(PyObject *obj)
			{
				if (PyTuple_SetItem(tup.obj, idx, obj) == -1) { throw PyError(); }
				return *this;
			}

			template<class T>
			TupleItem& operator=(Managed<T> &other)
			{
				//cannot use .ret() because if fail, then it gets cleared out...
				if (PyTuple_SetItem(tup.obj, idx, other.obj) == -1) { throw PyError(); }
				other.obj = nullptr;
				return *this;
			}

			template<class T>
			TupleItem& operator=(T &&item)
			{
				//cannot use .ret() because if fail, then it gets cleared out...
				Object<T, true> tmp(item);
				if (PyTuple_SetItem(tup.obj, idx, tmp.obj)  == -1) { throw PyError(); }
				tmp.obj = nullptr;
				return *this;
			}
		};
		//Assign values to index
		TupleItem operator()(Py_ssize_t pos) { return TupleItem{*this, pos}; }
	};

	template<> struct Object<Tuple_, true>: Managed<Tuple_>, Make<Tuple_, true>
	{
		using Base = Make<Tuple_, true>;
		using Base::Base;

		Object(Py_ssize_t length): Base(success(PyTuple_New(length))) {}

		//template<class...T>
		//Object(T&&...items):
		//	Object(sizeof...(T))
		//{ set(std::forward<T>(items)...); }
	};


	// Call a C++ callable by converting arguments from a python tuple.
	template<
		class Callable, class...Args,
		typename enabled<(sizeof...(Args) < function_signature<Callable>::arguments_type::size)>::type = true
	>
	typename function_signature<Callable>::return_type call(
		Callable &&callable, const Tuple<> &args, Args&&...converted)
	{
		return call(
			std::forward<Callable>(callable), args, std::forward<Args>(converted)...,
			Object<typename function_signature<Callable>::arguments_type::get<sizeof...(Args)>::type>(args[sizeof...(Args)]).checkthrow()
		);
	}

	template<
		class Callable, class...Args,
		typename enabled<sizeof...(Args) == function_signature<Callable>::arguments_type::size>::type = true
	>
	typename function_signature<Callable>::return_type call(
		Callable &&callable, const Tuple<> &args, Args&&...converted)
	{
		return callable(std::forward<Args>(converted)...);
	}
}
#endif//CPPY_TUPLE_HPP
