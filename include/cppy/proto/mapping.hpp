#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/convert/pyobject.hpp>
#include <cppy/mixin/sized.hpp>


namespace cppy  {
	template<class Base> struct PyMapping;


	template<class T, template<class>class Object>
	struct PyMapping:
		CheckThrow<PyMapping<Object<T&>>>,
		Sized<Object<T&>, PyMapping_Size>
	{
		using Base = Object<T&>;

		PyMapping<Base>&& sequence() && { return std::move(*this); }
		PyMapping<Base>& sequence() & { return *this; }
		const PyMapping<Base>& sequence() const& { return *this; }

		bool check() const
		{ return PyMapping_Check(static_cast<const Base*>(this)->obj); }


		//------------------------------
		//get item
		//------------------------------
		//raw PyObject* or NULL, null means error occurred
		template<class Key>
		PyObject* getitem_(Key &&key) const {
			return PyObject_GetItem(
				static_cast<const Base*>(this)->obj,
				PyObjectConverter<Object>{}(std::forward<Key>(key)));
		}
		PyObject* getitem_(const char *key) const
		{ return PyMapping_GetItemString(static_cast<const Base*>(this)->obj, key); }

		template<class Key>
		Object<PyObject> getitem(Key &&key) const {
			auto ret = getitem_(std::forward<Key>(key));
			if (ret) { return Object<PyObject>(ret); }
			throw PyError();
		}

		template<class Key, class Default>
		Object<PyObject> getitem(Key &&key, Default &&dval) {
			if (PyObject *ptr = getitem_(std::forard<Key>(key)))
			{ return Object<PyObject>(ptr); }
			else
			{ return Object<PyObject>(StealConverter<Object>{}(std::forward<Default>(dval))); }
		}

		template<class Key>
		Object<PyObject> getitem(Key &&key, std::nullptr_t) {
			if (PyObject *ptr = getitem_(std::forward<Key>(key)))
			{ return Object<PyObject>(ptr); }
			else
			{ return Object<PyObject>(nullptr); }
		}

	};
}
