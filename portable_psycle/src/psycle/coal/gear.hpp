// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::coal::gear
#pragma once
#include "detail/project.hpp"

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__COAL__GEAR
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace psycle
{
	namespace coal
	{
		class host;

		class UNIVERSALIS__COMPILER__DYNAMIC_LINK gear
		{
		public:
			//Members
			gear(void);
			virtual ~gear(void) throw();

			int addconnector(connector &newconnector);
			void removeconnector(connector &thisconnector);
			//When a connector is modified, it sends a "ConnectorModified" to gear. (signal/slot?)
			void connectormodified(connector &thisconnector);

			// There should be a function (or variable) indicating that there is no input, so only update internal variables
			// but don't need to process the input.
			void generate();

			void load();
			void save();

			//Properties
			host& hoster() { return m_hoster; }
			void hoster(host &newhost) { m_hoster = &newhost }

			eventlist &geteventlist() { return m_eventlist; }
			void seteventlist(eventlist &neweventlist) { m_eventlist = neweventlist; }




		protected:
			host* m_hoster;
			eventlist *m_eventlist;
		};
	}
}

#include <universalis/compiler/dynamic_link/end.hpp>
