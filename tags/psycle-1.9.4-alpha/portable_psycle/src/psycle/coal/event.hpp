// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::coal::event
#pragma once
#include "detail/project.hpp"

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__COAL__EVENT
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace psycle
{
	namespace coal
	{
		typedef enum 
		{
			
		}eventtype;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK event
		{
		public:
			event(void);
			virtual ~event(void) throw();


		protected:
			int time;
			eventtype type;
			unsigned char track;
			unsigned char machine;
			unsigned char data1;
			unsigned char data2;
		};
	}
}

#include <universalis/compiler/dynamic_link/end.hpp>
