// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "plugincatcher.h"
#include "plugin.h"
#include "vstplugin.h"
#include <dir.h>
#include <stdlib.h>

static int OnEnumDir(PluginCatcher*, const char* path, int flag);
static void OnDestroyMachineInfo(Property*);

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
		Properties* properties;
		
		properties = properties_append_int(self->plugins, path, type, 0, 0);		
		properties->children = properties_create();
		properties_append_int(properties->children, "mode", GENERATOR, 0, 0);
		properties_append_string(properties->children, "name", "Sampler");
		properties_append_string(properties->children, "shortname", "Sampler");
		properties_append_string(properties->children, "author", "psycedelics");
		properties_append_string(properties->children, "command", "");		
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
		Properties* properties;

		properties = properties_append_int(self->plugins, path, type, 0, 0);		
		properties->children = properties_create();
		properties_append_int(properties->children, "mode", pInfo->Flags, 0, 0);
		properties_append_string(properties->children, "name", pInfo->Name);
		properties_append_string(properties->children, "shortname", pInfo->ShortName);
		properties_append_string(properties->children, "author", pInfo->Author);
		properties_append_string(properties->children, "command", pInfo->Command);		
	}	
	return 1;
}

void OnDestroyMachineInfo(Property* property)
{
	CMachineInfo* pInfo = (CMachineInfo*) property->value.ud;	
	free((void*)pInfo->Name);
	free((void*)pInfo->ShortName);	
	free((void*)pInfo->Author);
	free((void*)pInfo->Command);
}
