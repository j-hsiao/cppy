//Wrap python callable object.
#ifndef CPPY_FUNCTION_HPP
#define CPPY_FUNCTION_HPP

namespace cppy
{
	template<bool m> struct Callable;

	template<> struct Callable<false>: Object<Callable<false>, false>
	{
	};

	template<>
	struct Callable<true>: Managed<Callable<false>>
	{
	};
}
#endif//CPPY_FUNCTION_HPP
