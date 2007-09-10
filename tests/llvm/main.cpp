#include <iostream>
#include <cmath>
int main(int, char**) {
	float f(2);
	std::cout << std::log(f) << "\n";
	for(int i(0); i < 10; ++i) f += std::log(f);
	std::cout << f << "\n";
	return 0;
}
