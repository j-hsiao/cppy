// Python str
// https://docs.python.org/3/c-api/unicode.html
#ifndef CPPY_STRING_HPP
#define CPPY_STRING_HPP

//This will be automatically included by object.hpp
//for use with repr/str.  It should not be explicitly included.
#include "cppy/mixin.hpp"
#include <string>
#include <ostream>

namespace cppy
{
	template<>
	struct Object<const char*, false>: Object<>, CheckThrow<const char*>
	{
		using Base = Object<>;
		using Base::Base;

		bool check() const { return PyUnicode_Check(obj); }
		static constexpr const char* name() { return "str"; }

		std::size_t size() const { return PyUnicode_GET_LENGTH(obj); }

		struct UTF8
		{
			const char *data;
			Py_ssize_t size;
		};
		UTF8 utf8() const {
			UTF8 ret;
			ret.data = PyUnicode_AsUTF8AndSize(obj, &ret.size);
			return ret;
		};

		const char* c_str() const { return PyUnicode_AsUTF8(obj); }
		operator const char*() const { return c_str(); }

		std::string string() const {
			UTF8 tmp = utf8();
			return std::string(tmp.data, tmp.size);
		}
	};

	template<>
	struct Object<const char*, true>: Managed<const char*>
	{
		using Base = Managed<const char*>;
		using Base::Base;

		//Construct new str
		Object(const char *data, Py_ssize_t size):
			Base(success(PyUnicode_FromStringAndSize(data, size)))
		{}
		Object(const std::string &s): Object(s.c_str(), static_cast<Py_ssize_t>(s.size())) {}
	};

	std::ostream& operator<<(std::ostream &o, const Object<const char*, false> &str)
	{
		auto tmp = str.utf8();
		o.write(tmp.data, tmp.size);
		return o;
	}
}
#endif//CPPY_STRING_HPP
