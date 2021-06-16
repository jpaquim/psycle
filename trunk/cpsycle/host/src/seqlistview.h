/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQLISTVIEW_H)
#define SEQLISTVIEW_H

/* host */
#include "seqviewstate.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SeqView;

/* SeqViewTrack */
typedef struct SeqViewTrack {
	/* inherits */
	psy_ui_Component component;
	uintptr_t trackindex;
	/* references */
	SeqViewState* state;
	psy_audio_SequenceTrack* track;	
} SeqViewTrack;

void seqviewtrack_init(SeqViewTrack*, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t trackindex, psy_audio_SequenceTrack*,
	SeqViewState*);

SeqViewTrack* seqviewtrack_alloc(void);
SeqViewTrack* seqviewtrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	uintptr_t trackindex, psy_audio_SequenceTrack*, SeqViewState*);

/* SeqviewList */
typedef struct SeqviewList {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	int foundselected;	
	double avgcharwidth;
	bool showpatternnames;
	psy_dsp_beat_t lastplayposition;	
	int refreshcount;
	uintptr_t lastplayrow;	
	/* references */
	SeqViewState* state;	
} SeqviewList;

void seqviewlist_init(SeqviewList*, psy_ui_Component* parent, SeqViewState*);
void seqviewlist_showpatternnames(SeqviewList*);
void seqviewlist_showpatternslots(SeqviewList*);
void seqviewlist_rename(SeqviewList*);
void seqviewlist_computetextsizes(SeqviewList*);
void seqviewlist_onpatternnamechanged(SeqviewList*,
	psy_audio_Patterns*, uintptr_t slot);
void seqviewlist_build(SeqviewList*);

INLINE psy_ui_Component* seqviewlist_base(SeqviewList* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQLISTVIEW_H */
