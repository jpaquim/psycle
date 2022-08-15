/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQUENCERVIEW_H)
#define SEQUENCERVIEW_H

/* host */
#include "sequencebuttons.h"
#include "seqlistview.h"
#include "seqviewduration.h"
#include "seqviewtrackheaders.h"
/* ui */
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqView */
typedef struct SeqView {
	/* inherits */
	psy_ui_Component component;
	/* ui elements */	
	SeqviewList listview;
	psy_ui_Scroller scroller;
	SequenceButtons buttons;
	psy_ui_Component spacer;
	SeqviewTrackHeaders trackheader;
	SeqviewDuration duration;
	psy_ui_Button focus;
	/* internal */
	SeqViewState state;
	SequenceCmds cmds;		
} SeqView;

void seqview_init(SeqView*, psy_ui_Component* parent, Workspace*);

void seqview_clear(SeqView*);
void seqview_idle(SeqView*);
void seqview_realign(SeqView*, SeqViewAlign);

INLINE psy_ui_Component* seqview_base(SeqView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCERVIEW_H */
