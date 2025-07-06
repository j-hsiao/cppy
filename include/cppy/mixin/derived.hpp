#ifndef CPPY_MIXIN_DERIVED_HPP
#define CPPY_MIXIN_DERIVED_HPP

#	define MIXIN_DEFINE_DERIVED \
	Derived&& derived() && { return reinterpret_cast<Derived&&>(*this); } \
	Derived& derived() & { return reinterpret_cast<Derived&>(*this); } \
	const Derived& derived() const& { return reinterpret_cast<const Derived&>(*this); }


#endif//CPPY_MIXIN_DERIVED_HPP

