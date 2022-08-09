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
	uintptr_t trackindex, psy_audio_SequenceTrack*, SeqViewState*);

SeqViewTrack* seqviewtrack_alloc(void);
SeqViewTrack* seqviewtrack_allocinit(psy_ui_Component* parent,
	uintptr_t trackindex, psy_audio_SequenceTrack*, SeqViewState*);

/* SeqViewItem */

typedef struct SeqViewItem {
	/* inherits */
	psy_ui_Component component;
	psy_audio_SequenceEntry* seqentry;
	psy_audio_OrderIndex order_index;	
	Workspace* workspace;
	SeqViewState* state;
} SeqViewItem;

void seqviewitem_init(SeqViewItem*, psy_ui_Component* parent,
	psy_audio_SequenceEntry*, psy_audio_OrderIndex, SeqViewState*, Workspace*);

SeqViewItem* seqviewitem_alloc(void);
SeqViewItem* seqviewitem_allocinit(psy_ui_Component* parent,
	psy_audio_SequenceEntry*, psy_audio_OrderIndex, SeqViewState*, Workspace*);

/* SeqviewList */

typedef struct SeqviewList {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	int foundselected;	
	double avgcharwidth;	
	/* references */
	SeqViewState* state;	
} SeqviewList;

void seqviewlist_init(SeqviewList*, psy_ui_Component* parent, SeqViewState*);
void seqviewlist_rename(SeqviewList*);
void seqviewlist_build(SeqviewList*);
void seqviewlist_on_pattern_name_changed(SeqviewList*, psy_audio_Patterns*,
	uintptr_t slot);

INLINE psy_ui_Component* seqviewlist_base(SeqviewList* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQLISTVIEW_H */
