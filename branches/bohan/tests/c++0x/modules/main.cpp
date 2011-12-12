
#include <module.hpp>
#include <crapculator.hpp>
#include <iostream>

module_begin(main_module)
	module_import(crapculator)

	int main() {
		crapculator::number n;
		n = crapculator::add(1, 2);
		std::cout << n << '\n';
		return 0;
	}
	
	module_export(main)
module_end

int main() {
	module_import(main_module)
	return main_module::main();
}

