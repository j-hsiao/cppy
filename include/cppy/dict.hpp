#ifndef CPPY_DICT_HPP
#define CPPY_DICT_HPP
#include <cppy/errors.hpp>
#include <cppy/object.hpp>
#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/mixin/mapping.hpp>
#include <cppy/string.hpp>
#include <cppy/convert/pyobject.hpp>


namespace cppy
{
	struct Dict_{};

	template<> struct Object<Dict_&>:
		CheckThrow<Object<Dict_&>>,
		Mapping<Object<List_&>>,
		Sized<Object<List_&>, PyDict_Size>,
		Borrowed
	{
		using Borrowed::Borrowed;
		using Sized<Object<List_&>, PyDict_Size>::size;
		using Mapping<Object<List_&>>::operator[];

		bool check() const { return PyDict_Check(obj); }
		static constexpr const char* name() { return "dict"; }

		Object<Dict_&>& clear() { PyDict_Clear(obj); return *this; }

		Object<Dict_> copy() const;

		template<class Key>
		bool contains(Key &&key) const {
			PyOjectConverter<Object> cvt;
			if (int val = PyDict_Contains(obj, cvt(std::forward<Key>(key)))) {
				if (val == 1) { return true; }
				else { throw PyError(); }
			}
			else { return false; }
		}
		////special case for const char*, on 3.13+
		//bool contains(const char* key) const {
		//	PyOjectConverter<Object> cvt;
		//	if (int val = PyDict_ContainsString(obj, key)) {
		//		if (val == 1) { return true; }
		//		else { throw PyError(); }
		//	}
		//	else { return false; }
		//}

		//------------------------------
		//setitem
		//------------------------------
		template<class Key, class Value>
		Object<Dict_&>& setitem(Key &&key, Value &&value) {
			PyObjectConverter<Object> cvt;
			if (PyDict_SetItem(obj, cvt(std::forward<Key>(key)), cvt(std::forward<Value>(value))))
			{ throw PyError(); }
			return *this;
		}
		Object<Dict_&>& setitem(const char *key, Value &&value) {
			PyObjectConverter<Object> cvt;
			if (PyDict_SetItemString(obj, key, cvt(std::forward<Value>(value))))
			{ throw PyError(); }
			return *this;
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
#			if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 4
				return Object<>(success(PyDict_SetDefault(obj, key, cvt(std::forward<Value>(value)))));
#			else
				auto tmpkey = cvt(std::forward<Key>(key));
				Object<> ret = getitem_(static_cast<PyObject*>(tmpkey));
				PyObject *ret = PyDict_GetItemWithError(obj, tmpkey);
				if (ret) { return Object<>(ret); }
				else if (PyErr_Occurred()) { throw PyError(); }
				else {
					auto tmpval = cvt(std::forward<Value>(value));
					if (PyDict_SetItem(obj, tmpkey, tmpval)) { throw PyError(); }
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
			{ throw PyError(); }
			return *this;
		}
		Object<Dict_&>& delitem(const char *key) {
			if (PyDict_DelItemString(obj, key)) { throw PyError(); }
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

		//return empty object (nullptr) if key not found.
		template<class Key>
		Object<> getitem_(Key &&key) const {
			PyObject *ret = PyDict_GetItemWithError(
				obj, PyObjectConverter<Object>{}(std::forward<Key>(key)));
			if (ret) { return Object<>(ret); }
			else if (PyErr_Occurred()) { throw PyError(); }
			else { return Object<>(nullptr); }
		}
		template<class Key>
		Object<> getitem(Key &&key) const {
			auto ret = getitem(std::forward<Key>(key));
			if (!ret.obj) { throw KeyError(); }
			return ret;
		}


	};

	template<> struct Object<Dict_>: Owned<Dict_> {
		using Owned<Dict_>::Owned;
	};

	template<> Object<Dict_> Object<Dict_&>::copy() const
	{ return Object<Dict_>(success(PyDict_Copy(obj))); }

}
#endif//CPPY_DICT_HPP
