#ifndef CPPY_PYOBJ_HPP
#define CPPY_PYOBJ_HPP

#include <cppy/mixin.hpp>
#
#include <cstddef>
#include <limits>
#include <utility>

namespace cppy
{
	template<class T=PyObject*, bool Managed=false> struct Object;

	//------------------------------
	//Generic base python object methods.
	//------------------------------
	template<> struct Object<PyObject*, false>: CheckThrow<PyObject*>, Make<PyObject*>
	{
		PyObject *obj;

		using Make::Make;

		//All subclasses easy access to generic object interface.
		Object<PyObject*, false>& object() { return *this; }
		const Object<PyObject*, false>& object() const { return *this; }

		bool check() const { return true; }
		static constexpr const char* name() { return "Object"; }

		// repr
		Object<const char*, true> repr() const;
		// str
		Object<const char*, true> str() const;

		//getattr
		Object<PyObject*, true> get(const char *attr_name) const;
		//__getitem__

		Object<PyObject*, true> getitem(PyObject *key) const;
		Object<PyObject*, true> getitem(const Object<>&key) const { getitem(key.obj); }
		template<class T> Object<PyObject*, true> getitem(T &&key) const
		{ return getitem(Object<T,true> k(std::forward<T>(key)).obj); }

		template<class Key>
		struct ItemProxy
		{
			Object<> obj;
			Key key;

			Object<PyObject*, true> object() const { return obj.getitem(key); }
			operator Object<PyObject*, true>() const { return obj.getitem(key); }

			template<class T>
			// ItemProxy& operator=(T &&value) { object.setitem(key, std::forward<T>(value)); }
		}

		ItemProxy<PyObject*> operator[](PyObject *key) const { return {obj, key}; }
		template<class T, bool b> ItemProxy<Object<T,b>> operator[](const Object<T,b>&key) const
		{ return {obj, key}; }
		template<class T> ItemProxy<Object<T,true>> operator[](T &&key) const
		{ return {obj, Object<T,true>(std::forward<T>(key))}; }

		//TODO
		//Object<PyObject*, false> operator()(...)
		//how to wrap, convert to tuple, call function...?

		//len
		Py_ssize_t size() const
		{
			auto ret = PyObject_Size(obj);
			if (ret < 0) { throw PyError(); }
			return ret;
		}
		//bool()
		operator bool() const
		{
			int result = PyObject_IsTrue(obj);
			if (result < 0) { throw PyError(); }
			return result == 1;
		}
	};

	template<class T>
	struct Managed: Object<T>
	{
		//Transfer ownership of the wrapped PyObject*
		PyObject* ret() noexcept
		{
			PyObject *ptr = this->obj;
			this->obj = nullptr;
			return ptr;
		}
		~Managed() { Py_XDECREF(this->obj); }
	};

	template<>
	struct Object<PyObject*, true>: Managed<PyObject*>, Make<PyObject*, true>
	{ using Make<PyObject*, true>::Make; };

	//getattr
	inline Object<PyObject*, true> Object<>::get(const char *attr_name) const
	{
		PyObject *ret = PyObject_GetAttrString(obj, attr_name);
		if (!ret) { throw PyError(); }
		return ret;
	}
	//__getitem__
	inline Object<PyObject*, true> Object<>::getitem(PyObject *key) const
	{
		PyObject *ret = PyObject_GetItem(obj, key);
		if (!ret) { throw PyError(); }
		return ret;
	}
	inline Object<PyObject*, true> Object<>::getitem(const Object<>&key) const
	{ return getitem(key.obj); }

	template<class T, bool b> struct Object<T&, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<T&&, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<const T, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<const T&, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<const T&&, b>: Object<T,b>{ using Object<T,b>::Object; };

	//More convenient for argument conversion
	template<class T, bool b> struct Object<Object<T,b>, false>
	{
		Object<> obj;

		Object(PyObject *ptr): obj(ptr) {}
		Object(const Object<> &other): obj(other.obj) {}

		operator Object<T,b>() const { return Object<T,b>(obj); }
	};

}

#include "cppy/string.hpp"
namespace cppy
{
	Object<const char*, true> Object<>::repr() const
	{
		PyObject *ret = PyObject_Repr(obj);
		if (!ret) { throw PyError(); }
		return ret;
	}

	Object<const char*, true> Object<>::str() const
	{
		PyObject *ret = PyObject_Str(obj);
		if (!ret) { throw PyError(); }
		return ret;
	}
}
#endif//CPPY_PYOBJ_HPP
