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
	void* context;	
} SequenceButtons;

typedef struct {
	ui_component component;
	Sequence* sequence;
	Patterns* patterns;	
	int selected;
	int selectedtrack;
	int foundselected;
	int lineheight;
	int trackwidth;
	int identwidth;   
	int avgcharwidth;
	int dx;
	int dy;	
} SequenceListView;

typedef struct {
	ui_component component;
	ui_label desc;
	ui_label duration;	
	Sequence* sequence;
} SequenceViewDuration;

void InitSequenceViewDuration(SequenceViewDuration*, ui_component* parent,
	Sequence*);

typedef struct {
	ui_component component;	
	SequenceListView listview;
	SequenceButtons buttons;
	SequenceViewDuration duration;
	ui_checkbox followsong;
	ui_checkbox shownames;	
	Patterns* patterns;
	Sequence* sequence;	
} SequenceView;

void InitSequenceView(SequenceView*, ui_component* parent, Workspace*);
void InitSequenceListView(SequenceListView*, ui_component* parent, Sequence*,
	Patterns*);
void InitSequenceButtons(SequenceButtons*, ui_component* parent);

#endif
