/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternhostcmds.h"
/* host*/
#include "patterncmds.h"
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* audio */
#include <exclusivelock.h>
#include <patternio.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static char patternfilter[] = "Pattern (*.psb)" "|*.psb";

/* implementation */
void patterncmds_init(PatternCmds* self, psy_audio_Sequence* sequence,
	psy_audio_Player* player, psy_UndoRedo* undoredo,
	psy_audio_Pattern* patternpaste, DirConfig* dirconfig)
{
	assert(self);
	assert(undoredo);	
	
	self->pattern = NULL;
	self->sequence = sequence;
	self->player = player;
	self->undoredo = undoredo;
	self->patternpaste = patternpaste;
	self->dirconfig = dirconfig;
}

void patterncmds_setsequence(PatternCmds* self, psy_audio_Sequence* sequence)
{
	self->sequence = sequence;
}

void patterncmds_setpattern(PatternCmds* self, psy_audio_Pattern* pattern)
{	
	self->pattern = pattern;
}

void patterncmds_blocktranspose(PatternCmds* self,
	psy_audio_BlockSelection selection,
	psy_audio_SequenceCursor cursor, intptr_t offset)
{	
	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(self->undoredo,
			&blocktransposecommand_alloc(self->pattern,
				selection, cursor, offset, self->sequence)->command);
	}
}

void patterncmds_blockdelete(PatternCmds* self, psy_audio_BlockSelection
	selection)
{
	assert(self);

	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(self->undoredo,
			&blockremovecommand_alloc(self->pattern,
				selection, self->sequence)->command);
	}
}

void patterncmds_blockcopy(PatternCmds* self, psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_audio_pattern_blockcopy(self->patternpaste,
			self->pattern, selection);
	}
}


void patterncmds_blockpaste(PatternCmds* self, psy_audio_SequenceCursor cursor,
	bool mix)
{
	assert(self);

	if (self->pattern && !psy_audio_pattern_empty(
			self->patternpaste)) {
		psy_undoredo_execute(self->undoredo,
			&blockpastecommand_alloc(self->pattern,
				self->patternpaste, cursor,
				1.0 / (double)cursor.lpb, mix)->command);		
	}
}

void patterncmds_changeinstrument(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

/*	if (self->pattern && self->sequence) {
		psy_audio_pattern_changeinstrument(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_instruments_selected(&self->song->instruments).subslot);
	}*/
}

void patterncmds_changemachine(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

/*	if (self->pattern && self->song) {
		psy_audio_pattern_changemachine(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_machines_selected(&self->song->machines));
	}*/
}

void patterncmds_importpattern(PatternCmds* self, psy_dsp_big_beat_t bpl)
{
	if (self->pattern) {
		psy_ui_OpenDialog dialog;

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", patternfilter,
			"PSB", dirconfig_songs(self->dirconfig));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(self->pattern,
				psy_ui_opendialog_path(&dialog), bpl);
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void patterncmds_exportpattern(PatternCmds* self, psy_dsp_big_beat_t bpl,
	uintptr_t numtracks)
{
	if (self->pattern) {
		psy_ui_SaveDialog dialog;

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", patternfilter,
			"PSB", dirconfig_songs(self->dirconfig));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(self->pattern,
				psy_ui_savedialog_path(&dialog), bpl,
				numtracks);				
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}
