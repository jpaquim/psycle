#if USE_IOSTREAM
	#include <iostream>
#else
	#include <cstdio>
#endif

#include <cmath>

int main(int, char**) {
	float f(1);
	for(int i(0); i < 10000000; ++i) f += std::log(f);
	#if USE_IOSTREAM
		std::cout << f << "\n";
	#else
		std::printf("%f\n", f);
	#endif
	return 0;
}
