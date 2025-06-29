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

//#include <cppy/mixin.hpp>
#include <cppy/errors.hpp>

#include <cstddef>
#include <limits>
#include <utility>

#include <iostream>

namespace cppy
{
	//Item proxy for getitem/setitem
	template<class Container, class Key>
	struct ItemProxy
	{
		Container obj;
		Key key;

		template<class T, class V>
		ItemProxy(T &&cont, V &&k): obj(std::forward<T>(cont)), key(std::forward<V>(k)) {}

		decltype(const_cast<const Container&>(obj).getitem(key)) operator()() const { return obj.getitem(key); }
		operator decltype(const_cast<const Container&>(obj).getitem(key))() const { return obj.getitem(key); }

		template<class T>
		ItemProxy& operator=(T &&value) {
			obj.setitem(key, std::forward<T>(value));
			return *this;
		}
	};

	//------------------------------
	//Generic base python object and methods.
	//------------------------------
	template<class T=PyObject&, class Actual=T> struct Object;

	//Immutable borrowed reference.
	template<> struct Object<PyObject&> {
		PyObject *obj;

		Object() noexcept: obj(nullptr) {}
		Object(PyObject *obj) noexcept: obj(obj) {}
		Object(PyObject &obj) noexcept: obj(&obj) {}

		operator PyObject&() const { return *this->obj; }

		const Object& object() const { return *this; }
		Object& object() { return *this; }

		PyObject* ret() const { return obj; }
		bool check() const { return true; }
		static constexpr const char* name() { return "Object"; }

		//repr
		Object<const char*> repr() const;

		//str
		Object<const char*> str() const;

		// getattr()
		Object<PyObject> attr(const char *attr_name) const;

		// __getitem__
		Object<PyObject> getitem(PyObject*) const;
		Object<PyObject> operator[](PyObject*) const;
		template<class Key> Object<PyObject> getitem(const Key &key) const;
		template<class Key> Object<PyObject> operator[](const Key &key) const;
		template<class c1, class c2> Object<PyObject> getitem(const Object<c1, c2> &key) const;
		template<class c1, class c2> Object<PyObject> operator[](const Object<c1, c2> &key) const;


		//mutable __getitem__
		ItemProxy<Object<>, PyObject*> getitem(PyObject*);
		ItemProxy<Object<>, PyObject*> operator[](PyObject *key);
		template<class Key> ItemProxy<Object<>, const Key&> getitem(const Key &key);
		template<class Key> ItemProxy<Object<>, const Key&> operator[](const Key &key);
		template<class c1, class c2> ItemProxy<Object<>, const Object<c1, c2>&> getitem(const Object<c1, c2> &key);
		template<class c1, class c2> ItemProxy<Object<>, const Object<c1, c2>&> operator[](const Object<c1, c2> &key);

		// __setitem__
		void setitem(PyObject *key, PyObject *val)
		{ if (PyObject_SetItem(obj, key, val) == -1) { throw PyError(); } }
		void setitem(const PyObject &key, PyObject *val) { setitem(const_cast<PyObject*>(&key), val); }
		void setitem(PyObject *key, const PyObject &val) { setitem(key, const_cast<PyObject*>(&val)); }
		void setitem(const PyObject &key, const PyObject &val) { setitem(key, const_cast<PyObject*>(&val)); }

		Py_ssize_t size() const {
			auto ret = PyObject_Size(obj);
			if (ret < 0) { throw PyError(); }
			return ret;
		}
		operator bool() const
		{
			int result = PyObject_IsTrue(obj);
			if (result < 0) { throw PyError(); }
			return result == 1;
		}
	};

	//Owned object.
	template<class Actual> struct Object<PyObject, Actual>: Object<Actual&> {
		Object(PyObject *obj, bool preincr) noexcept: Object<Actual&>(obj) {
			if (!preincr) {
				std::cout << "manual incref" << std::endl;
				Py_INCREF(obj);
			}
		}
		Object(PyObject *obj) noexcept: Object<Actual&>(obj) { Py_INCREF(obj); }
		Object(const PyObject &obj) noexcept: Object<Actual&>(const_cast<PyObject*>(&obj))
		{
			std::cout << "Manual incref*" << std::endl;
			Py_INCREF(&obj);
		}

		template<class OActual>
		Object(Object<PyObject, OActual> &&o) noexcept: Object<Actual&>(o.ret(), true) {}

		PyObject* ret() {
			PyObject *ret = this->obj;
			this->obj = nullptr;
			return ret;
		}
		~Object() { Py_XDECREF(this->obj); }
	};

	//getattr
	inline Object<PyObject> Object<>::attr(const char *attr_name) const
	{
		PyObject *ret = PyObject_GetAttrString(obj, attr_name);
		if (!ret) { throw PyError(); }
		return ret;
	}

	//const __getitem__
	inline Object<PyObject> Object<>::getitem(PyObject *key) const
	{
		PyObject *ret = PyObject_GetItem(obj, key);
		if (!ret) { throw PyError(); }
		return Object<PyObject>(ret, true);
	}
	Object<PyObject> Object<>::operator[](PyObject *key) const
	{ return getitem(key); }
	template<class Key> Object<PyObject> Object<>::getitem(const Key &key) const
	{ return getitem(Object<Key>(key)); }
	template<class Key> Object<PyObject> Object<>::operator[](const Key &key) const
	{ return getitem(Object<Key>(key)); }
	template<class c1, class c2> Object<PyObject> Object<>::getitem(const Object<c1, c2> &key) const
	{ return getitem(key.obj); }
	template<class c1, class c2> Object<PyObject> Object<>::operator[](const Object<c1, c2> &key) const
	{ return getitem(key.obj); }

	//mutable __getitem__
	ItemProxy<Object<>, PyObject*> Object<>::getitem(PyObject *key)
	{ return ItemProxy<Object<>, PyObject*>(*this, key); }

	ItemProxy<Object<>, PyObject*> Object<>::operator[](PyObject *key) { return getitem(key); }

	template<class Key> ItemProxy<Object<>, const Key&> Object<>::getitem(const Key &key)
	{ return ItemProxy<Object<>, const Key&>(*this, key); }
	template<class Key> ItemProxy<Object<>, const Key&> Object<>::operator[](const Key &key)
	{ return ItemProxy<Object<>, const Key&>(*this, key); }

	template<class c1, class c2> ItemProxy<Object<>, const Object<c1, c2>&> Object<>::getitem(const Object<c1, c2> &key)
	{ return ItemProxy<Object<>, PyObject*>(*this, key); }
	template<class c1, class c2> ItemProxy<Object<>, const Object<c1, c2>&> Object<>::operator[](const Object<c1, c2> &key)
	{ return ItemProxy<Object<>, PyObject*>(*this, key); }

	//More convenient for argument conversion
	template<class T> struct Object<Object<T>>
	{
		Object<T> obj;
		template<class V> Object(V &&v): obj(std::forward<V>(v)) {}
		operator Object<T>() const { return obj; }
	};
}

#include "cppy/string.hpp"
namespace cppy
{
	Object<const char*> Object<>::repr() const
	{
		PyObject *ret = PyObject_Repr(obj);
		if (!ret) { throw PyError(); }
		return Object<const char*>(ret, true);
	}

	Object<const char*> Object<>::str() const
	{
		PyObject *ret = PyObject_Str(obj);
		if (!ret) { throw PyError(); }
		return Object<const char*>(ret, true);
	}
}
#endif//CPPY_PYOBJ_HPP
