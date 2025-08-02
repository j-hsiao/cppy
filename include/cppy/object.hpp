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
#include <cppy/mixin/sized.hpp>
#include <cppy/convert/pyobject.hpp>

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

	template<> struct Object<PyObject&>:
		Mapping<Object<>>,
		Sized<Object<>, PyObject_Size>
	{
		PyObject *obj;

		Object() noexcept: obj(nullptr) {}
		Object(PyObject *obj) noexcept: obj(obj) {}
		Object(const Object &obj) noexcept: obj(obj.obj) {}
		//No references to temporaries.
		Object(Object<PyObject> &&o) = delete;

		const Object& object() const& { return *this; }
		Object& object() & { return *this; }
		Object&& object() && { return static_cast<Object&&>(*this); }

		//Give a PyObject* suitable as return value (it owns reference)
		PyObject* ret() const {
			Py_INCREF(obj);
			return obj;
		}
		bool check() const { return true; }
		static constexpr const char* name() { return "object"; }

		//repr
		Object<const char*> repr() const;

		//str
		Object<const char*> str() const;

		//getattr()
		Object<PyObject> attr(const char *attr_name) const;

		//__getitem__ (const)
		template<class T>
		Object<PyObject> getitem(T &&t) const;

		//__setitem__
		template<class Key, class Val>
		void setitem(Key &&key, Val &&val) {
			PyObjectConverter<Object> cvt;
			if (PyObject_SetItem(obj, cvt(key), cvt(val)) == -1)
			{ throw PyError(); }
		}

		explicit operator bool() const {
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
	template<class Actual=PyObject>
	struct Owned: Object<Actual&> {
		using Base = Object<Actual&>;
		using Base::Base;

		Owned(const Owned &o) noexcept: Base(o.obj) { Py_INCREF(o.obj); }
		Owned(Owned &&o) noexcept: Base(o.obj) { o.obj = nullptr; }

		Owned(const Object<> &o) noexcept: Base(o.obj) { Py_INCREF(o.obj); }
		template<class OActual>
		Owned(Owned<OActual> &&o) noexcept: Base(o.obj) { o.obj = nullptr; }

		using Object<>::ret;
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
	template<class Key>
	inline Object<PyObject> Object<>::getitem(Key &&key) const
	{
		PyObject *ret = PyObject_GetItem(obj, PyObjectConverter<Object>{}(key));
		if (!ret) { throw PyError(); }
		return Object<PyObject>(ret);
	}

	//More convenient for argument conversion
	template<class T> struct Object<Object<T>>
	{
		Object<T> obj;
		template<class V> Object(V &&v): obj(std::forward<V>(v)) {}
		operator Object<T>() const { return obj; }
	};

	typedef Object<PyObject> Obj;
	typedef Object<> ObjRef;
}

#endif//CPPY_PYOBJ_HPP
