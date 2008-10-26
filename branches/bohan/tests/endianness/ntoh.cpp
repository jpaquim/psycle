#include <stdint.h>
#include <cstring>
#include <iostream>

int main() {
	{
		uint32_t i(0x11223344);
		uint8_t c[4];
		std::memcpy(c, &i, sizeof c);
	
		std::cout << std::hex;

		std::cout << "physical layout: ";
		for(int i = 0; i < sizeof c; ++i) std::cout << (unsigned int)(c[i]);
		std::cout << ", ";
	
		// reverse the byte order if it's a little endian machine
		// this actually does nothing on a big endian machine.
		i = c[3] << 24 | c[2] << 16 | c[1] << 8 | c[0];
		std::cout << "ntoh: " << i << '\n';
	}
	{
		uint64_t i(0x1122334455667788);
		uint8_t c[8];
		std::memcpy(c, &i, sizeof c);
	
		std::cout << std::hex;

		std::cout << "physical layout: ";
		for(int i = 0; i < sizeof c; ++i) std::cout << (unsigned int)(c[i]);
		std::cout << ", ";
	
		// reverse the byte order if it's a little endian machine
		// this actually does nothing on a big endian machine.
		i =
			uint64_t(c[7]) << 56 |
			uint64_t(c[6]) << 48 |
			uint64_t(c[5]) << 40 |
			uint64_t(c[4]) << 32 |
			c[3] << 24 |
			c[2] << 16 |
			c[1] << 8 |
			c[0];
		std::cout << "ntoh: " << i << '\n';
	}
}
