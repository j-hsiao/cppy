#ifndef CPPY_MIXIN_KEYED_HPP
#define CPPY_MIXIN_KEYED_HPP

#include <cppy/errors.hpp>
#include <type_traits>
#include <utility>
namespace cppy {

	// Wrap an object to allow operator-> to access its members.
	template<class T>
	struct ArrowWrap {
		T item;
		T* operator->() { return &item; }
	};

	//Item proxy to allow get/assign via operator[]
	template<class Container_, class Key>
	struct ItemProxy {
		typedef typename std::remove_reference<Container_>::type Container;
		Container_ obj;
		//If a temporary (rvalue) key, then it should be stored to keep it alive until use.
		typename std::conditional<
			std::is_rvalue_reference<Key>::value,
			typename std::remove_const<typename std::remove_reference<Key>::type>::type,
			Key>::type key;

		template<class T, class K>
		ItemProxy(T &&cont, K &&k): obj(std::forward<T>(cont)), key(std::forward<K>(k)) {}

		typedef decltype(obj.getitem(key)) Value;
		ArrowWrap<Value> operator->() const { return { obj.getitem(key) }; }
		operator Value() const { return obj.getitem(key); }

		template<class T>
		ItemProxy& operator=(T &&value) {
			obj.setitem(key, std::forward<T>(value));
			return *this;
		}
		template<class C, class K>
		ItemProxy& operator=(const ItemProxy<C,K> &value) {
			obj.setitem(key, value.obj.getitem(value.key));
			return *this;
		}
	};

	template<class T> struct Object;

	//define other instances of getitem, setitem, and operator[]
	//Base class should define:
	//	getitem(PyObject*) const
	//	setitem(PyObject*, PyObject*)
	template<class T, template<class> class Object=Object>
	struct Mapping {
		using Derived = Object<T>;

		//__getitem__
		template<class Key> decltype(auto) getitem(const Key &key) const {
			return static_cast<const Derived&>(*this).getitem(Object<Key>(key).obj);
		}
		template<class Key> decltype(auto) getitem(const Object<Key> &key) const {
			return static_cast<const Derived&>(*this).getitem(key.obj);
		}

		//__setitem__
		template<class Key, class Value>
		void setitem(Key &&key, const Object<Value> &val) {
			static_cast<Derived&>(*this).setitem(std::forward<Key>(key), val.obj);
		}
		template<class Key, class Value>
		void setitem(Key &&key, const Value &val) {
			static_cast<Derived&>(*this).setitem(std::forward<Key>(key), Object<Value>(val).obj);
		}
		template<class Key>
		void setitem(const Key &key, PyObject *obj) {
			static_cast<Derived&>(*this).setitem(Object<Key>(key).obj, obj);
		}
		template<class Key>
		void setitem(const Object<Key> &key, PyObject *obj) {
			static_cast<Derived&>(*this).setitem(key.obj, obj);
		}

		// operator[] gives a proxy to support syntax:
		// auto value = mapping[key];
		// mapping[key] = value;
		template<class Key>
		ItemProxy<const Derived&, Key&&> operator[](Key &&key) const& {
			return ItemProxy<const Derived&, Key&&>(
				static_cast<const Derived&>(*this), std::forward<Key>(key));
		}
		template<class Key>
		ItemProxy<Derived&, Key&&> operator[](Key &&key) & {
			return ItemProxy<Derived&, Key&&>(
				static_cast<Derived&>(*this), std::forward<Key>(key));
		}
		template<class Key>
		ItemProxy<Derived, Key&&> operator[](Key &&key) && {
			//Derived to keep a this alive until used.
			return ItemProxy<Derived, Key&&>(
				static_cast<Derived&&>(*this), std::forward<Key>(key));
		}
	};
}
#endif//CPPY_MIXIN_KEYED_HPP
