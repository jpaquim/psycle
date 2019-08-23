// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sequenceview.h"
#include <stdio.h>

static void OnDraw(SequenceListView* self, ui_component* sender, ui_graphics* g);
static void DrawBackground(SequenceListView* self, ui_graphics* g);
static void DrawSequence(SequenceListView* self, ui_graphics* g);

static void OnControllerNewEntry(SequenceListView* self);
static void OnControllerDelEntry(SequenceListView* self);
static void OnNewEntry(SequenceButtons* self, ui_component* sender);
static void OnDelEntry(SequenceButtons* self, ui_component* sender);
static void OnSize(SequenceView* self, ui_component* sender, int width, int height);
static void OnMouseDown(SequenceListView* self, ui_component* sender, int x, int y, int button);

void InitSequenceView(SequenceView* self, ui_component* parent, Sequence* sequence, Patterns* patterns)
{	
	ui_component_init(&self->component, parent);		
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSequenceListView(&self->listview, &self->component, sequence, patterns);			
	ui_component_move(&self->listview.component, 0, 20);	
	self->buttons.context = &self->listview;
	InitSequenceButtons(&self->buttons, &self->component);			
	self->buttons.controller.newentry = OnControllerNewEntry;
	self->buttons.controller.delentry = OnControllerDelEntry;
}

void InitSequenceListView(SequenceListView* self, ui_component* parent, Sequence* sequence, Patterns* patterns)
{				
	self->sequence = sequence;
	self->patterns = patterns;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);	
	self->selected = 0;
	self->skin.font = ui_createfont("Tahoma", 12);
	self->lineheight = 12;
}

void InitSequenceButtons(SequenceButtons* self, ui_component* parent)
{
	self->font = ui_createfont("Tahoma", 12);	
	ui_component_init(&self->component, parent);
	ui_button_init(&self->newentry, &self->component);	
	ui_component_setfont(&self->newentry.component, self->font);
	ui_button_connect(&self->newentry, self);
	signal_connect(&self->newentry.signal_clicked, self, OnNewEntry);	
	ui_component_resize(&self->newentry.component, 40, 20);
	ui_button_settext(&self->newentry, "New");
	ui_button_init(&self->delentry, &self->component);	
	ui_component_setfont(&self->delentry.component, self->font);
	ui_button_connect(&self->delentry, self);
	signal_connect(&self->delentry.signal_clicked, self, OnDelEntry);	
	ui_component_move(&self->delentry.component, 45, 0);
	ui_component_resize(&self->delentry.component, 40, 20);
	ui_button_settext(&self->delentry, "Del");

}

void OnDraw(SequenceListView* self, ui_component* sender, ui_graphics* g)
{	
	if (self->skin.font) {
		ui_setfont(g, self->skin.font);
	}
	DrawBackground(self, g);
	DrawSequence(self, g);
}

void DrawBackground(SequenceListView* self, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);	
	ui_drawsolidrectangle(g, r, 0x00FFFFFF);
}

void DrawSequence(SequenceListView* self, ui_graphics* g)
{
	List* ptr;
	unsigned int c;
	int cpy;
	char buffer[20];
	
	c = 0;
	cpy = 0;
	ui_settextcolor(g, 0);
	ptr = self->sequence->entries;
	while (ptr != 0) {
		SequenceEntry* entry;
		// Pattern* pattern;
		entry = (SequenceEntry*)ptr->node;
		//pattern = patterns_at(&self->sequence->patterns, entry->pattern);
		//if (pattern) {
			_snprintf(buffer,20, "%02X:%02X  %4.2f", c, entry->pattern, entry->offset);
			if (self->selected == (int)c) {
				ui_setbackgroundcolor(g, 0x00FF0000);
			} else {
				ui_setbackgroundcolor(g, 0xFFFFFFFF);
			}
			ui_textout(g, 0, cpy, buffer, strlen(buffer));
		//}
		ptr = ptr->next;
		cpy += self->lineheight;
		++c;
	}
}

void OnSize(SequenceView* self, ui_component* sender, int width, int height)
{
	ui_component_resize(&self->buttons.component, width, 20);
	ui_component_resize(&self->listview.component, width, height - 20);
}

void OnNewEntry(SequenceButtons* self, ui_component* sender) {
	self->controller.newentry(self->context);	
}

void OnDelEntry(SequenceButtons* self, ui_component* sender) {
	self->controller.delentry(self->context);	
}

void OnControllerNewEntry(SequenceListView* self)
{
	int slot;
	Pattern* pattern = (Pattern*) malloc(sizeof(Pattern));
	pattern_init(pattern);
	slot = patterns_append(self->patterns, pattern);
	sequence_insert(self->sequence, sequence_editposition(self->sequence), slot);
	++self->selected;
	ui_invalidate(&self->component);	
}

void OnControllerDelEntry(SequenceListView* self)
{
	if (self->selected != -1) {
		SequencePtr ptr = sequence_editposition(self->sequence);
		sequence_remove(self->sequence, ptr);
		if (sequence_size(self->sequence) == 0) {
			sequence_insert(self->sequence, sequence_begin(self->sequence, 0), 0);
		}
		if (self->selected >= (int)sequence_size(self->sequence)) {
			self->selected = sequence_size(self->sequence) - 1;
		}
		ui_invalidate(&self->component);
	}
}

void OnMouseDown(SequenceListView* self, ui_component* sender, int x, int y, int button)
{
	unsigned int selected;

	selected = y / self->lineheight;
	if (selected < sequence_size(self->sequence)) {
		self->selected = selected;
	}
	ui_invalidate(&self->component);		
	sequence_seteditposition(self->sequence, sequence_at(self->sequence, self->selected));
}
