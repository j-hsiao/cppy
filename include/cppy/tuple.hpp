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
		template<class T=PyObject*>
		Object<T> getitem(Py_ssize_t pos) const
		{ return Object<>(success(PyTuple_GetItem(obj, pos))); }

		//setitem, steals a reference.
		void setitem(Py_ssize_t pos, PyObject *val)
		{ if (PyTuple_SetItem(obj, pos, val) == -1) { throw PyError(); } }
		//steal from a managed object.
		template<class T>
		void setitem(Py_ssize_t pos, Object<T,true> &&val)
		{
			if (PyTuple_SetItem(obj, pos, val.obj) == -1) { throw PyError(); }
			else { val.obj = nullptr; }
		}
		//incref from any other Object<> instance type.
		template<class T, bool b>
		void setitem(Py_ssize_t pos, const Object<T,b> &val)
		{
			Py_INCREF(val.obj);
			if (PyTuple_SetItem(obj, pos, val.obj) == -1) { throw PyError(); }
		}
		template<class T, bool b>
		void setitem(Py_ssize_t pos, Object<T,b> &val)
		{ setitem(pos, const_cast<const Object<T,b>&>(val)); }
		//Generic convert into pyobject.
		template<class T>
		void setitem(Py_ssize_t pos, T &&t)
		{ setitem(pos, Object<T,true>(std::forward<T>(t))); }

		template<Py_ssize_t pos=0, class First, class...Items>
		void setitems(First &&first, Items&&...items)
		{
			setitem(pos, Object<First, true>(first));
			setitems<pos+1>(std::forward<Items>(items)...);
		}
		template<Py_ssize_t pos=0> void setitems() {}

		// Represent an item at particular index of a tuple.
		typedef ItemProxy<Object<Tuple_, false>, Py_ssize_t> TupleItem;

		//Assign values to index
		TupleItem operator[](Py_ssize_t pos) { return TupleItem(*this, pos); }
		const TupleItem operator[](Py_ssize_t pos) const { return TupleItem(*this, pos); }
	};

	template<> struct Object<Tuple_, true>: Managed<Tuple_>, Make<Tuple_, true>
	{
		using Base = Make<Tuple_, true>;
		using Base::Base;

		template<class...T, Py_ssize_t pos=0>
		Object(T&&...items):
			Base(success(PyTuple_New(static_cast<Py_ssize_t>(sizeof...(T)))))
		{ setnewitems(std::forward<T>(items)...); }

		private:
			template<Py_ssize_t pos=0, class First, class...Items>
			void setnewitems(First &&first, Items&&...items)
			{
				//It seems PyTuple_SET_ITEM is a macro so it thinks
				//Object<First, true>... is 2 arguments must separate it out.
				Object<First, true> tmp(std::forward<First>(first));
				PyTuple_SET_ITEM(obj, pos, tmp.ret());
				setnewitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos=0, class First, class...Items>
			void setnewitems(Object<First,true> &&first, Items&&...items)
			{
				PyTuple_SET_ITEM(obj, pos, first.ret());
				setnewitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos=0, class First, bool b, class...Items>
			void setnewitems(const Object<First,b> &first, Items&&...items)
			{
				Py_INCREF(first.obj);
				PyTuple_SET_ITEM(obj, pos, first.obj);
				setnewitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos=0, class First, bool b, class...Items>
			void setnewitems(Object<First,b> &first, Items&&...items)
			{
				setnewitems(
					const_cast<const Object<First,b>&>(first),
					std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos=0> void setnewitems() {}
	};


	// Call a C++ functor by converting arguments from a python tuple.
	template<
		class Callable, class...Args,
		typename enabled<(sizeof...(Args) < function_signature<Callable>::arguments_type::count)>::type = true
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
		typename enabled<sizeof...(Args) == function_signature<Callable>::arguments_type::count>::type = true
	>
	typename function_signature<Callable>::return_type call(
		Callable &&callable, const Tuple<> &args, Args&&...converted)
	{
		return callable(std::forward<Args>(converted)...);
	}


	template<class...Args>
	PyObject* callpy(PyObject *callable, Args&&...args)
	{
		Tuple<true> tupargs(std::forward<Args>(args)...);
		return success(PyObject_Call(callable, tupargs.obj, NULL));
	}


}
#endif//CPPY_TUPLE_HPP
