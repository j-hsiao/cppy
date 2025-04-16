#ifndef CPPY_DICT_HPP
#define CPPY_DICT_HPP
#include "cppy/object.hpp"
#include "cppy/errors.hpp"
#include "cppy/mixin.hpp"
#include "cppy/string.hpp"


namespace cppy
{
	struct Dict_{};

	template<bool m=false>
	using Dict = Object<Dict_,m>;

	template<> struct Object<Dict_, false>: CheckThrow<Dict_, Object<>>, Make<Dict_>
	{
		using Make<Dict_>::Make;
		bool check() const { return PyDict_Check(obj); }
		static constexpr const char* name() { return "Dict"; }

		Py_ssize_t size() const {
			Py_ssize_t ret = PyTuple_Size(obj);
			if (ret == -1) { throw PyError(); }
			return ret;
		}

		void clear() { PyDict_Clear(obj); }

		Object<Dict_, true> copy() const; {
			PyObject *ret = PyDict_Copy(obj);
			if (ret)
			{
				return Object<Dict_, true> copy()
			}
		}

		Object<> operator[](const Object<> &key) const
		{
			PyObject *ret = PyDict_GetItemWithError(obj, key.obj);
			if (ret) { return Object<>(ret); }
			if (PyErr_Occurred) { throw PyError(); }
			else { throw Error("Key was not found."); }
		}
		template<class T>
		Object<> operator[](T &&key) const
		{
			return (*this)[Object<T, true>(std::forward<T>(key))];
		}

		struct ItemProxy
		{
			PyObject *key;
			ItemProxy& operator=()
			{
				//TODO
			}
		};

	};

	template<> struct Object<Dict_, false>: Managed<<Dict_>, Make<Dict_, true>
	{
		using Make<Dict_, true>::Make;
	};

	Object<Dict_, true> copy() const {
		PyObject *ret = PyDict_Copy(obj);
		if (ret) { return Object<Dict_, true>(ret); }
		else { throw PyError(); }
	}

}
#endif//CPPY_DICT_HPP
