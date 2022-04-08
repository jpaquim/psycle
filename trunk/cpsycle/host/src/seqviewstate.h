/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQVIEWSTATE_H)
#define SEQVIEWSTATE_H

/* host */
#include "sequencehostcmds.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SEQLVCMD_NONE = 0,
	SEQLVCMD_NEWTRACK = 1,
	SEQLVCMD_DELTRACK = 2
} SeqLVCmd;

/* SeqViewState */
typedef struct SeqViewState {
	/* public */
	psy_ui_Value trackwidth;
	psy_ui_Value line_height;
	psy_ui_RealSize digitsize;
	double colwidth;
	SeqLVCmd cmd;	
	psy_audio_OrderIndex cmd_orderindex;
	uintptr_t col;
	bool active;	
	bool showpatternnames;
	/* references */
	SequenceCmds* cmds;
} SeqViewState;

void seqviewstate_init(SeqViewState*, SequenceCmds*);

void sequencelistviewstate_update(SeqViewState*);

#ifdef __cplusplus
}
#endif

#endif /* SEQVIEWSTATE_H */
