
#include <iostream>
#include <vector>
#include <boost/type_traits/is_base_and_derived.hpp>

struct b {
	int i;
	virtual void f() { std::cout << "b\n"; }
};

struct d : public b {
	int j;
	void f() { std::cout << "d\n"; }
};

template<typename D, typename B = void>
struct v : public v<B, typename B::b> {
	typedef B b;
	
	void push_back(D & d) { B::push_back(d); }
};

template<typename T>
struct v<T, void> : public std::vector<T*> {
	typedef void b;
	
	void push_back(T & t) { std::vector<T*>::push_back(&t); }
	T & operator[](std::vector<T*>::size_type s) { return *std::vector<T*>::operator[](s); }
};

int main() {
	d d0;
	v<d, b> vd0;
	vd0.push_back(d0);
	v<b> const & vb0 = vd0;
	vb0[0].f();
}
