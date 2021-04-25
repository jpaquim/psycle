// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "config.h"
#include "skinio.h"

// host
#include "defaultlang.h"
#include "skinio.h"
// ui
#include <uicomponent.h> // Translator
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/os.h"

#if defined DIVERSALIS__COMPILER__GNU || defined DIVERSALIS__OS__POSIX
	#define _MAX_PATH 4096
#endif

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
	psy_property_init(&self->config);
	psy_property_setcomment(&self->config,
		"Psycle Configuration File created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	psycleconfig_definelanguage(self);
	psycleconfig_initsections(self, player, machinefactory);
}

void psycleconfig_dispose(PsycleConfig* self)
{	
	generalconfig_dispose(&self->general);
	audioconfig_dispose(&self->audio);
	eventdriverconfig_dispose(&self->input);
	languageconfig_dispose(&self->language);
	dirconfig_dispose(&self->directories);
	patternviewconfig_dispose(&self->patview);
	machineviewconfig_dispose(&self->macview);
	machineparamconfig_dispose(&self->macparam);
	metronomeconfig_dispose(&self->metronome);
	keyboardmiscconfig_dispose(&self->misc);
	midiviewconfig_dispose(&self->midi);
	compatconfig_dispose(&self->compat);
	predefsconfig_dispose(&self->predefs);
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
	predefsconfig_init(&self->predefs, &self->config);
	metronomeconfig_init(&self->metronome, &self->config);
}

void psycleconfig_makeglobal(PsycleConfig* self)
{
	assert(self);

	self->global = psy_property_settext(
		psy_property_append_section(&self->config, "global"),
		"settingsview.global.configuration");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->global, "enableaudio", TRUE),
		"settingsview.global.enable-audio"),
		PROPERTY_ID_ENABLEAUDIO);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->global, "regenerateplugincache"),
		"settingsview.global.regenerate-plugincache"),
		PROPERTY_ID_REGENERATEPLUGINCACHE);
	languageconfig_init(&self->language, self->global, psy_ui_translator());
}

void psycleconfig_makevisual(PsycleConfig* self)
{
	assert(self);

	self->visual = psy_property_settext(
		psy_property_append_section(&self->config, "visual"),
		"settingsview.visual.visual");
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->visual, "loadskin"),
		"settingsview.visual.load-skin"),
		PROPERTY_ID_LOADSKIN);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->visual, "defaultskin"),
		"settingsview.visual.default-skin"),
		PROPERTY_ID_DEFAULTSKIN);
	self->defaultfont = psy_property_setid(psy_property_settext(
		psy_property_append_font(self->visual, "defaultfont",
			PSYCLE_DEFAULT_FONT),
		"settingsview.visual.default-font"),
		PROPERTY_ID_DEFAULTFONT);
	self->apptheme = psy_property_setid(psy_property_settext(
		psy_property_append_choice(self->visual,
			"apptheme", 1),
		"settingsview.visual.apptheme"),
		PROPERTY_ID_APPTHEME);
	psy_property_settext(
		psy_property_append_int(self->apptheme, "light", psy_ui_LIGHTTHEME, 0, 1),
		"settingsview.visual.light");
	psy_property_settext(
		psy_property_append_int(self->apptheme, "dark", psy_ui_DARKTHEME, 0, 1),
		"settingsview.visual.dark");
	patternviewconfig_init(&self->patview, self->visual);
	machineviewconfig_init(&self->macview, self->visual);
	machineparamconfig_init(&self->macparam, self->visual);
}

// settings
void psycleconfig_loadskin(PsycleConfig* self, const psy_Path* path)
{
	psy_Property skin;
	const char* machine_gui_bitmap;

	assert(self);

	psy_property_init(&skin);
	skin_load(&skin, path);
	machine_gui_bitmap = psy_property_at_str(&skin, "machineguibitmap", 0);
	if (machine_gui_bitmap) {
		char psc[_MAX_PATH];		
				
		psy_dir_findfile(dirconfig_skins(&self->directories),
			machine_gui_bitmap, psc);
		if (psc[0] != '\0') {
			if (skin_loadpsc(&skin, psc) == PSY_OK) {
				const char* bpm;								

				bpm = psy_property_at_str(&skin, "machinedialbmp", NULL);
				if (bpm) {					
					psy_Path full;

					psy_path_init(&full, psc);
					psy_path_setname(&full, "");
					psy_path_setext(&full, "");
					psy_path_setname(&full, bpm);
					psy_property_set_str(&skin, "machinedialbmp",
						psy_path_full(&full));
					psy_path_dispose(&full);
				}
			}
		}		
	}
	machineparamconfig_settheme(&self->macparam, &skin);
	machineviewconfig_settheme(&self->macview, &skin);
	patternviewconfig_settheme(&self->patview, &skin);
	psy_property_dispose(&skin);
}

void psycleconfig_resetskin(PsycleConfig* self)
{	
	assert(self);
		
	patternviewconfig_resettheme(&self->patview);
	machineviewconfig_resettheme(&self->macview);
	machineparamconfig_resettheme(&self->macparam);
}

void psycleconfig_resetcontrolskin(PsycleConfig* self)
{
	assert(self);
	
	machineparamconfig_resettheme(&self->macparam);
}

const char* psycleconfig_defaultfontstr(const PsycleConfig* self)
{
	assert(self);

	return psy_property_at_str(self->visual, "defaultfont",
		PSYCLE_DEFAULT_FONT);
}

bool psycleconfig_audioenabled(const PsycleConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->global, "enableaudio", TRUE);
}

void psycleconfig_enableaudio(PsycleConfig* self, bool on)
{
	assert(self);

	psy_property_set_bool(self->global, "enableaudio", on);
	audioconfig_enableaudio(&self->audio, on);
}

void psycleconfig_notify_changed(PsycleConfig* self, psy_Property* property)
{			
	if (machineviewconfig_hasproperty(&self->macview, property)) {
		machineviewconfig_onchanged(&self->macview, property);
	} else if (patternviewconfig_hasproperty(&self->patview, property)) {
		patternviewconfig_onchanged(&self->patview, property);
	} else if (machineparamconfig_hasproperty(&self->macparam, property)) {
		machineparamconfig_onchanged(&self->macparam, property);
	} else if (generalconfig_hasproperty(&self->general, property)) {
		generalconfig_onchanged(&self->general, property);
	} else if (audioconfig_hasproperty(&self->audio, property)) {
		audioconfig_onchanged(&self->audio, property);	
	} else if (eventdriverconfig_hasproperty(&self->input, property)) {
		eventdriverconfig_onchanged(&self->input, property);
	} else if (languageconfig_hasproperty(&self->language, property)) {
		languageconfig_onchanged(&self->language, property);
	} else if (dirconfig_hasproperty(&self->directories, property)) {
		dirconfig_onchanged(&self->directories, property);
	} else if (keyboardmiscconfig_hasproperty(&self->misc, property)) {
		keyboardmiscconfig_onchanged(&self->misc, property);
	} else if (midiviewconfig_hasproperty(&self->midi, property)) {
		midiviewconfig_onchanged(&self->midi, property);
	} else if (compatconfig_hasproperty(&self->compat, property)) {
		compatconfig_onchanged(&self->compat, property);
	} else if (predefsconfig_hasproperty(&self->predefs, property)) {
		predefsconfig_onchanged(&self->predefs, property);
	}
}

void psycleconfig_notifyall_changed(PsycleConfig* self)
{
	generalconfig_onchanged(&self->general, &self->config);
	audioconfig_onchanged(&self->audio, &self->config);
	eventdriverconfig_onchanged(&self->input, &self->config);
	languageconfig_onchanged(&self->language, &self->config);
	dirconfig_onchanged(&self->directories, &self->config);
	machineviewconfig_onchanged(&self->macview, &self->config);
	machineparamconfig_onchanged(&self->macparam, &self->config);
	patternviewconfig_onchanged(&self->patview, &self->config);
	keyboardmiscconfig_onchanged(&self->misc, &self->config);
	midiviewconfig_onchanged(&self->midi, &self->config);
	compatconfig_onchanged(&self->compat, &self->config);
	predefsconfig_onchanged(&self->predefs, &self->config);
}

void psycleconfig_notify_skinchanged(PsycleConfig* self, psy_Property* property)
{
	patternviewconfig_onthemechanged(&self->patview, property);
	machineviewconfig_onthemechanged(&self->macview, property);
	machineparamconfig_onthemechanged(&self->macparam, property);
}
