
#include "crapculator.hpp"

module_begin(crapculator)
	number add_impl(number a, number b) {
		return ops::add(a, b);
	}
module_end

