/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqviewduration.h"
/* host */
#include "trackercmds.h"
#include "styles.h"
#include "workspace.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define COLMAX 13

/* SeqviewDuration */

/* prototypes */
static void seqviewduration_on_destroyed(SeqviewDuration* self);	

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(SeqviewDuration* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			seqviewduration_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void seqviewduration_init(SeqviewDuration* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->workspace = workspace;
	self->duration_ms = 0;
	self->duration_bts = 0.0;
	self->calcduration = FALSE;
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 0.5));
	psy_ui_label_init_text(&self->desc, &self->component,
		"seqview.duration");
	psy_ui_component_set_padding(&self->desc.component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));	
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->duration, &self->component);	
	psy_ui_component_set_align(&self->duration.component, psy_ui_ALIGN_LEFT);	
	psy_ui_label_set_charnumber(&self->duration, 18.0);
	psy_ui_label_prevent_translation(&self->duration);
	psy_ui_component_set_style_type(psy_ui_label_base(&self->duration),
		STYLE_DURATION_TIME);	
	seqviewduration_update(self, FALSE);
}

void seqviewduration_on_destroyed(SeqviewDuration* self)
{		
	seqviewduration_stopdurationcalc(self);
}

void seqviewduration_stopdurationcalc(SeqviewDuration* self)
{
	if (workspace_song(self->workspace) && self->calcduration) {
		psy_audio_sequence_endcalcdurationinmsresult(
			&workspace_song(self->workspace)->sequence);
		self->calcduration = FALSE;
	}
}

void seqviewduration_update(SeqviewDuration* self, bool force)
{		
	if (workspace_song(self->workspace)) {
		psy_dsp_big_beat_t duration_bts;

		duration_bts = psy_audio_sequence_duration(
			&workspace_song(self->workspace)->sequence);
		if (self->duration_bts != duration_bts || force) {
			char text[64];

			self->duration_bts = duration_bts;
			if (self->calcduration) {
				self->duration_ms =
					psy_audio_sequence_endcalcdurationinmsresult(
						&workspace_song(self->workspace)->sequence);
				self->calcduration = FALSE;
			}
			if (!self->calcduration) {
				self->calcduration = TRUE;
				psy_audio_sequence_startcalcdurationinms(
					&workspace_song(self->workspace)->sequence);
				seqviewduration_idle(self);
			}
			psy_snprintf(text, 64, "--m--s %.2fb", (float)self->duration_bts);
			psy_ui_label_set_text(&self->duration, text);
		}
	}
}

void seqviewduration_idle(SeqviewDuration* self)
{
	if (self->calcduration && workspace_song(self->workspace)) {
		uintptr_t i;

		for (i = 0; i < 20; ++i) {
			if (!psy_audio_sequence_calcdurationinms(
					&workspace_song(self->workspace)->sequence)) {
				char text[64];

				self->duration_ms = psy_audio_sequence_endcalcdurationinmsresult(
					&workspace_song(self->workspace)->sequence);
				psy_snprintf(text, 64, " %02dm%02ds %.2fb",
					(int)(self->duration_ms / 60), ((int)self->duration_ms % 60),
					(float)self->duration_bts);
				psy_ui_label_set_text(&self->duration, text);
				self->calcduration = FALSE;
				break;
			}
		}
	}
}
