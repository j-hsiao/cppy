#ifndef CPPY_PYOBJ_HPP
#define CPPY_PYOBJ_HPP

//#include <cppy/mixin.hpp>
#include <cppy/errors.hpp>

#include <cstddef>
#include <limits>
#include <utility>

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

		decltype(obj.getitem(key)) operator()() const { return obj.getitem(key); }
		operator decltype(obj.getitem(key))() const { return obj.getitem(key); }

		template<class T>
		ItemProxy& operator=(T &&value) {
			obj.setitem(key, std::forward<T>(value));
			return *this;
		}
	};

	//------------------------------
	//Generic base python object and methods.
	//------------------------------
	template<

	template<class T=const PyObject&> struct Object;
	//Immutable borrowed reference.
	template<> struct Object<const PyObject&> {
		PyObject *obj;

		Object() noexcept: obj(nullptr) {}
		Object(PyObject *obj) noexcept: obj(obj) {}
		Object(const PyObject &obj) noexcept: obj(const_cast<PyObject*>(&obj)) {}

		operator const PyObject& () { return *obj; }

		PyObject* ret() const { return obj; }
		bool check() const { return true; }
		static constexpr const char* name() { return "Object"; }
		Object<const char*> repr() const;
		Object<const char*> str() const;

		Object<PyObject> attr(const char *attr_name) const;
		Object<PyObject> getitem(PyObject*) const;
		Object<PyObject> getitem(const PyObject &key) const;
		Object<PyObject> operator[](PyObject *key) const;
		Object<PyObject> operator[](const PyObject &key) const;

		const Object& object() const { return *this; }

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

	//Mutable borrowed reference.
	template<> struct Object<PyObject&>: Object<const PyObject&> {
		using Object<const PyObject&>::Object;
		Object(const PyObject&) = delete;
		Object(PyObject &obj): Object<const PyObject&>(&obj) {}

		void setitem(PyObject *key, PyObject *val)
		{ if (PyObject_SetItem(obj, key, val) == -1) { throw PyError(); } }
		void setitem(const PyObject &key, PyObject *val) { setitem(const_cast<PyObject*>(&key), val); }
		void setitem(PyObject *key, const PyObject &val) { setitem(key, const_cast<PyObject*>(&val)); }
		void setitem(const PyObject &key, const PyObject &val) { setitem(key, const_cast<PyObject*>(&val)); }

		ItemProxy<Object<PyObject&>, PyObject*> operator[](PyObject *key)
		{ return ItemProxy<Object<PyObject&>, PyObject*>(*this, key); }
		ItemProxy<Object<PyObject&>, PyObject*> operator[](const PyObject &key)
		{ return ItemProxy<Object<PyObject&>, PyObject*>(*this, const_cast<PyObject*>(&key)); }

		operator PyObject&() { return *this->obj; }
		Object& object() { return *this; }
	};

	//Owned object.
	template<> struct Object<PyObject>: Object<PyObject&> {
		Object(PyObject *obj, bool preincr): Object<PyObject&>(obj) {
			if (!preincr) { Py_INCREF(obj); }
		}
		Object(PyObject *obj): Object<PyObject&>(obj) { Py_INCREF(obj); }
		Object(const PyObject &obj): Object<PyObject&>(const_cast<PyObject*>(&obj)) { Py_INCREF(&obj); }

		PyObject* ret() {
			PyObject *ret = this->obj;
			this->obj = nullptr;
			return ret;
		}
		~Object() { Py_XDECREF(this->obj); }

		operator const PyObject&() const& { return *this->obj; }
		operator PyObject&() & { return *this->obj; }
	};




//	template<> struct Object<PyObject*, false>: CheckThrow<PyObject*>, Make<PyObject*>
//	{
//	};

//	template<class T>
//	struct Managed: Object<T>
//	{
//		//Transfer ownership of the wrapped PyObject*
//		PyObject* ret() noexcept
//		{
//			PyObject *ptr = this->obj;
//			this->obj = nullptr;
//			return ptr;
//		}
//		~Managed() { Py_XDECREF(this->obj); }
//	};

//	template<>
//	struct Object<PyObject*, true>: Managed<PyObject*>, Make<PyObject*, true>
//	{ using Make<PyObject*, true>::Make; };

//	//getattr
//	inline Object<PyObject*, true> Object<>::get(const char *attr_name) const
//	{
//		PyObject *ret = PyObject_GetAttrString(obj, attr_name);
//		if (!ret) { throw PyError(); }
//		return ret;
//	}
//	//__getitem__
//	inline Object<PyObject*, true> Object<>::getitem(PyObject *key) const
//	{
//		PyObject *ret = PyObject_GetItem(obj, key);
//		if (!ret) { throw PyError(); }
//		return ret;
//	}

//	template<class T, bool b>
//	inline Object<PyObject*, true> Object<>::getitem(const Object<T,b>&key) const
//	{ return getitem(key.obj); }

//	template<class T>
//	inline Object<PyObject*, true> Object<>::getitem(const T &key) const
//	{ return getitem(Object<T,true>(key).obj); }

//	template<class T, bool b> struct Object<T&, b>: Object<T,b>{ using Object<T,b>::Object; };
//	template<class T, bool b> struct Object<T&&, b>: Object<T,b>{ using Object<T,b>::Object; };
//	template<class T, bool b> struct Object<const T, b>: Object<T,b>{ using Object<T,b>::Object; };
//	template<class T, bool b> struct Object<const T&, b>: Object<T,b>{ using Object<T,b>::Object; };
//	template<class T, bool b> struct Object<const T&&, b>: Object<T,b>{ using Object<T,b>::Object; };

//	//More convenient for argument conversion
//	template<class T, bool b> struct Object<Object<T,b>, false>
//	{
//		Object<> obj;

//		Object(PyObject *ptr): obj(ptr) {}
//		Object(const Object<> &other): obj(other.obj) {}

//		operator Object<T,b>() const { return Object<T,b>(obj); }
//	};

}

#include "cppy/string.hpp"
namespace cppy
{
	Object<const char*> Object<>::repr() const
	{
		PyObject *ret = PyObject_Repr(obj);
		if (!ret) { throw PyError(); }
		return ret;
	}

	Object<const char*> Object<>::str() const
	{
		PyObject *ret = PyObject_Str(obj);
		if (!ret) { throw PyError(); }
		return ret;
	}
}
#endif//CPPY_PYOBJ_HPP
