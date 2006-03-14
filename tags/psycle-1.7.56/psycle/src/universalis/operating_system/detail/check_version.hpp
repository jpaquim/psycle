// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule, Magnus Johnson

///\file
///\interface universalis::operating_system::detail::check_version
#pragma once
#include <universalis/operating_system/exception.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace detail
		{
			/// excludes poor operating systems
			void check_version() throw(exception);
		}
	}
}
