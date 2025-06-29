// Python str
// https://docs.python.org/3/c-api/unicode.html
#ifndef CPPY_STRING_HPP
#define CPPY_STRING_HPP

//This will be automatically included by object.hpp
//for use with repr/str.  It should not be explicitly included.
#include <cppy/errors.hpp>
#include <cstring>
#include <string>
#include <ostream>

namespace cppy
{
	template<> struct Object<const char*&>: Object<PyObject&>
	{
		using Object<PyObject&>::Object;

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
			if (ret.data) { return ret; }
			throw PyError();
		};

		const char* c_str() const {
			const char *ret = PyUnicode_AsUTF8(obj);
			if (ret) { return ret; }
			throw PyError();
		}
		operator const char*() const { return c_str(); }

		std::string string() const {
			UTF8 tmp = utf8();
			return std::string(tmp.data, tmp.size);
		}
	};

	template<> struct Object<const char*>: Object<PyObject, const char*>
	{
		using Object<PyObject, const char*>::Object;

		Object(const char *data, Py_ssize_t size):
			Object<PyObject, const char*>(success(PyUnicode_FromStringAndSize(data, size)), true)
		{}
		template<std::size_t N> Object(const char (&data)[N]): Object(data, N-1) {}
		Object(const char *data): Object(data, std::strlen(data)) {}
		Object(const std::string &s): Object(s.c_str(), static_cast<Py_ssize_t>(s.size())) {}
	};

	//string literal
	template<std::size_t N>
	struct Object<const char (&)[N]>: Object<const char*>
	{ using Object<const char*>::Object; };

	std::ostream& operator<<(std::ostream &o, const Object<const char*> &str)
	{
		auto tmp = str.utf8();
		o.write(tmp.data, tmp.size);
		return o;
	}
}
#endif//CPPY_STRING_HPP
