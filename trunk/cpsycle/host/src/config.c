/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "config.h"
/* host */
#include "defaultlang.h"
#include "skinio.h"
#include "styles.h"
#include "configimport.h"
/* ui */
#include <uicomponent.h> /* Translator */
#include <uiapp.h> /* Styles */
#include <uiopendialog.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/os.h"

#if defined DIVERSALIS__COMPILER__GNU || defined DIVERSALIS__OS__POSIX
	#define _MAX_PATH 4096
#endif

/* prototypes */
static void psycleconfig_define_language(PsycleConfig*);
static void psycleconfig_init_sections(PsycleConfig*, psy_audio_Player*,
	psy_audio_MachineFactory*);
static void psycleconfig_make_global(PsycleConfig*);
static void psycleconfig_make_visual(PsycleConfig*);
static void psycleconfig_make_midi_configuration(PsycleConfig*);
static void psycleconfig_make_midi_controllers(PsycleConfig*);
static void psycleconfig_on_load_skin(PsycleConfig*);
static void psycleconfig_on_import_config(PsycleConfig*);
static void psycleconfig_import_config(PsycleConfig*, const char* path);

/* implementation */
void psycleconfig_init(PsycleConfig* self, psy_audio_Player* player,
	psy_audio_MachineFactory* machinefactory)
{	
	psy_property_init(&self->config);
	psy_property_set_comment(&self->config,
		"Psycle Configuration File created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	psycleconfig_define_language(self);
	psycleconfig_init_sections(self, player, machinefactory);
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
	seqeditconfig_dispose(&self->seqedit);
	keyboardmiscconfig_dispose(&self->misc);
	midiviewconfig_dispose(&self->midi);
	compatconfig_dispose(&self->compat);
	predefsconfig_dispose(&self->predefs);
	psy_property_dispose(&self->config);	
}

void psycleconfig_define_language(PsycleConfig* self)
{
	psy_Property defaultlang;

	psy_property_init(&defaultlang);
	make_translator_default(&defaultlang);
	psy_translator_setdefault(psy_ui_translator(), &defaultlang);
	psy_property_dispose(&defaultlang);
}

/* creation */
void psycleconfig_init_sections(PsycleConfig* self, psy_audio_Player* player,
	psy_audio_MachineFactory* machinefactory)
{
	psycleconfig_make_global(self);
	generalconfig_init(&self->general, &self->config);
	psycleconfig_make_visual(self);
	keyboardmiscconfig_init(&self->misc, &self->config);
	dirconfig_init(&self->directories, &self->config);
	audioconfig_init(&self->audio, &self->config, player);
	eventdriverconfig_init(&self->input, &self->config, player);
	midiviewconfig_init(&self->midi, &self->config, player);	
	compatconfig_init(&self->compat, &self->config, machinefactory);
	predefsconfig_init(&self->predefs, &self->config);
	metronomeconfig_init(&self->metronome, &self->config, player);
	seqeditconfig_init(&self->seqedit, &self->config);	
	patternviewconfig_set_directories(&self->patview, &self->directories);
	machineparamconfig_setdirectories(&self->macparam, &self->directories);
	machineviewconfig_setdirectories(&self->macview, &self->directories);	
}

void psycleconfig_make_global(PsycleConfig* self)
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
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->global, "importconfig"),
		"settingsview.global.importconfig"),
		PROPERTY_ID_IMPORTCONFIG);
}

void psycleconfig_make_visual(PsycleConfig* self)
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
		psy_property_append_int(self->apptheme, "light", psy_ui_LIGHTTHEME, 0, 2),
		"settingsview.visual.light");
	psy_property_settext(
		psy_property_append_int(self->apptheme, "dark", psy_ui_DARKTHEME, 0, 2),
		"settingsview.visual.dark");
	psy_property_settext(
		psy_property_append_int(self->apptheme, "win98", psy_ui_WIN98THEME, 0, 2),
		"Windows 98");	
	patternviewconfig_init(&self->patview, self->visual,		
		PSYCLE_SKINS_DEFAULT_DIR);
	machineviewconfig_init(&self->macview, self->visual);
	machineparamconfig_init(&self->macparam, self->visual);
}

void psycleconfig_loadskin(PsycleConfig* self, const char* path)
{
	psy_Property skin;
	const char* machine_gui_bitmap;

	assert(self);

	psy_property_init(&skin);
	skin_load(&skin, path);
	machine_gui_bitmap = psy_property_at_str(&skin, "machine_GUI_bitmap", 0);
	if (machine_gui_bitmap) {
		char psc[_MAX_PATH];		
				
		psy_dir_findfile(dirconfig_skins(&self->directories),
			machine_gui_bitmap, psc);
		if (psc[0] != '\0') {
			psy_Path path;

			psy_path_init(&path, psc);
			if (strcmp(psy_path_ext(&path), "bmp") == 0) {
				psy_property_set_str(&skin, "machinedial_bmp",
					psy_path_full(&path));
			} else if (skin_load_psc(&skin, psc) == PSY_OK) {
				const char* bpm;								

				bpm = psy_property_at_str(&skin, "machinedial_bmp", NULL);
				if (bpm) {					
					psy_Path full;

					psy_path_init(&full, psc);
					psy_path_set_name(&full, "");
					psy_path_setext(&full, "");
					psy_path_set_name(&full, bpm);
					psy_property_set_str(&skin, "machinedial_bmp",
						psy_path_full(&full));
					psy_path_dispose(&full);
				}
			}
		}		
	}
	machineparamconfig_settheme(&self->macparam, &skin);
	machineviewconfig_settheme(&self->macview, &skin);	
	patternviewconfig_set_theme(&self->patview, &skin);
	psy_property_dispose(&skin);
}

void psycleconfig_reset_skin(PsycleConfig* self)
{	
	assert(self);
		
	patternviewconfig_reset_theme(&self->patview);
	machineviewconfig_resettheme(&self->macview);	
	machineparamconfig_resettheme(&self->macparam);
}

void psycleconfig_reset_control_skin(PsycleConfig* self)
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

bool psycleconfig_audio_enabled(const PsycleConfig* self)
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

int psycleconfig_notify_changed(PsycleConfig* self, psy_Property* property)
{		
	switch (psy_property_id(property)) {	
	case PROPERTY_ID_DEFAULTSKIN:
		psycleconfig_reset_skin(self);
		return 1;		
	case PROPERTY_ID_LOADSKIN:
		psycleconfig_on_load_skin(self);
		return 1;
	case PROPERTY_ID_IMPORTCONFIG:
		psycleconfig_on_import_config(self);
		return 1;
	default:
		break;
	}
	if (machineviewconfig_hasproperty(&self->macview, property)) {
		return machineviewconfig_onchanged(&self->macview, property);
	} else if (patternviewconfig_has_property(&self->patview, property)) {
		return patternviewconfig_onchanged(&self->patview, property);
	} else if (machineparamconfig_hasproperty(&self->macparam, property)) {
		return machineparamconfig_onchanged(&self->macparam, property);
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
	} else if (metronomeconfig_hasproperty(&self->metronome, property)) {
		return metronomeconfig_onchanged(&self->metronome, property);
	} else if (seqeditconfig_hasproperty(&self->seqedit, property)) {
		seqeditconfig_onchanged(&self->seqedit, property);
	}
	return 0;
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
	metronomeconfig_onchanged(&self->metronome, &self->config);
	seqeditconfig_onchanged(&self->seqedit, &self->config);
}

void psycleconfig_on_load_skin(PsycleConfig* self)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Load Theme", "Psycle Display Presets|*.psv", "PSV",
		dirconfig_skins(&self->directories));
	if (psy_ui_opendialog_execute(&opendialog)) {
		psycleconfig_loadskin(self, psy_path_full(psy_ui_opendialog_path(
			&opendialog)));
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void psycleconfig_on_import_config(PsycleConfig* self)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Import Config", "Psycle Ini Files|*.ini", "INI",
		dirconfig_app(&self->directories));
	if (psy_ui_opendialog_execute(&opendialog)) {
		psycleconfig_import_config(self, psy_path_full(psy_ui_opendialog_path(
			&opendialog)));
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void psycleconfig_import_config(PsycleConfig* self, const char* path)
{
	PsycleConfigImport import;

	psycleconfigimport_init(&import, self);
	psycleconfigimport_read(&import, path);
	psycleconfigimport_dispose(&import);
}
