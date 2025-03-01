#include <cppy/method.hpp>
#include <iostream>


unsigned char func1(float f, double d)
{
	std::cout << "float: " << f << ", and double: " << d << std::endl;
	return 1;
}

struct Functor
{
	unsigned char operator()(float f, double d)
	{
		std::cout << "float: " << f << ", and double: " << d << std::endl;
		return 1;
	}
};

int main(int argc, char *argv[])
{

	{
		typedef cppy::func_info<decltype(func1)> info;
		info::arg_types tup{3.14f, 6.42};
		std::cout << std::get<0>(tup) << " and " << std::get<1>(tup) << std::endl;
		std::cout << sizeof(std::get<0>(tup)) << " and " << sizeof(std::get<1>(tup)) << std::endl;
		std::cout << sizeof(info::return_type) << std::endl;
	}

	{
		typedef cppy::func_info<decltype(&Functor::operator())> info;
		info::arg_types tup{3.14f, 6.42};
		std::cout << std::get<0>(tup) << " and " << std::get<1>(tup) << std::endl;
		std::cout << sizeof(std::get<0>(tup)) << " and " << sizeof(std::get<1>(tup)) << std::endl;
		std::cout << sizeof(info::return_type) << std::endl;
	}

	{
		typedef cppy::func_info<Functor> info;
		info::arg_types tup{3.14f, 6.42};
		std::cout << std::get<0>(tup) << " and " << std::get<1>(tup) << std::endl;
		std::cout << sizeof(std::get<0>(tup)) << " and " << sizeof(std::get<1>(tup)) << std::endl;
		std::cout << sizeof(info::return_type) << std::endl;
	}

	{
		auto thing = [](float f, double d)
		{
			std::cout << "float: " << f << ", and double: " << d << std::endl;
			return '1';
		};
		typedef cppy::func_info<decltype(thing)> info;
		info::arg_types tup{3.14f, 6.42};
		std::cout << std::get<0>(tup) << " and " << std::get<1>(tup) << std::endl;
		std::cout << sizeof(std::get<0>(tup)) << " and " << sizeof(std::get<1>(tup)) << std::endl;
		std::cout << sizeof(info::return_type) << std::endl;
	}



	std::cout << "Hello world" << std::endl;
	return 0;
}
