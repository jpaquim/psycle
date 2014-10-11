
#include <iostream>
#include <csetjmp> // TODO getcontext, setcontext, makecontext, swapcontext

namespace test {

int sum(int min, int max) {
	int r = 0;
	for(int i = min; i < max; ++i) r += i;
	return r;
}

int sum_sjlj(int min, int max) {
	const int len = max - min;
	if(len < 10) return sum(min, max);
	std::jmp_buf env;
	int r = setjmp(env);
	if(!r) {
		const int mid = min + len / 2;
		const int val = sum_sjlj(min, mid) + sum_sjlj(mid, max);
		std::longjmp(env, val);
	}
	return r;
}

}

int main() {
	const int min = 0, max = 1000;
	std::cout << test::sum(min, max) << ' ' << test::sum_sjlj(min, max) << '\n';
}

