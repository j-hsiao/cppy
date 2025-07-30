#ifndef CPPY_LIST_HPP
#define CPPY_LIST_HPP
#include <cppy/errors.hpp>
#include <cppy/object.hpp>
#include <cppy/mixin/mapping.hpp>
#include <cppy/mixin/sized.hpp>
#include <cppy/convert/int.hpp>
#include <cppy/tuple.hpp>

namespace cppy {
	struct List_{};

	template<> struct Object<List_&>:
		CheckThrow<Object<List_&>>,
		Mapping<Object<List_&>>,
		Sized<Object<List_&>, PyList_Size>,
		Borrowed
	{
		using Borrowed::Borrowed;
		using Sized<Object<List_&>, PyList_Size>::size;
		using Mapping<Object<List_&>>::operator[];

		bool check() const { return PyList_Check(obj); }
		static constexpr const char* name() { return "List"; }

		Py_ssize_t size_() const { return PyList_GET_SIZE(obj); }

		Object<List_> slice(Py_ssize_t start, Py_ssize_t stop) const;

		//TODO? PyList_GET_ITEM? (no error checking)
		//NOTE: must be 0 to size()-1 (negative not supported)
		template<class Idx>
		Object<> getitem(Idx &&idx) const
		{ return Object<>(success(PyList_GetItem(obj, IndexConverter<Object>{}(std::forward<Idx>(idx))))); }

		//TODO PyList_SET_ITEM?:  steal reference, existing items not decrefed no error checking, is macro
		//setitem steals reference.
		template<class Idx, class Value>
		Object<List_&>& setitem(Idx &&idx, Value &&value)
		{
			IndexConverter<Object> intcvt;
			StealConverter<Object> stealcvt;
			if (PyList_SetItem(obj, intcvt(std::forward<Idx>(idx)), stealcvt(std::forward<Value>(value))) == -1)
			{ throw PyError(); }
			return *this;
		}

		//insert does not steal
		template<class Idx, class Value>
		Object<List_&>& insert(Idx &&idx, Value &&value) {
			IndexConverter<Object> intcvt;
			PyObjectConverter<Object> objcvt;
			if (PyList_Insert(obj, intcvt(std::forward<Idx>(idx)), objcvt(std::forward<Value>(value))) == -1)
			{ throw PyError(); }
			return *this;
		}

		//append does not steal
		template<class Value>
		Object<List_&>& append(Value &&value) {
			PyObjectConverter<Object> cvt;
			if (PyList_Append(obj, cvt(std::forward<Value>(value))) == -1)
			{ throw PyError(); }
			return *this;
		}

		template<Py_ssize_t start=0, class First, class...Items>
		void setitems(First &&first, Items&&...items) {
			if (start + sizeof...(Items) + 1> size()) {
				throw IndexError("Setting too many items to a tuple.");
			}
			setitems_(std::forward<First>(first), std::forward<Items>(items)...);
		}

		Tuple tuple() const { return Tuple(success(PyList_AsTuple(obj))); }

		Object<List_&>& sort() {
			if (PyList_Sort(obj) == -1) { throw PyError{}; }
			return *this;
		}

		Object<List_&>& reverse() {
			if (PyList_Reverse(obj) == -1) { throw PyError{}; }
			return *this;
		}

		Object<List_&>& clear() { return set_slice(0, PY_SSIZE_T_MAX, nullptr); }
		Object<List_&>& extend(PyObject *seq) { return set_slice(PY_SSIZE_T_MAX, PY_SSIZE_T_MAX, seq); }
		Object<List_&>& extend(const Object<> &seq) { return extend(seq.obj); }

		Object<List_&>& set_slice(Py_ssize_t low, Py_ssize_t high, PyObject *itemseq) {
			if (PyList_SetSlice(obj, low, high, itemseq) == -1) { throw PyError{}; }
			return *this;
		}
		Object<List_&>& set_slice(Py_ssize_t low, Py_ssize_t high, const Object<> &itemseq)
		{ return set_slice(low, high, itemseq.obj); }

		private:
			template<Py_ssize_t pos=0, class First, class...Items>
			void setitems_(First &&first, Items&&...items)
			{
				setitem(pos, std::forward<First>(first));
				setitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos> void setitems_() {}
	};

	template<> struct Object<List_>: Owned<List_>
	{
		using Owned<List_>::Owned;

		Object(): Owned<List_>(success(PyList_New(0))) {}

		//NOTE: PyList_Pack exists, but it seems to incref everything, does not steal.
		//setitem, steal when non-const Owned.  Otherwise, incref.
		template<class...T>
		Object(T&&...items):
			Owned<List_>(success(PyList_New(static_cast<Py_ssize_t>(sizeof...(T)))))
		{ setnewitems(std::forward<T>(items)...); }

		private:
			template<Py_ssize_t pos=0, class First, class...Items>
			void setnewitems(First &&first, Items&&...items) {
				StealConverter<Object> stealcvt;
				PyList_SET_ITEM(obj, pos, stealcvt(std::forward<First>(first)));
				setnewitems<pos+1>(std::forward<Items>(items)...);
			}
			template<Py_ssize_t pos=0> void setnewitems() {}
	};

	inline Object<List_> Object<List_&>::slice(Py_ssize_t start, Py_ssize_t stop) const
	{ return Object<List_>(success(PyList_GetSlice(obj, start, stop))); }

	struct List: Object<List_> { using Object<List_>::Object; };
	struct ListRef: Object<List_&> { using Object<List_&>::Object; };
}
#endif//CPPY_LIST_HPP
