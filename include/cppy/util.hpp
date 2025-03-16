namespace cppy
{
	template<bool enable, class T=bool> struct enabled {};
	template<class T> struct enabled<true, T> { typedef T type; };

	template<class T, V> struct same { static constexpr bool value = false; };
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
	};
	template<class T> struct Types<T&>: Types<T> {};
	template<class T> struct Types<T&&>: Types<T> {};
	template<class T> struct Types<const T>: Types<T> {};
	template<class T> struct Types<const T&>: Types<T> {};
	template<class T> struct Types<const T&&>: Types<T> {};
}
