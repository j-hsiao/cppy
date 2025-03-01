#ifndef CPPY_TUPLE_HPP
#define CPPY_TUPLE_HPP

#include "cppy/object.hpp"
#include "cppy/int.hpp"
#include <tuple>

namespace cppy
{
	struct Tuple_{};

	template<bool m=false>
	using Tuple = Object<Tuple_,m>;

	template<> struct Object<Tuple_, false>: public Object<>
	{
		using Object<>::Object;
		bool check() const { return PyTuple_Check(obj); }
		void checkthrow() const { throwifnot(check(), "tuple"); }

		Py_ssize_t size() const {
			Py_ssize_t ret = PyTuple_Size(obj);
			if (ret == -1) { throw PyError(); }
			return ret;
		}
		// Tuples return a borrowed reference
		Object<> operator[](Py_ssize_t pos) const
		{ return Object<>(success(PyTuple_GetItem(obj, pos))); }

		Object<> operator[](PyObject *pos) const
		{ return (*this)[static_cast<Py_ssize_t>(Object<int, false>(pos).checkthrow())]; }

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

	template<> struct Object<Tuple_, true>: Managed<Tuple_>
	{
		using Managed<Tuple_>::Managed;

		Object(Py_ssize_t length): Managed<Tuple_>(success(PyTuple_New(length))) {}
	};

}
#endif//CPPY_TUPLE_HPP
