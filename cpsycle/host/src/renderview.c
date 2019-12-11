// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "renderview.h"

#include <fileoutdriver.h>

static void renderview_ondestroy(RenderView*, ui_component* sender);
static void renderview_makeproperties(RenderView*);
static void renderview_onsettingsviewchanged(RenderView*, SettingsView* sender,
	Properties*);
static void renderview_render(RenderView*);
static void renderview_onstoprendering(RenderView*, Driver* sender);

void renderview_init(RenderView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self,
		renderview_ondestroy);
	ui_component_enablealign(&self->component);
	renderview_makeproperties(self);
	settingsview_init(&self->view, &self->component, tabbarparent,
		self->properties);
	psy_signal_connect(&self->view.signal_changed, self,
		renderview_onsettingsviewchanged);
	ui_component_setalign(&self->view.component, UI_ALIGN_CLIENT);
	self->fileoutdriver = create_fileout_driver();
}

void renderview_ondestroy(RenderView* self, ui_component* sender)
{
	properties_free(self->properties);	
	self->fileoutdriver->dispose(self->fileoutdriver);
	self->fileoutdriver->free(self->fileoutdriver);
}

void renderview_makeproperties(RenderView* self)
{
	Properties* filesave;
	Properties* savechoice;	
	Properties* record;
	Properties* recordchoice;
	Properties* recordpatnum;
	Properties* recordseqpos;
	Properties* quality;
	Properties* actions;

	self->properties = properties_create();
	actions = properties_settext(
		properties_createsection(self->properties, "actions"),
		"Render");
	properties_settext(
		properties_append_action(actions, "savewave"),
		"Save wave");
	filesave = properties_settext(
		properties_createsection(self->properties, "filesave"),
		"File");
	properties_settext(
		properties_append_string(filesave, "filesave-outputpath", "Untitled.wav"),
		"Output Path");
	savechoice = properties_settext(
		properties_append_choice(filesave, "filesave-choice", 0),
		"Save each unmuted");
	properties_settext(
		properties_append_string(savechoice, "filesave-channel", ""),
		"input to master as a separated wav (wire number"
		" will be appended to filename)"
	);
	properties_settext(
		properties_append_string(savechoice, "filesave-track", ""),
		"track as a separated"
		"wav (track number will be appended to filename) ** may suffer from"
		"'delay bleed' - insert silence at the end of your file if this is a "
		"problem."
	);
	properties_settext(
		properties_append_string(savechoice, "filesave-generator", ""),
		"generator as a separated wav (generator number will"
		"be appended to filename) ** may suffer from 'delay bleed' - insert"
		" silence at the end of your file if this is a problem."
	);
	record = properties_settext(
		properties_createsection(self->properties, "record"),
		"Selection");	
	recordchoice = properties_settext(
		properties_append_choice(record, "record", 0),
		"Record");
	properties_settext(
		properties_append_string(recordchoice, "record-entiresong", ""),
		"the entire song"
	);
	recordpatnum = properties_settext(
		properties_append_string(recordchoice, "record-pattern", ""),
		"pattern"
	);
	properties_settext(
		properties_append_int(recordpatnum, "record-pattern-number", 0, 0, 256),
		"number"
	);
	recordseqpos = properties_settext(
		properties_append_string(recordchoice, "record-seqpos", ""),
		"sequence positions"
	);
	properties_settext(
		properties_append_int(recordseqpos, "record-seqpos-from", 0, 0, 256),
		"from"
	);
	properties_settext(
		properties_append_int(recordseqpos, "record-seqpos-to", 0, 0, 256),
		"to"
	);
	quality = properties_settext(
		properties_createsection(self->properties, "quality"),
		"Quality");	
	properties_settext(
		properties_append_int(quality, "quality-samplerate", 44100, 0, 96000),
		"Sample Rate"
	);
	properties_settext(
		properties_append_int(quality, "quality-bitdepth", 8, 8, 16),
		"Bit Depth"
	);
	properties_settext(
		properties_append_int(quality, "quality-channels", 2, 0, 2),
		"Channels"
	);	
}

void renderview_onsettingsviewchanged(RenderView* self, SettingsView* sender,
	Properties* property)
{
	if (properties_type(property) == PROPERTY_TYP_ACTION) {
		if (strcmp(properties_key(property), "savewave") == 0) {
			renderview_render(self);
		}
	}
}

void renderview_render(RenderView* self)
{	
	self->curraudiodriver = player_audiodriver(&self->workspace->player);
	self->curraudiodriver->close(self->curraudiodriver);
	player_setaudiodriver(&self->workspace->player, self->fileoutdriver);
	self->restoreloopmode = self->workspace->player.sequencer.looping;
	self->workspace->player.sequencer.looping = 0;
	player_setposition(&self->workspace->player, 0);
	player_start(&self->workspace->player);	
	psy_signal_connect(&self->fileoutdriver->signal_stop, self,
		renderview_onstoprendering);
	self->fileoutdriver->open(self->fileoutdriver);
}

void renderview_onstoprendering(RenderView* self, Driver* sender)
{
	player_stop(&self->workspace->player);
	player_setaudiodriver(&self->workspace->player, self->curraudiodriver);
	self->workspace->player.sequencer.looping = self->restoreloopmode;
	self->curraudiodriver->open(self->curraudiodriver);
	self->fileoutdriver->close(self->fileoutdriver);
}
