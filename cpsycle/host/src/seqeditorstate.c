/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditorstate.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

void seqeditstate_init(SeqEditState* self, SequenceCmds* cmds,
	psy_ui_Component* view)
{	
	psy_signal_init(&self->signal_cursorchanged);
	psy_signal_init(&self->signal_itemselected);
	psy_signal_init(&self->signal_timesigchanged);
	psy_signal_init(&self->signal_loopchanged);
	self->workspace = cmds->workspace;
	self->cmds = cmds;	
	self->view = view;
	self->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;	
	self->defaultpxperbeat = 12;
	self->pxperbeat = self->defaultpxperbeat;	
	self->defaultlineheight = psy_ui_value_make_eh(2.0);	
	self->line_height = self->defaultlineheight;	
	self->cursorposition = 0.0;
	self->cursoractive = FALSE;
	self->drawcursor = TRUE;
	self->drawpatternevents = TRUE;		
	self->dragposition = 0.0;	
	self->dragseqpos = psy_audio_orderindex_make_invalid();
	self->dragtype = SEQEDIT_DRAGTYPE_MOVE;
	self->dragstatus = SEQEDIT_DRAG_NONE;
	self->updatecursorposition = TRUE;
	self->seqentry = NULL;
	self->cmd = SEQEDTCMD_NONE;
	self->propertiesview_desc_column_width = 14.0;
	self->showpatternnames = FALSE;	
}

void seqeditstate_dispose(SeqEditState* self)
{
	psy_signal_dispose(&self->signal_cursorchanged);
	psy_signal_dispose(&self->signal_itemselected);
	psy_signal_dispose(&self->signal_timesigchanged);
	psy_signal_dispose(&self->signal_loopchanged);
}

psy_audio_Sequence* seqeditstate_sequence(SeqEditState* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->sequence;
	}
	return NULL;
}

psy_audio_Patterns* seqeditstate_patterns(SeqEditState* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->patterns;
	}
	return NULL;
}

const psy_audio_Sequence* seqeditstate_sequence_const(const SeqEditState* self)
{
	if (workspace_song_const(self->workspace)) {
		return &workspace_song_const(self->workspace)->sequence;
	}
	return NULL;
}

psy_dsp_big_beat_t seqeditstate_quantize(const SeqEditState* self,
	psy_dsp_big_beat_t position)
{
	uintptr_t lpb;

	lpb = psy_audio_player_lpb(workspace_player(self->workspace));
	return floor(position * lpb) / (psy_dsp_big_beat_t)lpb;
}

void seqeditstate_outputstatusposition(SeqEditState* self)
{
	char text[256];

	psy_snprintf(text, 256, "Sequence Position %.3fb",
		(float)self->cursorposition);
	workspace_output_status(self->workspace, text);
}

psy_audio_PatternNode* seqeditstate_node(SeqEditState* self,
	psy_ui_RealPoint pt, psy_dsp_big_beat_t d1, psy_dsp_big_beat_t d2,
	psy_audio_PatternNode** prev)
{
	psy_audio_Pattern* pattern;

	pattern = seqeditstate_globalpattern(self);
	if (pattern) {		
		psy_dsp_big_beat_t offset;

		offset = seqeditstate_quantize(self,
			seqeditstate_pxtobeat(self, pt.x));
		return psy_audio_pattern_find_node(pattern, 0, offset + d1, -d1 + d2,
			prev);
	}
	return NULL;
}

psy_audio_Pattern* seqeditstate_globalpattern(SeqEditState* self)
{
	psy_audio_Sequence* sequence;

	assert(self);

	sequence = seqeditstate_sequence(self);
	if (sequence && sequence->patterns) {
		return psy_audio_patterns_at(sequence->patterns,
			psy_audio_GLOBALPATTERN);
	}
	return NULL;
}
