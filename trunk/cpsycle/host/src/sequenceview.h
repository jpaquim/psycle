// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCERVIEW)
#define SEQUENCERVIEW

#include <patterns.h>
#include <sequence.h>
#include <uibutton.h>

typedef struct {
   void (*newentry)(void*);
} SequenceController;

typedef struct {
	ui_component component;
	SequenceController controller;
	ui_button newentry;
	void* context;
	HFONT font;
} SequenceButtons;

typedef struct {
	HFONT font;
} SequenceListViewSkin;

typedef struct {
	ui_component component;
	Sequence* sequence;
	Patterns* patterns;
	SequenceController* controller;
	int selected;
	int lineheight;
	SequenceListViewSkin skin;
	void (*selchanged)(void*, SequenceEntry*);
	void* selcontext;
} SequenceListView;

typedef struct {
	ui_component component;
	SequenceListView listview;
	SequenceButtons buttons;
	Sequence* sequence;
} SequenceView;

void InitSequenceView(SequenceView*, ui_component* parent, Sequence* sequence, Patterns* patterns);
void InitSequenceListView(SequenceListView*, ui_component* parent, Sequence* sequence, Patterns* patterns);
void InitSequenceButtons(SequenceButtons* self, ui_component* parent);
void SequenceViewConnect(SequenceView*, void* context);

#endif
