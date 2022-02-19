/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTIMESIG_H)
#define SEQEDITTIMESIG_H

/* host */
#include "seqeditorstate.h"
#include "workspace.h"
/* ui */
#include <uitextinput.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditTimeSigState */
typedef struct SeqEditTimeSigState {
	bool drag;
	bool remove;
	psy_audio_PatternNode* start;
	
} SeqEditTimeSigState;

void seqedittimesigstate_init(SeqEditTimeSigState*);

void seqedittimesigstate_startdrag(SeqEditTimeSigState*,
	psy_audio_PatternNode*);
void seqedittimesigstate_remove(SeqEditTimeSigState*,
	psy_audio_PatternNode*);
void seqedittimesigstate_reset(SeqEditTimeSigState*);


/* SeqEditTimeSig */
typedef struct SeqEditTimeSig {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	/* references */
	SeqEditState* state;
	SeqEditTimeSigState* timesigstate;
	psy_audio_PatternNode* node;	
	psy_audio_Pattern* pattern;
} SeqEditTimeSig;

void seqedittimesig_init(SeqEditTimeSig*, psy_ui_Component* parent,
	SeqEditTimeSigState*, SeqEditState*, psy_audio_PatternNode*);

SeqEditTimeSig* seqedittimesig_alloc(void);
SeqEditTimeSig* seqedittimesig_allocinit(psy_ui_Component* parent,
	SeqEditTimeSigState*, SeqEditState*, psy_audio_PatternNode*);

void seqedittimesig_updateposition(SeqEditTimeSig*);
void seqedittimesig_select(SeqEditTimeSig*);

INLINE psy_audio_PatternEntry* seqedittimesig_entry(
	const SeqEditTimeSig* self)
{		
	return (psy_audio_PatternEntry*)(self->node->entry);
}

INLINE const psy_audio_PatternEntry* seqedittimesig_entry_const(
	const SeqEditTimeSig* self)
{		
	return (const psy_audio_PatternEntry*)(self->node->entry);
}

INLINE psy_dsp_big_beat_t seqedittimesig_offset(const SeqEditTimeSig* self)
{	
	if (seqedittimesig_entry_const(self)) {
		return seqedittimesig_entry_const(self)->offset;
	}
	return 0.0;					
}


/* SeqEditTimesig */
typedef struct SeqEditTimeSigs {
	/* inherits */
	psy_ui_Component component;
	SeqEditTimeSigState timesigstate;
	/* internal */			
	/* references */
	SeqEditState* state;	
	psy_List* entries;
} SeqEditTimeSigs;

void seqedittimesigs_init(SeqEditTimeSigs*, psy_ui_Component* parent,
	SeqEditState*);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTIMESIG_H */
