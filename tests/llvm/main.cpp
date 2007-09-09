//#include <iostream>
#include <cmath>

int main(int, char**) {
	float f(1);
	for(int i(0); i < 10000000; ++i) f += std::log(f);
	//std::cout << f << "\n";
	return !f;
}
