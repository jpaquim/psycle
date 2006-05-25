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

			int add_connector(connector &new_connector);
			void remove_connector(connector &this_connector);
			//When a connector is modified, it sends a "ConnectorModified" to gear. (signal/slot?)
			void connector_modified(connector &this_connector);

			// There should be a function (or variable) indicating that there is no input, so only update internal variables
			// but don't need to process the input.
			void generate();

			void load();
			void save();

			//Properties
			host& hoster() { return m_hoster; }
			void hoster(host &new_host) { m_hoster = &newhost }

			event_list &ge_tevent_list() { return m_eventlist; }
			void set_event_list(event_list &new_event_list) { m_event_list = &new_event_list; }




		protected:
			host* m_hoster;
			event_list *m_event_list;
		};
	}
}

#include <universalis/compiler/dynamic_link/end.hpp>
