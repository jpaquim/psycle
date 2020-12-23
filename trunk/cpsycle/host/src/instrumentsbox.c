// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentsbox.h"
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"

#define FT2_INSTRUMENTNAME_LEN 22

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
static const char* instrumentsbox_groupname(InstrumentsBox*,
	uintptr_t groupslot);

void instrumentsbox_init(InstrumentsBox* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	// Groups
	psy_ui_component_init(&self->groupheader, &self->component);
	psy_ui_component_setalign(&self->groupheader, psy_ui_ALIGN_TOP);	
	psy_ui_label_init_text(&self->header, &self->component,
		"instrumentsbox.instrument-groups");
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);	
	psy_ui_listbox_init(&self->grouplist, &self->component);
	psy_ui_component_setmaximumsize(&self->grouplist.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(10)));
	psy_ui_component_setminimumsize(&self->grouplist.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(10)));
	psy_ui_component_setalign(&self->grouplist.component, psy_ui_ALIGN_TOP);
	// Instruments
	psy_ui_component_init(&self->instheader, &self->component);
	psy_ui_component_setalign(&self->instheader, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->group, &self->component,
		"instrumentsbox.group-instruments");
	psy_ui_component_setalign(&self->group.component, psy_ui_ALIGN_TOP);
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
		for (; slot < 256; ++slot) {
			if (psy_audio_instruments_group_at(self->instruments, slot)) {
				psy_snprintf(text, 20, "%02X*:%s", slot,
					instrumentsbox_groupname(self, slot));
			} else {
				psy_snprintf(text, 20, "%02X:%s", slot,
					instrumentsbox_groupname(self, slot));
			}
			psy_ui_listbox_addtext(&self->grouplist, text);
		}
	}
}

void instrumentsbox_buildlist(InstrumentsBox* self)
{
	psy_audio_Instrument* instrument;
	int groupslot;
	int slot = 0;
	char buffer[FT2_INSTRUMENTNAME_LEN + 4];

	psy_ui_listbox_clear(&self->instrumentlist);
	groupslot = psy_ui_listbox_cursel(&self->grouplist);
	if (groupslot == -1) {
		groupslot = 0;
	}
	for ( ; slot < 256; ++slot) {		
		if (instrument = psy_audio_instruments_at(self->instruments,
			psy_audio_instrumentindex_make(groupslot, slot))) {			
			psy_snprintf(buffer, sizeof(buffer), "%02X*:%s", slot,
				psy_audio_instrument_name(instrument));
		} else {
			psy_snprintf(buffer, sizeof(buffer), "%02X:%s", slot, "");
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
	psy_audio_InstrumentIndex index;

	index = instrumentsbox_selected(self);
	instrumentsbox_buildlist(self);
	psy_ui_listbox_setcursel(&self->grouplist, index.groupslot);
	psy_ui_listbox_setcursel(&self->instrumentlist, index.subslot);
	psy_audio_instruments_select(self->instruments, index);
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
	psy_audio_instruments_select(self->instruments,
		psy_audio_instrumentindex_make(groupslot, slot));
	psy_signal_connect(&self->instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
}

void instrumentsbox_oninstrumentinsert(InstrumentsBox* self, psy_ui_Component*
	sender, const psy_audio_InstrumentIndex* slot)
{	
	psy_audio_Instrument* instrument;
	char buffer[FT2_INSTRUMENTNAME_LEN + 4];

	assert(slot);
	assert(!psy_audio_instrumentindex_invalid(slot));
	
	if (instrument = psy_audio_instruments_at(self->instruments, *slot)) {
		psy_snprintf(buffer, sizeof(buffer), "%02X*:%s", (int)slot->subslot,
			psy_audio_instrument_name(instrument));
	} else {
		psy_snprintf(buffer, sizeof(buffer), "%02X:%s", (int)slot->subslot, "");
	}
	psy_ui_listbox_settext(&self->instrumentlist, buffer, slot->subslot);

	if (psy_audio_instruments_group_at(self->instruments, slot->groupslot)) {
		psy_snprintf(buffer, 20, "%02X*:%s", slot->groupslot,
			instrumentsbox_groupname(self, slot->groupslot));
	} else {
		psy_snprintf(buffer, 20, "%02X:%s", slot->groupslot,
			instrumentsbox_groupname(self, slot->groupslot));
	}
	psy_ui_listbox_settext(&self->grouplist, buffer, slot->groupslot);
}

void instrumentsbox_oninstrumentremoved(InstrumentsBox* self, psy_ui_Component*
	sender, const psy_audio_InstrumentIndex* slot)
{	
	char buffer[FT2_INSTRUMENTNAME_LEN + 4];	

	assert(slot);
	assert(!psy_audio_instrumentindex_invalid(slot));	
	
	psy_snprintf(buffer, sizeof(buffer), "%02X:%s", (int)slot->subslot, "");	
	psy_ui_listbox_settext(&self->instrumentlist, buffer, slot->subslot);

	if (psy_audio_instruments_group_at(self->instruments, slot->groupslot)) {		
		psy_snprintf(buffer, 20, "%02X*:%s", slot->groupslot,
			instrumentsbox_groupname(self, slot->groupslot));
	} else {
		psy_snprintf(buffer, 20, "%02X:%s", slot->groupslot,
			instrumentsbox_groupname(self, slot->groupslot));
	}
	psy_ui_listbox_settext(&self->grouplist, buffer, slot->groupslot);
}

const char* instrumentsbox_groupname(InstrumentsBox* self, uintptr_t groupslot)
{
	if (groupslot == 0) {
		return "Sampler";
	} else if (groupslot == 1) {
		return "Sampulse";
	}
	return "";	
}

void instrumentsbox_oninstrumentslotchanged(InstrumentsBox* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot)
{	
	psy_audio_InstrumentIndex index;
	
	assert(slot);
	assert(!psy_audio_instrumentindex_invalid(slot));

	index = instrumentsbox_selected(self);

	if (slot->groupslot != index.groupslot && slot->subslot != index.subslot) {
		instrumentsbox_buildlist(self);
		psy_ui_listbox_setcursel(&self->grouplist, slot->groupslot);
		psy_ui_listbox_setcursel(&self->instrumentlist, slot->subslot);
	}
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

psy_audio_InstrumentIndex instrumentsbox_selected(InstrumentsBox* self)
{
	return psy_audio_instrumentindex_make(
		psy_ui_listbox_cursel(&self->grouplist),
		psy_ui_listbox_cursel(&self->instrumentlist));
}

void instrumentsbox_rebuild(InstrumentsBox* self)
{
	instrumentsbox_buildgroup(self);
	instrumentsbox_buildlist(self);
	if (self->instruments) {
		psy_ui_listbox_setcursel(&self->grouplist,
			psy_audio_instruments_selected(self->instruments).groupslot);
		psy_ui_listbox_setcursel(&self->instrumentlist,
			psy_audio_instruments_selected(self->instruments).subslot);
	}
}
