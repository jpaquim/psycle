/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PLUGINCATCHER_H
#define psy_audio_PLUGINCATCHER_H

/* local */
#include "machinedefs.h"
#include "machineinfo.h"
#include "plugincategorylist.h"
/* container */
#include <hashtbl.h>
#include <propertiesio.h>
#include <signal.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_PluginSections {
	psy_Property* sections;
	char* inipath;
} psy_audio_PluginSections;

void psy_audio_pluginsections_init(psy_audio_PluginSections*);
void psy_audio_pluginsections_dispose(psy_audio_PluginSections*);

void psy_audio_pluginsections_reset(psy_audio_PluginSections*);
void psy_audio_pluginsections_clear(psy_audio_PluginSections*);
int psy_audio_pluginsections_load(psy_audio_PluginSections*);
int psy_audio_pluginsections_save(psy_audio_PluginSections*);
void psy_audio_pluginsections_add(psy_audio_PluginSections*,
	const char* sectionkey, psy_audio_MachineInfo*);
psy_Property* psy_audio_pluginsections_add_property(psy_audio_PluginSections*,
	psy_Property* section, psy_Property* macinfo);
void psy_audio_pluginsections_remove(psy_audio_PluginSections*,
	psy_Property* section, const char* id);
void psy_audio_pluginsections_removesection(psy_audio_PluginSections*,
	psy_Property* section);
psy_Property* psy_audio_pluginsections_addsection(psy_audio_PluginSections*,
	const char* sectionkey);
void psy_audio_pluginsections_clear_plugins(psy_audio_PluginSections*,
	const char* sectionkey);
psy_Property* psy_audio_pluginsections_plugin_exists(psy_audio_PluginSections*,
	psy_Property* section, psy_audio_MachineInfo* macinfo);
psy_Property* psy_audio_pluginsections_pluginbyid(psy_audio_PluginSections*,
	psy_Property* section, const char* id);
psy_Property* psy_audio_pluginsections_section(psy_audio_PluginSections*,
	const char* key);
psy_Property* psy_audio_pluginsections_section_plugins(
	psy_audio_PluginSections*, const char* key);
uintptr_t psy_audio_pluginsections_num_plugins(const psy_audio_PluginSections*,
	const char* sectionkey);

/* psy_audio_PluginScanTask */
typedef struct psy_audio_PluginScanTask {
	psy_audio_MachineType type;
	char wildcard[256];
	char label[256];	
	char key[256];
	bool recursive;
} psy_audio_PluginScanTask;

void psy_audio_pluginscantask_init_all(psy_audio_PluginScanTask*,
	psy_audio_MachineType type, const char* wildcard, const char* label, const char* key,
	int recursive);

/* psy_audio_PluginCategories */
typedef struct psy_audio_PluginCategories
{
	psy_Table container;
} psy_audio_PluginCategories;

void psy_audio_plugincategories_init(psy_audio_PluginCategories*,
	psy_Property* plugins);
void psy_audio_plugincategories_dispose(psy_audio_PluginCategories*);

psy_TableIterator psy_audio_plugincategories_begin(
	psy_audio_PluginCategories* self);
void psy_audio_plugincategories_update(psy_audio_PluginCategories*,
	psy_Property* plugins);

/* psy_audio_PluginCatcher */
typedef struct psy_audio_PluginCatcher {
	psy_audio_PluginSections sections;	
	char* nativeroot;
	psy_Property* directories;	
	psy_Signal signal_changed;
	psy_Signal signal_scanstart;
	psy_Signal signal_scanend;
	psy_Signal signal_scanprogress;
	psy_Signal signal_scanfile;	
	psy_Signal signal_taskstart;
	bool saveafterscan;
	bool hasplugincache;
	psy_audio_PluginCategoryList categorydefaults;	
	bool scanning;
	bool abort;
	psy_List* scantasks;
	/* refercences */
	psy_Property* all;
} psy_audio_PluginCatcher;

void psy_audio_plugincatcher_init(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_dispose(psy_audio_PluginCatcher*);

void psy_audio_plugincatcher_scan(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_clear(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_abort(psy_audio_PluginCatcher*);
int psy_audio_plugincatcher_load(psy_audio_PluginCatcher*);
int psy_audio_plugincatcher_save(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_set_directories(psy_audio_PluginCatcher*,
	psy_Property*);
char* psy_audio_plugincatcher_modulepath(psy_audio_PluginCatcher*,
	psy_audio_MachineType, int newgamefxblitzifversionunknown,
	const char* path,
	char* fullpath);
uintptr_t psy_audio_plugincatcher_extractshellidx(const char* path);
void psy_audio_plugincatcher_catchername(const char* path,
	char* name, uintptr_t shellidx);
const char* psy_audio_plugincatcher_search_path(psy_audio_PluginCatcher*,
	const char* name, int type);
psy_Property* psy_audio_plugincatcher_at(psy_audio_PluginCatcher* self,
	const char* id);
void plugincatcher_incfavorite(psy_audio_PluginCatcher*, const char* id);
bool psy_audio_plugincatcher_scanning(const psy_audio_PluginCatcher* self);

INLINE bool psy_audio_plugincatcher_hascache(const psy_audio_PluginCatcher*
	self)
{
	assert(self);

	return self->hasplugincache;
}

INLINE psy_Property* psy_audio_plugincatcher_plugins(psy_audio_PluginCatcher*
	self)
{
	assert(self);

	self->all = psy_audio_pluginsections_section_plugins(&self->sections,
		"all");
	return self->all;
}

INLINE void psy_audio_plugincatcher_notifychange(psy_audio_PluginCatcher* self)
{
	psy_signal_emit(&self->signal_changed, self, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLUGINCATCHER_H */
