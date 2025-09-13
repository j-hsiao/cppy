#ifndef CPPY_DICT_HPP
#define CPPY_DICT_HPP
#include <cppy/errors.hpp>
#include <cppy/object.hpp>
#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/mixin/mapping.hpp>
#include <cppy/proto/mapping.hpp>
#include <cppy/string.hpp>
#include <cppy/convert/pyobject.hpp>
#include <cppy/util.hpp>

#include <cstddef>

namespace cppy
{
	struct Dict_{};

	template<> struct Object<Dict_&>:
		CheckThrow<Object<Dict_&>>,
		Mapping<Object<Dict_&>>,
		PythonMapping<Object<Dict_&>>,
		Sized<Object<Dict_&>, PyDict_Size>,
		Borrowed
	{
		using Borrowed::Borrowed;
		using CheckThrow<Object<Dict_&>>::checkthrow;
		using Sized<Object<Dict_&>, PyDict_Size>::size;
		using Mapping<Object<Dict_&>>::operator[];

		bool check() const { return PyDict_Check(obj); }
		static constexpr const char* name() { return "dict"; }

		Object<Dict_&>& clear() { PyDict_Clear(obj); return *this; }

		Object<Dict_> copy() const;

		template<class Key>
		bool contains(Key &&key) const {
			PyObjectConverter<Object> cvt;
			if (int val = PyDict_Contains(obj, cvt(std::forward<Key>(key)))) {
				if (val == 1) { return true; }
				else { throw PythonError(); }
			}
			else { return false; }
		}
#		if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
		bool contains(const char* key) const {
			if (int val = PyDict_ContainsString(obj, key)) {
				if (val == 1) { return true; }
				else { throw PythonError(); }
			}
			else { return false; }
		}
#		endif

		//------------------------------
		//setitem
		//------------------------------
		template<class Key, class Value>
		Object<Dict_&>& setitem(Key &&key, Value &&value) {
			PyObjectConverter<Object> cvt;
			if (PyDict_SetItem(obj, cvt(std::forward<Key>(key)), cvt(std::forward<Value>(value))))
			{ throw PythonError(); }
			return *this;
		}
		template<class Value>
		Object<Dict_&>& setitem(const char *key, Value &&value) {
			PyObjectConverter<Object> cvt;
			if (PyDict_SetItemString(obj, key, cvt(std::forward<Value>(value))))
			{ throw PythonError(); }
			return *this;
		}
		//set multiple items, grouped by 2
		template<class A, class B, class...T>
		Object<Dict_&>& setitem(A &&a, B &&b, T&&...t) {
			static_assert(sizeof...(T)%2 == 0, "Expect pairs of key,value");
			setitem(std::forward<A>(a), std::forward<B>(b));
			return setitem(std::forward<T>(t)...);
		}

		//------------------------------
		//setdefault
		//------------------------------
		template<class Key, class Value>
		Object<> setdefault(Key &&key, Value &&value) {
			PyObjectConverter<Object> cvt;
			//NOTE: the PyDict_SetDefault might be better if the hash is lengthy
			//However, the value would always have to be evaluated into a PyObject*
			//which is an overhead that the manual version won't have if the
			//key already exists...
#			if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 4
				return Object<>(success(PyDict_SetDefault(obj, key, cvt(std::forward<Value>(value)))));
#			else
				auto tmpkey = cvt(std::forward<Key>(key));
				PyObject *ret = PyDict_GetItemWithError(obj, tmpkey);
				if (ret) { return Object<>(ret); }
				else if (PyErr_Occurred()) { throw PythonError(); }
				else {
					auto tmpval = cvt(std::forward<Value>(value));
					if (PyDict_SetItem(obj, tmpkey, tmpval)) { throw PythonError(); }
					return Object<>(static_cast<PyObject*>(tmpval));
				}
#			endif
		}

		//------------------------------
		//delitem
		//------------------------------
		template<class Key>
		Object<Dict_&>& delitem(Key &&key) {
			if (PyDict_DelItem(obj, PyObjectConverter<Object>{}(std::forward<Key>(key))))
			{ throw PythonError(); }
			return *this;
		}
		Object<Dict_&>& delitem(const char *key) {
			if (PyDict_DelItemString(obj, key)) { throw PythonError(); }
			return *this;
		}

		//------------------------------
		//getitem
		//------------------------------
		//Other variants are PyDict_GetItem (no errors set regardless of
		//__hash__/__eq__ failure), or PyDict_GetItemString with const char*
		//also ignores errors creating temporary str.
		//?Does this mean the *String variants really create a temporary str
		//(PyUnicode_FromString)?

		//Get an item, throw if key not found.
		template<class Key>
		Object<> getitem(Key &&key) const {
			PyObject *ret = PyDict_GetItemWithError(
				obj, PyObjectConverter<Object>{}(std::forward<Key>(key)));
			if (ret) { return Object<>(ret); }
			else if (PyErr_Occurred()) { throw PythonError(); }
			else { throw KeyError(); }
		}

		template<class Key>
		Object<> getitem(Key &&key, std::nullptr_t) const {
			PyObject *ret = PyDict_GetItemWithError(
				obj, PyObjectConverter<Object>{}(std::forward<Key>(key)));
			if (ret) { return Object<>(ret); }
			else if (PyErr_Occurred()) { throw PythonError(); }
			else { return Object<>(nullptr); }
		}

		private:
			// cpp default value, return must be owned.
			struct CppDefaultGetItem {
				typedef Object<PyObject> type;
				template<class T>
				type operator()(T &&t) const
				{ return Object<typename std::decay<T>::type>(std::forward<T>(t)); }
			};

			// python default value, ok to return borrowed reference.
			struct PyObjectDefaultGetItem {
				typedef Object<> type;
				type operator()(type obj) const { return obj; }
			};

			// rvalue owned reference, may be decref after call so must
			// return owned reference.
			struct PyObjectRvalueDefaultGetItem {
				typedef Object<PyObject> type;
				template<class T>
				type operator()(Owned<T> &&o) { return o.ret(); }
			};

			template<class Default>
			using Converter = typename std::conditional<
				decltype(is<Object>(std::declval<Default&&>()))::value,
				typename std::conditional<
					decltype(is<Owned>(std::declval<Default&&>()))::value
						&& std::is_rvalue_reference<Default&&>::value,
					PyObjectRvalueDefaultGetItem,
					PyObjectDefaultGetItem
				>::type,
				CppDefaultGetItem
			>::type;
		public:
		template<class Key, class Default>
		typename Converter<Default>::type getitem(Key &&key, Default &&value) const {
			PyObject *ret = PyDict_GetItemWithError(
				obj, PyObjectConverter<Object>{}(std::forward<Key>(key)));
			if (ret) { return Object<>(ret); }
			else if (PyErr_Occurred()) { throw PythonError(); }
			else { return Converter<Default>{}(std::forward<Default>(value)); }
		}

	};

	template<> struct Object<Dict_>: Owned<Dict_> {
		using Owned<Dict_>::Owned;
		template<class...T>
		Object(T&&...items): Owned<Dict_>(success(PyDict_New()))
		{ setitem(std::forward<T>(items)...); }

	};

	Object<Dict_> Object<Dict_&>::copy() const
	{ return Object<Dict_>(success(PyDict_Copy(obj))); }

	typedef Object<Dict_&> DictRef;
	typedef Object<Dict_> Dict;

}
#endif//CPPY_DICT_HPP
