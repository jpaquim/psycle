// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentsbox.h"
#include <stdio.h>
#include "../../detail/portable.h"

static void BuildInstrumentList(InstrumentsBox* self);
static void AddString(InstrumentsBox* self, const char* text);
static void OnInstrumentSlotChanged(InstrumentsBox* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* slot);
static void OnInstrumentInsert(InstrumentsBox* self, psy_ui_Component* sender,
	const psy_audio_InstrumentIndex* slot);
static void OnInstrumentRemoved(InstrumentsBox* self, psy_ui_Component* sender,
	const psy_audio_InstrumentIndex* slot);
static void OnInstrumentGroupListChanged(InstrumentsBox* self, psy_ui_Component* sender,
	int slot);
static void OnInstrumentListChanged(InstrumentsBox* self, psy_ui_Component* sender,
	int slot);

void instrumentsbox_init(InstrumentsBox* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments)
{		
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_label_settext(&self->header, "Instrument Group");
	psy_ui_listbox_init(&self->grouplist, &self->component);
	psy_ui_component_setalign(&self->grouplist.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->group, &self->component);
	psy_ui_component_setalign(&self->group.component, psy_ui_ALIGN_TOP);
	psy_ui_label_settext(&self->group, "Group Instruments");
	psy_ui_listbox_init(&self->instrumentlist, &self->component);
	psy_ui_component_setalign(&self->instrumentlist.component, psy_ui_ALIGN_CLIENT);
	instrumentsbox_setinstruments(self, instruments);
	psy_signal_connect(&self->grouplist.signal_selchanged, self,
		OnInstrumentGroupListChanged);
	psy_signal_connect(&self->instrumentlist.signal_selchanged, self,
		OnInstrumentListChanged);
}

void samplesbox_buildinstrumentgroup(InstrumentsBox* self)
{
	if (self->instruments) {
		uintptr_t slot = 0;
		char text[40];

		psy_ui_listbox_clear(&self->grouplist);
		for (; slot < max(psy_audio_instruments_groupsize(self->instruments), 1); ++slot) {
			psy_snprintf(text, 20, "%02X:%s", slot, "");
			psy_ui_listbox_addtext(&self->grouplist, text);
		}
	}
}

void BuildInstrumentList(InstrumentsBox* self)
{
	psy_audio_Instrument* instrument;
	int groupslot;
	int slot = 0;
	char buffer[20];

	psy_ui_listbox_clear(&self->instrumentlist);
	groupslot = psy_ui_listbox_cursel(&self->grouplist);
	if (groupslot == -1) {
		groupslot = 0;
	}
	for ( ; slot < 256; ++slot) {		
		if (instrument = instruments_at(self->instruments,
			instrumentindex_make(groupslot, slot))) {
			psy_snprintf(buffer, 20, "%02X*:%s", slot,
				psy_audio_instrument_name(instrument));
		} else {
			psy_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void AddString(InstrumentsBox* self, const char* text)
{
	psy_ui_listbox_addtext(&self->instrumentlist, text);
}

void OnInstrumentGroupListChanged(InstrumentsBox* self, psy_ui_Component* sender, int slot)
{
	instruments_changeslot(self->instruments, instrumentindex_make(slot, 0));
}

void OnInstrumentListChanged(InstrumentsBox* self, psy_ui_Component* sender, int slot)
{
	int groupslot;

	psy_signal_disconnect(&self->instruments->signal_slotchange, self,
		OnInstrumentSlotChanged);
	groupslot = psy_ui_listbox_cursel(&self->grouplist);
	if (groupslot == -1) {
		groupslot = 0;
	}
	instruments_changeslot(self->instruments,
		instrumentindex_make(groupslot, slot));
	psy_signal_connect(&self->instruments->signal_slotchange, self,
		OnInstrumentSlotChanged);
}

void OnInstrumentInsert(InstrumentsBox* self, psy_ui_Component* sender,
	const psy_audio_InstrumentIndex* slot)
{
	BuildInstrumentList(self);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot->slot);		
}

void OnInstrumentRemoved(InstrumentsBox* self, psy_ui_Component* sender,
	const psy_audio_InstrumentIndex* slot)
{
	BuildInstrumentList(self);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot->slot);		
}

void OnInstrumentSlotChanged(InstrumentsBox* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* slot)
{	
	BuildInstrumentList(self);	
	psy_ui_listbox_setcursel(&self->grouplist, slot->slot);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot->subslot);	
}

void instrumentsbox_setinstruments(InstrumentsBox* self, psy_audio_Instruments* instruments)
{
	self->instruments = instruments;
	samplesbox_buildinstrumentgroup(self);
	BuildInstrumentList(self);
	psy_ui_listbox_setcursel(&self->grouplist, 0);
	psy_ui_listbox_setcursel(&self->instrumentlist, 0);	
	psy_signal_connect(&instruments->signal_insert, self,
		OnInstrumentInsert);
	psy_signal_connect(&instruments->signal_removed, self,
		OnInstrumentRemoved);
	psy_signal_connect(&instruments->signal_slotchange, self,
		OnInstrumentSlotChanged);
}

int instrumentsbox_selected(InstrumentsBox* self)
{
	return psy_ui_listbox_cursel(&self->instrumentlist);	
}
