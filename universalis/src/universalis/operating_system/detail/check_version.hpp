// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface universalis::operating_system::detail::check_version
#pragma once
#include <universalis/exception.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__DETAIL__CHECK_VERSION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace detail
		{
			/// excludes poor operating systems
			void UNIVERSALIS__COMPILER__DYNAMIC_LINK check_version() throw(universalis::exception);
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
