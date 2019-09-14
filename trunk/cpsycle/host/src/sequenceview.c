// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sequenceview.h"
#include <stdio.h>

static void OnDraw(SequenceListView* self, ui_component* sender, ui_graphics* g);
static void DrawSequence(SequenceListView* self, ui_graphics* g);
void DrawTrack(SequenceListView* self, ui_graphics* g, SequenceTrack* track,
	int trackindex, int x);
static void OnControllerNewEntry(SequenceListView* self);
static void OnControllerDelEntry(SequenceListView* self);
static void OnControllerNewTrack(SequenceListView* self);
static void OnNewEntry(SequenceButtons* self, ui_component* sender);
static void OnDelEntry(SequenceButtons* self, ui_component* sender);
static void OnNewTrack(SequenceButtons* self, ui_component* sender);
static void OnSize(SequenceView* self, ui_component* sender, int width, int height);
static void OnMouseDown(SequenceListView* self, ui_component* sender, int x, int y, int button);
static void OnSongChanged(SequenceView*, Workspace*);
static void OnEditPositionChanged(SequenceView* self, Sequence* sender);

static int trackwidth = 60;

void InitSequenceView(SequenceView* self, ui_component* parent,
					  Workspace* workspace)
{	
	ui_component_init(&self->component, parent);		
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 0x00EEEEEE);
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSequenceListView(&self->listview, &self->component, 
		&workspace->song->sequence, &workspace->song->patterns);			
	ui_component_move(&self->listview.component, 0, 25);
	self->buttons.context = &self->listview;
	InitSequenceButtons(&self->buttons, &self->component);			
	self->buttons.controller.newentry = OnControllerNewEntry;
	self->buttons.controller.delentry = OnControllerDelEntry;
	self->buttons.controller.newtrack = OnControllerNewTrack;
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	signal_connect(&workspace->song->sequence.signal_editpositionchanged, self, OnEditPositionChanged);
}

void InitSequenceListView(SequenceListView* self, ui_component* parent,
						  Sequence* sequence, Patterns* patterns)
{				
	self->sequence = sequence;
	self->patterns = patterns;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);	
	self->selected = 0;
	self->selectedtrack = 0;
	self->font = ui_createfont("Tahoma", 12);
	self->lineheight = 12;
}

void InitSequenceButtons(SequenceButtons* self, ui_component* parent)
{
	self->font = ui_createfont("Tahoma", 12);	
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 0x00FFFFFF);
	ui_button_init(&self->newentry, &self->component);	
	ui_component_setfont(&self->newentry.component, &self->font);	
	signal_connect(&self->newentry.signal_clicked, self, OnNewEntry);	
	ui_component_resize(&self->newentry.component, 40, 20);
	ui_button_settext(&self->newentry, "New");
	ui_button_init(&self->delentry, &self->component);	
	ui_component_setfont(&self->delentry.component, &self->font);
	signal_connect(&self->delentry.signal_clicked, self, OnDelEntry);	
	ui_component_move(&self->delentry.component, 45, 0);
	ui_component_resize(&self->delentry.component, 30, 20);
	ui_button_settext(&self->delentry, "Del");	
	ui_button_init(&self->newtrack, &self->component);	
	ui_button_settext(&self->newtrack, "New Track");
	ui_component_move(&self->newtrack.component, 80, 0);
	ui_component_resize(&self->newtrack.component, 60, 20);	
	ui_component_setfont(&self->newtrack.component, &self->font);	
	signal_connect(&self->newtrack.signal_clicked, self, OnNewTrack);	

}

void OnDraw(SequenceListView* self, ui_component* sender, ui_graphics* g)
{		
	ui_setfont(g, &self->font);	
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
	ui_component_resize(&self->buttons.component, width, 25);
	ui_component_resize(&self->listview.component, width, height - 20);
}

void OnNewEntry(SequenceButtons* self, ui_component* sender) {
	self->controller.newentry(self->context);	
}

void OnDelEntry(SequenceButtons* self, ui_component* sender) {
	self->controller.delentry(self->context);	
}

void OnNewTrack(SequenceButtons* self, ui_component* sender) {
	self->controller.newtrack(self->context);	
}

void OnControllerNewEntry(SequenceListView* self)
{
	int slot;
	Pattern* pattern = (Pattern*) malloc(sizeof(Pattern));
	pattern_init(pattern);
	slot = patterns_append(self->patterns, pattern);
	sequence_insert(self->sequence, sequence_editposition(self->sequence), slot);	
}

void OnControllerDelEntry(SequenceListView* self)
{
	if (self->selected != -1) {
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

void OnControllerNewTrack(SequenceListView* self)
{
	SequenceTrack* track = (SequenceTrack*)malloc(sizeof(SequenceTrack));
	sequencetrack_init(track);
	sequence_appendtrack(self->sequence, track);
	ui_invalidate(&self->component);	
}

void OnMouseDown(SequenceListView* self, ui_component* sender, int x, int y, int button)
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
