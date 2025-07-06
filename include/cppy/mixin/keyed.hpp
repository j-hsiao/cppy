#ifndef CPPY_MIXIN_KEYED_HPP
#define CPPY_MIXIN_KEYED_HPP

#include <cppy/mixin/derived.hpp>

namespace cppy {

	//Item proxy for getitem/setitem
	template<class Container_, class Key>
	struct ItemProxy
	{
		typedef typename std::remove_reference<Container_>::type Container;
		Container_ obj;
		Key key;

		template<class T, class V>
		ItemProxy(T &&cont, V &&k): obj(std::forward<T>(cont)), key(std::forward<V>(k)) {}

		decltype(const_cast<const Container&>(obj).getitem(key)) operator()() const
		{ return const_cast<const Container&>(obj).getitem(key); }

		operator decltype(const_cast<const Container&>(obj).getitem(key))() const
		{ return const_cast<const Container&>(obj).getitem(key); }

		template<class T>
		ItemProxy& operator=(T &&value) {
			obj.setitem(key, std::forward<T>(value));
			return *this;
		}
	};

	template<class T> struct Object;
	//define other instances of getitem, setitem, and operator[]
	//Base class should define:
	//	getitem(PyObject*) const
	//	setitem(PyObject*, PyObject*)
	template<class T, template<class> class Object=Object>
	struct Keyed {
		using Derived = Object<T>;

		template<class Key> decltype(auto) getitem(const Key &key) const {
			return derived().getitem(Object<Key>(key).obj);
		}
		template<class Key> decltype(auto) getitem(const Object<Key> &key) const {
			return derived().getitem(key.obj);
		}

		template<class Key> ItemProxy<Derived&, const Key&> getitem(const Key &key) {
			return ItemProxy<Derived&, const Key&>(derived(), key);
		}

		decltype(auto) operator[](PyObject *p) const { return derived().getitem(p); }
		template<class Key> decltype(auto) operator[](Key &&key) const {
			return getitem(std::forward<Key>(key));
		}
		template<class Key> decltype(auto) operator[](Key &&key) {
			return getitem(std::forward<Key>(key));
		}

		private:
			MIXIN_DEFINE_DERIVED
	};

}
#endif//CPPY_MIXIN_KEYED_HPP
