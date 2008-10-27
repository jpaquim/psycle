#include <stdint.h>
#include <cstring>
#include <iostream>

int main() {
	std::cout << std::hex;
	{
		std::cout << "32-bit\n";
		
		uint32_t i(0x44332211);
		std::cout << "\t       logical value: " << i;

		uint8_t c[4];
		std::memcpy(c, &i, sizeof c);
		std::cout << "\n\t     physical layout: ";
		for(int i = 0; i < sizeof c; ++i) std::cout << (unsigned int)(c[i]);
	
		// reverse the byte order if it's a little endian machine
		// this actually does nothing on a big endian machine.
		i = c[0] << 24 | c[1] << 16 | c[2] << 8 | c[3];

		std::cout << "\n\thton physical layout: ";
		std::memcpy(c, &i, sizeof c);
		for(int i = 0; i < sizeof c; ++i) std::cout << (unsigned int)(c[i]);
		std::cout << '\n';
	}
	{
		std::cout << "64-bit\n";

		uint64_t i(0x8877665544332211ULL);
		std::cout << "\t       logical value: " << i;

		uint8_t c[8];
		std::memcpy(c, &i, sizeof c);
		std::cout << "\n\t     physical layout: ";
		for(int i = 0; i < sizeof c; ++i) std::cout << (unsigned int)(c[i]);
	
		// reverse the byte order if it's a little endian machine
		// this actually does nothing on a big endian machine.
		i =
			uint64_t(c[0]) << 56 |
			uint64_t(c[1]) << 48 |
			uint64_t(c[2]) << 40 |
			uint64_t(c[3]) << 32 |
			c[4] << 24 |
			c[5] << 16 |
			c[6] << 8 |
			c[7];

		std::cout << "\n\thton physical layout: ";
		std::memcpy(c, &i, sizeof c);
		for(int i = 0; i < sizeof c; ++i) std::cout << (unsigned int)(c[i]);
		std::cout << '\n';
	}
}
