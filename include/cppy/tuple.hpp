#ifndef CPPY_TUPLE_HPP
#define CPPY_TUPLE_HPP

#include "cppy/object.hpp"
#include "cppy/int.hpp"
#include "cppy/mixin.hpp"
#include "cppy/util.hpp"

#include <utility>

namespace cppy
{
	struct Tuple_{};

	template<bool m=false>
	using Tuple = Object<Tuple_,m>;

	template<> struct Object<Tuple_, false>: Object<>, Make<Tuple_>, CheckThrow<Object<Tuple_>>
	{
		using Make<Tuple_>::Make;
		bool check() const { return PyTuple_Check(obj); }
		static constexpr const char* name() { return "tuple"; }

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

		struct TupAssigner
		{
			PyObject *pytup;
			Py_ssize_t idx;

			Object<> obj() const
			{ return Object<>(success(PyTuple_GetItem(pytup, idx))); }

			//NOTE: this steals a reference
			TupAssigner& operator=(PyObject *obj)
			{
				if (PyTuple_SetItem(pytup, idx, obj) == -1) { throw PyError(); }
				return *this;
			}

			template<class T>
			TupAssigner& operator=(Managed<T> &other)
			{
				if (PyTuple_SetItem(pytup, idx, other.obj) == -1) { throw PyError(); }
				other.obj = nullptr;
				return *this;
			}
		};
		TupAssigner operator()(Py_ssize_t pos) const
		{ return TupAssigner{obj, pos}; }
	};

	template<> struct Object<Tuple_, true>: Managed<Tuple_>, Make<Tuple_, true>
	{
		using Base = Make<Tuple_, true>;
		using Base::Base;

		Object(Py_ssize_t length): Base(success(PyTuple_New(length))) {}
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
