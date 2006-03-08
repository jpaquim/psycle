// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\implementation universalis::compiler::detail::demangled
#include <universalis/detail/project.private.hpp>
#include "demangle.hpp"
#if defined DIVERSALIS__COMPILER__GNU
	#include <cxxabi.h>
#endif
namespace universalis
{
	namespace compiler
	{
		namespace detail
		{
			std::string demangled(std::string const & mangled_symbol)
			{
				#if defined DIVERSALIS__COMPILER__GNU
					int status;
					char * c(abi::__cxa_demangle(mangled_symbol.c_str(), 0, 0, &status));
					std::string s(c);
					std::free(c);
					return s;
				#else
					return mangled_symbol;
				#endif
			}
		}
	}
}

// arch-tag: 2121acc8-0f63-4046-a148-e95abcc35efa
