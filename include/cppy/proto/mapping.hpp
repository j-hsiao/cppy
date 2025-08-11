#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/convert/pyobject.hpp>
#include <cppy/mixin/sized.hpp>


namespace cppy  {
	template<class Base> struct PyMapping;


	template<class T, template<class>class Object>
	struct PyMapping<Object<T&>>:
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
		private:
			template<class Key>
			PyObject* getitem_(Key &&key) const {
				return PyObject_GetItem(
					static_cast<const Base*>(this)->obj,
					PyObjectConverter<Object>{}(std::forward<Key>(key)));
			}
			PyObject* getitem_(const char *key) const
			{ return PyMapping_GetItemString(static_cast<const Base*>(this)->obj, key); }

#			if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
			template<class Key>
			PyObject* getitem_opt(Key &&key) const {
				PyObject *ret;
				if (
					PyMapping_GetOptionalItem(
						static_cast<const Base*>(this)->obj,
						PyObjectConverter<Object>{}(std::forward<Key>(key)),
						&ret) < 0)
				{ throw PyError(); }
				rturn ret;
			}

			PyObject* getitem_opt(const char *key) const {
				PyObject *ret;
				if (
					PyMapping_GetOptionalItemString(
						static_cast<const Base*>(this)->obj, key, &ret) < 0)
				{ throw PyError(); }
				rturn ret;
			}
#			endif

		public:
			template<class Key>
			Object<PyObject> getitem(Key &&key) const {
				auto ret = getitem_(std::forward<Key>(key));
				if (ret) { return Object<PyObject>(ret); }
				throw PyError();
			}

			template<class Key, class Default>
			Object<PyObject> getitem(Key &&key, Default &&dval) {
#				if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
					if (PyObject *ptr = getitem_opt(std::forward<Key>(key)))
					{ return Object<PyObject>(ptr); }
#				else
					if (PyObject *ptr = getitem_(std::forard<Key>(key)))
					{ return Object<PyObject>(ptr); }
					else
					{ PyErr_Clear(); }
#				endif
				return Object<PyObject>(StealConverter<Object>{}(std::forward<Default>(dval)));
			}

			template<class Key>
			Object<PyObject> getitem(Key &&key, std::nullptr_t) {
#				if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
					if (PyObject *ptr = getitem_opt(std::forward<Key>(key)))
					{ return Object<PyObject>(ptr); }
#				else
					if (PyObject *ptr = getitem_(std::forard<Key>(key)))
					{ return Object<PyObject>(ptr); }
					else
					{ PyErr_Clear(); }
#				endif
				return Object<PyObject>(nullptr);
			}

			//------------------------------
			//setitem with const char*
			//------------------------------
			private:
				template<class Value>
				int setitem_(const char *key, Value &&value) {
					return PyMapping_SetItemString(
						static_cast<Base*>(this)->obj, key,
						PyObjectConverter<Object>{}(std::forward<Value>(value)));
				}
				template<class Key, class Value>
				int setitem_(Key &&key, Value &&value) {
					PyObjectConverter<Object> cvt;
					return PyObject_SetItemString(
						static_cast<Base*>(this)->obj,
						cvt(std::forward<Key>(key)),
						cvt(std::forward<Value>(value)));
				}

			public:
			template<class Key, class Value>
			PyMapping<Object<T&>> setitem_(Key &&key, Value &&value) {
				if (PyMapping_SetItemString(
						static_cast<Base*>(this)->obj, key,
						PyObjectConverter<Object>{}(std::forward<Value>(value))) == -1)
				{ throw PyError(); }
				return *this;
			}

			//------------------------------
			//delitem
			//------------------------------
			PyMapping<Object<T&>> delitem(const char *key) {
				if (PyMapping_DelItemString(static_cast<Base*>(this)->obj, key) == -1)
				{ throw PyError(); }
				return *this;
			}
			template<class Key>
			PyMapping<Object<T&>> delitem(Key &&key) {
				if (PyMapping_DelItemString(
					static_cast<Base*>(this)->obj,
					PyObjectConverter<Object>{}(std::forward<Key>(key))) == -1)
				{ throw PyError(); }
				return *this;
			}
	};
}
