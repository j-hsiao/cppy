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

#include <iostream> //
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
		private:
			PyObject *attr_(const char *attrname) const
			{ return PyObject_GetAttrString(obj, attrname); }
			template<class Name> PyObject *attr_(Name &&attrname) const
			{ return PyObject_GetAttrString(obj, PyObjectConverter<Object>{}(std::forward<Name>(attrname))); }

#		if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
			int attr_noexc(const char *attrname, PyObject **ptr) const
			{ return PyObject_GetOptionalAttrString(obj, attrname, ptr); }
			template<class Name> int attr_noexc(Name &&attrname, **ptr) const
			{ return PyObject_GetOptionalAttrString(obj, PyObjectConverter<Object>{}(std::forward<Name>(attrname)), ptr); }
			template<class Name>
			PyObject* attr_noexc(Name &&name) const {
				PyObject *ret;
				int result = attr_noexc(std::forward<Name>(name), &ret);
				if (result < 0) { throw PyError(); }
				return ret;
			}
#		else
			template<class Name>
			PyObject* attr_noexc(Name &&name) const {
				PyObject *ret = attr_(std::forward<Name>(name));
				if (!ret) { PyErr_Clear(); }
				return ret;
			}
#		endif
		public:
			template<class Name>
			Object<PyObject> attr(Name &&name) const;
			template<class Name, class Default>
			Object<PyObject> attr(Name &&name, Default &&value) const;
			template<class Name>
			Object<PyObject> attr(Name &&name, std::nullptr_t) const;

		//setattr()
		private:
			template<class Value>
			int setattr_(const char *name, Value &&value) {
				return PyObject_SetAttrString(
					obj, name, PyObjectConverter<Object>{}(std::forward<Value>(value)));
			}
			template<class Name, class Value>
			int setattr_(Name &&name, Value &&value) {
				PyObjectConverter<Object> cvt;
				return PyObject_SetAttr(
					obj, cvt(std::forward<Name>(name)), cvt(std::forward<Value>(value)));
			}
		public:
			template<class Name, class Value>
			Object<>& setattr(Name &&name, Value &&value) {
				if (setattr_(std::forward<Name>(name), std::forward<Value>(value)))
				{ throw PyError(); }
				return *this;
			}

		//delattr
		private:
#			if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
			int delattr_(const char *name)
			{ return PyObject_DelAttrString(obj, name); }
			template<class Name> int delattr_(Name &&name)
			{ return PyObject_DelAttr(obj, PyObjectConverter<Object>{}(std::forward<Name>(name))); }
#			else
			int delattr_(const char *name)
			{ return PyObject_SetAttrString(obj, name, NULL); }
			template<class Name> int delattr_(Name &&name)
			{ return PyObject_SetAttr(obj, PyObjectConverter<Object>{}(std::forward<Name>(name)), NULL); }
#			endif
		public:
			template<class Name>
			Object<>& delattr(Name &&name) {
				if (delattr_(std::forward<Name>(name))) { throw PyError(); }
				return *this;
			}

		//__getitem__ (const)
		template<class T>
		Object<PyObject> getitem(T &&t) const;

		//__setitem__
		template<class Key, class Val>
		Object<>& setitem(Key &&key, Val &&val) {
			PyObjectConverter<Object> cvt;
			if (PyObject_SetItem(obj, cvt(key), cvt(val)) == -1)
			{ throw PyError(); }
			return *this;
		}
		//__delitem__
		private:
			template<class Key> int delitem_(Key &&key)
			{ return PyObject_DelItem(obj, PyObjectConverter<Object>{}(std::forward<Key>(key))); }
			int delitem_(const char *key)
			{ return PyObject_DelItemString(obj, key); }
		public:
			template<class Key> Object<>& delitem(Key &&key) {
				if (delitem_(std::forward<Key>(key))) { throw PyError(); }
				return *this;
			}

		explicit operator bool() const {
			int result = PyObject_IsTrue(obj);
			if (result < 0) { throw PyError(); }
			return result == 1;
		}
		bool is_none() const { return obj == Py_None; }

		template<class T>
		Object<T> as() const { return Object<T>(*this); }
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
	template<class Name>
	Object<PyObject> Object<>::attr(Name &&name) const {
		if (PyObject *ret = attr_(std::forward<Name>(name))) { return Object<PyObject>(ret); }
		throw PyError();
	}
	template<class Name, class Default>
	Object<PyObject> Object<>::attr(Name &&name, Default &&value) const {
		if (PyObject *ret = attr_noexc(std::forward<Name>(name))) { return Object<PyObject>(ret); }
		return Object<PyObject>(StealConverter<Object>{}(std::forward<Default>(value)));
	}
	template<class Name>
	Object<PyObject> Object<>::attr(Name &&name, std::nullptr_t) const {
		if (PyObject *ret = attr_noexc(std::forward<Name>(name))) { return Object<PyObject>(ret); }
		return Object<PyObject>(nullptr);
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
