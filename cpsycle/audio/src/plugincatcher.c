// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "plugincatcher.h"
#include "plugin.h"
#include "vstplugin.h"
#include <dir.h>
#include <stdlib.h>

static int OnEnumDir(PluginCatcher*, const char* path, int flag);

void plugincatcher_init(PluginCatcher* self)
{
	self->plugins = properties_create();
}

void plugincatcher_dispose(PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = 0;
}

void plugincatcher_scan(PluginCatcher* self, const char* path, int type)
{
	if (type == MACH_SAMPLER) {
		Properties* p;
			 
		p = properties_createsection(self->plugins, path);	
		properties_append_int(p, "type", type, 0, 0);
		properties_append_int(p, "mode", GENERATOR, 0, 0);
		properties_append_string(p, "name", "Sampler");
		properties_append_string(p, "shortname", "Sampler");
		properties_append_string(p, "author", "psycedelics");
		properties_append_string(p, "command", "");		
	} else {
		dir_enum(self, path, "*.dll", type, OnEnumDir);
	}
}

int OnEnumDir(PluginCatcher* self, const char* path, int type)
{
	CMachineInfo* pInfo;

	if (type == MACH_PLUGIN) {
		pInfo = plugin_psycle_test(path);
	} else
	if (type == MACH_VST) {
		pInfo = plugin_vst_test(path);
	}
	if (pInfo) {
		Properties* p;

		p = properties_createsection(self->plugins, path);		
		properties_append_int(p, "type", type, 0, 0);
		properties_append_int(p, "mode", pInfo->Flags, 0, 0);
		properties_append_string(p, "name", pInfo->Name);
		properties_append_string(p, "shortname", pInfo->ShortName);
		properties_append_string(p, "author", pInfo->Author);
		properties_append_string(p, "command", pInfo->Command);		
	}	
	return 1;
}
