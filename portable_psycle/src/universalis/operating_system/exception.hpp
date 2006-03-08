// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::exception
#pragma once
#include <universalis/exception.hpp>
namespace universalis
{
	namespace operating_system
	{
		/// generic exception thrown by functions of the namespace universalis::operating_system.
		class exception : public universalis::exception
		{
			public:
				exception(std::string const & what, compiler::location const & location) throw();
		};
	}
}
