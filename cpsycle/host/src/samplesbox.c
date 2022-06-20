/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "samplesbox.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void samplesbox_on_destroyed(SamplesBox*);
static void samplesbox_buildsamplelist(SamplesBox*);
static void samplesbox_buildsubsamplelist(SamplesBox*, uintptr_t slot, bool create);
static void samplesbox_onsampleinsert(SamplesBox*, psy_ui_Component* sender,
	psy_audio_SampleIndex*);
static void samplesbox_onsampleremoved(SamplesBox*, psy_ui_Component* sender,
	psy_audio_SampleIndex*);
static void samplesbox_onsamplelistchanged(SamplesBox*, psy_ui_Component* sender,
	int slot);
static void samplesbox_onsubsamplelistchanged(SamplesBox*, psy_ui_Component* sender,
	int slot);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(SamplesBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			samplesbox_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

void samplesbox_init(SamplesBox* self, psy_ui_Component* parent,
	psy_audio_Samples* samples, Workspace* workspace)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init_em(&margin, 0.0, 0.0, 1.0, 0.0);	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_label_set_text(&self->header, "samplesview.groupsfirstsample");
	psy_ui_label_set_char_number(&self->header, 25);
	psy_ui_component_set_margin(&self->header.component, margin);		
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);	
	psy_ui_listbox_init(&self->samplelist, &self->component);	
	psy_ui_component_set_align(&self->samplelist.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_listbox_init(&self->subsamplelist, &self->component);
	psy_ui_component_setmaximumsize(&self->subsamplelist.component,
		psy_ui_size_make_em(0.0, 10.0));
	psy_ui_component_set_minimum_size(&self->subsamplelist.component,
		psy_ui_size_make_em(0.0, 10.0));
	psy_ui_component_set_align(&self->subsamplelist.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_init_text(&self->group, &self->component,
		"samplesview.groupsamples");
	psy_ui_component_set_align(&self->group.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->group.component, margin);	
	psy_signal_init(&self->signal_changed);
	psy_ui_component_set_margin(&self->samplelist.component, margin);
	samplesbox_setsamples(self, samples);	
	psy_signal_connect(&self->samplelist.signal_selchanged, self,
		samplesbox_onsamplelistchanged);
	psy_signal_connect(&self->subsamplelist.signal_selchanged, self,
		samplesbox_onsubsamplelistchanged);	
}

void samplesbox_on_destroyed(SamplesBox* self)
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
			psy_audio_sampleindex_make(slot, 0)) : 0;
		if (sample) {
			psy_snprintf(text, 20, "%02X*:%s", slot,
				psy_audio_sample_name(sample));
		} else {
			psy_snprintf(text, 20, "%02X:%s", slot, "");
		}
		psy_ui_listbox_add_text(&self->samplelist, text);
	}	
}

void samplesbox_buildsubsamplelist(SamplesBox* self, uintptr_t slot, bool create)
{	
	uintptr_t subslot = 0;
	char text[40];

	if (create) {
		psy_ui_listbox_clear(&self->subsamplelist);
	}
	for ( ;subslot < 256; ++subslot) {
		psy_audio_Sample* sample;		

		sample = self->samples
			? psy_audio_samples_at(self->samples,
				psy_audio_sampleindex_make(slot, subslot))
			: 0;
		if (sample != 0) {
			psy_snprintf(text, 20, "%02X:%s", subslot,
				psy_audio_sample_name(sample));
		} else {
			psy_snprintf(text, 20, "%02X:%s", subslot, "");
		}
		if (create) {
			psy_ui_listbox_add_text(&self->subsamplelist, text);
		} else {
			psy_ui_listbox_settext(&self->subsamplelist, text, subslot);
		}
	}	
}

void samplesbox_onsamplelistchanged(SamplesBox* self, psy_ui_Component* sender,
	int slot)
{
	samplesbox_buildsubsamplelist(self, slot, FALSE);
	psy_ui_listbox_setcursel(&self->subsamplelist, 0);	
	psy_signal_emit(&self->signal_changed, self, 0);
}

void samplesbox_onsubsamplelistchanged(SamplesBox* self, psy_ui_Component* sender,
	int slot)
{	
	psy_signal_emit(&self->signal_changed, self, 0);	
}

void samplesbox_onsampleinsert(SamplesBox* self, psy_ui_Component* sender,
	psy_audio_SampleIndex* index)
{
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, index->slot, FALSE);
}

void samplesbox_onsampleremoved(SamplesBox* self, psy_ui_Component* sender,
	psy_audio_SampleIndex* index)
{
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, index->slot, FALSE);
}

void samplesbox_setsamples(SamplesBox* self, psy_audio_Samples* samples)
{
	self->samples = samples;
	samplesbox_buildsamplelist(self);
	samplesbox_buildsubsamplelist(self, 0, TRUE);
	psy_ui_listbox_setcursel(&self->samplelist, 0);
	psy_ui_listbox_setcursel(&self->subsamplelist, 0);	
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
	psy_audio_SampleIndex currindex;
	
	currindex = samplesbox_selected(self);
	if (currindex.slot != index.slot) {
		psy_ui_listbox_setcursel(&self->samplelist, index.slot);
	}
	if (currindex.subslot != index.subslot) {
		psy_ui_listbox_setcursel(&self->subsamplelist, index.subslot);
	}
}
