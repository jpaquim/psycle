#include <iostream>

struct named {
	virtual std::string const name() const = 0;
};

template<typename Derived, typename Base = void>
struct covariant : covariant<Base> {
	covariant(Derived d) : covariant<Base>(d) {}
	Derived const get() const { return static_cast<Derived>(covariant<Base>::get()); }
};

template<typename T>
struct covariant<T> {
	covariant(T t) : t(t) {}
	T const get() const { return t; }
	private: T t;
};

struct base : named {
	std::string const name() const override { return "base"; }
};

struct derived : base {
	std::string const name() const override { return "derived"; }
};

void print(covariant<named*> const & c) {
	std::cout << c.get()->name() << '\n';
};

int main() {
	derived d;
	{ covariant<derived*> cd(&d); derived * d = cd.get(); }
	covariant<base*, named*> cb(&d);
	{ base * b = cb.get(); }
	print(cb);
	covariant<derived*, named*> cd(&d);
	{ derived * d = cd.get(); }
	print(cd);
	return 0;
}
