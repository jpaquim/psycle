// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "dirconfig.h"
// file
#include <dir.h>
// platform
#include "../../detail/cpu.h"
#include "../../detail/portable.h"

static void dirconfig_make(DirConfig*);
static void dirconfig_makedefaultuserpresets(DirConfig*);
static void dirconfig_append(DirConfig*, const char* key,
	const char* label, const char* defaultdir);

void dirconfig_init(DirConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	dirconfig_make(self);
	psy_signal_init(&self->signal_changed);
}

void dirconfig_dispose(DirConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void dirconfig_make(DirConfig* self)
{
#if defined(DIVERSALIS__OS__MICROSOFT)		
#else
	char path[4096];
#endif

	assert(self);
	
	self->directories = psy_property_settext(
		psy_property_append_section(self->parent, "directories"),
		"settingsview.dirs.dirs");
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)	
	psy_property_sethint(
		psy_property_settext(
			psy_property_append_str(self->directories,
				"app", PSYCLE_APP_DIR), "App directory"),
#else
	psy_property_sethint(
		psy_property_settext(
			psy_property_append_str(self->directories,
				"app", PSYCLE_APP64_DIR), "App directory"),
#endif
		PSY_PROPERTY_HINT_HIDE);
#if defined(DIVERSALIS__OS__MICROSOFT)		
	dirconfig_append(self, "songs", "settingsview.dirs.song",
		PSYCLE_SONGS_DEFAULT_DIR);
#else
	psy_snprintf(path, 4096, "%s", psy_dir_home());
	printf("path %s\n", path);
	//dirconfig_makedirectory(self, "songs", "settingsview.dirs.song",
		//path);
	dirconfig_append(self, "songs", "settingsview.dirs.song",
		PSYCLE_SONGS_DEFAULT_DIR);
#endif		
	dirconfig_append(self, "samples", "settingsview.dirs.samples",
		PSYCLE_SAMPLES_DEFAULT_DIR);
	dirconfig_append(self, "plugins32", "settingsview.dirs.plugin32",
		PSYCLE_PLUGINS32_DEFAULT_DIR);
	dirconfig_append(self, "plugins64", "settingsview.dirs.plugin64",
		PSYCLE_PLUGINS64_DEFAULT_DIR);
	dirconfig_append(self, "luascripts", "settingsview.dirs.lua",
		PSYCLE_LUASCRIPTS_DEFAULT_DIR);
	dirconfig_append(self, "vsts32", "settingsview.dirs.vst32",
		PSYCLE_VSTS32_DEFAULT_DIR);
	dirconfig_append(self, "vsts64", "settingsview.dirs.vst64",
		PSYCLE_VSTS64_DEFAULT_DIR);
	dirconfig_append(self, "ladspas", "settingsview.dirs.ladspa",
		PSYCLE_LADSPAS_DEFAULT_DIR);
	dirconfig_append(self, "skin", "settingsview.dirs.skin",
		"C:\\Programme\\Psycle\\Skins");
	dirconfig_makedefaultuserpresets(self);
}

void dirconfig_makedefaultuserpresets(DirConfig* self)
{
	psy_Path defaultuserpresetpath;

	assert(self);

	psy_path_init(&defaultuserpresetpath, psy_dir_home());
	psy_path_setname(&defaultuserpresetpath, "Presets");
	dirconfig_append(self, "presets", "User Presets directory",
		psy_path_full(&defaultuserpresetpath));
	psy_path_dispose(&defaultuserpresetpath);
}

void dirconfig_append(DirConfig* self, const char* key,
	const char* label, const char* defaultdir)
{
	assert(self);

	psy_property_sethint(
		psy_property_settext(
			psy_property_append_str(self->directories, key, defaultdir),
			label),
		PSY_PROPERTY_HINT_EDITDIR);
}
// getter
const char* dirconfig_songs(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "songs",
		PSYCLE_SONGS_DEFAULT_DIR);
}

const char* dirconfig_samples(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "samples",
		PSYCLE_SAMPLES_DEFAULT_DIR);
}

const char* dirconfig_plugins32(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "plugins",
		PSYCLE_PLUGINS32_DEFAULT_DIR);
}

const char* dirconfig_plugins64(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "plugins",
		PSYCLE_PLUGINS32_DEFAULT_DIR);
}

const char* dirconfig_pluginscurrplatform(const DirConfig* self)
{
	assert(self);

#if (DIVERSALIS__CPU__SIZEOF_POINTER == 4)
	return psy_property_at_str(self->directories, "plugins32",
		PSYCLE_PLUGINS32_DEFAULT_DIR);
#else
	return psy_property_at_str(self->directories, "plugins64",
		PSYCLE_PLUGINS32_DEFAULT_DIR);
#endif
}

const char* dirconfig_luascripts(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "luascripts",
		PSYCLE_LUASCRIPTS_DEFAULT_DIR);
}

const char* dirconfig_vsts32(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "vsts32",
		PSYCLE_VSTS32_DEFAULT_DIR);
}

const char* dirconfig_vsts64(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "vsts64",
		PSYCLE_VSTS64_DEFAULT_DIR);
}

const char* dirconfig_ladspa_dir(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "ladspas",
		PSYCLE_LADSPAS_DEFAULT_DIR);
}

const char* dirconfig_skins(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "skins",
		PSYCLE_SKINS_DEFAULT_DIR);
}

const char* dirconfig_doc(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "skins",
		PSYCLE_DOC_DEFAULT_DIR);
}

const char* dirconfig_config(const DirConfig* self)
{
	assert(self);

	return psy_dir_config();
}

const char* dirconfig_userpresets(const DirConfig* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "presets",
		PSYCLE_USERPRESETS_DEFAULT_DIR);
}
// events
bool dirconfig_onchanged(DirConfig* self, psy_Property*
	property)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, property);
	return TRUE;
}

bool dirconfig_hasproperty(const DirConfig* self, psy_Property* property)
{
	assert(self && self->directories);

	return psy_property_insection(property, self->directories);
}
