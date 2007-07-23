#include <windows.h>
#include <iostream>

int main()
{
	__try {
		std::cout << "in try\n";
		int i = 0;
		int j = 0;
		int k = i / j;
	} __except(UnhandledExceptionFilter(GetExceptionInformation())) {
		std::cout << "in except\n";
	}
}
