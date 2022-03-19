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
static void seqviewduration_ondestroy(SeqviewDuration* self,
	psy_ui_Component* sender);	

/* implementation */
void seqviewduration_init(SeqviewDuration* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
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
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->duration, &self->component);	
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_LEFT);	
	psy_ui_label_setcharnumber(&self->duration, 18.0);
	psy_ui_label_preventtranslation(&self->duration);
	psy_ui_component_setstyletype(psy_ui_label_base(&self->duration),
		STYLE_DURATION_TIME);		
	psy_signal_connect(&self->component.signal_destroy, self,
		seqviewduration_ondestroy);	
	seqviewduration_update(self, FALSE);
}

void seqviewduration_ondestroy(SeqviewDuration* self, psy_ui_Component* sender)
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
			psy_ui_label_settext(&self->duration, text);
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
				psy_ui_label_settext(&self->duration, text);
				self->calcduration = FALSE;
				break;
			}
		}
	}
}
