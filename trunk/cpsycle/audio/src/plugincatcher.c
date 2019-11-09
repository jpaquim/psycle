// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugincatcher.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "plugin.h"
#include "sampler.h"
#include "vstplugin.h"

#include <dir.h>

#include <stdlib.h>
#include <string.h>

#define __cplusplus
#include <diversalis/os.hpp>
#undef __cplusplus


#if defined DIVERSALIS__OS__MICROSOFT
#define MODULEEXT ".dll"
#elif defined DIVERSALIS__OS__APPLE
#define	".dylib"
#else
#define MODULEEXT ".so"
#endif

static void plugincatcher_makeinternals(PluginCatcher*);
static void plugincatcher_makeplugininfo(PluginCatcher*,
	const char* name,
	const char* path,
	unsigned int type,
	const MachineInfo* info);
static void plugincatcher_makesampler(PluginCatcher*);
static void plugincatcher_makeduplicator(PluginCatcher*);
static int isplugin(int type);
static int onenumdir(PluginCatcher*, const char* path, int flag);
static int onpropertiesenum(PluginCatcher*, Properties*, int level);
static int pathhasextension(const char* path);
static char* replace_char(char* str, char c, char r);

static const char* searchname;
static int searchtype;
static Properties* searchresult;


void plugincatcher_init(PluginCatcher* self, Properties* dirconfig)
{
	char inipath[_MAX_PATH];
	self->plugins = properties_create();
	plugincatcher_makeinternals(self);
	
	workdir(inipath);		
	strcat(inipath, "\\psycle-plugin-scanner-cache.ini");
	self->inipath = strdup(inipath);
	self->dirconfig = dirconfig;
	signal_init(&self->signal_changed);
	signal_init(&self->signal_scanprogress);	
}

void plugincatcher_dispose(PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = 0;
	free(self->inipath);	
	self->dirconfig = 0;	
	signal_dispose(&self->signal_changed);
	signal_dispose(&self->signal_scanprogress);
}

void plugincatcher_clear(PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = properties_create();
	plugincatcher_makeinternals(self);
}

void plugincatcher_makeinternals(PluginCatcher* self)
{		
	plugincatcher_makeplugininfo(self, "sampler", "", MACH_SAMPLER,
		sampler_info());
	plugincatcher_makeplugininfo(self, "dummy", "", MACH_DUMMY,
		dummymachine_info());
	plugincatcher_makeplugininfo(self, "master", "", MACH_MASTER,
		master_info());
	plugincatcher_makeplugininfo(self, "mixer", "", MACH_MIXER,
		mixer_info());
	plugincatcher_makeplugininfo(self, "duplicator", "", MACH_DUPLICATOR,
		duplicator_info());
	plugincatcher_makeplugininfo(self, "duplicator2", "", MACH_DUPLICATOR2,
		duplicator2_info());
}

void plugincatcher_makeplugininfo(PluginCatcher* self,
		const char* name,
		const char* path,
		unsigned int type,
		const MachineInfo* info) {

	if (info) {
		Properties* p;

		p = properties_createsection(self->plugins, name);
		properties_append_int(p, "type", type, 0, 0);
		properties_append_int(p, "mode", info->Flags, 0, 0);
		properties_append_string(p, "name", info->Name);
		properties_append_string(p, "shortname", info->ShortName);
		properties_append_string(p, "author", info->Author);
		properties_append_string(p, "command", info->Command);
		properties_append_string(p, "path", path);				
		if (type == MACH_PLUGIN) {
			char text[256];
			_snprintf(text, 256, "Psycle %s by %s ", 
				(info->Flags & 3) == 3 ? "instrument" : "effect", 
				info->Author);
			properties_append_string(p, "desc", text);
		} else {
			properties_append_string(p, "desc", "");
		}
	}
}

void plugincatcher_scan(PluginCatcher* self)
{	
	Properties* p;

	plugincatcher_clear(self);
	if (self->dirconfig) {
		p = properties_findsection(self->dirconfig, "plugin");
		if (p) {		
			dir_enum(self, properties_valuestring(p), "*"MODULEEXT, MACH_PLUGIN,
				onenumdir);
		}
		p = properties_findsection(self->dirconfig, "vst");
		if (p) {		
			dir_enum(self, properties_valuestring(p), "*"MODULEEXT, MACH_VST,
				onenumdir);
		}
	}
	signal_emit(&self->signal_changed, self, 0);
	signal_emit(&self->signal_scanprogress, self, 1, 0);
}

int isplugin(int type)
{
	return 
		type == MACH_PLUGIN ||
		type == MACH_VST;
}

char* replace_char(char* str, char c, char r)
{
	char* p;
		
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}

int onenumdir(PluginCatcher* self, const char* path, int type)
{	
	MachineInfo macinfo;
	char name[_MAX_PATH];	

	machineinfo_init(&macinfo);
	plugincatcher_catchername(self, path, name);
	if (type == MACH_PLUGIN) {		
		if (plugin_psycle_test(path, &macinfo)) {
			plugincatcher_makeplugininfo(self, name, path, type, &macinfo);
			signal_emit(&self->signal_scanprogress, self, 1, 1);
		}	
	} else
	if (type == MACH_VST) {
		if (plugin_vst_test(path, &macinfo)) {
			plugincatcher_makeplugininfo(self, name, path, type, &macinfo);
			signal_emit(&self->signal_scanprogress, self, 1, 1);
		}
	}
	machineinfo_dispose(&macinfo);
	return 1;
}

void plugincatcher_catchername(PluginCatcher* self, const char* path, char* name)
{	
	char ext[_MAX_PATH];

	extract_path(path, name, ext);
	strlwr(name);
	strlwr(ext);
	replace_char(name, ' ', '-');
}

int plugincatcher_load(PluginCatcher* self)
{
	int rv;

	plugincatcher_clear(self);	
	rv = propertiesio_load(self->plugins, self->inipath, 1);
	signal_emit(&self->signal_changed, self, 0);
	return rv;
}

void plugincatcher_save(PluginCatcher* self)
{	
	propertiesio_save(self->plugins, self->inipath);
}

char* plugincatcher_modulepath(PluginCatcher* self, MachineType machtype,
	const char* path, char* fullpath)
{	
	if (!path) {
		*fullpath = '\0';
	} else
	if (pathhasextension(path)) {
		strcpy(fullpath, path);
	} else {		
		searchname = path;
		searchtype = machtype;
		searchresult = 0;
		properties_enumerate(self->plugins, self, onpropertiesenum);
		if (searchresult) {						
			strcpy(fullpath, properties_readstring(searchresult, "path", ""));
		} else {
			strcpy(fullpath, path);
		}
	}
	return fullpath;
}

int onpropertiesenum(PluginCatcher* self, Properties* property, int level)
{
	if (properties_type(property) == PROPERTY_TYP_SECTION) {
		const char* key = properties_key(property);
		key = key;
		if ((strcmp(properties_key(property), searchname) == 0) &&
				properties_int(property, "type", 0) == searchtype) {
			searchresult = property;
			return 0;			
		}
	}
	return 1;
}

int pathhasextension(const char* path)
{
	return strrchr(path, '.') != 0;
}
