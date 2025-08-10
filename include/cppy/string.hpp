// Python str
// https://docs.python.org/3/c-api/unicode.html
#ifndef CPPY_STRING_HPP
#define CPPY_STRING_HPP

#include <cppy/object.hpp>
#include <cppy/errors.hpp>
#include <cppy/mixin/checkthrow.hpp>
#include <cppy/mixin/sized.hpp>
#include <cppy/proto/sequence.hpp>
#include <cppy/util.hpp>
#include <cppy/convert/int.hpp>

#include <cstring>
#include <string>
#include <ostream>

namespace cppy
{
	struct StringVersion {
#		if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3
	//		PyUnicode_GET_LENGTH
	//		PyUnicode_GetLength
	//		PyUnicode_AsUTF8AndSize
	//		PyUnicode_AsUTF8
	//		PyUnicode_Substring(obj, start, stop)
#		else
	//		PyUnicode_GET_SIZE
	//		PyUnicode_GetSize
	//		PyUnicode_AS_DATA (nothing to get data + size in one) (not checked...)
	//		PySequence_GetItem?
#		endif
	};

	template<> struct Object<const char*&>:
		CheckThrow<Object<const char*&>>,
		Sized<Object<const char*&>, PyUnicode_GetLength>,
		PySequence<Object<const char*&>>,
		Borrowed
	{
		using Borrowed::Borrowed;
		using Sized<Object<const char*&>, PyUnicode_GetLength>::size;
		using PySequence<Object<const char*&>>::getitem;

		bool check() const { return PyUnicode_Check(obj); }
		static constexpr const char* name() { return "str"; }

		std::size_t size_() const { return PyUnicode_GET_LENGTH(obj); }

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

		template<class Start, class Stop>
		Object<const char*> slice(Start &&start, Stop &&stop) const;

		const char* c_str() const {
			const char *ret = PyUnicode_AsUTF8(obj);
			if (ret) { return ret; }
			throw PyError();
		}
		operator const char*() const { return c_str(); }

		bool operator==(const char *s) const {
			auto info = utf8();
			return std::memcmp(info.data, s, info.size) == 0;
		}
		bool operator==(std::string &s) const {
			auto info = utf8();
			return std::memcmp(info.data, s.c_str(), info.size) == 0;
		}
		bool operator>=(const char *s) const {
			auto info = utf8();
			return std::memcmp(info.data, s, info.size) >= 0;
		}
		bool operator>=(std::string &s) const {
			auto info = utf8();
			return std::memcmp(info.data, s.c_str(), info.size) >= 0;
		}
		bool operator<=(const char *s) const {
			auto info = utf8();
			return std::memcmp(info.data, s, info.size) <= 0;
		}
		bool operator<=(std::string &s) const {
			auto info = utf8();
			return std::memcmp(info.data, s.c_str(), info.size) <= 0;
		}
		bool operator>(const char *s) const {
			auto info = utf8();
			return std::memcmp(info.data, s, info.size) > 0;
		}
		bool operator>(std::string &s) const {
			auto info = utf8();
			return std::memcmp(info.data, s.c_str(), info.size) > 0;
		}
		bool operator<(const char *s) const {
			auto info = utf8();
			return std::memcmp(info.data, s, info.size) < 0;
		}
		bool operator<(std::string &s) const {
			auto info = utf8();
			return std::memcmp(info.data, s.c_str(), info.size) < 0;
		}

		std::string string() const {
			UTF8 tmp = utf8();
			return std::string(tmp.data, tmp.size);
		}
		explicit operator std::string() const { return string(); }

		Object<const char*> slice(Py_ssize_t start, Py_ssize_t stop) const;
	};

	template<> struct Object<const char*>: Owned<const char*>
	{
		using Owned<const char*>::Owned;

		Object(const char *data, Py_ssize_t size):
			Owned<const char*>(success(PyUnicode_FromStringAndSize(data, size)))
		{}
		template<std::size_t N> Object(const char (&data)[N]): Object(data, N-1) {}
		Object(const char *data): Object(data, std::strlen(data)) {}
		Object(const std::string &s): Object(s.c_str(), static_cast<Py_ssize_t>(s.size())) {}
	};

	template<class Start, class Stop>
	inline Object<const char*> Object<const char*&>::slice(Start &&start, Stop &&stop) const {
#		if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3
			IntConverter<Object> cvt;
			PyObject *ret = PyUnicode_Substring(
				obj,
				cvt(std::forward<Start>(start)),
				cvt(std::forward<Stop>(stop)));
			if (ret) { return Object<const char*>(ret); }
			throw PyError();
#		else
			return sequence().slice(std::forward<Start>(start), std::forward<Stop>(stop));
#		endif
	}

	template<class V> bool operator==(V &&v, const Object<const char*&>&o) { return o == v; }
	template<class V> bool operator!=(const Object<const char*&>&o, V &&v) { return !(o == v); }
	template<class V> bool operator!=(V &&v, const Object<const char*&>&o) { return !(o == v); }
	template<class V> bool operator<(V &&v, const Object<const char*&>&o) { return (o > v); }
	template<class V> bool operator>(V &&v, const Object<const char*&>&o) { return (o < v); }
	template<class V> bool operator<=(V &&v, const Object<const char*&>&o) { return (o >= v); }
	template<class V> bool operator>=(V &&v, const Object<const char*&>&o) { return (o <= v); }

	//string literal
	template<std::size_t N>
	struct Object<const char (&)[N]>: Object<const char*>
	{ using Object<const char*>::Object; };

	//------------------------------
	//generic object methods that return strs
	//------------------------------
	Object<const char*> Object<>::repr() const
	{ return Object<const char*>(success(PyObject_Repr(obj))); }
	Object<const char*> Object<>::str() const
	{ return Object<const char*>(success(PyObject_Str(obj))); }

	//------------------------------
	//operator<< for ostream
	//------------------------------
	std::ostream& operator<<(std::ostream &o, const Object<const char*&> &str) {
		auto tmp = str.utf8();
		o.write(tmp.data, tmp.size);
		return o;
	}
	std::ostream& operator<<(std::ostream &o, const Object<const char*> &str) {
		auto tmp = str.utf8();
		o.write(tmp.data, tmp.size);
		return o;
	}

	std::ostream& operator<<(std::ostream &o, const Object<> &obj) {
		o << obj.str();
		return o;
	}

	template<class T, typename std::enable_if<decltype(star_is<Object>(std::declval<T&&>()))::value, bool>::type=true>
	std::ostream& operator<<(std::ostream &o, T &&t) {
		o << t->str();
		return o;
	}
}
#endif//CPPY_STRING_HPP
