/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "plugincatcher.h"
/* local */
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
#ifdef PSYCLE_USE_VST2
#include "vstplugin.h"
#endif
#include "ladspaplugin.h"
#ifdef PSYCLE_USE_LV2
#include "lv2plugin.h"
#endif
/* file */
#include <dir.h>
/* platform*/
#include "../../detail/portable.h"
#include "../../detail/cpu.h"
#include "../../detail/os.h"


#if defined DIVERSALIS__OS__MICROSOFT
#define SLASH "\\"
#define MODULEEXT ".dll"
#elif defined DIVERSALIS__OS__APPLE
#define SLASH "/"
#define	".dylib"
#define _MAX_PATH 4096
#else
#define SLASH "/"
#define MODULEEXT ".so"
#define _MAX_PATH 4096
#endif

static void psy_audio_machineinfo_make(psy_Property* parent,
	const psy_audio_MachineInfo* info, psy_audio_PluginCategoryList* categories)
{
	char key[512];		
	psy_Property* p;
	
	assert(parent);
	
	if (!info) {
		return;
	}
	if ((psy_strlen(info->modulepath) > 0)) {
		psy_audio_plugincatcher_catchername(info->modulepath, key,
			info->shellidx);			
	} else {
		psy_audio_plugincatcher_catchername(info->shortname, key,
			psy_INDEX_INVALID);
	}
	p = psy_property_append_section(parent, key);
	psy_property_append_int(p, "type", info->type, 0, 0);
	psy_property_append_int(p, "flags", info->flags, 0, 0);
	psy_property_append_int(p, "mode", info->mode, 0, 0);
	psy_property_append_str(p, "name", info->name);
	psy_property_append_str(p, "shortname", info->shortname);
	psy_property_append_str(p, "author", info->author);
	psy_property_append_str(p, "command", info->command);
	psy_property_append_str(p, "path", info->modulepath);
	psy_property_append_str(p, "desc", info->desc);		
	psy_property_append_int(p, "shellidx", info->shellidx, 0, 0);
	psy_property_append_int(p, "apiversion", info->apiversion, 0, 0);
	psy_property_append_int(p, "plugversion", info->plugversion, 0, 0);
	psy_property_append_int(p, "favorite", 0, 0, 0);
	if (categories && psy_strlen(info->category) == 0) {
		psy_property_append_str(p, "category",
			psy_audio_plugincategorylist_category(categories, key));
	} else {
		psy_property_append_str(p, "category", info->category);
	}
}

/* psy_audio_PluginSections */

/* implementation */
void psy_audio_pluginsections_init(psy_audio_PluginSections* self)
{
	char inipath[_MAX_PATH];

	inipath[0] = '\0';
	if (psy_dir_config()) {
		psy_snprintf(inipath, _MAX_PATH, "%s", psy_dir_config());
	}	
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	strcat(inipath, SLASH "psycle-plugin-scanner-cache32.ini");
#else
	strcat(inipath, SLASH "psycle-plugin-scanner-cache64.ini");
#endif
	self->inipath = psy_strdup(inipath);
	self->sections = NULL;
	psy_audio_pluginsections_reset(self);
}

void psy_audio_pluginsections_dispose(psy_audio_PluginSections* self)
{
	psy_property_deallocate(self->sections);
	self->sections = NULL;
	free(self->inipath);
	self->inipath = NULL;
}

void psy_audio_pluginsections_reset(psy_audio_PluginSections* self)
{
	psy_audio_pluginsections_clear(self);
	/* will contain all scanned plugins	*/
	psy_audio_pluginsections_addsection(self, "all");
	/* add a default favorite section */
	psy_audio_pluginsections_addsection(self, "favorites");
}

void psy_audio_pluginsections_clear(psy_audio_PluginSections* self)
{
	if (self->sections) {
		psy_property_deallocate(self->sections);
	}
	self->sections = psy_property_set_comment(
		psy_property_allocinit_key(NULL),
		"Psycle Plugin Scanner Cache created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
}

int psy_audio_pluginsections_load(psy_audio_PluginSections* self)
{
	int rv;
	psy_PropertyReader propertyreader;
	
	psy_audio_pluginsections_reset(self);
	psy_propertyreader_init(&propertyreader, self->sections,
		self->inipath);
	psy_propertyreader_allow_append(&propertyreader);	
	rv = psy_propertyreader_load(&propertyreader);	
	psy_propertyreader_dispose(&propertyreader);
	return rv;
}

int psy_audio_pluginsections_save(psy_audio_PluginSections* self)
{
	int rv;
	psy_PropertyWriter propertywriter;
	
	psy_propertywriter_init(&propertywriter, self->sections, self->inipath);
	rv = psy_propertywriter_save(&propertywriter);
	psy_propertywriter_dispose(&propertywriter);
	return rv;
}

psy_Property* psy_audio_pluginsections_addsection(psy_audio_PluginSections* self,
	const char* sectionkey)
{
	psy_Property* rv;

	rv = psy_audio_pluginsections_section(self, sectionkey);
	if (!rv) {
		rv = psy_property_append_section(self->sections, sectionkey);
		psy_property_append_str(rv, "name", sectionkey);
		psy_property_append_section(rv, "plugins");
	}
	return rv;
}

void psy_audio_pluginsections_add(psy_audio_PluginSections* self,
	const char* sectionkey, psy_audio_MachineInfo* macinfo)
{
	psy_Property* section;

	section = psy_audio_pluginsections_addsection(self, sectionkey);
	if (macinfo) {		
		psy_Property* plugins;

		plugins = psy_property_at(section, "plugins",
			PSY_PROPERTY_TYPE_SECTION);
		if (plugins) {			
			if (!psy_audio_pluginsections_plugin_exists(self, section, macinfo)) {
				psy_audio_machineinfo_make(plugins, macinfo, NULL);
			}
		}
	}
}

psy_Property* psy_audio_pluginsections_add_property(psy_audio_PluginSections* self,
	psy_Property* section, psy_Property* macinfo)
{
	psy_Property* plugins;

	if (!section || !macinfo || psy_audio_pluginsections_pluginbyid(self,
			section, psy_property_key(macinfo))) {
		return NULL;
	}
	plugins = psy_property_at(section, "plugins",
		PSY_PROPERTY_TYPE_SECTION);
	if (plugins) {
		psy_Property* rv;

		rv = psy_property_clone(macinfo);
		psy_property_append_property(plugins, rv);
		return rv;
	}
	return NULL;
}

void psy_audio_pluginsections_remove(psy_audio_PluginSections* self,
	psy_Property* section, const char* id)
{
	if (section) {
		psy_Property* plugin;

		plugin = psy_audio_pluginsections_pluginbyid(self, section, id);
		if (plugin && psy_property_parent(plugin)) {
			psy_property_remove(psy_property_parent(plugin), plugin);
		}
	}
}

void psy_audio_pluginsections_removesection(psy_audio_PluginSections* self,
	psy_Property* section)
{
	if (section) {
		psy_property_remove(self->sections, section);
	}
}

void psy_audio_pluginsections_clear_plugins(psy_audio_PluginSections* self,
	const char* sectionkey)
{
	psy_Property* section;

	section = psy_audio_pluginsections_section(self, sectionkey);
	if (section) {
		psy_Property* plugins;

		plugins = psy_property_at(section, "plugins",
			PSY_PROPERTY_TYPE_SECTION);
		if (plugins) {
			psy_property_clear(plugins);
		}
	}
}

psy_Property* psy_audio_pluginsections_section(psy_audio_PluginSections* self,
	const char* sectionkey)
{
	if (psy_strlen(sectionkey) > 0) {
		psy_Property* rv;
		psy_List* p;

		rv = NULL;
		for (p = psy_property_begin(self->sections); p != NULL; p = p->next) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (strcmp(psy_property_key(property), sectionkey) == 0) {
				rv = property;
				break;
			}
		}
		return rv;
	}
	return NULL;
}

psy_Property* psy_audio_pluginsections_section_plugins(psy_audio_PluginSections* self,
	const char* key)
{
	psy_Property* section;

	section = psy_audio_pluginsections_section(self, key);
	if (section) {
		return psy_property_at_section(section, "plugins");
	}
	return NULL;
}

psy_Property* psy_audio_pluginsections_pluginbyid(psy_audio_PluginSections* self,
	psy_Property* section, const char* id)
{
	if (section && (psy_strlen(id) > 0)) {
		psy_Property* plugins;

		plugins = psy_property_at(section, "plugins",
			PSY_PROPERTY_TYPE_SECTION);
		if (plugins) {
			psy_Property* rv;
			psy_List* p;

			rv = NULL;
			for (p = psy_property_begin(plugins); p != NULL; p = p->next) {
				psy_Property* property;

				property = (psy_Property*)psy_list_entry(p);
				if (strcmp(psy_property_key(property), id) == 0) {
					rv = property;
					break;
				}
			}
			return rv;
		}
	}
	return NULL;
}

psy_Property* psy_audio_pluginsections_plugin_exists(
	psy_audio_PluginSections* self, psy_Property* section,
	psy_audio_MachineInfo* macinfo)
{
	if (section) {
		psy_Property* plugins;

		plugins = psy_property_at(section, "plugins",
			PSY_PROPERTY_TYPE_SECTION);
		if (plugins) {
			char name[_MAX_PATH];

			psy_audio_plugincatcher_catchername(
				(psy_strlen(macinfo->modulepath) > 0)
				? macinfo->modulepath
				: macinfo->shortname,
				name,
				(psy_strlen(macinfo->modulepath) > 0)
				? macinfo->shellidx
				: psy_INDEX_INVALID);
			if (psy_strlen(name) > 0) {
				psy_Property* rv;
				psy_List* p;

				rv = NULL;
				for (p = psy_property_begin(plugins); p != NULL; p = p->next) {
					psy_Property* property;

					property = (psy_Property*)psy_list_entry(p);
					if (strcmp(psy_property_key(property), name) == 0) {
						rv = property;
						break;
					}
				}
				return rv;
			}
		}
	}
	return NULL;
}

uintptr_t psy_audio_pluginsections_num_plugins(const psy_audio_PluginSections*
	self, const char* sectionkey)
{
	psy_Property* plugins;
	
	plugins = psy_audio_pluginsections_section_plugins(
		(psy_audio_PluginSections*)self, sectionkey);
	if (plugins) {
		return psy_property_size(plugins);
	}
	return 0;
}

/* psy_audio_PluginScanTask */
void psy_audio_pluginscantask_init_all(psy_audio_PluginScanTask* self,
	psy_audio_MachineType type, const char* wildcard, const char* label,
	const char* key, int recursive)
{
	self->type = type;
	psy_snprintf(self->wildcard, 255, "%s", wildcard);
	psy_snprintf(self->label, 255, "%s", label);
	psy_snprintf(self->key, 255, "%s", key);
	self->recursive = recursive;
}

/* psy_audio_PluginCategories */
void psy_audio_plugincategories_init(psy_audio_PluginCategories* self,
	psy_Property* plugins)
{
	assert(self);

	psy_table_init(&self->container);
	psy_audio_plugincategories_update(self, plugins);
}

void psy_audio_plugincategories_dispose(psy_audio_PluginCategories* self)
{
	assert(self);

	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)NULL);
}

void psy_audio_plugincategories_update(psy_audio_PluginCategories* self,
	psy_Property* plugins)
{
	psy_List* p;	
	
	assert(self);

	psy_table_dispose_all(&self->container, NULL);
	psy_table_init(&self->container);
	if (!plugins) {
		return;
	}	
	for (p = psy_property_begin(plugins); p != NULL; psy_list_next(&p)) {
		psy_Property* q;
		const char* category;		

		q = (psy_Property*)psy_list_entry(p);
		category = psy_property_at_str(q, "category", "");
		if (category && (psy_strlen(category) > 0) &&
				(!psy_table_exists_strhash(&self->container, category))) {
			psy_table_insert_strhash(&self->container, category,
				(void*)psy_strdup(category));
		}		
	}
}

psy_TableIterator psy_audio_plugincategories_begin(
	psy_audio_PluginCategories* self)
{
	return psy_table_begin(&self->container);
}


/* psy_audio_PluginCatcher */

/* prototypes */
static void plugincatcher_initscantasks(psy_audio_PluginCatcher*);
static void plugincatcher_makeinternals(psy_audio_PluginCatcher*);
static void plugincatcher_makesampler(psy_audio_PluginCatcher*);
static void plugincatcher_makeduplicator(psy_audio_PluginCatcher*);
static int isplugin(int type);
static int on_enum_dir(psy_audio_PluginCatcher*, const char* path, int flag);
static int on_properties_enum(psy_audio_PluginCatcher*, psy_Property*, int level);
static int pathhasextension(const char* path);
static void plugincatcher_scan_multipath(psy_audio_PluginCatcher*,
	const char* multipath, const char* wildcard, int option,
	bool recursive);

static const char* searchname;
static int searchtype;
static psy_Property* searchresult;

/* implementation */
void psy_audio_plugincatcher_init(psy_audio_PluginCatcher* self)
{
	char inipath[_MAX_PATH];

	psy_audio_pluginsections_init(&self->sections);
	self->all = NULL;
	psy_audio_plugincategorylist_init(&self->categorydefaults);
	plugincatcher_makeinternals(self);
	strcpy(inipath, psy_dir_config());
	self->directories = NULL;
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	self->nativeroot = psy_strdup(PSYCLE_APP_DIR);
#else
	self->nativeroot = psy_strdup(PSYCLE_APP64_DIR);
#endif
	self->saveafterscan = TRUE;
	self->hasplugincache = FALSE;
	self->scanning = FALSE;
	self->abort = FALSE;
	self->scantasks = NULL;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_scanstart);
	psy_signal_init(&self->signal_scanend);
	psy_signal_init(&self->signal_scanprogress);
	psy_signal_init(&self->signal_scanfile);
	psy_signal_init(&self->signal_taskstart);
	plugincatcher_initscantasks(self);
}

void psy_audio_plugincatcher_dispose(psy_audio_PluginCatcher* self)
{
	psy_audio_pluginsections_dispose(&self->sections);
	self->directories = NULL;
	free(self->nativeroot);
	self->nativeroot = NULL;
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_scanstart);
	psy_signal_dispose(&self->signal_scanend);
	psy_signal_dispose(&self->signal_scanprogress);
	psy_signal_dispose(&self->signal_scanfile);
	psy_signal_dispose(&self->signal_taskstart);
	psy_audio_plugincategorylist_dispose(&self->categorydefaults);
	psy_list_deallocate(&self->scantasks, (psy_fp_disposefunc)NULL);
}

void psy_audio_plugincatcher_set_directories(psy_audio_PluginCatcher* self, psy_Property*
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
	psy_audio_pluginsections_clear_plugins(&self->sections, "all");
	plugincatcher_makeinternals(self);
	self->hasplugincache = FALSE;
}

void plugincatcher_initscantasks(psy_audio_PluginCatcher* self)
{
	psy_audio_PluginScanTask* task;

	/* natives */
	task = (psy_audio_PluginScanTask*)malloc(sizeof(psy_audio_PluginScanTask));
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	psy_audio_pluginscantask_init_all(task, psy_audio_PLUGIN,
		"*"MODULEEXT, "Natives 32bit", "plugins32", TRUE);
#else
	psy_audio_pluginscantask_init_all(task, psy_audio_PLUGIN,
		"*"MODULEEXT, "Natives 64bit", "plugins64", TRUE);
#endif
	psy_list_append(&self->scantasks, task);

	/* lua */
	task = (psy_audio_PluginScanTask*)malloc(sizeof(psy_audio_PluginScanTask));
	psy_audio_pluginscantask_init_all(task, psy_audio_LUA,
		"*.lua", "Luas", "luascripts", FALSE);
	psy_list_append(&self->scantasks, task);

#ifdef PSYCLE_USE_VST2	
	task = (psy_audio_PluginScanTask*)malloc(sizeof(psy_audio_PluginScanTask));
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	psy_audio_pluginscantask_init_all(task, psy_audio_VST,
		"*"MODULEEXT, "Vsts 32bit", "vsts32", TRUE);
#else
	psy_audio_pluginscantask_init_all(task, psy_audio_VST,
		"*"MODULEEXT, "Vsts 64bit", "vsts64", TRUE);
#endif
	psy_list_append(&self->scantasks, task);
#endif	

	/* ladspas */
	task = (psy_audio_PluginScanTask*)malloc(sizeof(psy_audio_PluginScanTask));
	psy_audio_pluginscantask_init_all(task, psy_audio_LADSPA,
		"*"MODULEEXT, "Ladspas", "ladspas", TRUE);
	psy_list_append(&self->scantasks, task);
	
#ifdef PSYCLE_USE_LV2	
	task = (psy_audio_PluginScanTask*)malloc(sizeof(psy_audio_PluginScanTask));
	psy_audio_pluginscantask_init_all(task, psy_audio_LV2,
		"*.lv2", "LV2s", "lv2s", FALSE);
	psy_list_append(&self->scantasks, task);
#endif	
}

void plugincatcher_makeinternals(psy_audio_PluginCatcher* self)
{
	psy_Property* all;

	all = psy_audio_pluginsections_section_plugins(&self->sections, "all");
	if (!all) {
		return;
	}
	psy_audio_machineinfo_make(all, psy_audio_xmsampler_info(),
		&self->categorydefaults);
	psy_audio_machineinfo_make(all, psy_audio_sampler_info(),
		&self->categorydefaults);
	psy_audio_machineinfo_make(all, psy_audio_dummymachine_info(),
		&self->categorydefaults);
	/* plugincatcher_makeplugininfo(self, "master", "", psy_audio_MASTER,
		psy_audio_master_info()); */
	psy_audio_machineinfo_make(all, psy_audio_mixer_info(),
		&self->categorydefaults);
	psy_audio_machineinfo_make(all, psy_audio_duplicator_info(),
		&self->categorydefaults);
	psy_audio_machineinfo_make(all, psy_audio_duplicator2_info(),
		&self->categorydefaults);
	psy_audio_machineinfo_make(all, psy_audio_audiorecorder_info(),
		&self->categorydefaults);
}


void plugincatcher_scan_multipath(psy_audio_PluginCatcher* self,
	const char* multipath, const char* wildcard, int option,
	bool recursive)
{
	char text[4096];
	char seps[] = ";,";
	char *token;

	strcpy(text, multipath);	
	token = strtok(text, seps);
	while (token != NULL && !self->abort) {
		if (recursive) {			
			psy_dir_enumerate_recursive(self, token, wildcard, option,
				(psy_fp_findfile)on_enum_dir);
		} else {
			psy_dir_enumerate(self, token, wildcard, option,
				(psy_fp_findfile)on_enum_dir);
		}
		token = strtok(0, seps );
	}
}

void psy_audio_plugincatcher_scan(psy_audio_PluginCatcher* self)
{
	psy_Property* plugin_dirs;
	
	self->abort = FALSE;
	self->scanning = TRUE;
	self->all = psy_audio_pluginsections_section_plugins(&self->sections,
		"all");
	if (!self->all) {
		return;
	}
	plugin_dirs = psy_property_at_section(self->directories, "plugins");
	if (!plugin_dirs) {		
		return;
	}
	psy_signal_emit(&self->signal_scanstart, self, 0);	
	psy_audio_plugincatcher_clear(self);
	if (self->directories) {
		psy_List* p;

		for (p = self->scantasks; p != NULL; p = p->next) {
			psy_audio_PluginScanTask* task;
			const char* path;

			task = (psy_audio_PluginScanTask*)p->entry;
			psy_signal_emit(&self->signal_taskstart, self, 1, task);			
			path = psy_property_at_str(plugin_dirs, task->key, NULL);
			if (path) {				
				plugincatcher_scan_multipath(self, path, task->wildcard,
					task->type, task->recursive);
			}
			if (self->abort) {
				break;
			}
		}
	}
	if (self->saveafterscan) {
		psy_audio_plugincatcher_save(self);
	}
	psy_signal_emit(&self->signal_changed, self, 0);
	psy_signal_emit(&self->signal_scanprogress, self, 1, 0);
	self->scanning = FALSE;
	psy_signal_emit(&self->signal_scanend, self, 0);
}

void psy_audio_plugincatcher_abort(psy_audio_PluginCatcher* self)
{
	self->abort = TRUE;
}

int isplugin(int type)
{
	return
		type == psy_audio_PLUGIN ||
		type == psy_audio_VST ||
		type == psy_audio_LADSPA ||
		type == psy_audio_LV2;
}

int on_enum_dir(psy_audio_PluginCatcher* self, const char* path, int type)
{
	psy_audio_MachineInfo macinfo;	
	
	psy_signal_emit(&self->signal_scanfile, self, 2, path, type);
	machineinfo_init(&macinfo);	
	switch (type) {
		case psy_audio_PLUGIN:
			if (psy_audio_plugin_psycle_test(path, self->nativeroot, &macinfo)) {				
				psy_audio_machineinfo_make(self->all, &macinfo,
					&self->categorydefaults);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break;
		case psy_audio_LUA:			
			if (psy_audio_luaplugin_test(path, &macinfo)) {				
				psy_audio_machineinfo_make(self->all, &macinfo,
					&self->categorydefaults);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break;
#ifdef PSYCLE_USE_VST2			
		case psy_audio_VST:						
			if (psy_audio_vstplugin_test(path, &macinfo)) {								
				psy_audio_machineinfo_make(self->all, &macinfo,
					&self->categorydefaults);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break;
#endif			
		case psy_audio_LADSPA: {
			uintptr_t shellidx;			

			shellidx = 0;
			for (; psy_audio_ladspaplugin_test(path, &macinfo, shellidx) != 0;
					++shellidx) {				
				psy_audio_machineinfo_make(self->all, &macinfo,
					&self->categorydefaults);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break; }
#ifdef PSYCLE_USE_LV2
		case psy_audio_LV2: {
			uintptr_t shellidx;			
			
			shellidx = 0;										
			for (; psy_audio_lv2plugin_test(path, &macinfo,
					shellidx) != 0; ++shellidx) {								
				psy_audio_machineinfo_make(self->all, &macinfo,
					&self->categorydefaults);
				psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
			}
			break; }
#endif						
		default:
			break;
	}
	machineinfo_dispose(&macinfo);
	if (self->abort) {
		return 0;
	}
	return 1;
}

void psy_audio_plugincatcher_catchername(const char* filename, char* rv,
	uintptr_t shellidx)
{
	psy_Path path;
	char tmp[256];

	psy_path_init(&path, filename);
	psy_snprintf(tmp, 256, psy_path_name(&path));
	psy_strlwr(tmp);
	psy_replacechar(tmp, ' ', '-');
	psy_replacechar(tmp, '_', '-');
	if (shellidx != psy_INDEX_INVALID) {
		psy_snprintf(rv, 256, "%s:%d", tmp, (int)shellidx);
	} else {
		psy_snprintf(rv, 256, "%s", tmp);
	}
	psy_path_dispose(&path);
}

int psy_audio_plugincatcher_load(psy_audio_PluginCatcher* self)
{
	int rv;

	rv = psy_audio_pluginsections_load(&self->sections);
	self->hasplugincache = (rv == PSY_OK);
	psy_signal_emit(&self->signal_changed, self, 0);
	return rv;
}

int psy_audio_plugincatcher_save(psy_audio_PluginCatcher* self)
{
	return psy_audio_pluginsections_save(&self->sections);
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
	psy_audio_MachineType machtype,
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
		self->all = psy_audio_pluginsections_section_plugins(&self->sections,
			"all");
		psy_property_enumerate(self->all, self,
			(psy_PropertyCallback)on_properties_enum);
		if (!searchresult) {
			if (strstr(path, "blitz")) {
				if (newgamefxblitzifversionunknown) {
					searchname = "blitz16:0";
				} else {
					searchname = "blitz12:0";
				}
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->all, self,
					(psy_PropertyCallback)on_properties_enum);
				if (!searchresult) {
					searchname = "blitzn:0";
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->all, self,
						(psy_PropertyCallback)on_properties_enum);
				}
				if (!searchresult) {
					if (newgamefxblitzifversionunknown) {
						searchname = "blitz12:0";
					} else {
						searchname = "blitz16:0";
					}
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->all, self,
						(psy_PropertyCallback)on_properties_enum);
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
				psy_property_enumerate(self->all, self,
					(psy_PropertyCallback)on_properties_enum);
				if (!searchresult) {
					searchname = "gamefxn:0";
					searchtype = machtype;
					searchresult = 0;
					psy_property_enumerate(self->all, self,
						(psy_PropertyCallback)on_properties_enum);
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
					psy_property_enumerate(self->all, self,
						(psy_PropertyCallback)on_properties_enum);
				}
			}
		}
		if (!searchresult) {
			if (strstr(path, "blitzn")) {
				searchname = "blitz:0";
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->all, self,
					(psy_PropertyCallback)on_properties_enum);
			}
		}
		if (!searchresult) {
			if (strstr(path, "blitz16")) {
				searchname = "blitzn:0";
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->all, self,
					(psy_PropertyCallback)on_properties_enum);
			}
		}
		if (!searchresult) {
			if (strstr(path, "blitzn")) {
				searchname = "blitz16:0";
				searchtype = machtype;
				searchresult = 0;
				psy_property_enumerate(self->all, self,
					(psy_PropertyCallback)on_properties_enum);
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

int on_properties_enum(psy_audio_PluginCatcher* self, psy_Property* property,
	int level)
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

const char* psy_audio_plugincatcher_search_path(psy_audio_PluginCatcher* self,
	const char* name, int machtype)
{
	searchname = name;
	searchtype = machtype;
	searchresult = 0;
	self->all = psy_audio_pluginsections_section_plugins(&self->sections,
		"all");
	psy_property_enumerate(self->all, self,
		(psy_PropertyCallback)on_properties_enum);
	if (searchresult) {
		return psy_property_at_str(searchresult, "path", 0);
	}
	return "";
}

psy_Property* psy_audio_plugincatcher_at(psy_audio_PluginCatcher* self,
	const char* id)
{
	psy_Property* rv;
	psy_List* p;

	self->all = psy_audio_pluginsections_section_plugins(&self->sections,
		"all");
	if (!self->all) {
		return NULL;
	}
	rv = NULL;
	for (p = psy_property_begin(self->all); p != NULL; p = p->next) {
		psy_Property* property;

		property = (psy_Property*)psy_list_entry(p);
		if (strcmp(psy_property_key(property), id) == 0) {
			rv = property;
			break;
		}
	}
	return rv;
}

void plugincatcher_incfavorite(psy_audio_PluginCatcher* self, const char* id)
{
	psy_Property* plugin;

	plugin = psy_audio_plugincatcher_at(self, id);
	if (plugin) {
		intptr_t favorite;

		favorite = psy_property_at_int(plugin, "favorite", 0);
		psy_property_set_int(plugin, "favorite", ++favorite);
		psy_audio_plugincatcher_save(self);
	}
}
bool psy_audio_plugincatcher_scanning(const psy_audio_PluginCatcher* self)
{
	return self->scanning;
}
