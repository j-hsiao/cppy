#ifndef CPPY_UTIL_HPP
#define CPPY_UTIL_HPP
#include <type_traits>
#include <utility>
namespace cppy
{
//	//Add exist argument to enabled
//	template<bool enable, class T=bool, class Exist=void> struct enabled {};
//	template<class T, class Exist> struct enabled<true, T, Exist> { typedef T type; };

//	template<class T, class V> struct same { static constexpr bool value = false; };
//	template<class T> struct same<T,T> { static constexpr bool value = true; };

//	template<class T>
//	struct Types
//	{
//		typedef T type;
//		typedef T &ref_type;
//		typedef T &&rref_type;

//		typedef const T const_type;
//		typedef const T &const_ref_type;
//		typedef const T &&const_rref_type;

//		static ref_type ref();
//		static rref_type rref();
//		static const_ref_type cref();
//		static const_rref_type crref();
//	};
//	template<class T> struct Types<T&>: Types<T> {};
//	template<class T> struct Types<T&&>: Types<T> {};
//	template<class T> struct Types<const T>: Types<T> {};
//	template<class T> struct Types<const T&>: Types<T> {};
//	template<class T> struct Types<const T&&>: Types<T> {};

//	//hold arguments
//	template<class...Args> struct Arguments {
//		static constexpr std::size_t count = sizeof...(Args);

//		//Get a type by index
//		template<std::size_t idx, class cls=Arguments<Args...>> struct get
//		{
//			template<class T, class...Remain>
//			static Arguments<Remain...> rtp(Arguments<T, Remain...>);

//			typedef typename get<idx-1, decltype(rtp(cls{}))>::type type;
//		};
//		template<class args> struct get<0, args>
//		{
//			template<class T, class...Remain>
//			static T rtp(Arguments<T, Remain...>);
//			typedef decltype(rtp(args{})) type;
//		};
//	};

//	//function signature
//	template<class T> struct function_signature;
//	template<class T> struct function_signature<T&>: function_signature<T> {};
//	template<class T> struct function_signature<T&&>: function_signature<T> {};

//	//function
//	template<class ret, class...Args>
//	struct function_signature<ret (Args...)>
//	{
//		typedef ret return_type;
//		typedef Arguments<Args...> arguments_type;
//	};

//	//function pointer
//	template<class ret, class...Args>
//	struct function_signature<ret (*)(Args...)>: function_signature<ret (Args...)> {};

//	//------------------------------
//	// member functions
//	//------------------------------
#	define CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(trail) \
	template<class Functor, class ret, class...Args> \
	struct function_signature<ret (Functor::*)(Args...) trail> \
	{ \
		typedef ret return_type; \
		typedef Arguments<Args...> arguments_type; \
	}

//	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(const);
//	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(const&);
//	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(&);
//	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(&&);
//	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR();

//	//functor
//	template<class Functor>
//	struct function_signature: function_signature<decltype(&Functor::operator())> {};

//	//NOTE: function pointers/function types, do NOT store default values
//	//so this would only work with functors.
//	template<class T, class...V>
//	auto is_callable(T&&t, V&&...v)
//		-> typename enabled<true, std::true_type, decltype(t(std::forward<V>(v)...))>::type;
//	std::false_type is_callable(...);

//	//Default constructible type
//	template<class T>
//	class default_constructible
//	{
//		template<class V, typename enabled<true, bool, decltype(V())>::type = true>
//		static std::true_type check(const V&);
//		static std::false_type check(...);

//		public:
//		static constexpr bool value = decltype(check(Types<T>::cref()))::value;
//	};

//	template<class T, bool b> struct Object;
//	template<class T> class is_object
//	{
//		template<class V, bool b>
//		static std::true_type check(const Object<V,b>&);
//		static std::false_type check(...);

//		public:
//			static constexpr bool value = decltype(check(Types<T>::cref()))::value;
//	};

//	template<class T> struct alignas(T) AlignedBuffer
//	{
//		char buf[sizeof(T)];
//		T& ref() { return *reinterpret_cast<T*>(buf); }

//		AlignedBuffer(){}

//		template<class...Args>
//		AlignedBuffer(Args&&...args)
//		{ new(buf) T(std::forward<Args>(args)...); }

//		AlignedBuffer& operator=(const T &inst) {
//			new(buf) T(inst);
//			return *this;
//		}
//	};

}
#endif//CPPY_UTIL_HPP
