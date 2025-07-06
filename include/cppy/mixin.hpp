#ifndef CPPY_MIXIN_HPP
#define CPPY_MIXIN_HPP

#include <cppy/errors.hpp>

#include <utility>
namespace cppy
{
	template<class T> struct Object;

#	define MIXIN_DEFINE_DERIVED \
	Derived&& derived() && { return static_cast<Derived&&>(*this); } \
	Derived& derived() & { return static_cast<Derived&>(*this); } \
	const Derived& derived() const& { return static_cast<Derived&>(*this); }

	//Mix in to add checkthrow methods.
	//The Derived class should define a bool check() const
	//method.
	template<class T>
	struct CheckThrow
	{
		using Derived = Object<T>;
#		define CHECKTHROW(prefix, suffix) \
		prefix Derived suffix checkthrow() prefix suffix { \
			throwifnot(derived().check(), derived().name()); \
			return derived(); \
		}
		CHECKTHROW(const, &)
		CHECKTHROW(, &)
		CHECKTHROW(, &&)
#		undef CHECKTHROW

		private:
			MIXIN_DEFINE_DERIVED
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
		using Derived = Object<T>;

		template<class O>
		O to() const {
			typedef Converter<O> conv;
			O val = conv::toc(static_cast<const Derived*>(this)->obj);
			if (val == conv::badc() && PyErr_Occurred() != NULL) { throw PyError(); }
			return val;
		}
		template<class O> operator O() const { return to<O>(); }
	};


	//define other instances of getitem, setitem, and operator[]
	//Base class should define:
	//	getitem(PyObject*) const
	//	setitem(PyObject*, PyObject*)
	template<class T>
	struct Keyed {
		using Derived = Object<T>;

		decltype(auto) getitem(const Object<> &key) const {
			return derived().getitem(key.obj);
		}
		template<class Key> decltype(auto) getitem(const Key &key) const {
			return derived().getitem(Object<Key>(key).obj);
		}
		template<class Key> decltype(auto) getitem(const Object<Key> &key) const {
			return derived().getitem(key.obj);
		}

		template<class Key> ItemProxy<Derived&, Key&> getitem(const Key &key) {
			return ItemProxy<Derived&, Key&>(derived(), key);
		}


		template<class Key> decltype(auto) operator[](Key &&key) const {
			return derived().getitem(std::forward<Key>(key));
		}
		template<class Key> decltype(auto) operator[](Key &&key) {
			return derived().getitem(std::forward<Key>(key));
		}

		private:
			MIXIN_DEFINE_DERIVED
	};
}
#endif//CPPY_MIXIN_HPP
