//Python Buffer protocol
//https://docs.python.org/3/c-api/buffer.html

#ifndef CPPY_BUFFER_HPP
#define CPPY_BUFFER_HPP

#include "cppy/object.hpp"

#include <utility>
namespace cppy
{
		// format: https://docs.python.org/3/library/struct.html#module-struct
		// c  char
		// b  schar
		// B  uchar
		// ?  bool
		// h  short
		// H  ushort
		// i  int
		// I  uint
		// l  long
		// L  ulong
		// q  long long
		// Q  ulong long
		// n  ssize_t
		// N  size_t
		// e  half
		// f  float
		// d  double
		// s  string
		// p  max 255 length string "Pascal string"
		// P  void*
		//flags: 
		// PyBuf_WRITABLE
		// PyBUF_FORMAT: must fill in the format field
		//
		// layout
		// PyBUF_INDIRECT   random-ish access
		// PyBUF_STRIDES    ND, with strides (might not be contiguous but ordered)
		// PyBUF_ND         ND contiguous
		// PyBUF_SIMPLE     1D contiguous
		//
		// contiguous: same as PyBUF_STRIDES
		// PyBUF_C_CONTIGUOUS
		// PyBUF_F_CONTIGUOUS
		// PyBUF_ANY_CONTIGUOUS


	template<bool Managed, int flags=PyBUF_STRIDES>
	struct Buffer: BasicObject<Buffer<Managed>, Managed>
	{
		using Base = BasicObject<Buffer<Managed>, Managed>;

		template<class...T>
		Buffer(T&&...args): Base(std::forward<T>(args)...)
		{
			if (PyObject_GetBuffer(this->obj, &buf, flags) != 0)
			{ throw Error(); }
		}


		Buffer(Buffer &&o): Base(o)
		{
			buf = o.buf;
			o.buf.obj = NULL;
		}

		//number of bytes
		std::size_t size() const { return buf.len; }
		std::size_t itemsize() const { return buf.itemsize; }
		std::size_t ndim() const { return buf.ndim; }
		const Py_ssize_t* shape() const { return buf.shape; }
		//strides in bytes
		const Py_ssize_t* strides() const { return buf.strides; }

		~Buffer() { if (buf.obj) { PyBuffer_Release(&buf); } }

		bool check() const { return PyObject_CheckBuffer(this->obj); }

		Py_buffer buf;
	};
}
#endif//CPPY_BUFFER_HPP
