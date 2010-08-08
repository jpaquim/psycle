// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface universalis::compiler::typenameof

#ifndef UNIVERSALIS__COMPILER__TYPENAMEOF__INCLUDED
#define UNIVERSALIS__COMPILER__TYPENAMEOF__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include <typeinfo>
#include <string>

namespace universalis { namespace compiler {

/// gets the compiler symbol string of any type.
template<typename X>
std::string inline typenameof(X const & x) { return typenameof(typeid(x)); }


/********************************************************************************************/
// implementation details

///\internal
namespace detail {
	///\internal
	/// demangling of compiler symbols strings.
	std::string UNIVERSALIS__DECL demangle(std::string const & mangled_symbol);
}

template<>
std::string inline typenameof<std::type_info>(std::type_info const & type_info) {
	return detail::demangle(std::string(type_info.name()));
}

}}

#endif
