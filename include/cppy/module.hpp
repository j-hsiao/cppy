#ifndef CPPY_MODULE_HPP
#define CPPY_MODULE_HPP
#include <cppy/object.hpp>
namespace cppy
{

	template<class T> struct statesize { static constexpr Py_ssize_t size = static_cast<Py_ssize_t>(sizeof(T)); };
	template<> struct statesize<void> { static constexpr Py_ssize_t size = -1; };

	struct Module {
		struct PyModuleDef mdef;
		PyObject md;

		template<
			class State=void,
			class Methods=PyMethodDef*,
			class Slots=PyModuleDef_Slot*
		>
		Module(
			const char *name,
			State state,
			Methods methods=nullptr,
			Slots slots=nullptr,
			traverseproc=nullptr,
			inquiry=nullptr,
			freefunc=nullptr
		):
			mdef{
				PyModuleDef_HEAD_INIT,
				name,
				docs,
				statesize<State>::size,
				methods,
				slots,
				traverse,
				clear,
				free
			},
			md(PyModule_Create(&mdef))
		{
			//TODO multistage initialization?
		}



	};

}
#endif//CPPY_MODULE_HPP
