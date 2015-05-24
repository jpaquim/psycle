import foo;
#include <iostream>

int main() {
	foo foo;
	std::cout << foo.compute(3) << std::endl;
}

#if 0 // boggus
	#ifdef FOO
		#error foo was not imported as a module
	#endif
#endif

#ifdef BAR
	#error bar was not imported as a module
#endif

