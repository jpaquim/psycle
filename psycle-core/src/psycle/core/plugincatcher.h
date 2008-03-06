// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef PSYCLE__CORE__PLUGIN_CATCHER
#define PSYCLE__CORE__PLUGIN_CATCHEr

#include "machine.h"
#include "pluginFinderKey.hpp"
#include "pluginfinder.h"

namespace psy
{
	namespace core
	{

		class PluginCatcher: public PluginFinder
		{
			public:
				PluginCatcher(std::string const & psycle_path, std::string const & ladspa_path);
				virtual ~PluginCatcher();

				virtual void rescanAll();
			
			protected:
				std::uint32_t _numPlugins;

				virtual bool loadInfo();
				bool loadCache();
				bool saveCache();
				void deleteCache();
				virtual void scanInternal();
				virtual void scanLadspa();
				virtual void scanNatives();
		};
	}
}
#endif
