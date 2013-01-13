
namespace std {
	template<typename T>
	class allocator {};
	
	template<typename T, typename Allocator = allocator<T>>
	class vector {
		typedef Allocator allocator_type;
		T * array_;
	};
}

/////////////////////////////////////////////////////////////////

template<typename T, template<typename> class Underlying>
class Foo {
	Underlying<T> data_;
};

/////////////////////////////////////////////////////////////////
// with non-standard container, lacking the Allocator parameter

template<typename T>
class ctn {
	T * array_;
};

Foo<int, ctn> x;

/////////////////////////////////////////////////////////////////
// using C++2011 template aliases

template<template<typename, typename> class Underlying, template<typename> class Allocator = std::allocator>
struct remove_allocator_parameter {
	template<typename T>
	using template_type = Underlying<T, Allocator<T>>;
};

/////////////////////////////////////////////////////////////////
// with standard container, having an Allocator parameter

Foo<int, remove_allocator_parameter<std::vector>::template_type> y;

/////////////////////////////////////////////////////////////////
int main() {}

