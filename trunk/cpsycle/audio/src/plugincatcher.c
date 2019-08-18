// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "plugincatcher.h"
#include "plugin.h"
#include "vstplugin.h"
#include <dir.h>

static int OnEnumDir(PluginCatcher* self, const char* path, int flag);
static void OnDestroyMachineInfo(Property* property);

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
	dir_enum(self, path, "*.dll", type, OnEnumDir);
}

int OnEnumDir(PluginCatcher* self, const char* path, int flag)
{
	CMachineInfo* pInfo;

	if (flag == 0) {
		pInfo = plugin_psycle_test(path);
	} else
	if (flag == 1) {
		pInfo = plugin_vst_test(path);
	}
	if (pInfo) {
		properties_append_userdata(self->plugins, path, pInfo, OnDestroyMachineInfo);
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
