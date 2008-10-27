#include <stdint.h>
#include <cstring>
#include <iostream>

int main() {
	std::cout << std::hex;
	{
		std::cout << "32-bit\n";
		
		union u {
			uint32_t i;
			uint8_t c[sizeof(uint32_t)];
		} x;
		x.i = 0x44332211;
		std::cout << "\t       logical value: 0x" << x.i;

		std::cout << "\n\t     physical layout: 0x";
		for(int i = 0; i < sizeof x.c; ++i) std::cout << (unsigned int)(x.c[i]);
	
		// reverse the byte order if it's a little endian machine
		// this actually does nothing on a big endian machine.
		x.i = x.c[0] << 24 | x.c[1] << 16 | x.c[2] << 8 | x.c[3];

		std::cout << "\n\thton physical layout: 0x";
		for(int i = 0; i < sizeof x.c; ++i) std::cout << (unsigned int)(x.c[i]);
		std::cout << '\n';
	}
	{
		std::cout << "64-bit\n";

		union u {
			uint64_t i;
			uint8_t c[sizeof(uint64_t)];
		} x;
		x.i = 0x8877665544332211ULL;
		std::cout << "\t       logical value: 0x" << x.i;

		std::cout << "\n\t     physical layout: 0x";
		for(int i = 0; i < sizeof x.c; ++i) std::cout << (unsigned int)(x.c[i]);
	
		// reverse the byte order if it's a little endian machine
		// this actually does nothing on a big endian machine.
		x.i =
			uint64_t(x.c[0]) << 56 |
			uint64_t(x.c[1]) << 48 |
			uint64_t(x.c[2]) << 40 |
			uint64_t(x.c[3]) << 32 |
			x.c[4] << 24 |
			x.c[5] << 16 |
			x.c[6] << 8 |
			x.c[7];

		std::cout << "\n\thton physical layout: 0x";
		for(int i = 0; i < sizeof x.c; ++i) std::cout << (unsigned int)(x.c[i]);
		std::cout << '\n';
	}
}
