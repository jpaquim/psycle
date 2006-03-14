// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::compiler::typenameof
#pragma once
#include "detail/pragmatic/demangle.hpp"
#include <typeinfo>
#include <string>
namespace universalis
{
	namespace compiler
	{
		/// gets the compiler symbol string of any type.
		template<typename x>
		std::string inline typenameof(x const &);
	}
}
namespace universalis
{
	namespace compiler
	{
		template<typename X>
		std::string inline typenameof(X const & x)
		{
			return detail::demangled(std::string(typeid(x).name()));
		}
	}
}
// arch-tag: 8b21f7de-43d8-4ecc-9b30-b7a1de66e0be
