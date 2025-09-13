#ifndef CPPY_TUPLE_HPP
#define CPPY_TUPLE_HPP

#include <cppy/errors.hpp>
#include <cppy/object.hpp>
#include <cppy/mixin/mapping.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/mixin/sized.hpp>
#include <cppy/proto/sequence.hpp>
#include <cppy/convert/int.hpp>
#include <cppy/proto/call.hpp>
#include <cppy/dict.hpp>
#include <cppy/util.hpp>
#include <iostream>

#include <utility>

namespace cppy
{
	struct Tuple_{};

	template<> struct Object<Tuple_&>:
		CheckThrow<Object<Tuple_&>>,
		Mapping<Object<Tuple_&>>,
		Sized<Object<Tuple_&>, PyTuple_Size>,
		PythonSequence<Object<Tuple_&>>,
		Borrowed
	{
		using Borrowed::Borrowed;
		using CheckThrow<Object<Tuple_&>>::checkthrow;
		using Mapping<Object<Tuple_&>>::operator[];
		using Sized<Object<Tuple_&>, PyTuple_Size>::size;

		bool check() const { return PyTuple_Check(obj); }
		static constexpr const char* name() { return "tuple"; }

		//no error checking.
		Py_ssize_t size_() const { return PyTuple_GET_SIZE(obj); }

		Object<Tuple_> slice(Py_ssize_t start, Py_ssize_t stop) const;

		// Tuples return a borrowed reference
		template<class Idx>
		Object<> getitem(Idx &&idx) const
		{ return Object<>(success(PyTuple_GetItem(obj, IntConverter<Object>{}(idx)))); }

		//setitem, steal when non-const Owned.  Otherwise, incref.
		template<class Idx, class Value>
		void setitem(Idx &&idx, Value &&value)
		{
			IntConverter<Object> intcvt;
			StealConverter<Object> stealcvt;
			if (PyTuple_SetItem(obj, intcvt(idx), stealcvt(value)) == -1)
			{ throw PythonError(); }
		}

		template<Py_ssize_t start=0, class First, class...Items>
		void setitems(First &&first, Items&&...items) {
			if (start + sizeof...(Items) + 1> size()) {
				throw IndexError("Setting too many items to a tuple.");
			}
			setitems_(std::forward<First>(first), std::forward<Items>(items)...);
		}
		private:
			template<Py_ssize_t pos=0, class First, class...Items>
			void setitems_(First &&first, Items&&...items)
			{
				setitem(pos, std::forward<First>(first));
				setitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos> void setitems_() {}
	};

	template<> struct Object<Tuple_>: Owned<Tuple_>
	{
		using Owned<Tuple_>::Owned;

		//NOTE: PyTuple_Pack exists, but it seems to incref everything, does not steal.
		//setitem, steal when non-const Owned.  Otherwise, incref.
		template<class...T>
		Object(T&&...items):
			Owned<Tuple_>(success(PyTuple_New(static_cast<Py_ssize_t>(sizeof...(T)))))
		{ setnewitems(std::forward<T>(items)...); }

		Object(): Owned<Tuple_>(success(PyTuple_New(static_cast<Py_ssize_t>(0)))) {}

		private:
			template<Py_ssize_t pos=0, class First, class...Items>
			void setnewitems(First &&first, Items&&...items) {
				StealConverter<Object> stealcvt;
				PyTuple_SET_ITEM(obj, pos, stealcvt(std::forward<First>(first)));
				setnewitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos=0> void setnewitems() {}
	};

	inline Object<Tuple_> Object<Tuple_&>::slice(Py_ssize_t start, Py_ssize_t stop) const
	{ return Object<Tuple_>(success(PyTuple_GetSlice(obj, start, stop))); }

	template<class T, template<class>class Object>
	Object<Tuple_> PythonSequence<Object<T&>>::tuple() const
	{ return success(PySequence_Tuple(static_cast<const Object<T&>*>(this)->obj)); }

	typedef Object<Tuple_> Tuple;
	typedef Object<Tuple_&> TupleRef;

	template<class T, template<class> class Object>
	template<class...Args>
	Object<PyObject> PythonCallable<Object<T>>::operator()(Args&&...args) const
	{ return call(Tuple(std::forward<Args>(args)...)); }

	template<class T, template<class> class Object>
	Object<PyObject> PythonCallable<Object<T>>::call(const Object<Tuple_&> &args) const
	{ return success(PyObject_CallObject(static_cast<const Object<T&>*>(this)->obj, args.obj)); }

	template<class T, template<class> class Object>
	Object<PyObject> PythonCallable<Object<T>>::call(const Object<Dict_&> &kwargs) const {
		static Tuple tup;
		return success(
			PyObject_Call(static_cast<const Object<T&>*>(this)->obj, tup.obj, kwargs.obj));
	}

	template<class T, template<class> class Object>
	Object<PyObject> PythonCallable<Object<T>>::call(
		const Object<Tuple_&> &args, const Object<Dict_&> &kwargs) const
	{
		return success(
			PyObject_Call(static_cast<const Object<T&>*>(this)->obj, args.obj, kwargs.obj));
	}

	// ------------------------------
	// Call a C++ functor by converting arguments from a python tuple.
	// ------------------------------
	template<
		int tupidx=0, class Callable, class...Args,
		typename std::enable_if<!decltype(is_callable(std::declval<Callable>(), std::declval<Args>()...))::value, bool>::type = true
	>
	typename function_signature<Callable>::return_type callcpp(
		Callable &&callable, const TupleRef &args, Args&&...converted)
	{
		assert(sizeof...(converted) < args.size());
		return callcpp<tupidx+1>(
			std::forward<Callable>(callable), args, std::forward<Args>(converted)...,
			Object<typename function_signature<Callable>::arguments_type::get<sizeof...(Args)>::type>(*args[tupidx]).checkthrow());
	}

	template<
		int tupidx=0, class Callable, class...Args,
		typename std::enable_if<decltype(is_callable(std::declval<Callable>(), std::declval<Args>()...))::value, bool>::type = true,
		typename std::enable_if<(sizeof...(Args) < function_signature<Callable>::arguments_type::count), bool>::type = true
	>
	typename function_signature<Callable>::return_type callcpp(
		Callable &&callable, const TupleRef &args, Args&&...converted)
	{
		if (args.size() == tupidx) {
			// if callable has default arguments, then sizeof...(Args) can be less than
			// the number of arguments that callable takes (Must be functor, functions don't store default
			// arguments in their signatures.)
			return callable(std::forward<Args>(converted)...);
		}
		else {
			return callcpp<tupidx+1>(
				std::forward<Callable>(callable), args, std::forward<Args>(converted)...,
				Object<typename function_signature<Callable>::arguments_type::get<sizeof...(Args)>::type>(*args[tupidx]).checkthrow());
		}
	}

	template<
		int tupidx=0, class Callable, class...Args,
		typename std::enable_if<decltype(is_callable(std::declval<Callable>(), std::declval<Args>()...))::value, bool>::type = true,
		typename std::enable_if<(sizeof...(Args) == function_signature<Callable>::arguments_type::count), bool>::type = true
	>
	typename function_signature<Callable>::return_type callcpp(
		Callable &&callable, const TupleRef &args, Args&&...converted)
	{
		if (tupidx != args.size())
		{ throw TypeError("Arguments do not match."); }
		return callable(std::forward<Args>(converted)...);
	}


	template<std::size_t N>
	struct Kwargs {
		Object<const char*> names[N];


	};


	template<class Callable, class...Names>
	typename function_signature<Callable>::return_type callcpp(
		Callable &&callable, const TupleRef &args, const DictRef &kwargs, Names&&...names)
	{
		if (args.size() == sizeof...(Names))
		{ return callcpp(std::forward<Callable>(callable)); }
		else if (args.size() > sizeof...(Names))
		{ throw TypeError("Too many arguments"); }
		else {
		}
	}

}
#endif//CPPY_TUPLE_HPP
