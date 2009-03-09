// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PLUGIN_CATCHER
#define PSYCLE__CORE__PLUGIN_CATCHER
#pragma once

#include "machine.h"
#include "machinekey.hpp"
#include "pluginfinder.h"

namespace psy { namespace core {

//\todo: PluginFinderBase, else we can't have getInstance ?
class PSYCLE__CORE__DECL PluginFinderCache: public PluginFinder {
	protected:
		PluginFinderCache();
		~PluginFinderCache();
	public:
		//static PluginFinderCache& getInstance();

		void Rescan();

	protected:
		std::uint32_t _numPlugins;

		bool loadCache();
		bool saveCache();
		void deleteCache();
};

}}
#endif
