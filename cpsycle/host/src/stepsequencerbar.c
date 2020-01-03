// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepsequencerbar.h"
#include <exclusivelock.h>

#define TIMERID_STEPSEQUENCERBAR 9000

static void stepsequencerbar_ondraw(StepsequencerBar*, psy_ui_Graphics*);
static void stepsequencerbar_drawbackground(StepsequencerBar*,
	psy_ui_Graphics*);
static void stepsequencerbar_drawstep(StepsequencerBar*, psy_ui_Graphics*,
	int step, int mode);
static void stepsequencerbar_ontimer(StepsequencerBar*,
	psy_ui_Component* sender, int timerid);
static void stepsequencerbar_onsongchanged(StepsequencerBar*, Workspace* sender);
static void stepsequencerbar_oneditpositionchanged(StepsequencerBar*,
	Workspace* sender);
static void stepsequencerbar_onsequenceselectionchanged(StepsequencerBar*,
	Workspace*);
static void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void stepsequencerbar_setdefaultevent(StepsequencerBar*,
	uintptr_t track,
	psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent*);
static int offsettoline(psy_dsp_beat_t offset);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(StepsequencerBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.draw = (psy_ui_fp_draw)stepsequencerbar_ondraw;
	}
}

void stepsequencerbar_init(StepsequencerBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	self->pattern = 0;
	self->lastplayposition = 0;
	self->sequenceentryoffset = 0;
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->component.doublebuffered = 1;
	ui_component_resize(&self->component, 200, 35);
	psy_signal_connect(&self->component.signal_mousedown,
		self, stepsequencerbar_onmousedown);
	psy_signal_connect(&workspace->signal_songchanged, self,
		stepsequencerbar_onsongchanged);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		stepsequencerbar_oneditpositionchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, stepsequencerbar_onsequenceselectionchanged);	
	psy_signal_connect(&self->component.signal_timer, self,
		stepsequencerbar_ontimer);
	self->pattern = patterns_at(&workspace->song->patterns, 0);
	ui_component_starttimer(&self->component, TIMERID_STEPSEQUENCERBAR, 50);
}

void stepsequencerbar_ondraw(StepsequencerBar* self, psy_ui_Graphics* g)
{					
	stepsequencerbar_drawbackground(self, g);
	if (self->pattern) {
		PatternEditPosition cursor;
		PatternNode* curr;
				
		cursor = workspace_patterneditposition(self->workspace);
		curr = self->pattern->events;
		while (curr) {			
			psy_audio_PatternEntry* entry;			
			int line;
			
			entry = (psy_audio_PatternEntry*)(curr->entry);
			if (entry->track == cursor.track) {
				line = offsettoline(entry->offset);			
				stepsequencerbar_drawstep(self, g, line % 16, 2);
			}			
			curr = curr->next;
		}
	}
	
	if (player_playing(&self->workspace->player)) {
		stepsequencerbar_drawstep(self, g, offsettoline(
			self->lastplayposition - self->sequenceentryoffset) % 16, 2);
	}
	
}

void stepsequencerbar_drawbackground(StepsequencerBar* self, psy_ui_Graphics* g)
{
	int i;

	for (i = 0; i < 16; ++i) {
		stepsequencerbar_drawstep(self, g, i, 1);
	}
}

void stepsequencerbar_drawstep(StepsequencerBar* self, psy_ui_Graphics* g,
	int step, int mode)
{
	int cpx;
	ui_rectangle r;
	ui_size corner = { 5, 5 };

	cpx = step * 40;
	ui_setrectangle(&r, cpx, 5, 20, 30);
	if (mode == 1) {
		ui_drawsolidroundrectangle(g, r, corner, 0x00666666);
	} else
	if (mode == 2) {
		ui_drawsolidroundrectangle(g, r, corner, 0x00999999);
	} else
	if (mode == 3) {
		ui_drawsolidroundrectangle(g, r, corner, 0x00FFFFFF);
	}
}

int offsettoline(psy_dsp_beat_t offset)
{	
	return (int)(offset * 4);	
}

void stepsequencerbar_onsequenceselectionchanged(StepsequencerBar* self,
	Workspace* workspace)
{
	SequenceSelection selection;
	SequenceEntry* entry;

	selection = workspace_sequenceselection(workspace);
	entry = sequenceposition_entry(&selection.editposition);
	if (entry) {
		psy_audio_Pattern* pattern;

		pattern = patterns_at(&workspace->song->patterns,
			entry->pattern);
		self->pattern = pattern;
		self->sequenceentryoffset = entry->offset;		
	} else {
		self->pattern = 0;
		self->sequenceentryoffset = 0.f;		
	}
	ui_component_invalidate(&self->component);
}

void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (self->pattern && self->workspace->song) {
		int step;
		psy_audio_PatternEvent event;
		PatternNode* node;
		PatternNode* prev;
		PatternEditPosition cursor;
		psy_dsp_beat_t bpl;

		bpl = (psy_dsp_beat_t) 0.25f;
		step = ev->x /40;

		cursor = workspace_patterneditposition(self->workspace);
		cursor.column = 0;	
		cursor.offset = step * 0.25;		
		
		patternevent_clear(&event);
		event.note = 48;
		event.inst = (uint16_t) instruments_slot(
			&self->workspace->song->instruments);
		event.mach = (uint8_t) machines_slot(&self->workspace->song->machines);
		stepsequencerbar_setdefaultevent(self,
			cursor.track,
			&self->workspace->player.patterndefaults, &event);
		node = pattern_findnode(self->pattern,
			cursor.track,
			(psy_dsp_beat_t)cursor.offset,
			(psy_dsp_beat_t)bpl, &prev);
		if (node) {		
			if (ev->button == 1) {
				pattern_setevent(self->pattern, node, &event);
			} else
			if (ev->button == 2) {
				lock_enter();
				pattern_remove(self->pattern, node);
				lock_leave();
			}			
		} else {
			node = pattern_insert(self->pattern,
				prev,
				cursor.track, 
				(psy_dsp_beat_t)cursor.offset,
				&event);		
		}
		ui_component_invalidate(&self->component);
	}
}

void stepsequencerbar_setdefaultevent(StepsequencerBar* self,
	uintptr_t track,
	psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent* event)
{
	PatternNode* node;
	PatternNode* prev;
				
	node = pattern_findnode(patterndefaults,
		track,
		0,
		(psy_dsp_beat_t)0.25f,
		&prev);
	if (node) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*) node->entry;
		if (event->inst != NOTECOMMANDS_INST_EMPTY) {
			event->inst = patternentry_front(entry)->inst;
		}
		if (event->mach != NOTECOMMANDS_MACH_EMPTY) {
			event->mach = patternentry_front(entry)->mach;
		}
	}
}

void stepsequencerbar_ontimer(StepsequencerBar* self, psy_ui_Component* sender,
	int timerid)
{
	if (timerid == TIMERID_STEPSEQUENCERBAR && self->pattern) {
		if (player_playing(&self->workspace->player)) {			
			ui_component_invalidate(&self->component);
			self->lastplayposition = player_position(&self->workspace->player);
		}		
	}
}

void stepsequencerbar_oneditpositionchanged(StepsequencerBar* self,
	Workspace* sender)
{
	ui_component_invalidate(&self->component);
}

void stepsequencerbar_onsongchanged(StepsequencerBar* self, Workspace* workspace)
{
	SequenceSelection selection;	
	
	selection = workspace_sequenceselection(workspace);	
	if (selection.editposition.trackposition.tracknode) {
		SequenceEntry* entry;

		entry = (SequenceEntry*)
			selection.editposition.trackposition.tracknode->entry;
		self->pattern = patterns_at(&workspace->song->patterns,
			entry->pattern);	
	} else {
		self->pattern = 0;
	}
	self->sequenceentryoffset = 0.f;	
	ui_component_invalidate(&self->component);	
}