
/////////////////////////////////////////////////////////////////
// with non-standard container, lacking the Allocator parameter

template<typename T, template<typename> class Underlying>
class Foo1 {
	Underlying<T> data_;
};

template<typename T>
class ctn {
	T * array_;
};

Foo1<int, ctn> x;

/////////////////////////////////////////////////////////////////
// with standard container, having an Allocator parameter

#include <vector>

template<typename T, template<typename, typename> class Underlying, template<typename> class Allocator = std::allocator>
class Foo2 {
	Underlying<T, Allocator<T> > data_;
};

Foo2<int, std::vector> z;

/////////////////////////////////////////////////////////////////
int main() {}

