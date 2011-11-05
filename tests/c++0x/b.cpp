
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
// using C++2011 template aliases

template<typename T, template<typename> class Underlying>
class Foo1 {
	Underlying<T> data_;
};

template<template<typename, typename> class Underlying, template<typename> class Allocator = std::allocator>
struct remove_allocator_parameter {
	template<typename T>
	using template_type = Underlying<T, Allocator<T>>;
};

Foo1<int, remove_allocator_parameter<std::vector>::template_type> x;

/////////////////////////////////////////////////////////////////
int main() {}

