// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepsequencerbar.h"
#include <exclusivelock.h>
#include <uibitmap.h>
#include "resources/resource.h"

#define TIMERID_STEPSEQUENCERVIEW 9000

static void stepsequencerbar_ondraw(StepsequencerBar*, psy_ui_Graphics*);
static void stepsequencerbar_drawbackground(StepsequencerBar*,
	psy_ui_Graphics*);
static void stepsequencerbar_drawstep(StepsequencerBar*, psy_ui_Graphics*,
	int step, int mode);
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
static int offsettoline(StepsequencerView*, psy_dsp_beat_t offset);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

extern psy_ui_Bitmap knobs;

static void vtable_init(StepsequencerBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.draw = (psy_ui_fp_draw)stepsequencerbar_ondraw;
	}
}

void stepsequencerbar_init(StepsequencerBar* self, psy_ui_Component* parent,
	StepsequencerView* view, Workspace* workspace)
{
	self->view = view;
	self->workspace = workspace;
	self->pattern = 0;	
	self->stepwidth = 30;
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->component.doublebuffered = 1;	
	psy_signal_connect(&self->component.signal_mousedown,
		self, stepsequencerbar_onmousedown);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		stepsequencerbar_oneditpositionchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, stepsequencerbar_onsequenceselectionchanged);	
	self->pattern = patterns_at(&workspace->song->patterns, 0);	
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
			line = offsettoline(self->view, entry->offset);
			if (entry->track == cursor.track) {
				if (line >= self->view->bar * 16 &&
						line < self->view->bar * 16 + 16) {
					stepsequencerbar_drawstep(self, g, line % 16, 2);
				}
			}			
			curr = curr->next;
		}
	}
	
	if (player_playing(&self->workspace->player)) {
		if (self->view->line >= self->view->bar * 16 &&
				self->view->line < self->view->bar * 16 + 16) {
			stepsequencerbar_drawstep(self, g, self->view->line % 16, 2);
		}
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

	cpx = step * self->stepwidth;
	ui_setrectangle(&r, cpx, 0, 20, 30);
	if (mode == 1) {
		ui_drawsolidroundrectangle(g, r, corner, 0x00666666);
	} else
	if (mode == 2) {
		ui_drawsolidroundrectangle(g, r, corner, 0x00999999);
	} else
	if (mode == 3) {
		ui_drawsolidroundrectangle(g, r, corner, 0x00FFFFFF);
	}
	if ((step % 4) == 0) {
		ui_setcolor(g, 0x00CACACA);
		ui_drawroundrectangle(g, r, corner);
	}
}

int offsettoline(StepsequencerView* self, psy_dsp_beat_t offset)
{		
	return (int)(offset * player_lpb(&self->workspace->player));
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
		self->view->sequenceentryoffset = entry->offset;		
	} else {
		self->pattern = 0;
		self->view->sequenceentryoffset = 0.f;		
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

		bpl = (psy_dsp_beat_t) 1 / player_lpb(&self->workspace->player);
		step = ev->x / self->stepwidth + self->view->bar * 16;

		cursor = workspace_patterneditposition(self->workspace);
		cursor.column = 0;	
		cursor.offset = step / (psy_dsp_beat_t) player_lpb(&self->workspace->player);		
		
		patternevent_clear(&event);
		event.note = 48;
		event.inst = (uint16_t) instruments_slot(
			&self->workspace->song->instruments);
		event.mach = (uint8_t) machines_slot(&self->workspace->song->machines);
		// event.cmd = GATE;
		// event.parameter = 0x80;
		stepsequencerbar_setdefaultevent(self,
			cursor.track,
			&self->workspace->player.patterndefaults, &event);
		node = pattern_findnode(self->pattern,
			cursor.track,
			(psy_dsp_beat_t) cursor.offset,
			(psy_dsp_beat_t) bpl, &prev);
		if (node) {								
			lock_enter();
			sequencer_checkiterators(&self->workspace->player.sequencer,
				node);
			pattern_remove(self->pattern, node);
			lock_leave();						
		} else {			
			node = pattern_insert(self->pattern,
				prev,
				cursor.track, 
				(psy_dsp_beat_t) cursor.offset,
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
		if (patternentry_front(entry)->note != NOTECOMMANDS_EMPTY) {
			event->note = patternentry_front(entry)->note;
		}
		if (patternentry_front(entry)->inst != NOTECOMMANDS_INST_EMPTY) {
			event->inst = patternentry_front(entry)->inst;
		}
		if (patternentry_front(entry)->mach != NOTECOMMANDS_MACH_EMPTY) {
			event->mach = patternentry_front(entry)->mach;
		}
		if (patternentry_front(entry)->vol != NOTECOMMANDS_VOL_EMPTY) {
			event->vol = patternentry_front(entry)->vol;
		}
		if (patternentry_front(entry)->cmd != 0) {
			event->cmd = patternentry_front(entry)->cmd;
		}
		if (patternentry_front(entry)->parameter != 0) {
			event->parameter = patternentry_front(entry)->parameter;
		}	
	}
}

void stepsequencerbar_oneditpositionchanged(StepsequencerBar* self,
	Workspace* sender)
{
	ui_component_invalidate(&self->component);
}

// stepsequencerbarselect
static void stepsequencerbarselect_ondraw(StepsequencerBarSelect*, psy_ui_Graphics*);
static void stepsequencerbarselect_onmousedown(StepsequencerBarSelect*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);

static psy_ui_ComponentVtable stepsequencerbarselect_vtable;
static int stepsequencerbarselect_vtable_initialized = 0;

static void stepsequencerbarselect_vtable_init(StepsequencerBarSelect* self)
{
	if (!stepsequencerbarselect_vtable_initialized) {
		stepsequencerbarselect_vtable = *(self->component.vtable);
		stepsequencerbarselect_vtable.draw =
			(psy_ui_fp_draw)stepsequencerbarselect_ondraw;
	}
}

void stepsequencerbarselect_init(StepsequencerBarSelect* self, psy_ui_Component* parent,
	StepsequencerView* view, Workspace* workspace)
{
	self->workspace = workspace;
	self->view = view;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	stepsequencerbarselect_vtable_init(self);
	self->component.vtable = &stepsequencerbarselect_vtable;
	self->component.doublebuffered = 1;
	psy_signal_connect(&self->component.signal_mousedown,
		self, stepsequencerbarselect_onmousedown);
	ui_component_resize(&self->component, 140, 30);
}

void stepsequencerbarselect_ondraw(StepsequencerBarSelect* self, psy_ui_Graphics* g)
{
	int i;
	int cpx;
	int cpy;
	ui_rectangle r;	
	int numbars;

	cpx = 0;
	cpy = 0;
	numbars = 4;
	if (self->view->stepsequencerbar.pattern) {
		numbars = (int)(pattern_length(self->view->stepsequencerbar.pattern) + 0.5) /
			player_lpb(&self->workspace->player);
	}

	for (i = 0; i < numbars; ++i) {
		if (i != 0 && (i % 4) == 0) {
			cpy += 15;
			cpx = 0;
		}
		ui_setrectangle(&r, cpx, cpy, 20, 10);
		if (i == self->view->bar) {
			ui_drawsolidrectangle(g, r, 0x00999999);
		}
		ui_drawrectangle(g, r);		
		cpx += 30;		
	}
}

void stepsequencerbarselect_onmousedown(StepsequencerBarSelect* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{	
	int row;
	int bar;

	row = ev->y / 15;
	bar = row * 4 + (ev->x / 30);	
	self->view->bar = bar;
	ui_component_invalidate(&self->component);
	ui_component_invalidate(&self->view->stepsequencerbar.component);	
}

// stepsequencerview
static void stepsequencerview_ontimer(StepsequencerView*,
	psy_ui_Component* sender, int timerid);
static void stepsequencerview_onsongchanged(StepsequencerView*, Workspace* sender);

void stepsequencerview_init(StepsequencerView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makeeh(1), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
	self->workspace = workspace;
	self->lastplayposition = 0;
	self->sequenceentryoffset = 0;
	self->line = 0;
	self->bar = 0;
	self->follow = 1;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	stepsequencerbarselect_init(&self->stepsequencerbarselect, &self->component,
		self, workspace);
	ui_component_setalign(&self->stepsequencerbarselect.component, UI_ALIGN_LEFT);
	ui_component_setmargin(&self->stepsequencerbarselect.component, &margin);
	stepsequencerbar_init(&self->stepsequencerbar, &self->component, self, workspace);
	ui_component_setalign(&self->stepsequencerbar.component, UI_ALIGN_CLIENT);
	ui_component_setmargin(&self->stepsequencerbar.component, &margin);
	psy_signal_connect(&workspace->signal_songchanged, self,
		stepsequencerview_onsongchanged);
	psy_signal_connect(&self->component.signal_timer, self,
		stepsequencerview_ontimer);	
	ui_component_starttimer(&self->component, TIMERID_STEPSEQUENCERVIEW, 50);
}

void stepsequencerview_ontimer(StepsequencerView* self, psy_ui_Component* sender,
	int timerid)
{
	if (timerid == TIMERID_STEPSEQUENCERVIEW && self->stepsequencerbar.pattern) {
		if (player_playing(&self->workspace->player)) {
			int oldline;

			oldline = self->line;
			self->lastplayposition = player_position(&self->workspace->player);			
			self->line = offsettoline(self, self->lastplayposition -
				self->sequenceentryoffset);
			if (self->line != oldline) {
				ui_component_invalidate(&self->stepsequencerbar.component);
				ui_component_invalidate(&self->stepsequencerbarselect.component);
				if (self->follow) {
					self->bar = self->line / 16;
				}
			}
		}
	}
}

void stepsequencerview_onsongchanged(StepsequencerView* self, Workspace* workspace)
{
	SequenceSelection selection;

	selection = workspace_sequenceselection(workspace);
	if (selection.editposition.trackposition.tracknode) {
		SequenceEntry* entry;

		entry = (SequenceEntry*)
			selection.editposition.trackposition.tracknode->entry;
		self->stepsequencerbar.pattern = patterns_at(&workspace->song->patterns,
			entry->pattern);
	} else {
		self->stepsequencerbar.pattern = 0;
	}
	self->sequenceentryoffset = 0.f;
	ui_component_invalidate(&self->component);
}
