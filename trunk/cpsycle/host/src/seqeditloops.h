/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITLOOPS_H)
#define SEQEDITLOOPS_H

/* host */
#include "seqeditorstate.h"
/* audio */
#include <pattern.h>
/* std */
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditLoopState */
typedef struct SeqEditLoopState {
	bool drag;
	bool remove;
	psy_audio_PatternNode* start;
	psy_audio_PatternNode* end;
	psy_audio_PatternNode* prev;
	psy_audio_PatternNode* next;
} SeqEditLoopState;

void seqeditloopstate_init(SeqEditLoopState*);

void seqeditloopstate_start_drag(SeqEditLoopState*, psy_audio_PatternNode*);
void seqeditloopstate_remove(SeqEditLoopState*, psy_audio_PatternNode*);
void seqeditloopstate_reset(SeqEditLoopState*);


/* SeqEditLoops */
typedef struct SeqEditLoop {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	/* references */
	SeqEditState* state;
	SeqEditLoopState* loopstate;
	psy_audio_PatternNode* node;
	psy_audio_Pattern* pattern;	
} SeqEditLoop;

void seqeditloop_init(SeqEditLoop*, psy_ui_Component* parent,
	SeqEditLoopState*, SeqEditState*,
	psy_audio_PatternNode*);

SeqEditLoop* seqeditloop_alloc(void);
SeqEditLoop* seqeditloop_allocinit(psy_ui_Component* parent, SeqEditLoopState*,
	SeqEditState*, psy_audio_PatternNode*);

void seqeditloop_updateposition(SeqEditLoop*);

INLINE psy_audio_PatternEntry* seqeditloop_entry(SeqEditLoop* self)
{		
	return (psy_audio_PatternEntry*)(self->node->entry);
}

INLINE const psy_audio_PatternEntry* seqeditloop_entry_const(
	const SeqEditLoop* self)
{		
	return (const psy_audio_PatternEntry*)(self->node->entry);
}

INLINE psy_dsp_big_beat_t seqeditloop_offset(const SeqEditLoop* self)
{	
	if (seqeditloop_entry_const(self)) {
		return seqeditloop_entry_const(self)->offset;
	}
	return 0.0;					
}

INLINE psy_dsp_big_beat_t seqeditloop_endoffset(const SeqEditLoop* self)
{	
	if (self->loopstate->end) {
		const psy_audio_PatternEntry* loopendentry;	
			
		loopendentry = (const psy_audio_PatternEntry*)
			(self->loopstate->end->entry);		
		return loopendentry->offset;		
	}
	return 0.0;					
}

INLINE psy_dsp_big_beat_t seqeditloop_nextoffset(const SeqEditLoop* self)
{	
	if (self->loopstate->next) {
		const psy_audio_PatternEntry* entry;	
			
		entry = (const psy_audio_PatternEntry*)
			(self->loopstate->next->entry);		
		return entry->offset;		
	}
	return (double)UINTPTR_MAX;
}

INLINE psy_dsp_big_beat_t seqeditloop_prevoffset(const SeqEditLoop* self)
{	
	if (self->loopstate->prev) {
		const psy_audio_PatternEntry* entry;	
			
		entry = (const psy_audio_PatternEntry*)
			(self->loopstate->prev->entry);		
		return entry->offset;		
	}
	return 0.0;					
}

INLINE psy_dsp_big_beat_t seqeditloop_startoffset(const SeqEditLoop* self)
{	
	if (self->loopstate->start) {
		const psy_audio_PatternEntry* loopendentry;	
			
		loopendentry = (const psy_audio_PatternEntry*)
			(self->loopstate->start->entry);
		return loopendentry->offset;		
	}
	return 0.0;					
}

INLINE uint8_t seqeditloop_repeat(const SeqEditLoop* self)
{	
	if (seqeditloop_entry_const(self)) {
		 psy_audio_PatternEvent e;
		 
		 e = *psy_audio_patternentry_front(
			 seqeditloop_entry((SeqEditLoop*)self));
		 return (e.parameter & 0x0F);
	}
	return 0;					
}

INLINE void seqeditloop_setnode(SeqEditLoop* self, psy_audio_PatternNode* node)
{
	assert(self);

	self->node = node;
	seqeditloop_updateposition(self);
}

/* SeqEditLoops */
typedef struct SeqEditLoops {
	/* inherits */
	psy_ui_Component component;	
	SeqEditLoopState loopstate;
	/* internal */	
	/* references */
	SeqEditState* state;
	psy_List* entries;
} SeqEditLoops;

void seqeditloops_init(SeqEditLoops*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditloops_build(SeqEditLoops*);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITLOOPS_H */
