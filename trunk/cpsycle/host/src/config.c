/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "config.h"
/* host */
#include "configimport.h"
#include "defaultlang.h"
#include "resources/resource.h"
#include "styles.h"
/* ui */
#include <uicomponent.h> /* Translator */
#include <uiopendialog.h>
/* file */
#include <dir.h>


/* prototypes */
static void psycleconfig_define_language(PsycleConfig*);
static void psycleconfig_init_sections(PsycleConfig*, psy_audio_Player*,
	struct PluginScanThread*, psy_audio_MachineFactory*);
static void psycleconfig_connect_import(PsycleConfig*);
static void psycleconfig_on_import_config(PsycleConfig*, psy_Property* sender);

/* implementation */
void psycleconfig_init(PsycleConfig* self, psy_audio_Player* player,
	struct PluginScanThread* pluginscanthread, psy_audio_MachineFactory* machinefactory)
{
	assert(self);

	psy_property_init(&self->config);
	psy_property_set_comment(&self->config,
		"Psycle Configuration File created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));	
	psycleconfig_define_language(self);
	psycleconfig_init_sections(self, player, pluginscanthread,
		machinefactory);
	psycleconfig_connect_import(self);
}

void psycleconfig_dispose(PsycleConfig* self)
{	
	assert(self);

	generalconfig_dispose(&self->general);
	audioconfig_dispose(&self->audio);
	eventdriverconfig_dispose(&self->input);
	globalconfig_dispose(&self->global);
	dirconfig_dispose(&self->directories);
	visualconfig_dispose(&self->visual);	
	metronomeconfig_dispose(&self->metronome);
	seqeditconfig_dispose(&self->seqedit);
	keyboardmiscconfig_dispose(&self->misc);
	midiviewconfig_dispose(&self->midi);
	compatconfig_dispose(&self->compat);
	predefsconfig_dispose(&self->predefs);
	psy_property_dispose(&self->config);	
}

void psycleconfig_notify_all(PsycleConfig* self)
{
	assert(self);

	psy_property_notify_all(&self->config);
}

void psycleconfig_define_language(PsycleConfig* self)
{
	psy_Dictionary defaultlang;

	assert(self);

	psy_dictionary_init(&defaultlang);
	make_translator_default(&defaultlang);
	psy_translator_set_default(psy_ui_translator(), &defaultlang);
	psy_dictionary_dispose(&defaultlang);
}

void psycleconfig_init_sections(PsycleConfig* self, psy_audio_Player* player,
	struct PluginScanThread* pluginscanthread,
	psy_audio_MachineFactory* machinefactory)
{
	assert(self);

	globalconfig_init(&self->global, &self->config, player, pluginscanthread);	
	generalconfig_init(&self->general, &self->config);
	visualconfig_init(&self->visual, &self->config, player);
	keyboardmiscconfig_init(&self->misc, &self->config, player);
	dirconfig_init(&self->directories, &self->config);
	audioconfig_init(&self->audio, &self->config, player);
	psy_property_set_icon(self->audio.inputoutput, IDB_PULSE_LIGHT,
		IDB_PULSE_DARK);
	eventdriverconfig_init(&self->input, &self->config, player);
	midiviewconfig_init(&self->midi, &self->config, player);	
	compatconfig_init(&self->compat, &self->config, machinefactory);
	predefsconfig_init(&self->predefs, &self->config);
	metronomeconfig_init(&self->metronome, &self->config, player);
	seqeditconfig_init(&self->seqedit, &self->config);	
	visualconfig_set_directories(&self->visual, &self->directories);
}

void psycleconfig_connect_import(PsycleConfig* self)
{	
	assert(self);

	psy_signal_connect(&self->global.import_config->changed,
		self, psycleconfig_on_import_config);	
}

void psycleconfig_on_import_config(PsycleConfig* self, psy_Property* sender)
{
	psy_ui_OpenDialog opendialog;

	assert(self);

	psy_ui_opendialog_init_all(&opendialog, 0, "Import Config",
		"Psycle Ini Files|*.ini", "INI", dirconfig_app(&self->directories));
	if (psy_ui_opendialog_execute(&opendialog)) {
		PsycleConfigImport import;

		psycleconfigimport_init(&import, self);
		psycleconfigimport_read(&import, psy_path_full(
			psy_ui_opendialog_path(&opendialog)));
		psycleconfigimport_dispose(&import);
	}
	psy_ui_opendialog_dispose(&opendialog);
}
