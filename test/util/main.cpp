#include <cppy/util.hpp>
#include <iostream>
#include <type_traits>


template<class T=void>
struct ta {};
struct tb: ta<> {};

struct a{};

struct b: a{};

struct c {
	a operator*() const { return {}; }
};


int main(int argc, char *argv[]) {
	{
#		define CHECK(...) \
		std::cout << #__VA_ARGS__ << ": "; \
		if (__VA_ARGS__) \
		{ std::cout << "pass" << std::endl; } \
		else { \
			std::cout << "fail" << std::endl; \
			return 1; \
		}

		{
			std::cout << "------------------------------" << std::endl
			          << "non-template is" << std::endl
			          << "------------------------------" << std::endl;
			a ainst;
			const a cainst;
			b binst;
			const b cbinst;
			c cinst;
			const c ccinst;

			CHECK(decltype(cppy::is<a>(a{}))::value)
			CHECK(decltype(cppy::is<a>(ainst))::value)
			CHECK(decltype(cppy::is<a>(cainst))::value)

			CHECK(decltype(cppy::is<a>(b{}))::value)
			CHECK(decltype(cppy::is<a>(binst))::value)
			CHECK(decltype(cppy::is<a>(cbinst))::value)

			CHECK(!decltype(cppy::is<a>(c{}))::value)
			CHECK(!decltype(cppy::is<a>(cinst))::value)
			CHECK(!decltype(cppy::is<a>(ccinst))::value)

			CHECK(decltype(cppy::star_is<a>(c{}))::value)
			CHECK(decltype(cppy::star_is<a>(cinst))::value)
			CHECK(decltype(cppy::star_is<a>(ccinst))::value)
		}
		{
			std::cout << "------------------------------" << std::endl
			          << "templated is" << std::endl
			          << "------------------------------" << std::endl;
			ta<> ainst;
			const ta<> cainst;
			tb binst;
			const tb cbinst;
			c cinst;
			const c ccinst;

			CHECK(decltype(cppy::is<ta>(ta<>{}))::value)
			CHECK(decltype(cppy::is<ta>(ainst))::value)
			CHECK(decltype(cppy::is<ta>(cainst))::value)

			CHECK(decltype(cppy::is<ta>(tb{}))::value)
			CHECK(decltype(cppy::is<ta>(binst))::value)
			CHECK(decltype(cppy::is<ta>(cbinst))::value)

			CHECK(decltype(cppy::is<ta<>>(tb{}))::value)
			CHECK(decltype(cppy::is<ta<>>(binst))::value)
			CHECK(decltype(cppy::is<ta<>>(cbinst))::value)

			CHECK(!decltype(cppy::is<ta>(c{}))::value)
			CHECK(!decltype(cppy::is<ta>(cinst))::value)
			CHECK(!decltype(cppy::is<ta>(ccinst))::value)
		}
	}
	{
		std::cout << "------------------------------" << std::endl
		          << "as" << std::endl
		          << "------------------------------" << std::endl;
		a ainst;
		const a cainst;
		b binst;
		const b cbinst;
		c cinst;
		const c ccinst;

		CHECK(std::is_same<decltype(cppy::as<a>(ainst)), a&>::value)
		CHECK(std::is_same<decltype(cppy::as<a>(cainst)), const a&>::value)
		CHECK(std::is_same<decltype(cppy::as<a>(a{})), a&&>::value)

		CHECK(std::is_same<decltype(cppy::as<a>(binst)), a&>::value)
		CHECK(std::is_same<decltype(cppy::as<a>(cbinst)), const a&>::value)
		CHECK(std::is_same<decltype(cppy::as<a>(b{})), a&&>::value)

		CHECK(std::is_same<decltype(cppy::as<a>(cinst)), c&>::value)
		CHECK(std::is_same<decltype(cppy::as<a>(ccinst)), const c&>::value)
		CHECK(std::is_same<decltype(cppy::as<a>(c{})), c&&>::value)
	}
	{
		std::cout << "------------------------------" << std::endl
		          << "startype" << std::endl
		          << "------------------------------" << std::endl;
		a ainst;
		c cinst;
		CHECK(std::is_same<decltype(cppy::startype(ainst)), void>::value)
		CHECK(std::is_same<decltype(cppy::startype(cinst)), a>::value)
	}



	return 0;
}
