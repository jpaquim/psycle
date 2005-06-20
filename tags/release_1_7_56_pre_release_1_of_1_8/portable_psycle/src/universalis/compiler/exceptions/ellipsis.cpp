// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\implementation gets information from ellipsis exceptions
#include <universalis/detail/project.private.hpp>
#include "ellipsis.hpp"
#include <sstream>
#if defined DIVERSALIS__COMPILER__GNU
	#include <cxxabi.h>
#endif
namespace universalis
{
	namespace compiler
	{
		namespace exceptions
		{
			std::string ellipsis()
			{
				#if defined DIVERSALIS__COMPILER__GNU
				{
					std::type_info * type_info(abi::__cxa_current_exception_type());
					std::ostringstream s;
					s << "ellipsis: " << (type_info ? type_info->name() : "unknown type");
					return s.str();
				}
				#elif defined DIVERSALIS__COMPILER__BORLAND
				{
					std::ostringstream s;
					s << "ellipsis: " << __ThrowExceptionName() << " was thrown by source file " << __ThrowFileName() << "#" << __ThrowLineNumber();
					return s.str();
				}
				#else
				{
					return "ellipsis: unknown type";
				}
				#endif
			}
		}
	}
}

// arch-tag: f0e20bb3-4baf-4064-9df9-4bf030a13f7c
