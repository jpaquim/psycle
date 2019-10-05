// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "plugincatcher.h"
#include "plugin.h"
#include "vstplugin.h"
#include <dir.h>
#include <stdlib.h>
#include <string.h>

#define MODULEEXT ".dll"

static void plugincatcher_makeinternals(PluginCatcher*);
static void plugincatcher_makesampler(PluginCatcher*);
static int isplugin(int type);
static int onenumdir(PluginCatcher*, const char* path, int flag);

void plugincatcher_init(PluginCatcher* self, Properties* dirconfig)
{
	self->plugins = properties_create();
	plugincatcher_makeinternals(self);
	self->inipath = strdup("psycle-plugin-scanner-cache.ini");
	self->dirconfig = dirconfig;
	signal_init(&self->signal_changed);
}

void plugincatcher_dispose(PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = 0;
	free(self->inipath);	
	self->dirconfig = 0;
	signal_dispose(&self->signal_changed);
}

void plugincatcher_clear(PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = properties_create();
	plugincatcher_makeinternals(self);
}

void plugincatcher_makeinternals(PluginCatcher* self)
{
	plugincatcher_makesampler(self);
}

void plugincatcher_makesampler(PluginCatcher* self)
{
	Properties* p;

	p = properties_createsection(self->plugins, "sampler");	
	properties_append_int(p, "type", MACH_SAMPLER, 0, 0);
	properties_append_int(p, "mode", GENERATOR, 0, 0);
	properties_append_string(p, "name", "Sampler");
	properties_append_string(p, "shortname", "Sampler");
	properties_append_string(p, "author", "psycedelics");
	properties_append_string(p, "command", "");
	properties_append_string(p, "modulepath", "");
}

void plugincatcher_scan(PluginCatcher* self)
{	
	Properties* p;

	plugincatcher_clear(self);
	if (self->dirconfig) {
		p = properties_findsection(self->dirconfig, "plugin");
		if (p) {		
			dir_enum(self, properties_valuestring(p), "*."MODULEEXT, MACH_PLUGIN, onenumdir);		
		}
		p = properties_findsection(self->dirconfig, "vst");
		if (p) {		
			dir_enum(self, properties_valuestring(p), "*."MODULEEXT, MACH_VST, onenumdir);		
		}
	}
	signal_emit(&self->signal_changed, self, 0);
}

int isplugin(int type)
{
	return 
		type == MACH_PLUGIN ||
		type == MACH_VST;
}

static char* replace_char(char* str, char c, char r)
{
	char* p;
		
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}

int onenumdir(PluginCatcher* self, const char* path, int type)
{
	CMachineInfo* pInfo;
	char name[_MAX_PATH];
	char ext[_MAX_PATH];

	extract_path(path, name, ext);
	strlwr(name);
	strlwr(ext);
	replace_char(name, ' ', '-');


	if (type == MACH_PLUGIN) {
		pInfo = plugin_psycle_test(path);
	} else
	if (type == MACH_VST) {
		pInfo = plugin_vst_test(path);
	}
	if (pInfo) {
		Properties* p;
		
		p = properties_createsection(self->plugins, name);
		properties_append_int(p, "type", type, 0, 0);
		properties_append_int(p, "mode", pInfo->Flags, 0, 0);
		properties_append_string(p, "name", pInfo->Name);
		properties_append_string(p, "shortname", pInfo->ShortName);
		properties_append_string(p, "author", pInfo->Author);
		properties_append_string(p, "command", pInfo->Command);
		properties_append_string(p, "path", path);
	}	
	return 1;
}

int plugincatcher_load(PluginCatcher* self)
{
	int rv;

	plugincatcher_clear(self);	
	rv = properties_load(self->plugins, self->inipath, 1);
	signal_emit(&self->signal_changed, self, 0);
	return rv;
}

void plugincatcher_save(PluginCatcher* self)
{	
	properties_save(self->plugins, self->inipath);
}
