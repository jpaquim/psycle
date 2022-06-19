/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqviewstate.h"

/* SeqViewState */
void seqviewstate_init(SeqViewState* self, SequenceCmds* cmds)
{	
	assert(self);
	assert(cmds);

	self->cmds = cmds;
	self->trackwidth = psy_ui_value_make_ew(16.0);	
	self->line_height = psy_ui_value_make_eh(1.2);	
	self->colwidth = 10.0;
	self->cmd = SEQLVCMD_NONE;
	self->cmd_orderindex = psy_audio_orderindex_make_invalid();
	self->col = 0;
	self->active = FALSE;
	self->showpatternnames = FALSE;
}
