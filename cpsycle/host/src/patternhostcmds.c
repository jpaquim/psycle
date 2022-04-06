/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
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
#include <sequence.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static char patternfilter[] = "Pattern (*.psb)" "|*.psb";

/* implementation */
void patterncmds_init(PatternCmds* self,
	psy_audio_Player* player, psy_UndoRedo* undoredo,
	DirConfig* dirconfig)
{
	assert(self);
	assert(undoredo);	
	
	self->sequence = NULL;
	self->player = player;
	self->undoredo = undoredo;
	self->patternpaste = NULL;
	self->dirconfig = dirconfig;
}

void patterncmds_set_sequence(PatternCmds* self, psy_audio_Sequence* sequence)
{
	self->sequence = sequence;
}

void patterncmds_block_delete(PatternCmds* self, psy_audio_BlockSelection
	selection)
{
	assert(self);

	if (psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(self->undoredo,
			&blockremovecommand_alloc(self->sequence, selection)->command);
	}
}

void patterncmds_block_copy(PatternCmds* self, psy_audio_BlockSelection selection)
{
	assert(self);

	if (psy_audio_blockselection_valid(&selection)) {
		psy_audio_sequence_blockcopypattern(self->sequence, 
			selection, self->patternpaste);
	}	
}

void patterncmds_block_paste(PatternCmds* self, psy_audio_SequenceCursor cursor,
	bool mix)
{
	assert(self);

	if (self->sequence && !psy_audio_pattern_empty(
			self->patternpaste)) {
		psy_undoredo_execute(self->undoredo,
			&blockpastecommand_alloc(self->sequence,
				self->patternpaste, cursor,
				1.0 / (double)cursor.lpb, mix)->command);		
	}
}

void patterncmds_change_instrument(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

/*	if (self->pattern && self->sequence) {
		psy_audio_pattern_changeinstrument(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_instruments_selected(&self->song->instruments).subslot);
	}*/
}

void patterncmds_change_machine(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

/*	if (self->pattern && self->song) {
		psy_audio_pattern_changemachine(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_machines_selected(&self->song->machines));
	}*/
}

void patterncmds_import_pattern(PatternCmds* self, psy_dsp_big_beat_t bpl)
{
	/*if (self->pattern) {
		psy_ui_OpenDialog dialog;

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", patternfilter,
			"PSB", dirconfig_songs(self->dirconfig));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(self->pattern,
				psy_ui_opendialog_path(&dialog), bpl);
		}
		psy_ui_opendialog_dispose(&dialog);
	}*/
}

void patterncmds_export_pattern(PatternCmds* self, psy_dsp_big_beat_t bpl,
	uintptr_t numtracks)
{
	/*if (self->pattern) {
		psy_ui_SaveDialog dialog;

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", patternfilter,
			"PSB", dirconfig_songs(self->dirconfig));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(self->pattern,
				psy_ui_savedialog_path(&dialog), bpl,
				numtracks);				
		}
		psy_ui_savedialog_dispose(&dialog);
	}*/
}
