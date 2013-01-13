#ifndef OPS__INCLUDED
#define OPS__INCLUDED
#pragma once

#include <module.hpp>

module_begin(ops)
	float typedef number;
	number add_impl(number, number);
	number inline add(number a, number b) { return add_impl(a, b); }
	
	module_export(number)
	module_export(add)
module_end

#endif

