// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::coal::host
#pragma once
#include "detail/project.hpp"

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__COAL__HOST
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace psycle
{
	namespace coal
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK host
		{
		public:
			host(void);
			virtual ~host(void) throw();
		};
	}
}

#include <universalis/compiler/dynamic_link/end.hpp>
