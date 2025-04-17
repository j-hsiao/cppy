#ifndef CPPY_UTIL_HPP
namespace cppy
{
	template<bool enable, class T=bool> struct enabled {};
	template<class T> struct enabled<true, T> { typedef T type; };

	template<class T, class V> struct same { static constexpr bool value = false; };
	template<class T> struct same<T,T> { static constexpr bool value = true; };

	template<class T>
	struct Types
	{
		typedef T type;
		typedef T &ref_type;
		typedef T &&rref_type;

		typedef const T const_type;
		typedef const T &const_ref_type;
		typedef const T &&const_rref_type;

		static ref_type ref();
		static rref_type rref();
		static const_ref_type cref();
		static const_rref_type crref();
	};
	template<class T> struct Types<T&>: Types<T> {};
	template<class T> struct Types<T&&>: Types<T> {};
	template<class T> struct Types<const T>: Types<T> {};
	template<class T> struct Types<const T&>: Types<T> {};
	template<class T> struct Types<const T&&>: Types<T> {};

	//hold arguments
	template<class...Args> struct Arguments {
		static constexpr std::size_t size = sizeof...(Args);

		//Get a type by index
		template<std::size_t idx, class cls=Arguments<Args...>> struct get
		{
			template<class T, class...Remain>
			static Arguments<Remain...> rtp(Arguments<T, Remain...>);

			typedef typename get<idx-1, decltype(rtp(cls{}))>::type type;
		};
		template<class args> struct get<0, args>
		{
			template<class T, class...Remain>
			static T rtp(Arguments<T, Remain...>);
			typedef decltype(rtp(args{})) type;
		};
	};

	//function signature
	template<class T> struct function_signature;
	template<class T> struct function_signature<T&>: function_signature<T> {};
	template<class T> struct function_signature<T&&>: function_signature<T> {};

	//function
	template<class ret, class...Args>
	struct function_signature<ret (Args...)>
	{
		typedef ret return_type;
		typedef Arguments<Args...> arguments_type;
	};

	//function pointer
	template<class ret, class...Args>
	struct function_signature<ret (*)(Args...)>: function_signature<ret (Args...)> {};

	//------------------------------
	// member functions
	//------------------------------
#	define CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(trail) \
	template<class Functor, class ret, class...Args> \
	struct function_signature<ret (Functor::*)(Args...) trail> \
	{ \
		typedef ret return_type; \
		typedef Arguments<Args...> arguments_type; \
	}

	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(const);
	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(const&);
	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(&);
	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR(&&);
	CPPY_MAKE_FUNCTION_SIGNATURE_MEMBER_FUNCTION_PTR();

	//functor
	template<class Functor>
	struct function_signature: function_signature<decltype(&Functor::operator())> {};

	struct True { static constexpr bool value = true; };
	struct False { static constexpr bool value = false; };

	template<class V, class B=bool> struct Exist { typedef B type;};

	//Default constructible type
	template<class T>
	class default_constructible
	{
		template<class V, typename Exist<decltype(V())>::type = true>
		static True check(const V&);
		static False check(...);

		public:
		static constexpr bool value = decltype(check(Types<T>::cref()))::value;
	};

	template<class T> struct alignas(T) AlignedBuffer { static char buf[sizeof(T)] };

}
#endif//CPPY_UTIL_HPP
