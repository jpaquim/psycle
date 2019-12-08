// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "renderview.h"

static void renderview_ondestroy(RenderView*, ui_component* sender);
static void renderview_makeproperties(RenderView*);

void renderview_init(RenderView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{	
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_destroy, self,
		renderview_ondestroy);
	ui_component_enablealign(&self->component);
	renderview_makeproperties(self);
	settingsview_init(&self->view, &self->component, tabbarparent,
		self->properties);
	ui_component_setalign(&self->view.component, UI_ALIGN_CLIENT);
}

void renderview_ondestroy(RenderView* self, ui_component* sender)
{
	properties_free(self->properties);
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
	properties_append_action(actions, "Save Wave");	
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

