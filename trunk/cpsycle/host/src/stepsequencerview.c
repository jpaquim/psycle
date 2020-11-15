// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepsequencerview.h"

#include <songio.h>
#include <exclusivelock.h>
#include "resources/resource.h"
#include "../../detail/portable.h"

// barselect
static void stepsequencerbarselect_ondestroy(StepsequencerBarSelect*,
	psy_ui_Component* sender);
static void stepsequencerbarselect_onpreferredsize(StepsequencerBarSelect*,
	psy_ui_Size* limit, psy_ui_Size* size);
static void stepsequencerbarselect_onlinetick(StepsequencerBarSelect*,
	StepTimer*);
static void stepsequencerbarselect_ondraw(StepsequencerBarSelect*,
	psy_ui_Graphics*);
static void stepsequencerbarselect_onmousedown(StepsequencerBarSelect*,
	psy_ui_MouseEvent*);
static void stepsequencerbarselect_onsize(StepsequencerBarSelect* self,
	psy_ui_Size*);
static void stepsequencerbarselect_setposition(StepsequencerBarSelect* self,
	StepSequencerPosition position);
static void stepsequencerbarselect_oneditpositionchanged(
	StepsequencerBarSelect*,
	Workspace* sender);

// bar
static void stepsequencerbar_ondraw(StepsequencerBar*, psy_ui_Graphics*);
static void stepsequencerbar_drawbackground(StepsequencerBar*,
	psy_ui_Graphics*);
static void stepsequencerbar_drawstep(StepsequencerBar*, psy_ui_Graphics*,
	int step, int mode);
static void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_MouseEvent*);
static void stepsequencerbar_setdefaultevent(StepsequencerBar*,
	uintptr_t track,
	psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent*);
static int offsettoline(psy_audio_Player*, psy_dsp_big_beat_t offset);
void stepsequencerbar_onsize(StepsequencerBar*, psy_ui_Size* size);
static void stepsequencerbar_onlpbchanged(StepsequencerBar*, psy_audio_Player* sender,
	uintptr_t lpb);
static void stepsequencerbar_setposition(StepsequencerBar* self,
	StepSequencerPosition position);
static void stepsequencerbar_oneditpositionchanged(StepsequencerBar*,
	Workspace* sender);
static void stepsequencerbar_onpreferredsize(StepsequencerBar*, psy_ui_Size* limit,
	psy_ui_Size* rv);

// bar vtable
static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(StepsequencerBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw)stepsequencerbar_ondraw;
		vtable.onsize = (psy_ui_fp_onsize) stepsequencerbar_onsize;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			stepsequencerbar_onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_onmousedown)
			stepsequencerbar_onmousedown;
	}
}

void stepsequencerbar_init(StepsequencerBar* self, psy_ui_Component* parent,
	StepTimer* steptimer, Workspace* workspace)
{	
	self->steptimer = steptimer;
	self->workspace = workspace;
	self->pattern = 0;	
	self->stepwidth = 25;
	self->stepheight = 25;
	stepsequencerposition_init(&self->position);	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);	
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		stepsequencerbar_onlpbchanged);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		stepsequencerbar_oneditpositionchanged);
}

void stepsequencerbar_onlinetick(StepsequencerBar* self,
	StepTimer* steptimer)
{
	if (workspace_followingsong(self->workspace)) {
		self->position = steptimer_position(steptimer);
	}
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbar_setposition(StepsequencerBar* self,
	StepSequencerPosition position)
{
	self->position = position;
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbar_ondraw(StepsequencerBar* self, psy_ui_Graphics* g)
{					
	stepsequencerbar_drawbackground(self, g);
	if (self->pattern) {
		psy_audio_PatternEditPosition cursor;
		psy_audio_PatternNode* curr;
				
		cursor = workspace_patterneditposition(self->workspace);
		curr = psy_audio_pattern_begin(self->pattern);
		while (curr) {			
			psy_audio_PatternEntry* entry;
			int line;
			
			entry = psy_audio_patternnode_entry(curr);
			line = offsettoline(&self->workspace->player, entry->offset);
			if (entry->track == cursor.track) {
				if (line >= self->position.steprow * 16 &&
					line < self->position.steprow * 16 + 16) {
					stepsequencerbar_drawstep(self, g, line % 16, 2);
				}
			}
			psy_audio_patternnode_next(&curr);
		}
	}
	if (psy_audio_player_playing(&self->workspace->player)) {
		if (self->steptimer->position.line >= self->position.steprow * 16 &&
				self->steptimer->position.line < self->position.steprow * 16 + 16) {
			stepsequencerbar_drawstep(self, g,
				self->steptimer->position.line % 16, 2);
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
	psy_ui_Rectangle r;
	psy_ui_Size corner;

	corner.width = psy_ui_value_makepx(5);
	corner.height = psy_ui_value_makepx(5);
	cpx = step * self->stepwidth;
	psy_ui_setrectangle(&r, cpx, 0, (int)(self->stepwidth * 0.8),
		self->stepheight);
	if (mode == 1) {
		psy_ui_drawsolidroundrectangle(g, r, corner, psy_ui_color_make(0x00666666));
	} else
	if (mode == 2) {
		psy_ui_drawsolidroundrectangle(g, r, corner, psy_ui_color_make(0x00999999));
	} else
	if (mode == 3) {
		psy_ui_drawsolidroundrectangle(g, r, corner, psy_ui_color_make(0x00FFFFFF));
	}
	if ((step % psy_audio_player_lpb(&self->workspace->player)) == 0) {
		psy_ui_setcolor(g, psy_ui_color_make(0x00CACACA));
		psy_ui_drawroundrectangle(g, r, corner);
	}
}

void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_MouseEvent* ev)
{
	if (self->pattern && self->workspace->song) {
		int step;
		psy_audio_PatternEvent event;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_PatternEditPosition cursor;
		psy_dsp_big_beat_t bpl;

		bpl = (psy_dsp_big_beat_t) 1 / psy_audio_player_lpb(&self->workspace->player);
		step = ev->x / self->stepwidth + self->position.steprow * 16;
		cursor = workspace_patterneditposition(self->workspace);
		cursor.column = 0;	
		cursor.offset = step / (psy_dsp_big_beat_t) psy_audio_player_lpb(
			&self->workspace->player);		
		psy_audio_patternevent_clear(&event);
		event.note = 48;
		event.inst = (uint16_t)psy_audio_instruments_selected(
			&self->workspace->song->instruments).subslot;
		event.mach = (uint8_t) psy_audio_machines_slot(&self->workspace->song->machines);
		// event.cmd = psy_audio_PATTERNCMD_GATE;
		// event.parameter = 0x80;
		stepsequencerbar_setdefaultevent(self,
			cursor.track,
			&self->workspace->player.patterndefaults, &event);
		node = psy_audio_pattern_findnode(self->pattern,
			cursor.track,
			(psy_dsp_big_beat_t) cursor.offset,
			(psy_dsp_big_beat_t) bpl, &prev);
		if (node) {								
			psy_audio_exclusivelock_enter();
			psy_audio_sequencer_checkiterators(
				&self->workspace->player.sequencer,
				node);
			psy_audio_pattern_remove(self->pattern, node);
			psy_audio_exclusivelock_leave();						
		} else {			
			node = psy_audio_pattern_insert(self->pattern,
				prev,
				cursor.track, 
				(psy_dsp_big_beat_t) cursor.offset,
				&event);		
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void stepsequencerbar_setdefaultevent(StepsequencerBar* self,
	uintptr_t track,
	psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent* event)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;	
				
	node = psy_audio_pattern_findnode(patterndefaults, track, 0, (psy_dsp_big_beat_t) 0.25f,
		&prev);
	if (node) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*) node->entry;
		if (psy_audio_patternentry_front(entry)->note != psy_audio_NOTECOMMANDS_EMPTY) {
			event->note = psy_audio_patternentry_front(entry)->note;
		}
		if (psy_audio_patternentry_front(entry)->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
			event->inst = psy_audio_patternentry_front(entry)->inst;
		}
		if (psy_audio_patternentry_front(entry)->mach != psy_audio_NOTECOMMANDS_MACH_EMPTY) {
			event->mach = psy_audio_patternentry_front(entry)->mach;
		}
		if (psy_audio_patternentry_front(entry)->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			event->vol = psy_audio_patternentry_front(entry)->vol;
		}
		if (psy_audio_patternentry_front(entry)->cmd != 0) {
			event->cmd = psy_audio_patternentry_front(entry)->cmd;
		}
		if (psy_audio_patternentry_front(entry)->parameter != 0) {
			event->parameter = psy_audio_patternentry_front(entry)->parameter;
		}	
	}
}

void stepsequencerbar_oneditpositionchanged(StepsequencerBar* self,
	Workspace* sender)
{
	psy_audio_PatternEditPosition cursor;
	StepSequencerPosition position;

	cursor = workspace_patterneditposition(self->workspace);
	position.line = offsettoline(&self->workspace->player,
		(psy_dsp_big_beat_t) cursor.offset);
	position.steprow = position.line / 16;
	stepsequencerbar_setposition(self, position);
}

void stepsequencerview_oneditpositionchanged(StepsequencerView* self,
	Workspace* sender)
{
	psy_audio_PatternEditPosition cursor;
	StepSequencerPosition position;

	cursor = workspace_patterneditposition(self->workspace);
	position.line = offsettoline(&self->workspace->player,
		(psy_dsp_big_beat_t) cursor.offset);
	position.steprow = position.line / 16;
	stepsequencerbar_setposition(&self->stepsequencerbar, position);
	stepsequencerbarselect_setposition(&self->stepsequencerbarselect,
		position);	
}

void stepsequencerbar_setpattern(StepsequencerBar* self,
	psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	stepsequencerposition_init(&self->position);
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbar_onlpbchanged(StepsequencerBar* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbar_onsize(StepsequencerBar* self, psy_ui_Size* size)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->stepwidth = tm.tmAveCharWidth * 4;
	if (self->stepwidth > psy_ui_value_px(&size->width, &tm) / 16) {
		self->stepwidth = psy_ui_value_px(&size->width, &tm) / 16;
	}
	self->stepheight = (int)(tm.tmHeight * 1.5);
}

void stepsequencerbar_onpreferredsize(StepsequencerBar* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	rv->height = psy_ui_value_makeeh(1.5);
	rv->width = psy_ui_value_makeew(4 * 16);
}

// barselect
static psy_ui_ComponentVtable stepsequencerbarselect_vtable;
static int stepsequencerbarselect_vtable_initialized = 0;

static void stepsequencerbarselect_vtable_init(StepsequencerBarSelect* self)
{
	if (!stepsequencerbarselect_vtable_initialized) {
		stepsequencerbarselect_vtable = *(self->component.vtable);
		stepsequencerbarselect_vtable.ondraw =
			(psy_ui_fp_ondraw) stepsequencerbarselect_ondraw;
		stepsequencerbarselect_vtable.onsize = (psy_ui_fp_onsize) stepsequencerbarselect_onsize;
		stepsequencerbarselect_vtable.onpreferredsize =
			(psy_ui_fp_onpreferredsize)stepsequencerbarselect_onpreferredsize;				
		stepsequencerbarselect_vtable.onmousedown = (psy_ui_fp_onmousedown)
			stepsequencerbarselect_onmousedown;
	}
}

void stepsequencerbarselect_init(StepsequencerBarSelect* self,
	psy_ui_Component* parent,
	StepTimer* steptimer,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	stepsequencerbarselect_vtable_init(self);
	self->component.vtable = &stepsequencerbarselect_vtable;
	self->steptimer = steptimer;
	self->workspace = workspace;	
	self->pattern = 0;
	self->lineheight = 15;
	self->colwidth = 25;
	stepsequencerposition_init(&self->position);	
	psy_ui_component_doublebuffer(&self->component);
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&self->component.signal_destroy,
		self, stepsequencerbarselect_ondestroy);	
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		stepsequencerbarselect_oneditpositionchanged);	
}

void stepsequencerbarselect_ondestroy(StepsequencerBarSelect* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selected);
}

void stepsequencerbarselect_onlinetick(StepsequencerBarSelect* self,
	StepTimer* steptimer)
{
	if (workspace_followingsong(self->workspace)) {
		self->position = steptimer_position(steptimer);
	}
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbarselect_setposition(StepsequencerBarSelect* self,
	StepSequencerPosition position)
{
	self->position = position;
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbarselect_oneditpositionchanged(StepsequencerBarSelect* self,
	Workspace* sender)
{
	psy_audio_PatternEditPosition cursor;
	StepSequencerPosition position;

	cursor = workspace_patterneditposition(self->workspace);
	position.line = offsettoline(&self->workspace->player,
		(psy_dsp_big_beat_t) cursor.offset);
	position.steprow = position.line / 16;
	stepsequencerbarselect_setposition(self, position);	
}

void stepsequencerbarselect_ondraw(StepsequencerBarSelect* self, psy_ui_Graphics* g)
{
	int i;
	int cpx;
	int cpy;
	psy_ui_Rectangle r_outter;
	psy_ui_Rectangle r_inner;
	int numsteprows;

	cpx = 0;
	cpy = 0;
	numsteprows = 4;
	if (self->pattern) {
		numsteprows = (int)(psy_audio_pattern_length(self->pattern) *
			psy_audio_player_lpb(&self->workspace->player) / 16 + 0.5f);
	}
	for (i = 0; i < numsteprows; ++i) {
		if (i != 0 && (i % 4) == 0) {
			cpy += self->lineheight;
			cpx = 0;
		}
		psy_ui_setrectangle(&r_inner,
			cpx + self->colwidth / 2 - (int)(self->colwidth * 0.2),
			cpy + self->lineheight / 2 - (int)(self->lineheight * 0.2),
			(int)(self->colwidth * 0.4),
			(int)(self->lineheight * 0.4));
		r_outter = r_inner;
		r_outter.left -= 3;
		r_outter.right += 3;
		r_outter.top -= 3;
		r_outter.bottom += 3;
		if (i == self->position.steprow) {
			psy_ui_drawsolidrectangle(g, r_outter, psy_ui_color_make(0x00444444));
			psy_ui_drawsolidrectangle(g, r_inner, psy_ui_color_make(0x00D1E8D0));
		} else
		if ((i == self->position.steprow - 1) && ((self->position.line % 16)  < 1)) {
			psy_ui_drawsolidrectangle(g, r_outter, psy_ui_color_make(0x00333333));
			psy_ui_drawsolidrectangle(g, r_inner, psy_ui_color_make(0x00D1C5B6));
		} else {
			psy_ui_drawsolidrectangle(g, r_inner, psy_ui_color_make(0x00A19586));
		}
		cpx += self->colwidth;
	}
}

void stepsequencerbarselect_onsize(StepsequencerBarSelect* self,
	psy_ui_Size* size)
{
	psy_ui_TextMetric tm;	

	tm = psy_ui_component_textmetric(&self->component);	
	self->colwidth = tm.tmAveCharWidth * 4;
	self->lineheight = (int)(0.8 * tm.tmHeight);	
}

void stepsequencerbarselect_onpreferredsize(StepsequencerBarSelect* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	rv->width = psy_ui_value_makeew(16);
	rv->height = psy_ui_value_makeeh(1.6);
}

void stepsequencerbarselect_onmousedown(StepsequencerBarSelect* self,
	psy_ui_MouseEvent* ev)
{	
	int row;
	int steprow;

	row = ev->y / self->lineheight;
	steprow = row * 4 + (ev->x / self->colwidth);
	self->position.steprow = steprow;
	psy_ui_component_invalidate(&self->component);
	psy_signal_emit(&self->signal_selected, self, 0);		
}

void stepsequencerbarselect_setpattern(StepsequencerBarSelect* self,
	psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	stepsequencerposition_init(&self->position);
	psy_ui_component_invalidate(&self->component);
}

// stepsequencerview
// view
static void stepsequencerview_ondestroy(StepsequencerView*, psy_ui_Component* sender);
static void stepsequencerview_ontimer(StepsequencerView*, uintptr_t timerid);
static void stepsequencerview_onsongchanged(StepsequencerView*,
	Workspace* sender, int flag, psy_audio_SongFile* songfile);
static void stepsequencerview_onsteprowselected(StepsequencerView*,
	psy_ui_Component* sender);
static void stepsequencerview_setpattern(StepsequencerView*,
	psy_audio_Pattern*);
static void stepsequencerview_onsequenceselectionchanged(StepsequencerView*,
	Workspace* sender);
static psy_ui_ComponentVtable stepsequencerview_vtable;
static int stepsequencerview_vtable_initialized = 0;
// vtable
static void stepsequencerview_vtable_init(StepsequencerView* self)
{
	if (!stepsequencerview_vtable_initialized) {
		stepsequencerview_vtable = *(self->component.vtable);
		stepsequencerview_vtable.ontimer = (psy_ui_fp_ontimer)
			stepsequencerview_ontimer;
		stepsequencerview_vtable_initialized = 1;
	}
}
// implementation
void stepsequencerview_init(StepsequencerView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5), psy_ui_value_makeew(2.0));
	self->workspace = workspace;	
	psy_ui_component_init(&self->component, parent);
	stepsequencerview_vtable_init(self);
	self->component.vtable = &stepsequencerview_vtable;
	stepsequencerbarselect_init(&self->stepsequencerbarselect, &self->component,
		&self->steptimer, workspace);
	psy_ui_component_setalign(&self->stepsequencerbarselect.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->stepsequencerbarselect.component, &margin);
	stepsequencerbar_init(&self->stepsequencerbar, &self->component,
		&self->steptimer, workspace);
	stepsequencerview_setpattern(self, psy_audio_patterns_at(&workspace->song->patterns,
		0));
	psy_ui_component_setalign(&self->stepsequencerbar.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->stepsequencerbar.component, &margin);
	psy_signal_connect(&workspace->signal_songchanged, self,
		stepsequencerview_onsongchanged);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		stepsequencerview_oneditpositionchanged);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, stepsequencerview_onsequenceselectionchanged);
	psy_signal_connect(&self->stepsequencerbarselect.signal_selected,
		self, stepsequencerview_onsteprowselected);		
	steptimer_init(&self->steptimer, &workspace->player);
	psy_signal_connect(&self->steptimer.signal_linetick,
		&self->stepsequencerbar,
		stepsequencerbar_onlinetick);
	psy_signal_connect(&self->steptimer.signal_linetick,
		&self->stepsequencerbarselect,
		stepsequencerbarselect_onlinetick);
	psy_signal_connect(&self->stepsequencerbar.component.signal_destroy,
		self, stepsequencerview_ondestroy);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void stepsequencerview_ondestroy(StepsequencerView* self, psy_ui_Component* sender)
{
	psy_ui_component_stoptimer(&self->component, 0);
	steptimer_dispose(&self->steptimer);
}

void stepsequencerview_ontimer(StepsequencerView* self, uintptr_t timerid)
{
	steptimer_tick(&self->steptimer);	
}

void stepsequencerview_onsequenceselectionchanged(StepsequencerView* self,
	Workspace* workspace)
{
	psy_audio_SequenceSelection selection;
	psy_audio_SequenceEntry* entry;
	psy_audio_Pattern* pattern;

	selection = workspace_sequenceselection(workspace);
	entry = psy_audio_sequenceposition_entry(&selection.editposition);	
	if (entry) {
		pattern = psy_audio_patterns_at(&workspace->song->patterns,
			entry->patternslot);
	} else {		
		pattern = 0;
	}
	stepsequencerview_setpattern(self, pattern);
	steptimer_reset(&self->steptimer, entry ? entry->offset : 0);	
}

void stepsequencerview_onsongchanged(StepsequencerView* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	psy_audio_SequenceSelection selection;	
	psy_audio_Pattern* pattern;

	selection = workspace_sequenceselection(workspace);
	if (selection.editposition.trackposition.tracknode) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)
			selection.editposition.trackposition.tracknode->entry;
		pattern = psy_audio_patterns_at(&workspace->song->patterns, entry->patternslot);
	} else {
		pattern = 0;
	}
	stepsequencerbar_setpattern(&self->stepsequencerbar, pattern);
	stepsequencerbarselect_setpattern(&self->stepsequencerbarselect, pattern);		
	steptimer_reset(&self->steptimer, 0);
}

void stepsequencerview_setpattern(StepsequencerView* self,
	psy_audio_Pattern* pattern)
{
	stepsequencerbar_setpattern(&self->stepsequencerbar, pattern);
	stepsequencerbarselect_setpattern(&self->stepsequencerbarselect,
		pattern);	
}

void stepsequencerview_onsteprowselected(StepsequencerView* self,
	psy_ui_Component* sender)
{
	stepsequencerbar_setposition(&self->stepsequencerbar,
		self->stepsequencerbarselect.position);	
}

void stepsequencerposition_init(StepSequencerPosition* self)
{
	self->line = 0;
	self->steprow = 0;	
}

// steptimer

static void steptimer_onnewline(StepTimer*, psy_audio_Sequencer* sender);

void steptimer_init(StepTimer* self, psy_audio_Player* player)
{
	self->player = player;	
	self->sequenceentryoffset = 0;
	self->doseqtick = 0;
	stepsequencerposition_init(&self->position);
	psy_signal_init(&self->signal_linetick);
	psy_signal_connect(&self->player->sequencer.signal_newline,
		self, steptimer_onnewline);
}

void steptimer_dispose(StepTimer* self)
{
	psy_signal_dispose(&self->signal_linetick);
}

// ui thread
void steptimer_tick(StepTimer* self)
{
	if (self->doseqtick) {
		self->doseqtick = 0;
		self->position.line = offsettoline(self->player,
			psy_audio_player_position(self->player) - self->sequenceentryoffset);
		self->position.steprow = self->position.line / 16;			
		psy_signal_emit(&self->signal_linetick, self, 0);				
	}
}

// audio thread
void steptimer_onnewline(StepTimer* self, psy_audio_Sequencer* sender)
{	
	if (psy_audio_sequencer_playing(sender)) {
		self->doseqtick = 1;
	}
}

int offsettoline(psy_audio_Player* player, psy_dsp_big_beat_t offset)
{		
	return (int)(offset * psy_audio_player_lpb(player));
}

StepSequencerPosition steptimer_position(StepTimer* self)
{
	return self->position;
}

void steptimer_reset(StepTimer* self, psy_dsp_big_beat_t entryoffset)
{
	stepsequencerposition_init(&self->position);
	self->sequenceentryoffset = entryoffset;
	self->doseqtick = 0;
}
