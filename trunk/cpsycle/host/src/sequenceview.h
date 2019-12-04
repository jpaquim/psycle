// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCERVIEW_H)
#define SEQUENCERVIEW_H

#include <patterns.h>
#include <sequence.h>
#include <uibutton.h>
#include <uilabel.h>
#include <uicheckbox.h>
#include "workspace.h"

typedef struct {
	ui_component component;	
	ui_button incpattern;
	ui_button decpattern;	
	ui_button newentry;
	ui_button insertentry;
	ui_button delentry;
	ui_button cloneentry;
	ui_button newtrack;
	ui_button deltrack;
	ui_button clear;
	ui_button cut;
	ui_button copy;
	ui_button paste;
	ui_button singlesel;
	ui_button multisel;	
	void* context;	
} SequenceButtons;

void sequencebuttons_init(SequenceButtons*, ui_component* parent);

typedef struct {
	ui_component component;
	Sequence* sequence;
	SequenceSelection* selection;	
	Patterns* patterns;	
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
	beat_t lastplayposition;
	Player* player;
	SequenceEntry* lastentry;	
	Workspace* workspace;
	struct SequenceView* view;
} SequenceListView;

void sequencelistview_init(SequenceListView*, ui_component* parent,
	struct SequenceView*, Sequence*, Patterns*, Workspace*);

typedef struct {
	ui_component component;
	ui_label desc;
	ui_label duration;	
	Sequence* sequence;
} SequenceViewDuration;

void sequenceduration_init(SequenceViewDuration*, ui_component* parent,
	Sequence*);

typedef struct {
	ui_component component;	
	struct SequenceView* view;
} SequenceViewTrackHeader;

void sequenceviewtrackheader_init(SequenceViewTrackHeader* self,
	ui_component* parent, struct SequenceView*);

typedef struct {
	ui_component component;
	ui_checkbox followsong;
	ui_checkbox shownames;	
	ui_checkbox recordtweaks;
	ui_checkbox multichannelaudition;
} SequencerOptionsBar;

void sequenceroptionsbar_init(SequencerOptionsBar* self, ui_component* parent);

typedef struct SequenceView {
	ui_component component;	
	SequenceListView listview;
	SequenceButtons buttons;
	SequenceViewTrackHeader trackheader;
	SequenceViewDuration duration;
	SequencerOptionsBar options;
	Patterns* patterns;
	Sequence* sequence;
	SequenceSelection* selection;
	Workspace* workspace;	
} SequenceView;

void sequenceview_init(SequenceView*, ui_component* parent, Workspace*);


#endif
