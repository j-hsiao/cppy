#ifndef CPPY_MIXIN_DERIVED_HPP
#define CPPY_MIXIN_DERIVED_HPP

#	define MIXIN_DEFINE_DERIVED \
	Derived&& derived() && { return static_cast<Derived&&>(*this); } \
	Derived& derived() & { return static_cast<Derived&>(*this); } \
	const Derived& derived() const& { return static_cast<const Derived&>(*this); }


#endif//CPPY_MIXIN_DERIVED_HPP

