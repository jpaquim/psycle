// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentsbox.h"
#include <stdio.h>
#include "../../detail/portable.h"

static void instrumentsbox_buildlist(InstrumentsBox*);
static void instrumentsbox_buildgroup(InstrumentsBox*);
static void instrumentsbox_addstring(InstrumentsBox*, const char* text);
static void instrumentsbox_oninstrumentslotchanged(InstrumentsBox*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentsbox_oninstrumentinsert(InstrumentsBox*,
	psy_ui_Component* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentsbox_oninstrumentremoved(InstrumentsBox*,
	psy_ui_Component* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentsbox_ongrouplistchanged(InstrumentsBox*,
	psy_ui_Component* sender, int slot);
static void instrumentsbox_onlistchanged(InstrumentsBox*,
	psy_ui_Component* sender, int slot);

void instrumentsbox_init(InstrumentsBox* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments)
{		
	psy_ui_component_init(&self->component, parent);	
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_label_settext(&self->header, "Instrument Group");
	psy_ui_listbox_init(&self->grouplist, &self->component);
	psy_ui_component_setmaximumsize(&self->grouplist.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(10)));
	psy_ui_component_setminimumsize(&self->grouplist.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(10)));
	psy_ui_component_setalign(&self->grouplist.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->group, &self->component);
	psy_ui_component_setalign(&self->group.component, psy_ui_ALIGN_TOP);
	psy_ui_label_settext(&self->group, "Group Instruments");
	psy_ui_listbox_init(&self->instrumentlist, &self->component);
	psy_ui_component_setalign(&self->instrumentlist.component,
		psy_ui_ALIGN_CLIENT);
	instrumentsbox_setinstruments(self, instruments);
	psy_signal_connect(&self->grouplist.signal_selchanged, self,
		instrumentsbox_ongrouplistchanged);
	psy_signal_connect(&self->instrumentlist.signal_selchanged, self,
		instrumentsbox_onlistchanged);
}

void instrumentsbox_buildgroup(InstrumentsBox* self)
{
	if (self->instruments) {
		uintptr_t slot = 0;
		char text[40];

		psy_ui_listbox_clear(&self->grouplist);
		for (; slot < max(psy_audio_instruments_groupsize(self->instruments),
				1); ++slot) {
			psy_snprintf(text, 20, "%02X:%s", slot, "");
			psy_ui_listbox_addtext(&self->grouplist, text);
		}
	}
}

void instrumentsbox_buildlist(InstrumentsBox* self)
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
		instrumentsbox_addstring(self, buffer);
	}
}

void instrumentsbox_addstring(InstrumentsBox* self, const char* text)
{
	psy_ui_listbox_addtext(&self->instrumentlist, text);
}

void instrumentsbox_ongrouplistchanged(InstrumentsBox* self, psy_ui_Component*
	sender, int slot)
{
	instruments_changeslot(self->instruments, instrumentindex_make(slot, 0));
}

void instrumentsbox_onlistchanged(InstrumentsBox* self, psy_ui_Component*
	sender, int slot)
{
	int groupslot;

	psy_signal_disconnect(&self->instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
	groupslot = psy_ui_listbox_cursel(&self->grouplist);
	if (groupslot == -1) {
		groupslot = 0;
	}
	instruments_changeslot(self->instruments,
		instrumentindex_make(groupslot, slot));
	psy_signal_connect(&self->instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
}

void instrumentsbox_oninstrumentinsert(InstrumentsBox* self, psy_ui_Component*
	sender, const psy_audio_InstrumentIndex* slot)
{
	instrumentsbox_buildlist(self);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot->slot);		
}

void instrumentsbox_oninstrumentremoved(InstrumentsBox* self, psy_ui_Component*
	sender, const psy_audio_InstrumentIndex* slot)
{
	instrumentsbox_buildlist(self);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot->slot);		
}

void instrumentsbox_oninstrumentslotchanged(InstrumentsBox* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot)
{	
	instrumentsbox_buildlist(self);
	psy_ui_listbox_setcursel(&self->grouplist, slot->slot);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot->subslot);	
}

void instrumentsbox_setinstruments(InstrumentsBox* self, psy_audio_Instruments*
	instruments)
{
	self->instruments = instruments;
	instrumentsbox_buildgroup(self);
	instrumentsbox_buildlist(self);
	psy_ui_listbox_setcursel(&self->grouplist, 0);
	psy_ui_listbox_setcursel(&self->instrumentlist, 0);	
	psy_signal_connect(&instruments->signal_insert, self,
		instrumentsbox_oninstrumentinsert);
	psy_signal_connect(&instruments->signal_removed, self,
		instrumentsbox_oninstrumentremoved);
	psy_signal_connect(&instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
}

int instrumentsbox_selected(InstrumentsBox* self)
{
	return psy_ui_listbox_cursel(&self->instrumentlist);	
}

void instrumentsbox_rebuild(InstrumentsBox* self)
{
	instrumentsbox_buildgroup(self);
	instrumentsbox_buildlist(self);
	if (self->instruments) {
		psy_ui_listbox_setcursel(&self->grouplist,
			self->instruments->slot.slot);
		psy_ui_listbox_setcursel(&self->instrumentlist,
			self->instruments->slot.subslot);
	}
}
