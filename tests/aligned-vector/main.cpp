#include <vector>
#include <cstddef>
#include <limits>
#include <iostream>
#include <universalis/os/aligned_memory_alloc.hpp>

int main() {
	std::size_t const alignment = 16;
	std::vector<float, universalis::os::aligned_alloc<float, alignment> > v;
	for(unsigned int i = 0; i < 50; ++i) {
		v.push_back(1);
		float * pf = &v[0];
		std::cout << (reinterpret_cast<unsigned long long int>(pf) % alignment) << '\n';
	}
	return 0;
}

