#ifndef CPPY_MIXIN_HPP
#define CPPY_MIXIN_HPP

#include <cppy/errors.hpp>
#include <utility>
namespace cppy
{
	template<class T, bool b> struct Object;

//Constructors that take any ancestor class will not be inherited.
//To get constructors from Object<>, must either
//1: define them explicitly
//2: Inherit from another class that does not inherit from Object<>
//   and add using *::*
//
//using a macro to define the constructors is probably easier...
#define CPPY_BORROWED_OBJECT_CONSTRUCTORS \
	Object() noexcept {} \
	Object(PyObject *obj) noexcept { this->obj = obj; } \
	Object(const Object<PyObject*, false> &other) noexcept { obj = other.obj; }

#define CPPY_OWNED_OBJECT_CONSTRUCTORS \
	Object() noexcept {} \
	Object(PyObject *obj) noexcept { this->obj = obj; } \
	Object(const Object<PyObject*, false> &other) noexcept: Object(other.obj, 1) {} \
	Object(PyObject *obj, int) noexcept: Object(obj) { Py_INCREF(obj); } \
	template<class O> \
	Object(Object<O, true> &&other) noexcept: Object(other.obj) \
	{ other.obj = nullptr; }

	template<class T, bool owned=false, template<class, bool> class Object=Object>
	struct Make
	{
		Make() noexcept {}
		Make(PyObject *obj) noexcept {
			static_cast<Object<T,false>*>(this)->obj = obj;
		}
		Make(const Object<PyObject*, false> &other) noexcept: Make(other.obj) {}
	};

	template<class T, template<class, bool> class Object> struct Make<T, true, Object>
	{
		Make() noexcept {}
		Make(PyObject *obj) noexcept {
			static_cast<Object<T,true>*>(this)->obj = obj;
		}
		Make(const Object<PyObject*, false> &other) noexcept: Make(other.obj, 1) {}
		Make(PyObject *obj, int) noexcept: Make(obj) { Py_INCREF(obj); }
		template<class O>
		Make(Object<O, true> &&other) noexcept: Make(other.obj)
		{ other.obj = nullptr; }
	};


	//Wrap a call to some python method that returns an object.
	//If nullptr, implies a python error occurred and throw the
	//corresponding exception.
	static inline PyObject* success(PyObject *obj)
	{
		if (obj) { return obj; }
		throw PyError();
	}
	void throwifnot(bool success, const char *msg="")
	{ if (not success) { throw TypeError(msg); } }


	//Mix in to add checkthrow methods.
	//The Derived class should define a bool check() const
	//method.
	template<class T>
	struct CheckThrow
	{
		using Derived = Object<T, false>;

		const Derived& checkthrow() const&
		{
			throwifnot(derived().check(), derived().name());
			return derived();
		}
		Derived& checkthrow() &
		{
			throwifnot(derived().check(), derived().name());
			return derived();
		}

		Derived&& checkthrow() &&
		{
			throwifnot(derived().check(), derived().name());
			return std::move(derived());
		}
		private:
			Derived& derived() { return static_cast<Derived&>(*this); }
			const Derived& derived() const { return static_cast<Derived&>(*this); }
	};

	// Mixin for derived objects that can be converted to some c type.
	// struct Converter<T>:
	// {
	//	 T toc(PyObject*);
	//	 static T bad;
	// }
	// toc converts PyObject* to a T type.  If the result == bad,
	// then check PyErr_Occurred to see if it was really a bad
	// conversion.
	template<class T, template<class> class Converter>
	struct Convertible
	{
		using Derived = Object<T, false>;

		template<class O>
		O to() const {
			typedef Converter<O> conv;
			O val = conv::toc(static_cast<const Derived*>(this)->obj);
			if (val == conv::badc() && PyErr_Occurred() != NULL)
			{ throw PyError(); }
			return val;
		}
		template<class O> operator O() const { return to<O>(); }
	};
}
#endif//CPPY_MIXIN_HPP
