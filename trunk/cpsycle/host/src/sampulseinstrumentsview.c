// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampulseinstrumentview.h"

static void OnSize(SampulseInstrumentView*, ui_component* sender, ui_size*);
static void AddString(SampulseInstrumentView*, const char* text);
static void AlignInstrumentView(SampulseInstrumentView*);
static void BuildInstrumentList(SampulseInstrumentView*);
static void OnSongChanged(SampulseInstrumentView*, Workspace*);

void InitSampulseInstrumentView(SampulseInstrumentView* self,
								ui_component* parent, Workspace* workspace)
{
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);	
	InitNoteMapEdit(&self->notemapedit, &self->component);
	signal_connect(&self->component.signal_size, self, OnSize);
	ui_listbox_init(&self->instrumentlist, &self->component);
	AlignInstrumentView(self);
	BuildInstrumentList(self);
//	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void OnSize(SampulseInstrumentView* self, ui_component* sender, ui_size* size)
{			
}

void AlignInstrumentView(SampulseInstrumentView* self)
{
	ui_component_setposition(&self->instrumentlist.component,		 5, 5, 210, 400);
	ui_component_setposition(&self->notemapedit.component,		   220, 5, 210, 200);
}

void BuildInstrumentList(SampulseInstrumentView* self)
{
	XMInstrument* instrument;
	int slot = 0;
	char buffer[20];
	for ( ; slot < 256; ++slot) {		
		if (instrument = table_at(&self->player->song->xminstruments.container, slot)) {
			_snprintf(buffer, 20, "%02X:%s", slot, xminstrument_name(instrument));
		} else {
			_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void OnSongChanged(SampulseInstrumentView* self, Workspace* workspace)
{		
	BuildInstrumentList(self);	
}

void AddString(SampulseInstrumentView* self, const char* text)
{
	ui_listbox_addstring(&self->instrumentlist, text);
}

