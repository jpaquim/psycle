// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCERVIEW_H)
#define SEQUENCERVIEW_H

// host
#include "sequencetrackbox.h"
#include "sequencebuttons.h"
// audio
#include <patterns.h>
#include <sequence.h>
// ui
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

typedef struct SequenceListViewState {
	// public data	
	psy_ui_Value trackwidth;
	psy_ui_Value lineheight;
	SeqLVCmd cmd;
	psy_audio_OrderIndex cmd_orderindex;
	bool showpatternnames;
	// references	
	SequenceCmds* cmds;
} SequenceListViewState;

void sequencelistviewstate_init(SequenceListViewState*, SequenceCmds*);

void sequencelistviewstate_update(SequenceListViewState*);

struct SequenceView;

typedef struct SequenceListTrack {
	// inherits
	psy_ui_Component component;
	uintptr_t trackindex;
	// references
	SequenceListViewState* state;
	psy_audio_SequenceTrack* track;	
} SequenceListTrack;

void sequencelisttrack_init(SequenceListTrack*, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t trackindex, psy_audio_SequenceTrack*,
	SequenceListViewState*);

SequenceListTrack* sequencelisttrack_alloc(void);
SequenceListTrack* sequencelisttrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	uintptr_t trackindex, psy_audio_SequenceTrack*, SequenceListViewState*);

typedef struct SequenceListView {
	// inherits
	psy_ui_Component component;
	// ui elements		
	// internal	
	int foundselected;	
	double avgcharwidth;
	bool showpatternnames;
	psy_dsp_beat_t lastplayposition;	
	int refreshcount;
	uintptr_t lastplayrow;			
	// references
	SequenceListViewState* state;	
} SequenceListView;

void sequencelistview_init(SequenceListView*, psy_ui_Component* parent,
	SequenceListViewState*, struct SequenceView*);
void sequencelistview_showpatternnames(SequenceListView*);
void sequencelistview_showpatternslots(SequenceListView*);
void sequencelistview_rename(SequenceListView*);
void sequencelistview_computetextsizes(SequenceListView*);

INLINE psy_ui_Component* sequencelistview_base(SequenceListView* self)
{
	return &self->component;
}

typedef struct SequenceViewDuration {
	// inherits
	psy_ui_Component component;
	// internal	
	psy_ui_Label desc;
	psy_ui_Label duration;			
	psy_dsp_big_seconds_t duration_ms;
	psy_dsp_big_beat_t duration_bts;
	bool calcduration;
	// references		
	Workspace* workspace;
} SequenceViewDuration;

void sequenceduration_init(SequenceViewDuration*, psy_ui_Component* parent,
	Workspace*);
void sequenceduration_update(SequenceViewDuration*, bool force);
void sequenceduration_stopdurationcalc(SequenceViewDuration*);

INLINE psy_ui_Component* sequenceduration_base(SequenceViewDuration* self)
{
	return &self->component;
}

// SequenceTrackHeaders
typedef struct SequenceTrackHeaders {
	// inherits
	psy_ui_Component component;
	// signals	
	psy_Signal signal_trackselected;	
	// internal
	psy_ui_Component client;	
	// references
	SequenceListViewState* state;	
} SequenceTrackHeaders;

void sequencetrackheaders_init(SequenceTrackHeaders* self,
	psy_ui_Component* parent, SequenceListViewState*);

void sequencetrackheaders_build(SequenceTrackHeaders*);

INLINE psy_ui_Component* sequencetrackheader_base(
	SequenceTrackHeaders* self)
{
	return &self->component;
}

typedef struct SequenceView {
	// inherits
	psy_ui_Component component;
	// ui elements	
	SequenceListView listview;
	psy_ui_Scroller scroller;
	SequenceButtons buttons;
	psy_ui_Component spacer;
	SequenceTrackHeaders trackheader;
	SequenceViewDuration duration;
	// internal
	SequenceListViewState state;
	SequenceCmds cmds;		
} SequenceView;

void sequenceview_init(SequenceView*, psy_ui_Component* parent, Workspace*);
void sequenceview_clear(SequenceView*);

INLINE psy_ui_Component* sequenceview_base(SequenceView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCERVIEW_H */
