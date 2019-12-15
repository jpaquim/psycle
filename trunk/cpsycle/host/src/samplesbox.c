// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplesbox.h"
#include <stdio.h>
#include <portable.h>

static void samplesbox_ondestroy(SamplesBox*);
static void samplesbox_buildsamplelist(SamplesBox*);
static void samplesbox_buildsubsamplelist(SamplesBox*, uintptr_t slot);
static void samplesbox_oninstrumentinsert(SamplesBox*, ui_component* sender, int slot);
static void samplesbox_oninstrumentremoved(SamplesBox*, ui_component* sender, int slot);
static void samplesbox_onsamplelistchanged(SamplesBox*, ui_component* sender,
	int slot);
static void samplesbox_onsubsamplelistchanged(SamplesBox*, ui_component* sender,
	int slot);

void samplesbox_init(SamplesBox* self, ui_component* parent,
	Samples* samples, Instruments* instruments)
{	
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makeeh(1), ui_value_makepx(0));	
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	psy_signal_init(&self->signal_changed);
	psy_signal_connect(&self->component.signal_destroy, self,
		samplesbox_ondestroy);	
	ui_label_init(&self->header, &self->component);
	ui_component_setalign(&self->header.component, UI_ALIGN_TOP);
	ui_label_settext(&self->header, "Groups first sample");
	ui_component_setmargin(&self->header.component, &margin);
	ui_listbox_init(&self->subsamplelist, &self->component);
	ui_component_setalign(&self->subsamplelist.component, UI_ALIGN_BOTTOM);
	ui_component_resize(&self->subsamplelist.component, 0, 100);
	ui_label_init(&self->group, &self->component);
	ui_component_setalign(&self->group.component, UI_ALIGN_BOTTOM);
	ui_label_settext(&self->group, "Group samples");
	ui_component_setmargin(&self->group.component, &margin);
	ui_listbox_init(&self->samplelist, &self->component);	
	ui_component_setalign(&self->samplelist.component, UI_ALIGN_CLIENT);
	ui_component_setmargin(&self->samplelist.component, &margin);
	samplesbox_setsamples(self, samples, instruments);	
	psy_signal_connect(&self->samplelist.signal_selchanged, self,
		samplesbox_onsamplelistchanged);
	psy_signal_connect(&self->subsamplelist.signal_selchanged, self,
		samplesbox_onsubsamplelistchanged);
}

void samplesbox_ondestroy(SamplesBox* self)
{
	psy_signal_dispose(&self->signal_changed);
}

void samplesbox_buildsamplelist(SamplesBox* self)
{				
	uintptr_t slot = 0;
	char text[40];
	
	ui_listbox_clear(&self->samplelist);	
	for ( ; slot < 256; ++slot) {
		Sample* sample;

		sample = self->samples ? samples_at(self->samples,
			sampleindex_make(slot, 0)) : 0;
		if (sample) {
			psy_snprintf(text, 20, "%02X:%s", slot,
				sample_name(sample));
		} else {
			psy_snprintf(text, 20, "%02X:%s", slot, "");
		}
		ui_listbox_addstring(&self->samplelist, text);
	}	
}

void samplesbox_buildsubsamplelist(SamplesBox* self, uintptr_t slot)
{	
	uintptr_t subslot = 0;
	char text[40];
	
	ui_listbox_clear(&self->subsamplelist);
	for ( ; subslot < 256; ++subslot) {
		Sample* sample;		

		sample = self->samples
			?  samples_at(self->samples, sampleindex_make(slot, subslot))
			: 0;
		if (sample != 0) {
			psy_snprintf(text, 20, "%02X:%s", subslot,
				sample_name(sample));
		} else {
			psy_snprintf(text, 20, "%02X:%s", subslot, "");
		}
		ui_listbox_addstring(&self->subsamplelist, text);
	}	
}

void samplesbox_onsamplelistchanged(SamplesBox* self, ui_component* sender,
	int slot)
{
	samplesbox_buildsubsamplelist(self, slot);
	ui_listbox_setcursel(&self->subsamplelist, 0);
	if (self->instruments) {
		instruments_changeslot(self->instruments, slot);
	}	
	psy_signal_emit(&self->signal_changed, self, 0);
}

void samplesbox_onsubsamplelistchanged(SamplesBox* self, ui_component* sender,
	int slot)
{	
	psy_signal_emit(&self->signal_changed, self, 0);	
}

void samplesbox_oninstrumentinsert(SamplesBox* self, ui_component* sender,
	int slot)
{
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, slot);
	ui_listbox_setcursel(&self->samplelist, slot);		
}

void samplesbox_oninstrumentremoved(SamplesBox* self, ui_component* sender,
	int slot)
{
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, slot);
	ui_listbox_setcursel(&self->samplelist, slot);
}

void samplesbox_oninstrumentsslotchanged(SamplesBox* self, Instrument* sender,
	int slot)
{
	ui_listbox_setcursel(&self->samplelist, slot);
	if ((uintptr_t)slot != samplesbox_selected(self).slot) {
		samplesbox_buildsubsamplelist(self, slot);
	}		
}

void samplesbox_setsamples(SamplesBox* self, Samples* samples,
	Instruments* instruments)
{
	self->samples = samples;
	self->instruments = instruments;
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, 0);
	if (self->instruments) {
		psy_signal_connect(&instruments->signal_insert, self,
			samplesbox_oninstrumentinsert);
		psy_signal_connect(&instruments->signal_removed, self,
			samplesbox_oninstrumentremoved);
		psy_signal_connect(&instruments->signal_slotchange, self,
			samplesbox_oninstrumentsslotchanged);
	}
}

SampleIndex samplesbox_selected(SamplesBox* self)
{
	SampleIndex rv;

	rv.slot = ui_listbox_cursel(&self->samplelist);
	rv.subslot = ui_listbox_cursel(&self->subsamplelist);
	return rv;	
}
