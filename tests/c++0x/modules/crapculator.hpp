#ifndef CRAPCULATOR__INCLUDED
#define CRAPCULATOR__INCLUDED
#pragma once

#include <module.hpp>
#include <ops.hpp>

module_begin(crapculator)
	module_import(ops)

	using ops::number;
	number add_impl(number, number);
	number inline add(number a, number b) { return add_impl(a, b); }

	module_export(number)
	module_export(add)
module_end

#endif

