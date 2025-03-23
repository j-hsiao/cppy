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

	//Inheritable constructors from Object<>s.
	//Otherwise, the constructors are not candidates because takes a
	//single related reference to base and derived types.
	template<class T=PyObject*, bool b=false, template<class, bool> class Derived=Object> struct Make
	{
		Make() noexcept {}
		Make(PyObject *obj) noexcept
		{ static_cast<Derived<T,false>*>(this)->obj = obj; }

		Make(const Derived<PyObject*, false> &other) noexcept:
			Make(other.obj)
		{}
	};
	template<class T, template<class, bool> class Derived> struct Make<T, true, Derived>
	{
		Make() noexcept {}

		// PyObject* is already increffed.
		Make(PyObject *obj) noexcept
		{ static_cast<Derived<T,true>*>(this)->obj = obj; }

		// Also incref the pointer
		Make(PyObject *obj, int) noexcept : Make(obj)
		{ Py_INCREF(obj); }

		//copy regardless of managed or not always increfs
		template<class O, bool b>
		Make(const Derived<O, b> &other) noexcept : Make(other.obj, 0) {}

		//Move from a managed Object
		template<class O>
		Make(Derived<O, true> &&other) noexcept : Make(other.obj)
		{ other.obj = nullptr; }
	};


	//------------------------------
	//Generic base python object methods.
	//------------------------------
	template<> struct Object<PyObject*, false>: Make<>
	{
		PyObject *obj;

		using Make::Make;

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
		template<class T> Object<PyObject*, true> operator[](T) const;

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
	inline Object<PyObject*, true> Object<>::operator[](PyObject *key) const
	{
		PyObject *ret = PyObject_GetItem(obj, key);
		if (!ret) { throw PyError(); }
		return ret;
	}
	inline Object<PyObject*, true> Object<>::operator[](const Object<>&key) const
	{ return operator[](key.obj); }

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
