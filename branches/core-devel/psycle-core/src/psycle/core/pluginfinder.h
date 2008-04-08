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
#ifndef PSYCLE__CORE__PLUGIN_FINDER
#define PSYCLE__CORE__PLUGIN_FINDER

#include "machinekey.hpp"
#include "plugininfo.h"

namespace psy
{
	namespace core
	{
		class PluginFinder
		{
			public:
				PluginFinder(std::string const & psycle_path, std::string const & ladspa_path);
				~PluginFinder();

				virtual void addHost(Hosts::type);
				virtual bool hasHost(Hosts::type);

				PluginInfo info( const MachineKey & key ) const;
			
				std::map< MachineKey, PluginInfo >::const_iterator begin(Hosts::type) const;
				std::map< MachineKey, PluginInfo >::const_iterator end(Hosts::type) const;
				
			public:
				std::string const & psycle_path() const { return psycle_path_; }
			private:
				std::string const psycle_path_;
				
			public:
				std::string const & ladspa_path() const { return ladspa_path_; }
			private:
				std::string const ladspa_path_;

			protected:
				static std::vector<std::map< MachineKey, PluginInfo >> maps_;

				virtual void scanLadspa();
				virtual void scanNatives();
				void LoadLadspaInfo(std::string fileName);
				void LoadNativeInfo(std::string fileName);
		};
	}
}
#endif
