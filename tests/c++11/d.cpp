
#include <iostream>

struct b {
	public: void f() { do_f(); }
	private: virtual void do_f() { std::cout << "b\n"; }
};

struct d : public b {
	private: void do_f() { std::cout << "d\n"; }
};

int main() {
	d d0;
	b & b0 = d0;
	b0.f();
}
