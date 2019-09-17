// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sequenceview.h"
#include <stdio.h>

static void OnDraw(SequenceListView*, ui_component* sender, ui_graphics* g);
static void DrawSequence(SequenceListView*, ui_graphics* g);
void DrawTrack(SequenceListView*, ui_graphics* g, SequenceTrack* track,
	int trackindex, int x);
static void OnNewEntry(SequenceView*);
static void OnInsertEntry(SequenceView*);
static void OnCloneEntry(SequenceView*);
static void OnDelEntry(SequenceView*);
static void OnNewTrack(SequenceView*);
static void OnDelTrack(SequenceView*);
static void OnSize(SequenceView*, ui_component* sender, int width, int height);
static void OnListViewMouseDown(SequenceListView*, ui_component* sender, int x, int y, int button);
static void OnSongChanged(SequenceView*, Workspace*);
static void OnEditPositionChanged(SequenceView*, Sequence* sender);
static void AlignSequenceButtons(SequenceButtons*);

static int trackwidth = 75;

void InitSequenceView(SequenceView* self, ui_component* parent,
	Workspace* workspace)
{	
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 0x00EEEEEE);
	signal_connect(&self->component.signal_size, self, OnSize);	
	InitSequenceListView(&self->listview, &self->component, 
		&workspace->song->sequence, &workspace->song->patterns);			
	ui_component_move(&self->listview.component, 0, 75);
	self->buttons.context = &self->listview;
	InitSequenceButtons(&self->buttons, &self->component);
	ui_component_resize(&self->buttons.component, 200, 70);
	signal_connect(&self->buttons.newentry.signal_clicked, self, OnNewEntry);
	signal_connect(&self->buttons.insertentry.signal_clicked, self, OnInsertEntry);
	signal_connect(&self->buttons.cloneentry.signal_clicked, self, OnCloneEntry);
	signal_connect(&self->buttons.delentry.signal_clicked, self, OnDelEntry);
	signal_connect(&self->buttons.newtrack.signal_clicked, self, OnNewTrack);
	signal_connect(&self->buttons.deltrack.signal_clicked, self, OnDelTrack);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	signal_connect(&workspace->song->sequence.signal_editpositionchanged, self, OnEditPositionChanged);
	ui_component_resize(&self->component, 150, 0);
}

void InitSequenceButtons(SequenceButtons* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 0x00FFFFFF);

	ui_button_init(&self->incpattern, &self->component);	
	ui_button_settext(&self->incpattern, "+");
	ui_button_init(&self->insertentry, &self->component);	
	ui_button_settext(&self->insertentry, "Ins");		
	ui_button_init(&self->decpattern, &self->component);	
	ui_button_settext(&self->decpattern, "-");		
	ui_button_init(&self->newentry, &self->component);	
	ui_button_settext(&self->newentry, "New");		
	ui_button_init(&self->cloneentry, &self->component);	
	ui_button_settext(&self->cloneentry, "Clone");		
	ui_button_init(&self->delentry, &self->component);	
	ui_button_settext(&self->delentry, "Del");		
	ui_button_init(&self->newtrack, &self->component);	
	ui_button_settext(&self->newtrack, "New Trk");
	ui_button_init(&self->deltrack, &self->component);	
	ui_button_settext(&self->deltrack, "Del Trk");
	AlignSequenceButtons(self);
}

void AlignSequenceButtons(SequenceButtons* self)
{
	int colx[4];
	List* p;
	int cpy = 0;
	int col = 0;

	colx[0] = 5;
	colx[1] = 55;
	colx[2] = 105;
	colx[3] = 145;

	for (p = ui_component_children(&self->component, 0); p != 0; p = p->next) {
		ui_component* component;

		component = (ui_component*)p->entry;		
		ui_component_setposition(component, colx[col], cpy, colx[col + 1] - colx[col] - 5, 20);
		++col;
		if (col == 3) {
			col = 0;
			cpy += 22;
		}
	}
}

void InitSequenceListView(SequenceListView* self, ui_component* parent,
	Sequence* sequence, Patterns* patterns)
{				
	self->sequence = sequence;
	self->patterns = patterns;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_mousedown, self, OnListViewMouseDown);	
	self->selected = 0;
	self->selectedtrack = 0;	
	self->lineheight = 12;
}

void OnDraw(SequenceListView* self, ui_component* sender, ui_graphics* g)
{	
	DrawSequence(self, g);
}

void DrawSequence(SequenceListView* self, ui_graphics* g)
{
	SequenceTracks* p;	
	int cpx = 0;
	int c = 0;	
	self->foundselected = 0;
	for (p = self->sequence->tracks; p != 0; p = p->next, cpx += trackwidth, ++c) {
		DrawTrack(self, g, (SequenceTrack*)p->entry, c, cpx);
	}
	if (!self->foundselected) {
		ui_setbackgroundcolor(g, 0x00FF0000);
		ui_textout(g, self->selectedtrack*trackwidth,
			self->selected * self->lineheight, "     ", 5);
	}
}

void DrawTrack(SequenceListView* self, ui_graphics* g, SequenceTrack* track, int trackindex, int x)
{
	List* p;	
	unsigned int c = 0;
	int cpy = 0;	
	char buffer[20];
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
		
	ui_setrectangle(&r, x, 0, trackwidth - 5, size.height);	
	if (trackindex == self->selectedtrack) {
		ui_drawsolidrectangle(g, r, 0x00CCCCCC);
	} else {		
		ui_drawsolidrectangle(g, r, 0x00DDDDDD);
	}
	ui_settextcolor(g, 0);
	p = track->entries;
	while (p != 0) {
		SequenceEntry* entry;
		// Pattern* pattern;
		entry = (SequenceEntry*)p->entry;
		//pattern = patterns_at(&self->sequence->patterns, entry->pattern);
		//if (pattern) {
			_snprintf(buffer,20, "%02X:%02X  %4.2f", c, entry->pattern, entry->offset);
			if (self->selected == (int)c && self->selectedtrack == trackindex) {
				ui_setbackgroundcolor(g, 0x00FF0000);
				self->foundselected = 1;				
			} else {
				ui_setbackgroundcolor(g, 0xFFFFFFFF);
			}
			ui_textout(g, x, cpy, buffer, strlen(buffer));
		//}
		p = p->next;
		cpy += self->lineheight;
		++c;
	}	
}

void OnSize(SequenceView* self, ui_component* sender, int width, int height)
{
	ui_size size = ui_component_size(&self->buttons.component);
	
	ui_component_resize(&self->buttons.component, width - 3, size.height);
	ui_component_resize(&self->listview.component, width - 3, height - size.height - 3);
}

void OnNewEntry(SequenceView* self)
{
	unsigned int slot;
	Pattern* pattern = (Pattern*) malloc(sizeof(Pattern));
	pattern_init(pattern);
	slot = patterns_append(self->patterns, pattern);
	sequence_insert(self->sequence, sequence_editposition(self->sequence), slot);	
}

void OnInsertEntry(SequenceView* self)
{
	unsigned int slot = 0;
	SequencePosition editposition;	
	
	editposition = sequence_editposition(self->sequence);
	if (editposition.trackposition.tracknode) {				
		slot = ((SequenceEntry*)(editposition.trackposition.tracknode->entry))->pattern;
	}		
	sequence_insert(self->sequence, editposition, slot);	
}

void OnCloneEntry(SequenceView* self)
{
	unsigned int slot = 0;
	SequencePosition editposition;	
	
	editposition = sequence_editposition(self->sequence);
	if (editposition.trackposition.tracknode) {				
		Pattern* pattern;
		slot = ((SequenceEntry*)(editposition.trackposition.tracknode->entry))->pattern;
		pattern = patterns_at(self->patterns, slot);
		if (pattern) {
			pattern = pattern_clone(pattern);
			slot = patterns_append(self->patterns, pattern);
		}
		if (pattern) {
			sequence_insert(self->sequence, editposition, slot);
		}
	}			
}

void OnDelEntry(SequenceView* self)
{
	if (self->listview.selected != -1) {
		SequencePosition p = sequence_editposition(self->sequence);
		sequence_remove(self->sequence, p);
		if (p.track == self->sequence->tracks &&
				sequence_size(self->sequence, p.track) == 0) {
			SequencePosition position;
			
			position.trackposition = sequence_begin(self->sequence, p.track, 0);
			position.track = p.track;
			sequence_insert(self->sequence, position, 0);
		}		
	}
}

void OnNewTrack(SequenceView* self)
{
	SequenceTrack* track = (SequenceTrack*)malloc(sizeof(SequenceTrack));
	sequencetrack_init(track);
	sequence_appendtrack(self->sequence, track);
	ui_invalidate(&self->component);	
}

void OnDelTrack(SequenceView* self)
{	
	SequencePosition position;
	position = sequence_at(self->sequence, self->listview.selectedtrack,
		self->listview.selected);	
	sequence_removetrack(self->sequence, position.track);
	ui_invalidate(&self->component);	
}

void OnListViewMouseDown(SequenceListView* self, ui_component* sender, int x, int y, int button)
{
	unsigned int selected;
	unsigned int selectedtrack;	

	selected = y / self->lineheight;
	selectedtrack = x / trackwidth;

	if (selectedtrack < sequence_sizetracks(self->sequence)) {
		SequencePosition position;

		position = sequence_at(self->sequence, selectedtrack, selected);
		sequence_seteditposition(self->sequence, position);
	}
}

void OnSongChanged(SequenceView* self, Workspace* workspace)
{
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;
	signal_connect(&workspace->song->sequence.signal_editpositionchanged, self, OnEditPositionChanged);
	self->listview.sequence = &workspace->song->sequence;
	self->listview.patterns = &workspace->song->patterns;
	self->listview.selected = 0;
	ui_invalidate(&self->component);
}

void OnEditPositionChanged(SequenceView* self, Sequence* sequence)
{
	SequencePosition position;
	SequenceTracks* p;
	List* q;
	int c = 0;

	position = sequence_editposition(sequence);
	
	p = sequence->tracks;
	while (p != 0) {
		if (position.track == p) {
			break;
		}
		++c;
		p = p->next;		
	}

	self->listview.selectedtrack = c;


	if (p) {
		q = ((SequenceTrack*)p->entry)->entries;
		c = 0;
		while (q) {
			if (q == position.trackposition.tracknode) {
				break;
			}
			++c;
			q = q->next;
		}
		self->listview.selected = c;
	}
	ui_invalidate(&self->listview.component);
}
