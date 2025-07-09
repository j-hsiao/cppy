// Generic object.
//
//Object<PyObject&>: A generic borrowed reference.
//Object<PyObject, actual>: An owned generic reference.
//
//Object<T&>: A typed borrowed reference
//	is a Object<PyObject&>
//
//Object<T>
//	is a Object<PyObject, T>
//		is a Ojbect<T&>
//


#ifndef CPPY_PYOBJ_HPP
#define CPPY_PYOBJ_HPP

#include <cppy/errors.hpp>
#include <cppy/mixin/mapping.hpp>

#include <cstddef>
#include <limits>
#include <utility>

#include <type_traits>

namespace cppy
{
	//------------------------------
	//Generic base python object and methods.
	//------------------------------
	template<class T=PyObject&> struct Object;

	template<> struct Object<PyObject&>: Mapping<PyObject&, Object<PyObject>> {
		PyObject *obj;

		Object() noexcept: obj(nullptr) {}
		Object(PyObject *obj) noexcept: obj(obj) {}
		Object(const Object &obj) noexcept: obj(obj.obj) {}
		Object(Object<PyObject> &&o) = delete;

		const Object& object() const& { return *this; }
		Object& object() & { return *this; }
		Object&& object() && { return static_cast<Object&&>(*this); }

		PyObject* ret() const { return obj; }
		bool check() const { return true; }
		static constexpr const char* name() { return "Object"; }

		//repr
		Object<const char*> repr() const;

		//str
		Object<const char*> str() const;

		//getattr()
		Object<PyObject> attr(const char *attr_name) const;

		//__getitem__ (const)
		Object<PyObject> getitem(PyObject*) const;
		//Object<PyObject> getitem(const Object<> &key) const;
		//template<class Key> Object<PyObject> getitem(const Key &key) const;
		//template<class Key> Object<PyObject> getitem(const Object<Key> &key) const;
		//template<class T> Object<PyObject> operator[](T &&t) const;

		using Mapping<PyObject&, Object<PyObject>>::getitem;

		//__setitem__
		void setitem(PyObject *key, PyObject *val)
		{ if (PyObject_SetItem(obj, key, val) == -1) { throw PyError(); } }
		void setitem(const Object<> &key, PyObject *val) { setitem(key.obj, val); }
		void setitem(PyObject *key, const Object<> &val) { setitem(key, val.obj); }
		void setitem(const Object<> &key, const Object<> &val) { setitem(key.obj, val.obj); }


		//__len__
		Py_ssize_t size() const {
			auto ret = PyObject_Size(obj);
			if (ret < 0) { throw PyError(); }
			return ret;
		}
		operator bool() const {
			int result = PyObject_IsTrue(obj);
			if (result < 0) { throw PyError(); }
			return result == 1;
		}
		bool is_none() const { return obj == Py_None; }
	};

	//Intermediate for copy constructors from borrowed refs.
	struct Borrowed: Object<PyObject&> {
		using Base = Object<PyObject&>;
		using Base::Base;
		Borrowed(const Base &o) noexcept: Base(o.obj) {}
	};

	//Intermediate owned object for automatic refcounting.
	template<class Actual> struct Owned: Object<Actual&> {
		using Base = Object<Actual&>;
		using Base::Base;

		Owned(const Owned &o) noexcept: Base(o.obj) { Py_INCREF(o.obj); }
		Owned(Owned &&o) noexcept: Base(o.obj) { o.obj = nullptr; }

		Owned(const Object<> &o) noexcept: Base(o.obj) { Py_INCREF(o.obj); }
		template<class OActual>
		Owned(Owned<OActual> &&o) noexcept: Base(o.obj) { o.obj = nullptr; }

		PyObject* ret() {
			PyObject *ret = this->obj;
			this->obj = nullptr;
			return ret;
		}
		~Owned() { Py_XDECREF(this->obj); }
	};

	//owned generic object
	template<> struct Object<PyObject>: Owned<PyObject> { using Owned<PyObject>::Owned; };

	//getattr
	inline Object<PyObject> Object<>::attr(const char *attr_name) const
	{
		PyObject *ret = PyObject_GetAttrString(obj, attr_name);
		if (!ret) { throw PyError(); }
		return Object<PyObject>(ret);
	}

	//const __getitem__
	inline Object<PyObject> Object<>::getitem(PyObject *key) const
	{
		PyObject *ret = PyObject_GetItem(obj, key);
		if (!ret) { throw PyError(); }
		return Object<PyObject>(ret);
	}
	//inline Object<PyObject> Object<>::getitem(const Object<> &key) const
	//{ return getitem(key.obj); }
	//template<class Key> Object<PyObject> Object<>::getitem(const Key &key) const
	//{ return getitem(Object<Key>(key)); }
	//template<class Key> Object<PyObject> Object<>::getitem(const Object<Key> &key) const
	//{ return getitem(key.obj); }
	//template<class T> Object<PyObject> Object<>::operator[](T &&t) const
	//{ return getitem(std::forward<T>(t)); }

	//More convenient for argument conversion
	template<class T> struct Object<Object<T>>
	{
		Object<T> obj;
		template<class V> Object(V &&v): obj(std::forward<V>(v)) {}
		operator Object<T>() const { return obj; }
	};
}

#endif//CPPY_PYOBJ_HPP
