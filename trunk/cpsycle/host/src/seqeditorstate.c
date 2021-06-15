/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditorstate.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 10.0

void seqeditstate_init(SeqEditState* self, SequenceCmds* cmds,
	psy_ui_Edit* edit, psy_ui_Component* view)
{	
	self->workspace = cmds->workspace;
	self->cmds = cmds;
	self->edit = edit;
	self->view = view;
	self->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
	psy_signal_init(&self->signal_cursorchanged);
	self->pxperbeat = DEFAULT_PXPERBEAT;
	self->defaultlineheight = psy_ui_value_make_eh(2.0);	
	self->lineheight = self->defaultlineheight;	
	self->cursorposition = 0.0;
	self->cursoractive = FALSE;
	self->drawcursor = TRUE;
	self->drawpatternevents = TRUE;		
	self->dragposition = 0.0;	
	self->dragseqpos = psy_audio_orderindex_zero();
	self->dragtype = SEQEDIT_DRAGTYPE_MOVE;
	self->dragstatus = SEQEDIT_DRAG_NONE;
	self->updatecursorposition = TRUE;
	self->seqentry = NULL;
	self->cmd = SEQEDTCMD_NONE;	
	self->showpatternnames = FALSE;	
}

void seqeditstate_dispose(SeqEditState* self)
{
	psy_signal_dispose(&self->signal_cursorchanged);
}

psy_audio_Sequence* seqeditstate_sequence(SeqEditState* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->sequence;
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
	workspace_outputstatus(self->workspace, text);
}