// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::coal::arranger
#pragma once
#include "detail/project.hpp"

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__COAL__ARRANGER
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace psycle
{
	namespace coal
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK arranger
		{
		public:
			arranger(void);
			virtual ~arranger(void) throw();


		protected:
			song* thesong
			buffers (audio and events. they are dynamic)
			connectors
			sequence
			patterns
			gui_patterns
		};
	}
}

#include <universalis/compiler/dynamic_link/end.hpp>
