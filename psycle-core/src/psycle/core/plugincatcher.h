// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PLUGIN_CATCHER
#define PSYCLE__CORE__PLUGIN_CATCHER
#pragma once

#include "machine.h"
#include "machinekey.hpp"
#include "pluginfinder.h"

namespace psycle { namespace core {

class PSYCLE__CORE__DECL PluginFinderCache: public PluginFinder {
	public:
		PluginFinderCache(bool delayedScan);
		~PluginFinderCache();
		
		virtual void Initialize(bool clear=false);
		virtual void EnablePlugin(const MachineKey & key, bool enable);
		virtual void PostInitialization();


	protected:
		std::uint32_t _numPlugins;

		bool loadCache();
		bool saveCache();
		void deleteCache();
};

}}
#endif
