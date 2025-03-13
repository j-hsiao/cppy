#ifndef CPPY_PYOBJ_HPP
#define CPPY_PYOBJ_HPP

#include "cppy/errors.hpp"

#include <cstddef>
#include <limits>

namespace cppy
{
	//Wrap a call to some python method that returns an object.
	//If nullptr, implies a python error occurred and throw the
	//corresponding exception.
	static inline PyObject* success(PyObject *obj)
	{
		if (obj) { return obj; }
		throw PyError();
	}

	template<class T=PyObject*, bool Managed=false> struct Object;
	//------------------------------
	//Generic base python object methods.
	//------------------------------
	template<> struct Object<PyObject*, false>
	{
		PyObject *obj;

		Object(PyObject *obj) noexcept : obj(obj) {}

		Object(const Object<PyObject*, false> &obj) noexcept : obj(obj.obj) {}

		//Allow access to basic object interface from derived classes.
		Object<PyObject*, false>& object() { return *this; }
		const Object<PyObject*, false>& object() const { return *this; }

		void throwifnot(bool success, const char *msg="") const
		{ if (not success) { throw TypeError(msg); } }

		// repr
		Object<const char*, true> repr() const;
		// str
		Object<const char*, true> str() const;

		//getattr
		Object<PyObject*, true> get(const char *attr_name) const;
		//__getitem__
		Object<PyObject*, true> operator[](PyObject *key) const;

		Object<PyObject*, true> operator[](const Object<>&key) const;

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
	struct Managed: public Object<T>
	{
		using Derived = Object<T>;
		using Object<T, false>::Object;

		//An additional int argument indicates that the input
		//pointer is a borrowed reference.  It will be
		//Py_INCREF()ed in this constructor.  Otherwise, the
		//ptr should be a strong reference whose ownership will
		//be transferred to the created object.
		Managed(PyObject *ptr, int) noexcept : Object<T>(ptr) { Py_INCREF(ptr); }
		//copy constructor, always create a new strong reference.
		Managed(const Object<PyObject*> &other) noexcept : Managed(other.obj, 0) {}

		//Move constructor from any other managed object.
		//Transfer ownership of the managed reference.
		template<class Tp>
		Managed(Managed<Tp> &&other) noexcept : Managed(other.obj)
		{ other.obj = nullptr; }

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
	struct Object<PyObject*, true>: public Managed<PyObject*>
	{ using Managed<PyObject*>::Managed; };

	//getattr
	Object<PyObject*, true> Object<>::get(const char *attr_name) const
	{
		PyObject *ret = PyObject_GetAttrString(obj, attr_name);
		if (!ret) { throw PyError(); }
		return ret;
	}
	//__getitem__
	Object<PyObject*, true> Object<>::operator[](PyObject *key) const
	{
		PyObject *ret = PyObject_GetItem(obj, key);
		if (!ret) { throw PyError(); }
		return ret;
	}

	Object<PyObject*, true> Object<>::operator[](const Object<>&key) const
	{ return operator[](key.obj); }

	template<class T, bool b> struct Object<T&, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<T&&, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<const T, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<const T&, b>: Object<T,b>{ using Object<T,b>::Object; };
	template<class T, bool b> struct Object<const T&&, b>: Object<T,b>{ using Object<T,b>::Object; };

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
