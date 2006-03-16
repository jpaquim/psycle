// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

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
		template<typename X>
		std::string inline typenameof(X const &);
	}
}
namespace universalis
{
	namespace compiler
	{
		template<typename X>
		std::string inline typenameof(X const & x)
		{
			return typenameof(typeid(x));
		}
		template<>
		std::string inline typenameof<std::type_info>(std::type_info const & type_info)
		{
			return detail::demangled(std::string(type_info.name()));
		}
	}
}
