// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "plugincatcher.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "audiorecorder.h"
#include "plugin.h"
#include "luaplugin.h"
#include "sampler.h"
#include "xmsampler.h"
#include "vstplugin.h"
#include "ladspaplugin.h"

#include <dir.h>

#include <stdlib.h>
#include <string.h>
#include "../../detail/portable.h"
#include "../../detail/cpu.h"


#if defined DIVERSALIS__OS__MICROSOFT
#define MODULEEXT ".dll"
#elif defined DIVERSALIS__OS__APPLE
#define	".dylib"
#define _MAX_PATH 4096
#else
#define MODULEEXT ".so"
#define _MAX_PATH 4096
#endif

static void plugincatcher_makeinternals(psy_audio_PluginCatcher*);
static void plugincatcher_makeplugininfo(psy_audio_PluginCatcher*,
	const char* name,
	const char* path,
	unsigned int type,
	const psy_audio_MachineInfo* info);
static void plugincatcher_makesampler(psy_audio_PluginCatcher*);
static void plugincatcher_makeduplicator(psy_audio_PluginCatcher*);
static int isplugin(int type);
static int onenumdir(psy_audio_PluginCatcher*, const char* path, int flag);
static int onpropertiesenum(psy_audio_PluginCatcher*, psy_Property*, int level);
static int pathhasextension(const char* path);
static void plugincatcher_scan_multipath(psy_audio_PluginCatcher*,
	const char* multipath, const char* wildcard, int option);

static const char* searchname;
static int searchtype;
static psy_Property* searchresult;

void psy_audio_plugincatcher_init(psy_audio_PluginCatcher* self)
{
	char inipath[_MAX_PATH];	

	self->plugins = psy_property_setcomment(
	psy_property_allocinit_key(NULL),
	"Psycle Plugin Scanner Cache created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	plugincatcher_makeinternals(self);	
	strcpy(inipath, psy_dir_config());
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	strcat(inipath, "\\psycle-plugin-scanner-cache32.ini");
#else
	strcat(inipath, "\\psycle-plugin-scanner-cache64.ini");
#endif
	self->inipath = strdup(inipath);
	self->directories = NULL;
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	self->nativeroot = psy_strdup(PSYCLE_APP_DIR);
#else
	self->nativeroot = psy_strdup(PSYCLE_APP64_DIR);
#endif
	self->saveafterscan = TRUE;
	self->hasplugincache = FALSE;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_scanprogress);
}

void psy_audio_plugincatcher_dispose(psy_audio_PluginCatcher* self)
{
	psy_property_deallocate(self->plugins);
	self->plugins = 0;
	free(self->inipath);	
	self->directories = NULL;
	free(self->nativeroot);
	self->nativeroot = NULL;
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_scanprogress);
}

void psy_audio_plugincatcher_setdirectories(psy_audio_PluginCatcher* self, psy_Property*
	directories)
{
	psy_Property* p;

	self->directories = directories;
	p = psy_property_find(self->directories, "app", PSY_PROPERTY_TYPE_NONE);
	if (p) {
		psy_strreset(&self->nativeroot, psy_property_item_str(p));
	} else {
		psy_strreset(&self->nativeroot, PSYCLE_APP_DIR);
	}
}

void psy_audio_plugincatcher_clear(psy_audio_PluginCatcher* self)
{
	psy_property_deallocate(self->plugins);
	self->plugins = psy_property_setcomment(
		psy_property_allocinit_key(NULL),
		"Psycle Plugin Scanner Cache created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	plugincatcher_makeinternals(self);
	self->hasplugincache = FALSE;
}

void plugincatcher_makeinternals(psy_audio_PluginCatcher* self)
{			
	plugincatcher_makeplugininfo(self, "sampulse", "", MACH_XMSAMPLER,
		psy_audio_xmsampler_info());
	plugincatcher_makeplugininfo(self, "sampler", "", MACH_SAMPLER,
		psy_audio_sampler_info());
	plugincatcher_makeplugininfo(self, "dummy", "", MACH_DUMMY,
		psy_audio_dummymachine_info());
	plugincatcher_makeplugininfo(self, "master", "", MACH_MASTER,
		psy_audio_master_info());
	plugincatcher_makeplugininfo(self, "mixer", "", MACH_MIXER,
		psy_audio_mixer_info());
	plugincatcher_makeplugininfo(self, "duplicator", "", MACH_DUPLICATOR,
		psy_audio_duplicator_info());
	plugincatcher_makeplugininfo(self, "duplicator2", "", MACH_DUPLICATOR2,
		psy_audio_duplicator2_info());
	plugincatcher_makeplugininfo(self, "audiorecorder", "", MACH_RECORDER,
		psy_audio_audiorecorder_info());
}

void plugincatcher_makeplugininfo(psy_audio_PluginCatcher* self,
		const char* name,
		const char* path,
		unsigned int type,
		const psy_audio_MachineInfo* info)
{
	if (info) {
		psy_Property* p;		
		
		p = psy_property_append_section(self->plugins, name);
		psy_property_append_int(p, "type", type, 0, 0);
		psy_property_append_int(p, "flags", info->Flags, 0, 0);
		psy_property_append_int(p, "mode", info->mode, 0, 0);
		psy_property_append_string(p, "name", info->Name);
		psy_property_append_string(p, "shortname", info->ShortName);
		psy_property_append_string(p, "author", info->Author);
		psy_property_append_string(p, "command", info->Command);
		psy_property_append_string(p, "path", path);				
		if (type == MACH_PLUGIN) {
			char text[256];
			psy_snprintf(text, 256, "Psycle %s by %s ", 
				info->mode == MACHMODE_FX ? "effect" : "instrument", 
				info->Author);
			psy_property_append_string(p, "desc", text);
		} else {
			psy_property_append_string(p, "desc", "");
		}
		psy_property_append_int(p, "shellidx", info->shellidx, 0, 0);
		psy_property_append_int(p, "favorite", 0, 0, 0);
	}
}

void plugincatcher_scan_multipath(psy_audio_PluginCatcher* self,
	const char* multipath, const char* wildcard, int option)
{
	char text[4096];
	char seps[] = ";,";
	char *token;
		
	strcpy(text, multipath);
	token = strtok(text, seps);
	while (token != NULL) {
		psy_dir_enumerate_recursive(self, token, wildcard, option,
			(psy_fp_findfile)onenumdir);
		token = strtok(0, seps );
	}
}

void psy_audio_plugincatcher_scan(psy_audio_PluginCatcher* self)
{
	psy_audio_plugincatcher_clear(self);
	if (self->directories) {
		const char* path;

#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
		path = psy_property_at_str(self->directories, "plugins32", NULL);
#else
		path = psy_property_at_str(self->directories, "plugins64", NULL);
#endif

		if (path) {			
			psy_dir_enumerate_recursive(self, path, "*"MODULEEXT, MACH_PLUGIN,
				(psy_fp_findfile)onenumdir);
		}
		path = psy_property_at_str(self->directories, "luascripts", NULL);
		if (path) {
			psy_dir_enumerate(self, path, "*.lua", MACH_LUA,
				(psy_fp_findfile)onenumdir);
		}
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
		path = psy_property_at_str(self->directories, "vsts32", NULL);
#else
		path = psy_property_at_str(self->directories, "vsts64", NULL);
#endif
		if (path) {
			plugincatcher_scan_multipath(self, path, "*"MODULEEXT, MACH_VST);
		}
		path = psy_property_at_str(self->directories, "ladspas", NULL);
		if (path) {
			plugincatcher_scan_multipath(self, path, "*"MODULEEXT,
				MACH_LADSPA);
		}
	}
	if (self->saveafterscan) {
		psy_audio_plugincatcher_save(self);
	}
	psy_signal_emit(&self->signal_changed, self, 0);
	psy_signal_emit(&self->signal_scanprogress, self, 1, 0);
}

int isplugin(int type)
{
	return 
		type == MACH_PLUGIN ||
		type == MACH_VST ||
		type == MACH_LADSPA;
}

int onenumdir(psy_audio_PluginCatcher* self, const char* path, int type)
{	
	psy_audio_MachineInfo macinfo;
	char name[_MAX_PATH];	

	machineinfo_init(&macinfo);
	switch (type) {
		case MACH_PLUGIN:
			if (psy_audio_plugin_psycle_test(path, self->nativeroot, &macinfo)) {
				psy_audio_plugincatcher_catchername(self, path, name, macinfo.shellidx);
				plugincatcher_makeplugininfo(self, name, path, macinfo.type,
					&macinfo);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break;
		case MACH_LUA:
			if (psy_audio_plugin_luascript_test(path, &macinfo)) {
				psy_audio_plugincatcher_catchername(self, path, name, macinfo.shellidx);
				plugincatcher_makeplugininfo(self, name, path, macinfo.type,
					&macinfo);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break;
		case MACH_VST:			
			if (psy_audio_plugin_vst_test(path, &macinfo)) {
				psy_audio_plugincatcher_catchername(self, path, name, macinfo.shellidx);
				plugincatcher_makeplugininfo(self, name, path, macinfo.type,
					&macinfo);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break;
		case MACH_LADSPA: {
			uintptr_t shellidx;

			for (shellidx = 0; psy_audio_plugin_ladspa_test(path, &macinfo, shellidx) != 0; ++shellidx) {
				psy_audio_plugincatcher_catchername(self, path, name, macinfo.shellidx);
				plugincatcher_makeplugininfo(self, name, path, macinfo.type,
					&macinfo);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break; }
		default:
			break;
	}
	machineinfo_dispose(&macinfo);
	return 1;
}

void psy_audio_plugincatcher_catchername(psy_audio_PluginCatcher* self,
	const char* filename, char* rv, uintptr_t shellidx)
{	
	psy_Path path;	
	
	psy_path_init(&path, filename);
	psy_snprintf(rv, _MAX_PATH, psy_path_name(&path));	
	psy_strlwr(rv);	
	psy_replacechar(rv, ' ', '-');
	psy_replacechar(rv, '_', '-');
	psy_snprintf(rv, 256, "%s:%d", rv, (int) shellidx);
	psy_path_dispose(&path);
}

int psy_audio_plugincatcher_load(psy_audio_PluginCatcher* self)
{
	int rv;
	psy_Path path;

	psy_path_init(&path, self->inipath);
	psy_audio_plugincatcher_clear(self);	
	rv = propertiesio_load(self->plugins, &path, 1);
	psy_path_dispose(&path);
	self->hasplugincache = rv;
	psy_signal_emit(&self->signal_changed, self, 0);
	return rv;
}

void psy_audio_plugincatcher_save(psy_audio_PluginCatcher* self)
{	
	psy_Path path;

	psy_path_init(&path, self->inipath);
	propertiesio_save(self->plugins, &path);
	psy_path_dispose(&path);
}

uintptr_t psy_audio_plugincatcher_extractshellidx(const char* path)
{	
	if (path) {
		char* str;

		str = strrchr(path, ':');
		if (str) {
			return atoi(str + 1);
		}		
	}
	return 0;
}

char* psy_audio_plugincatcher_modulepath(psy_audio_PluginCatcher* self,
	MachineType machtype,
	int newgamefxblitzifversionunknown,
	const char* path, char* fullpath)
{	
	if (!path) {
		*fullpath = '\0';
	} else
	if (pathhasextension(path)) {
		strcpy(fullpath, path);
	}
	else {					
		searchname = path;
		searchtype = machtype;
		searchresult = 0;
		psy_property_enumerate(self->plugins, self,
			(psy_PropertyCallback)onpropertiesenum);
		if (!searchresult) {
			if (strstr(path, "blitz")) {
				if (newgamefxblitzifversionunknown) {
					searchname = "blitz16:0";
				} else {
					searchname = "blitz12:0";
				}
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->plugins, self,
					(psy_PropertyCallback)onpropertiesenum);
				if (!searchresult) {
					searchname = "blitzn:0";
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->plugins, self,
						(psy_PropertyCallback)onpropertiesenum);
				}
				if (!searchresult) {
					if (newgamefxblitzifversionunknown) {						
						searchname = "blitz12:0";
					} else {
						searchname = "blitz16:0";
					}
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->plugins, self,
						(psy_PropertyCallback)onpropertiesenum);
				}
			} else
			if (strstr(path, "gamefx")) {
				if (newgamefxblitzifversionunknown) {
					searchname = "gamefx16:0";
				}
				else {
					searchname = "gamefx13:0";
				}
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->plugins, self,
					(psy_PropertyCallback)onpropertiesenum);
				if (!searchresult) {
					searchname = "gamefxn:0";
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->plugins, self,
						(psy_PropertyCallback)onpropertiesenum);
				}
				if (!searchresult) {
					if (newgamefxblitzifversionunknown) {
						searchname = "gamefx13:0";
					}
					else {
						searchname = "gamefx16:0";
					}
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->plugins, self,
						(psy_PropertyCallback)onpropertiesenum);
				}
			}
		}
		if (!searchresult) {
			if (strstr(path, "blitzn")) {
				searchname = "blitz:0";
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->plugins, self,
					(psy_PropertyCallback)onpropertiesenum);
			}
		}
		if (!searchresult) {
			if (strstr(path, "blitz16")) {
				searchname = "blitzn:0";
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->plugins, self,
					(psy_PropertyCallback)onpropertiesenum);
			}
		}
		if (!searchresult) {
			if (strstr(path, "blitzn")) {
				searchname = "blitz16:0";
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->plugins, self,
					(psy_PropertyCallback)onpropertiesenum);
			}
		}
		if (searchresult) {			
			strcpy(fullpath, psy_property_at_str(searchresult, "path", ""));
		} else {
			strcpy(fullpath, path);
		}
	}
	return fullpath;
}

int onpropertiesenum(psy_audio_PluginCatcher* self, psy_Property* property, int level)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {
		const char* key = psy_property_key(property);
		key = key;
		if ((strcmp(psy_property_key(property), searchname) == 0) &&
				psy_property_at_int(property, "type", 0) == searchtype) {
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

const char* psy_audio_plugincatcher_searchpath(psy_audio_PluginCatcher* self, const char* name,
	int machtype)
{	
	searchname = name;
	searchtype = machtype;
	searchresult = 0;
	psy_property_enumerate(self->plugins, self,
		(psy_PropertyCallback)onpropertiesenum);
	return psy_property_at_str(searchresult, "path", 0);
}
