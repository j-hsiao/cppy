#ifndef CPPY_MIXIN_KEYED_HPP
#define CPPY_MIXIN_KEYED_HPP

namespace cppy {

	template<class T>
	struct ArrowWrap {
		T item;
		T* operator->() { return &item; }
	};

	//Item proxy for getitem/setitem
	template<class Container_, class Key>
	struct ItemProxy
	{
		typedef typename std::remove_reference<Container_>::type Container;
		Container_ obj;
		Key key;

		template<class T, class V>
		ItemProxy(T &&cont, V &&k): obj(std::forward<T>(cont)), key(std::forward<V>(k)) {}


		typedef decltype(const_cast<const Container&>(obj).getitem(key)) Value;
		ArrowWrap<Value> operator->() const {
			return {const_cast<const Container&>(obj).getitem(key)};
		}

		operator Value() const
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
	template<class T, class constval, template<class> class Object=Object>
	struct Mapping {
		using Derived = Object<T>;

		//const __getitem__
		template<class Key> constval getitem(const Key &key) const {
			return static_cast<const Derived&>(*this).getitem(Object<Key>(key).obj);
		}
		template<class Key> constval getitem(const Object<Key> &key) const {
			return static_cast<const Derived&>(*this).getitem(key.obj);
		}
		template<class Key> constval operator[](const Key &key) const& {
			return static_cast<const Derived&>(*this).getitem(key);
		}

		//__getitem__ proxy
		template<class Key> ItemProxy<Derived&, const Key&> getitem(const Key &key) {
			return ItemProxy<Derived&, const Key&>(static_cast<Derived&>(*this), key);
		}
		template<class Key> ItemProxy<Derived&, const Key&> operator[](const Key &key) & {
			return getitem(key);
		}

		//__setitem__
		//template<class Key, class Value>
		//void setitem(Key &&key, const Object<Value> &val) {
		//	static_cast<Derived&>(*this).setitem(std::forward<Key>(key), val.obj);
		//}
		//template<class Key, class Value>
		//void setitem(Key &&key, const Value &val) {
		//	static_cast<Derived&>(*this).setitem(std::forward<Key>(key), Object<Value>(val).obj);
		//}
		//template<class Key, class Value>
		//void setitem(Key &&key, const Value &val) {
		//	static_cast<Derived&>(*this).setitem(std::forward<Key>(key), Object<Value>(val).obj);
		//}








	};

}
#endif//CPPY_MIXIN_KEYED_HPP
