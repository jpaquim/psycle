// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "config.h"
#include "skinio.h"

// host
#include "defaultlang.h"
#include "skinio.h"
// ui
#include "uicomponent.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void psycleconfig_definelanguage(PsycleConfig*);
static void psycleconfig_initsections(PsycleConfig*, psy_audio_Player*,
	psy_audio_MachineFactory*);
static void psycleconfig_makeglobal(PsycleConfig*);
static void psycleconfig_makevisual(PsycleConfig*);
static void psycleconfig_makemidiconfiguration(PsycleConfig*);
static void psycleconfig_makemidicontrollers(PsycleConfig*);
// implementation
void psycleconfig_init(PsycleConfig* self, psy_audio_Player* player,
	psy_audio_MachineFactory* machinefactory)
{
	self->dialbitmappath = 0;
	psy_property_init(&self->config);
	psy_property_setcomment(&self->config,
		"Psycle Configuration File created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	psycleconfig_definelanguage(self);
	psycleconfig_initsections(self, player, machinefactory);
}

void psycleconfig_dispose(PsycleConfig* self)
{
	free(self->dialbitmappath);
	psy_property_dispose(&self->config);
}

void psycleconfig_definelanguage(PsycleConfig* self)
{
	psy_Property defaultlang;

	psy_property_init(&defaultlang);
	make_translator_default(&defaultlang);
	psy_translator_setdefault(psy_ui_translator(), &defaultlang);
	psy_property_dispose(&defaultlang);
}

// creation
void psycleconfig_initsections(PsycleConfig* self, psy_audio_Player* player,
	psy_audio_MachineFactory* machinefactory)
{
	psycleconfig_makeglobal(self);
	generalconfig_init(&self->general, &self->config);
	psycleconfig_makevisual(self);
	keyboardmiscconfig_init(&self->misc, &self->config);
	dirconfig_init(&self->directories, &self->config);
	audioconfig_init(&self->audio, &self->config, player);
	eventdriverconfig_init(&self->input, &self->config, player);
	midiviewconfig_init(&self->midi, &self->config, player);	
	compatconfig_init(&self->compat, &self->config, machinefactory);
}

void psycleconfig_makeglobal(PsycleConfig* self)
{
	assert(self);

	self->global = psy_property_settext(
		psy_property_append_section(&self->config, "global"),
		"Configuration");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->global, "enableaudio", TRUE),
		"Enable audio"),
		PROPERTY_ID_ENABLEAUDIO);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->global, "regenerateplugincache"),
		"Regenerate the plugin cache"),
		PROPERTY_ID_REGENERATEPLUGINCACHE);
	languageconfig_init(&self->language, self->global, psy_ui_translator());
}

void psycleconfig_makevisual(PsycleConfig* self)
{
	assert(self);

	self->visual = psy_property_settext(
		psy_property_append_section(&self->config, "visual"),
		"settingsview.visual");
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->visual, "loadskin"),
		"settingsview.load-skin"),
		PROPERTY_ID_LOADSKIN);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->visual, "defaultskin"),
		"settingsview.default-skin"),
		PROPERTY_ID_DEFAULTSKIN);
	psy_property_setid(psy_property_settext(
		psy_property_append_font(self->visual, "defaultfont", PSYCLE_DEFAULT_FONT),
		"settingsview.default-font"),
		PROPERTY_ID_DEFAULTFONT);
	patternviewconfig_init(&self->patview, self->visual);
	machineviewconfig_init(&self->macview, self->visual);
	machineparamconfig_init(&self->macparam, self->visual);
}

// settings

void psycleconfig_loadskin(PsycleConfig* self, const char* path)
{
	psy_Property skin;

	assert(self);

	psy_property_init(&skin);
	skin_load(&skin, path);
	psy_property_sync(self->macparam.paramtheme, &skin);
	psy_property_sync(self->macview.theme, &skin);
	psy_property_sync(self->patview.theme, &skin);
	psy_property_dispose(&skin);
}

void psycleconfig_resetskin(PsycleConfig* self)
{
	psy_Property* view;
	psy_Property* theme;

	view = psy_property_findsection(&self->config, "visual.patternview");
	theme = psy_property_findsection(view, "theme");
	if (theme) {
		psy_property_remove(view, theme);
	}
	patternviewconfig_maketheme(&self->patview, view);
	view = psy_property_findsection(&self->config, "visual.machineview");
	theme = psy_property_findsection(view, "theme");
	if (theme) {
		psy_property_remove(view, theme);
	}
	machineviewconfig_maketheme(&self->macview, view);
	view = psy_property_findsection(&self->config, "visual.paramview");
	theme = psy_property_findsection(view, "theme");
	if (theme) {
		psy_property_remove(view, theme);
	}
	machineparamconfig_makeparamtheme(&self->macparam, view);	
}

void psycleconfig_setcontrolskinpath(PsycleConfig* self, const char* path)
{
	assert(self);

	psy_strreset(&self->dialbitmappath, path);
}

const char* psycleconfig_defaultfontstr(const PsycleConfig* self)
{
	assert(self && self->visual);
	return psy_property_at_str(self->visual, "defaultfont", PSYCLE_DEFAULT_FONT);
}

bool psycleconfig_enableaudio(const PsycleConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->global, "enableaudio", TRUE);
}
