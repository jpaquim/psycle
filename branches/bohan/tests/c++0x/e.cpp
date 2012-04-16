
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

template<typename T>
struct v : public std::vector<T*> {
	// warning: http://stackoverflow.com/questions/4807643/container-covariance-in-c
	template<typename U>
	operator v<U> const & () const {
		static_assert(boost::is_base_and_derived<U, T>::value, "target element type is base of this element type");
		return reinterpret_cast<v<U> const &>(*this);
	}
};

int main() {
	d d0;
	v<d> vd0;
	vd0.push_back(&d0);
	v<b> const & vb0 = vd0;
	vb0[0]->f();
}