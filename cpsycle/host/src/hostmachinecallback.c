/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "hostmachinecallback.h"
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* platform */
#include "../../detail/portable.h"


#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#include <unistd.h>
#endif

/* prototypes */
static psy_audio_MachineFactory* hostmachinecallback_on_machinefactory(HostMachineCallback*);
static bool hostmachinecallback_on_machine_file_select_load(HostMachineCallback*,
	char filter[], char inoutName[]);
static bool hostmachinecallback_on_machine_file_select_save(HostMachineCallback*,
	char filter[], char inoutName[]);
static void hostmachinecallback_on_machine_file_select_directory(HostMachineCallback*);
static void hostmachinecallback_on_machine_terminal_output(HostMachineCallback*,
	const char* text);
static bool hostmachinecallback_on_machine_edit_resize(HostMachineCallback*,
	psy_audio_Machine* sender, intptr_t w, intptr_t h);
static void hostmachinecallback_on_machine_bus_changed(HostMachineCallback*,
	psy_audio_Machine* sender);

/* vtable */
static psy_audio_MachineCallbackVtable hostmachinecallback_vtable;
static bool hostmachinecallback_vtable_initialized = FALSE;

static void hostmachinecallbackvtable_init(HostMachineCallback* self)
{
	assert(self);

	if (!hostmachinecallback_vtable_initialized) {
		hostmachinecallback_vtable = *self->machinecallback.vtable;
		hostmachinecallback_vtable.machinefactory =
			(fp_mcb_machinefactory)
			hostmachinecallback_on_machinefactory;
		hostmachinecallback_vtable.fileselect_load =
			(fp_mcb_fileselect_load)
			hostmachinecallback_on_machine_file_select_load;
		hostmachinecallback_vtable.fileselect_save =
			(fp_mcb_fileselect_save)
			hostmachinecallback_on_machine_file_select_save;
		hostmachinecallback_vtable.fileselect_directory =
			(fp_mcb_fileselect_directory)
			hostmachinecallback_on_machine_file_select_directory;
		hostmachinecallback_vtable.editresize =
			(fp_mcb_editresize)
			hostmachinecallback_on_machine_edit_resize;
		hostmachinecallback_vtable.buschanged =
			(fp_mcb_buschanged)
			hostmachinecallback_on_machine_bus_changed;
		hostmachinecallback_vtable.output =
			(fp_mcb_output)
			hostmachinecallback_on_machine_terminal_output;
		hostmachinecallback_vtable_initialized = TRUE;
	}
	self->machinecallback.vtable = &hostmachinecallback_vtable;
}

/* implementation */
void hostmachinecallback_init(HostMachineCallback* self,	
	DirConfig* dir_config,
	psy_Signal* signal_machineeditresize,
	psy_Signal* signal_buschanged)
{
	assert(self);
	assert(dir_config);	
	assert(signal_machineeditresize);

	psy_audio_machinecallback_init(&self->machinecallback);
	hostmachinecallbackvtable_init(self);
	self->machine_factory = NULL;
	self->dir_config = dir_config;
	self->logger = NULL;
	self->signal_machineeditresize = signal_machineeditresize;
	self->signal_buschanged = signal_buschanged;
}

void hostmachinecallback_set_machine_factory(HostMachineCallback* self,
	psy_audio_MachineFactory* factory)
{
	self->machine_factory = factory;
}

psy_audio_MachineFactory* hostmachinecallback_on_machinefactory(HostMachineCallback* self)
{
	return self->machine_factory;
}

bool hostmachinecallback_on_machine_file_select_load(HostMachineCallback* self, char filter[],
	char inout_name[])
{
	bool success;
	psy_ui_OpenDialog dialog;

	assert(self);

	psy_ui_opendialog_init_all(&dialog, NULL, "file.plugin-fileload", filter,
		"", dirconfig_vsts32(self->dir_config));
	success = psy_ui_opendialog_execute(&dialog);
	psy_snprintf(inout_name, _MAX_PATH, "%s",
		psy_path_full(psy_ui_opendialog_path(&dialog)));
	psy_ui_opendialog_dispose(&dialog);
	return success;
}

bool hostmachinecallback_on_machine_file_select_save(HostMachineCallback* self, char filter[],
	char inout_name[])
{
	bool success;
	psy_ui_SaveDialog dialog;

	assert(self);

	psy_ui_savedialog_init_all(&dialog, NULL, "file.plugin-filesave", filter,
		"", dirconfig_vsts32(self->dir_config));
	success = psy_ui_savedialog_execute(&dialog);
	psy_snprintf(inout_name, _MAX_PATH, "%s",
		psy_path_full(psy_ui_savedialog_path(&dialog)));
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

void hostmachinecallback_on_machine_file_select_directory(HostMachineCallback* self)
{

}

void hostmachinecallback_on_machine_terminal_output(HostMachineCallback* self, const char* text)
{
	if (self->logger) {
		// self->terminalstyleid = STYLE_TERM_BUTTON_ERROR;
		psy_logger_error(self->logger, text);
	}
}

bool hostmachinecallback_on_machine_edit_resize(HostMachineCallback* self, psy_audio_Machine* sender,
	intptr_t w, intptr_t h)
{
	psy_signal_emit(self->signal_machineeditresize, self, 3, sender, w, h);
	return TRUE;
}

void hostmachinecallback_on_machine_bus_changed(HostMachineCallback* self, psy_audio_Machine* sender)
{
	psy_signal_emit(self->signal_buschanged, self, 1, sender);
}
