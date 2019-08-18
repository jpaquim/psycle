// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sequenceview.h"
#include <stdio.h>

static void Draw(SequenceListView* self, ui_graphics* g);
static void DrawBackground(SequenceListView* self, ui_graphics* g);
static void DrawSequence(SequenceListView* self, ui_graphics* g);

static void OnControllerNewEntry(SequenceListView* self);
static void OnNewEntry(SequenceButtons* self);
void OnSize(SequenceView* self, int width, int height);
static void OnMouseDown(SequenceListView* self, int x, int y, int button);

void InitSequenceView(SequenceView* self, ui_component* parent, Sequence* sequence, Patterns* patterns)
{	
	ui_component_init(self, &self->component, parent);
	self->component.events.size = OnSize;
	InitSequenceListView(&self->listview, &self->component, sequence, patterns);			
	ui_component_move(&self->listview.component, 0, 20);	
	self->buttons.context = &self->listview;
	InitSequenceButtons(&self->buttons, &self->component);			
	self->buttons.controller.newentry = OnControllerNewEntry;
}

void InitSequenceListView(SequenceListView* self, ui_component* parent, Sequence* sequence, Patterns* patterns)
{				
	self->sequence = sequence;
	self->patterns = patterns;
	ui_component_init(self, &self->component, parent);
	self->component.events.draw = Draw;
	self->component.events.mousedown = OnMouseDown;
	self->selchanged = 0;
	self->selected = 0;
	self->skin.font = ui_createfont("Tahoma", 12);
	self->lineheight = 12;
}

void InitSequenceButtons(SequenceButtons* self, ui_component* parent)
{
	self->font = ui_createfont("Tahoma", 12);	
	ui_component_init(self, &self->component, parent);
	ui_button_init(&self->newentry, &self->component);
	ui_component_setfont(&self->newentry, self->font);
	ui_button_connect(&self->newentry, self);
	self->newentry.events.clicked = OnNewEntry;
	ui_component_resize(&self->newentry.component, 40, 20);
	ui_button_settext(&self->newentry, "New");
}

void Draw(SequenceListView* self, ui_graphics* g)
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

void OnSize(SequenceView* self, int width, int height)
{
	ui_component_resize(&self->buttons.component, width, 20);
	ui_component_resize(&self->listview.component, width, height - 20);
}

void OnNewEntry(SequenceButtons* self) {
	self->controller.newentry(self->context);	
}

void OnControllerNewEntry(SequenceListView* self)
{
	int slot;
	Pattern* pattern = (Pattern*) malloc(sizeof(Pattern));
	pattern_init(pattern);
	slot = patterns_append(self->patterns, pattern);
	sequence_append(self->sequence, slot);
	self->selected = sequence_size(self->sequence) - 1;
	ui_invalidate(&self->component);
	if (self->selchanged) {
		SequenceEntry* entry = sequence_at(self->sequence, self->selected);
		self->selchanged(self->selcontext, entry);
	}	
}

void OnMouseDown(SequenceListView* self, int x, int y, int button)
{
	unsigned int selected;

	selected = y / self->lineheight;
	if (selected < sequence_size(self->sequence)) {
		self->selected = selected;
	}
	ui_invalidate(&self->component);
	if (self->selchanged) {
		SequenceEntry* entry = sequence_at(self->sequence, self->selected);
		self->selchanged(self->selcontext, entry);
	}
}

void SequenceViewConnect(SequenceView* self, void* context)
{
	self->listview.selcontext = context;	
}