// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCERVIEW_H)
#define SEQUENCERVIEW_H

// host
#include "sequencetrackbox.h"
#include "workspace.h"
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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SequenceButtons {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Button incpattern;
	psy_ui_Button decpattern;	
	psy_ui_Button newentry;
	psy_ui_Button insertentry;
	psy_ui_Button delentry;
	psy_ui_Button cloneentry;	
	psy_ui_Button clear;
	psy_ui_Button rename;
	psy_ui_Button cut;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button singlesel;
	psy_ui_Button multisel;
	// references	
	Workspace* workspace;
} SequenceButtons;

void sequencebuttons_init(SequenceButtons*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* sequencebuttons_base(SequenceButtons* self)
{
	return &self->component;
}

typedef struct SequenceListViewState {
	// public data
	double margin;
	double trackwidth;	
	// references
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
} SequenceListViewState;

void sequencelistviewstate_init(SequenceListViewState*);

struct SequenceView;

typedef struct SequenceListView {
	// inherits
	psy_ui_Component component;
	// ui elements	
	psy_ui_Edit rename;
	// internal data	
	int foundselected;
	double lineheight;
	double textoffsety;
	double textheight;
	double identwidth;
	double avgcharwidth;
	bool showpatternnames;
	psy_dsp_beat_t lastplayposition;	
	int refreshcount;
	uintptr_t lastplayrow;
	// references
	psy_audio_Player* player;	
	psy_audio_Patterns* patterns;	
	// references
	SequenceListViewState* state;
	struct SequenceView* view;
	Workspace* workspace;
} SequenceListView;

void sequencelistview_init(SequenceListView*, psy_ui_Component* parent,
	SequenceListViewState*, struct SequenceView*,
	psy_audio_Patterns*, Workspace*);
void sequencelistview_showpatternnames(SequenceListView*);
void sequencelistview_showpatternslots(SequenceListView*);
void sequencelistview_rename(SequenceListView*);
void sequencelistview_computetextsizes(SequenceListView*);
void sequencelistview_select(SequenceListView*,
	uintptr_t track, uintptr_t row);

INLINE psy_ui_Component* sequencelistview_base(SequenceListView* self)
{
	return &self->component;
}

typedef struct SequenceViewDuration {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label desc;
	psy_ui_Label duration;		
	// internal data
	psy_dsp_big_seconds_t duration_ms;
	psy_dsp_big_beat_t duration_bts;
	// references
	psy_audio_Sequence* sequence;
	Workspace* workspace;
} SequenceViewDuration;

void sequenceduration_init(SequenceViewDuration*, psy_ui_Component* parent,
	psy_audio_Sequence*, Workspace*);
void sequenceduration_update(SequenceViewDuration*);

INLINE psy_ui_Component* sequenceduration_base(SequenceViewDuration* self)
{
	return &self->component;
}

typedef struct SequenceTrackHeaders {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_newtrack;
	psy_Signal signal_deltrack;
	psy_Signal signal_trackselected;
	psy_Signal signal_mutetrack;
	psy_Signal signal_solotrack;
	// internal data
	uintptr_t hovertrack;
	// references
	SequenceListViewState* state;	
} SequenceTrackHeaders;

void sequencetrackheaders_init(SequenceTrackHeaders* self,
	psy_ui_Component* parent, SequenceListViewState*);

INLINE psy_ui_Component* sequencetrackheader_base(
	SequenceTrackHeaders* self)
{
	return &self->component;
}

typedef struct SequencerOptionsBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_CheckBox followsong;
	psy_ui_CheckBox shownames;	
	psy_ui_CheckBox recordnoteoff;
	psy_ui_CheckBox recordtweak;
	psy_ui_CheckBox multichannelaudition;
	psy_ui_CheckBox allownotestoeffect;
	psy_ui_Component seqedit;
	psy_ui_Button toggleseqedit;
	psy_ui_Button toggleseqediticon;
	psy_ui_Component stepseq;
	psy_ui_Button togglestepseq;
	psy_ui_Button togglestepseqicon;
	// references
	Workspace* workspace;
} SequencerOptionsBar;

void sequenceroptionsbar_init(SequencerOptionsBar* self, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* sequenceroptionsbar_base(SequencerOptionsBar* self)
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
	SequencerOptionsBar options;
	// internal data
	SequenceListViewState state;
	// references
	psy_audio_Patterns* patterns;	
	Workspace* workspace;
} SequenceView;

void sequenceview_init(SequenceView*, psy_ui_Component* parent, Workspace*);
void sequenceview_updateplayposition(SequenceView*);
void sequenceview_changeplayposition(SequenceView*);
void sequenceview_clear(SequenceView*);

INLINE psy_ui_Component* sequenceview_base(SequenceView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCERVIEW_H */
