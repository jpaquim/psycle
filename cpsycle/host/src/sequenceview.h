// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCERVIEW_H)
#define SEQUENCERVIEW_H

#include <patterns.h>
#include <sequence.h>

#include <uibutton.h>
#include <uilabel.h>
#include <uicheckbox.h>
#include <uisplitbar.h>

#include "playlisteditor.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;	
	psy_ui_Button incpattern;
	psy_ui_Button decpattern;	
	psy_ui_Button newentry;
	psy_ui_Button insertentry;
	psy_ui_Button delentry;
	psy_ui_Button cloneentry;
	psy_ui_Button newtrack;
	psy_ui_Button deltrack;
	psy_ui_Button clear;
	psy_ui_Button cut;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button singlesel;
	psy_ui_Button multisel;	
	void* context;	
} SequenceButtons;

void sequencebuttons_init(SequenceButtons*, psy_ui_Component* parent);

typedef struct {
	psy_ui_Component component;
	psy_audio_Sequence* sequence;
	SequenceSelection* selection;	
	psy_audio_Patterns* patterns;	
	int selected;
	int selectedtrack;
	int foundselected;
	int lineheight;
	int textheight;
	int trackwidth;
	int identwidth;   
	int avgcharwidth;
	int dx;
	int dy;	
	psy_dsp_beat_t lastplayposition;
	psy_audio_Player* player;
	SequenceEntry* lastentry;	
	Workspace* workspace;
	struct SequenceView* view;
} SequenceListView;

void sequencelistview_init(SequenceListView*, psy_ui_Component* parent,
	struct SequenceView*, psy_audio_Sequence*, psy_audio_Patterns*, Workspace*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Label desc;
	psy_ui_Label duration;	
	psy_audio_Sequence* sequence;
} SequenceViewDuration;

void sequenceduration_init(SequenceViewDuration*, psy_ui_Component* parent,
	psy_audio_Sequence*);

typedef struct {
	psy_ui_Component component;	
	struct SequenceView* view;
} SequenceViewTrackHeader;

void sequenceviewtrackheader_init(SequenceViewTrackHeader* self,
	psy_ui_Component* parent, struct SequenceView*);

typedef struct {
	psy_ui_Component component;
	ui_checkbox followsong;
	ui_checkbox shownames;
	ui_checkbox showplaylist;
	ui_checkbox recordtweaks;
	ui_checkbox multichannelaudition;
} SequencerOptionsBar;

void sequenceroptionsbar_init(SequencerOptionsBar* self, psy_ui_Component* parent);

typedef struct SequenceView {
	psy_ui_Component component;	
	SequenceListView listview;
	SequenceButtons buttons;
	SequenceViewTrackHeader trackheader;
	SequenceViewDuration duration;
	PlayListEditor playlisteditor;
	psy_ui_SplitBar splitbar;
	SequencerOptionsBar options;
	psy_audio_Patterns* patterns;
	psy_audio_Sequence* sequence;
	SequenceSelection* selection;
	Workspace* workspace;	
} SequenceView;

void sequenceview_init(SequenceView*, psy_ui_Component* parent, Workspace*);


#endif
