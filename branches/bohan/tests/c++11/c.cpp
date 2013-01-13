
#if 0
namespace std {
	template<typename T>
	class allocator {};
	
	template<typename T, typename Allocator = allocator<T>>
	class vector {
		typedef Allocator allocator_type;
		T * array_;
	};
}
#endif

/////////////////////////////////////////////////////////////////
// using C++2011 variadic templates

template<typename T, template<typename, typename...> class Underlying, template<typename> class... ExtraArgs>
class Foo {
	Underlying<T, ExtraArgs<T>...> data_;
};

/////////////////////////////////////////////////////////////////
// with non-standard container, lacking the Allocator parameter

template<typename T>
class ctn {
	T * array_;
};

Foo<int, ctn> x;

/////////////////////////////////////////////////////////////////
// with standard container, having an Allocator parameter

#include <vector>
#include <list>

Foo<int, std::vector> y;
Foo<int, std::list, std::allocator> z;

/////////////////////////////////////////////////////////////////
int main() {}

