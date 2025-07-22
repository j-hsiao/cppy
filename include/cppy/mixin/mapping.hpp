#ifndef CPPY_MIXIN_KEYED_HPP
#define CPPY_MIXIN_KEYED_HPP

#include <cppy/errors.hpp>
#include <type_traits>
#include <utility>
namespace cppy {
	// Wrap an object to allow operator-> to a temporary.
	template<class T>
	struct ArrowWrap {
		T item;
		T* operator->() { return &item; }

		T& operator*() { return item; }
		const T& operator*() const { return item; }
	};

	//Item proxy to allow get/assign via operator[]
	template<class Container_, class Key>
	struct ItemProxy {
		typedef typename std::remove_reference<Container_>::type Container;
		Container_ obj;
		//If a temporary (rvalue) key, then it should be stored to keep it alive until use.
		typename std::conditional<
			std::is_rvalue_reference<Key>::value,
			typename std::decay<Key>::type,
			Key>::type key;

		template<class T, class K>
		ItemProxy(T &&container, K &&key):
			obj(std::forward<T>(container)),
			key(std::forward<K>(key))
		{}

		typedef decltype(obj.getitem(key)) Value;
		ArrowWrap<Value> operator->() const { return { obj.getitem(key) }; }
		operator Value() const { return obj.getitem(key); }
		Value operator*() const { return obj.getitem(key); }

		template<class V>
		ItemProxy& operator=(V &&value) {
			obj.setitem(key, std::forward<V>(value));
			return *this;
		}
		template<class C, class K>
		ItemProxy& operator=(const ItemProxy<C,K> &value) {
			obj.setitem(key, value.obj.getitem(value.key));
			return *this;
		}
	};

	template<class T> struct Object;

	//Implement operator[]
	template<class Derived>
	struct Mapping {
		// operator[] gives a proxy to support syntax:
		// auto value = mapping[key];
		// mapping[key] = value;
		template<class K>
		ItemProxy<const Derived&, K&&> operator[](K &&key) const& {
			return ItemProxy<const Derived&, K&&>(
				static_cast<const Derived&>(*this), std::forward<K>(key));
		}
		template<class K>
		ItemProxy<Derived&, K&&> operator[](K &&key) & {
			return ItemProxy<Derived&, K&&>(
				static_cast<Derived&>(*this), std::forward<K>(key));
		}
		template<class K>
		ItemProxy<Derived, K&&> operator[](K &&key) && {
			//Derived to keep a this alive until used.
			return ItemProxy<Derived, K&&>(
				static_cast<Derived&&>(*this), std::forward<K>(key));
		}
	};
}
#endif//CPPY_MIXIN_KEYED_HPP
