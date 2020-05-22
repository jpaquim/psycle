// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplesbox.h"
#include <stdio.h>
#include "../../detail/portable.h"

static void samplesbox_ondestroy(SamplesBox*);
static void samplesbox_buildsamplelist(SamplesBox*);
static void samplesbox_buildsubsamplelist(SamplesBox*, uintptr_t slot);
static void samplesbox_oninstrumentinsert(SamplesBox*, psy_ui_Component* sender, int slot);
static void samplesbox_oninstrumentremoved(SamplesBox*, psy_ui_Component* sender, int slot);
static void samplesbox_onsampleinsert(SamplesBox*, psy_ui_Component* sender, psy_audio_SampleIndex*);
static void samplesbox_onsampleremoved(SamplesBox*, psy_ui_Component* sender, psy_audio_SampleIndex*);
static void samplesbox_onsamplelistchanged(SamplesBox*, psy_ui_Component* sender,
	int slot);
static void samplesbox_onsubsamplelistchanged(SamplesBox*, psy_ui_Component* sender,
	int slot);

void samplesbox_init(SamplesBox* self, psy_ui_Component* parent,
	psy_audio_Samples* samples, psy_audio_Instruments* instruments)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeeh(1), psy_ui_value_makepx(0));
	self->changeinstrumentslot = 1;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);
	psy_signal_init(&self->signal_changed);
	psy_signal_connect(&self->component.signal_destroy, self,
		samplesbox_ondestroy);	
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_label_settext(&self->header, "Groups first sample");
	psy_ui_component_setmargin(&self->header.component, &margin);
	psy_ui_listbox_init(&self->subsamplelist, &self->component);
	psy_ui_component_setalign(&self->subsamplelist.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_resize(&self->subsamplelist.component, 0, 100);
	psy_ui_label_init(&self->group, &self->component);
	psy_ui_component_setalign(&self->group.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_settext(&self->group, "Group samples");
	psy_ui_component_setmargin(&self->group.component, &margin);
	psy_ui_listbox_init(&self->samplelist, &self->component);	
	psy_ui_component_setalign(&self->samplelist.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->samplelist.component, &margin);
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
	
	psy_ui_listbox_clear(&self->samplelist);	
	for ( ; slot < 256; ++slot) {
		psy_audio_Sample* sample;

		sample = self->samples ? psy_audio_samples_at(self->samples,
			sampleindex_make(slot, 0)) : 0;
		if (sample) {
			psy_snprintf(text, 20, "%02X:%s", slot,
				sample_name(sample));
		} else {
			psy_snprintf(text, 20, "%02X:%s", slot, "");
		}
		psy_ui_listbox_addtext(&self->samplelist, text);
	}	
}

void samplesbox_buildsubsamplelist(SamplesBox* self, uintptr_t slot)
{	
	uintptr_t subslot = 0;
	char text[40];
	
	psy_ui_listbox_clear(&self->subsamplelist);
	for ( ; subslot < 256; ++subslot) {
		psy_audio_Sample* sample;		

		sample = self->samples
			? psy_audio_samples_at(self->samples,
				sampleindex_make(slot, subslot))
			: 0;
		if (sample != 0) {
			psy_snprintf(text, 20, "%02X:%s", subslot,
				sample_name(sample));
		} else {
			psy_snprintf(text, 20, "%02X:%s", subslot, "");
		}
		psy_ui_listbox_addtext(&self->subsamplelist, text);
	}	
}

void samplesbox_onsamplelistchanged(SamplesBox* self, psy_ui_Component* sender,
	int slot)
{
	samplesbox_buildsubsamplelist(self, slot);
	psy_ui_listbox_setcursel(&self->subsamplelist, 0);
	if (self->instruments && self->changeinstrumentslot) {
		instruments_changeslot(self->instruments, slot);
	}	
	psy_signal_emit(&self->signal_changed, self, 0);
}

void samplesbox_onsubsamplelistchanged(SamplesBox* self, psy_ui_Component* sender,
	int slot)
{	
	psy_signal_emit(&self->signal_changed, self, 0);	
}

void samplesbox_oninstrumentinsert(SamplesBox* self, psy_ui_Component* sender,
	int slot)
{
	if (self->changeinstrumentslot) {
		samplesbox_buildsamplelist(self);
		samplesbox_buildsubsamplelist(self, slot);
		psy_ui_listbox_setcursel(&self->samplelist, slot);
	}
}

void samplesbox_onsampleinsert(SamplesBox* self, psy_ui_Component* sender,
	psy_audio_SampleIndex* index)
{
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, index->subslot);
}

void samplesbox_onsampleremoved(SamplesBox* self, psy_ui_Component* sender,
	psy_audio_SampleIndex* index)
{
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, index->subslot);
}

void samplesbox_oninstrumentremoved(SamplesBox* self, psy_ui_Component* sender,
	int slot)
{
	if (self->changeinstrumentslot) {
		samplesbox_buildsamplelist(self);
		samplesbox_buildsubsamplelist(self, slot);	
		psy_ui_listbox_setcursel(&self->samplelist, slot);
	}
}

void samplesbox_oninstrumentsslotchanged(SamplesBox* self, psy_audio_Instrument* sender,
	int slot)
{
	if (self->changeinstrumentslot) {
		psy_ui_listbox_setcursel(&self->samplelist, slot);
		if ((uintptr_t)slot != samplesbox_selected(self).slot) {
			samplesbox_buildsubsamplelist(self, slot);
		}
	}
}

void samplesbox_setsamples(SamplesBox* self, psy_audio_Samples* samples,
	psy_audio_Instruments* instruments)
{
	self->samples = samples;
	self->instruments = instruments;
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, 0);
	psy_ui_listbox_setcursel(&self->samplelist, 0);
	psy_ui_listbox_setcursel(&self->subsamplelist, 0);
	if (self->instruments) {
		psy_signal_connect(&instruments->signal_insert, self,
			samplesbox_oninstrumentinsert);
		psy_signal_connect(&instruments->signal_removed, self,
			samplesbox_oninstrumentremoved);
		psy_signal_connect(&instruments->signal_slotchange, self,
			samplesbox_oninstrumentsslotchanged);
	}
	if (self->samples) {
		psy_signal_connect(&samples->signal_insert, self,
			samplesbox_onsampleinsert);	
		psy_signal_connect(&samples->signal_removed, self,
			samplesbox_onsampleremoved);	
	}
}

psy_audio_SampleIndex samplesbox_selected(SamplesBox* self)
{
	psy_audio_SampleIndex rv;

	rv.slot = psy_ui_listbox_cursel(&self->samplelist);
	rv.subslot = psy_ui_listbox_cursel(&self->subsamplelist);
	return rv;	
}

void samplesbox_select(SamplesBox* self, psy_audio_SampleIndex index)
{
	psy_ui_listbox_setcursel(&self->samplelist, index.slot);
	psy_ui_listbox_setcursel(&self->subsamplelist, index.subslot);
}
