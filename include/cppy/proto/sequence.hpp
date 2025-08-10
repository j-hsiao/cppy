//Base class for sequences
#ifndef CPPY_PROTO_SEQUENCE_HPP
#define CPPY_PROTO_SEQUENCE_HPP
#include <cppy/errors.hpp>
#include <cppy/object.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/convert/int.hpp>
#include <cppy/convert/pyobject.hpp>
#include <type_traits>


namespace cppy {
	template<class Base> struct PySequence;

	struct List_;
	struct Tuple_;


	template<class T, template<class>class Object>
	struct PySequence<Object<T&>>: CheckThrow<PySequence<Object<T&>>> {
		using Base = Object<T&>;

		PySequence<Object<T&>>&& sequence() && { return std::move(*this); }
		PySequence<Object<T&>>& sequence() & { return *this; }
		const PySequence<Object<T&>>& sequence() const& { return *this; }

		bool check() const
		{ return PySequence_Check(static_cast<const Base*>(this)->obj); }

		Py_ssize_t size() const
		{
			Py_ssize_t length = PySequence_Size(static_cast<const Base*>(this)->obj);
			if (length < 0) { throw PyError(); }
			return length;
		}

		//------------------------------
		// +
		//------------------------------
		template<class V=T, class Actual>
		Object<V> operator+(Actual &&actual) const {
			PyObject *ret = PySequence_Concat(
				static_cast<const Base*>(this)->obj,
				PyObjectConverter<Object>{}(std::forward<Actual>(actual)));
			return Object<V>(success(ret));
		}

		template<class V=T, class Actual>
		Object<V> operator*(Actual &&actual) const {
			PyObject *ret = PySequence_Repeat(
				static_cast<const Base*>(this)->obj,
				IntConverter<Object>{}(std::forward<Actual>(actual)));
			return Object<V>(success(ret));
		}

		//------------------------------
		//inplace modifiers
		//------------------------------
		//Not all sequences support inplace modification.  In the case
		//where it is not supported, then a new object is returned which is owned.
		//The actual input sequence might or might not be owned, so it would
		//be unsafe to just swap out the pointer.  As a result, these methods
		//return a new owned object reference.
		// ie. (a+=b).obj != a.obj if a does not support in-place +=.
		// If it is supported, then (a+=b).obj == a.obj, and a += b return value
		// would just decref the extra incref caused by the pysequence method.
		// So be aware that if inplace modifications are not supported, the actual
		// values WILL NOT BE MODIFIED, the return value of the operator would
		// need to be saved.
		template<class V=T, class Actual>
		Object<V> operator+=(Actual &&actual) {
			PyObject *ret = PySequence_InPlaceConcat(
				static_cast<const Base*>(this)->obj,
				PyObjectConverter<Object>{}(std::forward<Actual>(actual)));
			return Object<V>(success(ret));
		}

		template<class V=T, class Actual>
		Object<V> operator*=(Actual &&actual) const {
			PyObject *ret = PySequence_InPlaceRepeat(
				static_cast<const Base*>(this)->obj,
				IntConverter<Object>{}(std::forward<Actual>(actual)));
			return Object<V>(success(ret));
		}

		//------------------------------
		//item access
		//------------------------------
		template<class Actual>
		Object<PyObject> getitem(Actual &&actual) {
			PyObject *item = PySequence_GetItem(
				static_cast<const Base*>(this)->obj,
				IntConverter<Object>{}(std::forward<Actual>(actual)));
			return Object<PyObject>(success(item));
		}
		template<class Idx, class Actual>
		Object<T&>& setitem(Idx &&idx, Actual &&actual) {
			if (PySequence_SetItem(
				static_cast<Base*>(this)->obj,
				IntConverter<Object>{}(std::forward<Idx>(idx)),
				PyObjectConverter<Object>{}(std::forward<Actual>(actual))))
			{ throw PyError(); }
			return *this;
		}
		template<class Idx>
		Object<T&>& delitem(Idx &&idx) {
			if (PySequence_SetItem(
				static_cast<Base*>(this)->obj,
				IntConverter<Object>{}(std::forward<Idx>(idx))))
			{ throw PyError(); }
			return *this;
		}

		//------------------------------
		//slice access
		//------------------------------
		template<class Start, class Stop>
		Object<PyObject> slice(Start &&start, Stop &&stop) const {
			IntConverter<Object> cvt;
			PyObject *result = PySequence_GetSlice(
				static_cast<const Base*>(this)->obj,
				cvt(std::forward<Start>(start)),
				cvt(std::forward<Stop>(stop)));
			return Object<PyObject>(success(result));
		}

		template<class Start, class Stop, class Seq>
		Object<T&>& setslice(Start &&start, Stop &&stop, Seq &&seq) {
			IntConverter<Object> icvt;
			if (PySequence_SetSlice(
				static_cast<Base*>(this)->obj,
				icvt(std::forward<Start>(start)),
				icvt(std::forward<Stop>(stop)),
				PyObjectConverter<Object>{}(std::forward<Seq>(seq))))
			{ throw PyError(); }
			return *this;
		}

		template<class Start, class Stop>
		Object<T&>& delslice(Start &&start, Stop &&stop) {
			IntConverter<Object> icvt;
			if (PySequence_DelSlice(
				static_cast<Base*>(this)->obj,
				icvt(std::forward<Start>(start)),
				icvt(std::forward<Stop>(stop))))
			{ throw PyError(); }
			return *this;
		}


		template<class Actual>
		Py_ssize_t count(Actual &&actual) const {
			Py_ssize_t result = PySequence_Count(
				static_cast<const Base*>(this)->obj,
				PyObjectConverter<Object>{}(std::forward<Actual>(actual)));
			if (result < 0) { throw PyError(); }
			return result;
		}

		template<class Actual>
		bool contains(Actual &&actual) const {
			int result = PySequence_Contains(
				static_cast<const Base*>(this)->obj,
				PyObjectConverter<Object>{}(std::forward<Actual>(actual)));
			if (result < 0) { throw PyError(); }
			return result;
		}

		template<class Actual>
		Py_ssize_t index(Actual &&actual) const {
			Py_ssize_t result = PySequence_Index(
				static_cast<const Base*>(this)->obj,
				PyObjectConverter<Object>{}(std::forward<Actual>(actual)));
			if (result < 0) { throw PyError(); }
			return result;
		}

		Object<List_> list() const;
		Object<Tuple_> tuple() const;


	};

	//?TODO?
	//?fast pysequence interface?

}
#endif//CPPY_PROTO_SEQUENCE_HPP
