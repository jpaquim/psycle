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

typedef enum SeqViewAlign {
	SEQVIEW_ALIGN_NONE = 0,
	SEQVIEW_ALIGN_FULL = 1,
	SEQVIEW_ALIGN_LIST = 2,
	SEQVIEW_ALIGN_REPAINT_LIST = 3
} SeqViewAlign;

/* SeqViewState */

struct SeqView;

typedef struct SeqViewState {
	/* public */
	psy_ui_Size item_size;	
	double colwidth;
	SeqLVCmd cmd;	
	psy_audio_OrderIndex cmd_orderindex;	
	bool showpatternnames;	
	/* references */
	SequenceCmds* cmds;
	struct SeqView* seqview;
} SeqViewState;

void seqviewstate_init(SeqViewState*, SequenceCmds*, struct SeqView*);

void sequencelistviewstate_realign(SeqViewState*, SeqViewAlign);
void sequencelistviewstate_realign_full(SeqViewState*);
void sequencelistviewstate_realign_list(SeqViewState*);
void sequencelistviewstate_repaint_list(SeqViewState*);

#ifdef __cplusplus
}
#endif

#endif /* SEQVIEWSTATE_H */
