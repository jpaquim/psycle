/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQUENCERVIEW_H)
#define SEQUENCERVIEW_H

/* host */
#include "sequencetrackbox.h"
#include "sequencebuttons.h"
/* audio */
#include <patterns.h>
#include <sequence.h>
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uiscroller.h>
#include <uisplitbar.h>
#include <uinotebook.h>

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
	psy_ui_Value lineheight;
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

INLINE psy_ui_Component* seqviewlist_base(SeqviewList* self)
{
	return &self->component;
}

/* SeqviewDuration */
typedef struct SeqviewDuration {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label desc;
	psy_ui_Label duration;			
	psy_dsp_big_seconds_t duration_ms;
	psy_dsp_big_beat_t duration_bts;
	bool calcduration;
	/* references */
	Workspace* workspace;
} SeqviewDuration;

void seqviewduration_init(SeqviewDuration*, psy_ui_Component* parent,
	Workspace*);
void seqviewduration_update(SeqviewDuration*, bool force);
void seqviewduration_stopdurationcalc(SeqviewDuration*);

INLINE psy_ui_Component* seqviewduration_base(SeqviewDuration* self)
{
	return &self->component;
}

/* SeqviewTrackHeaders */
typedef struct SeqviewTrackHeaders {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_trackselected;	
	/* internal */
	psy_ui_Component client;	
	/* references */
	SeqViewState* state;	
} SeqviewTrackHeaders;

void seqviewtrackheaders_init(SeqviewTrackHeaders* self,
	psy_ui_Component* parent, SeqViewState*);

void seqviewtrackheaders_build(SeqviewTrackHeaders*);

INLINE psy_ui_Component* seqviewtrackheader_base(SeqviewTrackHeaders* self)
{
	return &self->component;
}

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

INLINE psy_ui_Component* seqview_base(SeqView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCERVIEW_H */
