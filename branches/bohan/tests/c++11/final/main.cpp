#include "d.hpp"
#include <iostream>
#include <vector>

namespace test {
namespace {
	int test1() {
		std::vector<b*> v;
		for(int i = 0; i < 100; ++i)
			if(i % 2 == 0)
				v.push_back(new b);
			else
				v.push_back(new d);
		int i = 0;
		for(int x = 0; x < 1000000; ++x)
			for(auto e : v)
				i += e->f(i);
		return i;
	}

	int test2() {
		std::vector<d*> v;
		for(int i = 0; i < 100; ++i)
			v.push_back(new d);
		int i = 0;
		for(int x = 0; x < 1000000; ++x)
			for(auto e : v)
				i += e->f(i);
		return i;
	}

	int test3() {
		std::vector<d*> v;
		for(int i = 0; i < 100; ++i)
			v.push_back(new d);
		int (d::*f)(int) = &d::f;
		int i = 0;
		for(int x = 0; x < 1000000; ++x)
			for(auto e : v)
				i += (e->*f)(i);
		return i;
	}

	int test4() {
		std::vector<d> v(100);
		int i = 0;
		for(int x = 0; x < 1000000; ++x)
			for(auto e : v)
				i += e.f(i);
		return i;
	}

	struct s {
		int dummy;
		int f(int i) {
			return i % 17;
		}
	};
	
	inline int test5() {
		std::vector<s> v(100);
		int i = 0;
		for(int x = 0; x < 1000000; ++x)
			for(auto e : v)
				i += e.f(i);
		return i;
	}
}}

int main() {
	std::cout << test::test3() << '\n';
	return 0;
}
