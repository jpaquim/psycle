// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepsequencerview.h"

// host
#include "styles.h"
// audio
#include <songio.h>
#include <exclusivelock.h>
// platform
#include "../../detail/portable.h"

static int offsettoline(psy_audio_Player* player, psy_dsp_big_beat_t offset)
{
	return (int)(offset * psy_audio_player_lpb(player));
}

// StepSequencerTile
// prototypes
static void stepsequencertile_onpreferredsize(StepSequencerTile*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable stepsequencertile_vtable;
static bool stepsequencertile_vtable_initialized = FALSE;

static void stepsequencertile_vtableinit_init(StepSequencerTile* self)
{
	if (!stepsequencertile_vtable_initialized) {
		stepsequencertile_vtable = *(self->component.vtable);
		stepsequencertile_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			stepsequencertile_onpreferredsize;
		stepsequencertile_vtable_initialized = TRUE;
	}
	self->component.vtable = &stepsequencertile_vtable;
}
// implementation
void stepsequencertile_init(StepSequencerTile* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_component_init(&self->component, parent, view);
	stepsequencertile_vtableinit_init(self);
	stepsequencertile_turnoff(self);
}

StepSequencerTile* stepsequencertile_alloc(void)
{
	return (StepSequencerTile*)malloc(sizeof(StepSequencerTile));
}

StepSequencerTile* stepsequencertile_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view)
{
	StepSequencerTile* rv;

	rv = stepsequencertile_alloc();
	if (rv) {
		stepsequencertile_init(rv, parent, view);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void stepsequencertile_turnon(StepSequencerTile* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00CACACA));
}

void stepsequencertile_turnoff(StepSequencerTile* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00292929));
}

void stepsequencertile_play(StepSequencerTile* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x009F7B00));
}

void stepsequencertile_onpreferredsize(StepSequencerTile* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_size_setem(rv, 3.0, 2.0);
}

// StepsequencerBar
// prototypes
static void stepsequencerbar_ondestroy(StepsequencerBar*);
static void stepsequencerbar_build(StepsequencerBar*);
static void stepsequencerbar_update(StepsequencerBar*);
static void stepsequencerbar_onmousedown(StepsequencerBar*,
	psy_ui_MouseEvent*);
static void stepsequencerbar_setdefaultevent(StepsequencerBar*,
	uintptr_t track,
	psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent*);
static void stepsequencerbar_onlpbchanged(StepsequencerBar*,
	psy_audio_Player* sender, uintptr_t lpb);
static void stepsequencerbar_setposition(StepsequencerBar*,
	StepSequencerPosition position);
static void stepsequencerbar_oneditpositionchanged(StepsequencerBar*,
	Workspace* sender);
static void stepsequencerbar_turnoffall(StepsequencerBar*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(StepsequencerBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			stepsequencerbar_ondestroy;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			stepsequencerbar_onmousedown;
		vtable_initialized = TRUE;
	}
}
// implementation
void stepsequencerbar_init(StepsequencerBar* self, psy_ui_Component* parent,
	StepTimer* steptimer, Workspace* workspace)
{	
	self->steptimer = steptimer;
	self->workspace = workspace;
	self->pattern = NULL;	
	stepsequencerposition_init(&self->position);	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_makeem(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		stepsequencerbar_onlpbchanged);
	psy_signal_connect(&workspace->signal_patterncursorchanged, self,
		stepsequencerbar_oneditpositionchanged);
	stepsequencerbar_build(self);
	psy_table_init(&self->tiles);
}

void stepsequencerbar_ondestroy(StepsequencerBar* self)
{
	psy_table_dispose(&self->tiles);
}

void stepsequencerbar_onlinetick(StepsequencerBar* self,
	StepTimer* steptimer)
{
	if (workspace_followingsong(self->workspace)) {
		self->position = steptimer_position(steptimer);
	}
	stepsequencerbar_update(self);	
}

void stepsequencerbar_setposition(StepsequencerBar* self,
	StepSequencerPosition position)
{
	self->position = position;
	stepsequencerbar_update(self);
}

void stepsequencerbar_build(StepsequencerBar* self)
{	
	uintptr_t i;

	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->tiles);
	for (i = 0; i < 16; ++i) {
		StepSequencerTile* tile;

		tile = stepsequencertile_allocinit(&self->component, &self->component);
		psy_table_insert(&self->tiles, i, (void*)tile);
	}
	psy_ui_component_align(&self->component);
}

void stepsequencerbar_turnoffall(StepsequencerBar* self)
{
	uintptr_t i;
		
	for (i = 0; i < 16; ++i) {
		StepSequencerTile* tile;

		tile = (StepSequencerTile*)psy_table_at(&self->tiles, i);
		if (tile) {
			stepsequencertile_turnoff(tile);
		}
	}	
}

void stepsequencerbar_update(StepsequencerBar* self)
{					
	stepsequencerbar_turnoffall(self);	
	if (self->pattern) {
		psy_audio_PatternCursor cursor;
		psy_audio_PatternNode* curr;
				
		cursor = workspace_patterncursor(self->workspace);
		curr = psy_audio_pattern_begin(self->pattern);
		while (curr) {			
			psy_audio_PatternEntry* entry;
			int line;
			
			entry = psy_audio_patternnode_entry(curr);
			line = offsettoline(workspace_player(self->workspace), entry->offset);
			if (entry->track == cursor.track) {
				if (line >= self->position.steprow * 16 &&
					line < self->position.steprow * 16 + 16) {
					StepSequencerTile* tile;

					tile = (StepSequencerTile*)psy_table_at(&self->tiles, line % 16);
					if (tile) {
						stepsequencertile_turnon(tile);
					}					
				}
			}
			psy_audio_patternnode_next(&curr);
		}
	}	
	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		if (self->steptimer->position.line >= self->position.steprow * 16 &&
			self->steptimer->position.line < self->position.steprow * 16 + 16) {
			StepSequencerTile* tile;

			tile = (StepSequencerTile*)psy_table_at(&self->tiles,
				self->steptimer->position.line % 16);
			if (tile) {
				stepsequencertile_play(tile);
			}
		}
	}
	psy_ui_component_invalidate(&self->component);
}

void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_MouseEvent* ev)
{
	if (self->pattern && workspace_song(self->workspace)) {
		intptr_t step;
		psy_audio_PatternEvent event;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_PatternCursor cursor;
		psy_dsp_big_beat_t bpl;
		psy_ui_Value width;
		double stepwidth;

		width = psy_ui_value_makeew(4.0);
		stepwidth = psy_ui_value_px(&width,
			psy_ui_component_textmetric(&self->component));
		bpl = (psy_dsp_big_beat_t) 1 / psy_audio_player_lpb(workspace_player(self->workspace));
		step = (intptr_t)(ev->pt.x / stepwidth + self->position.steprow * 16);
		cursor = workspace_patterncursor(self->workspace);
		cursor.column = 0;	
		cursor.offset = step / (psy_dsp_big_beat_t) psy_audio_player_lpb(
			workspace_player(self->workspace));		
		psy_audio_patternevent_clear(&event);
		event.note = 48;
		event.inst = (uint16_t)psy_audio_instruments_selected(
			&workspace_song(self->workspace)->instruments).subslot;
		event.mach = (uint8_t) psy_audio_machines_selected(&workspace_song(self->workspace)->machines);
		// event.cmd = psy_audio_PATTERNCMD_GATE;
		// event.parameter = 0x80;
		stepsequencerbar_setdefaultevent(self,
			cursor.track,
			&workspace_player(self->workspace)->patterndefaults, &event);
		node = psy_audio_pattern_findnode(self->pattern,
			cursor.track, cursor.offset, bpl, &prev);
		if (node) {								
			psy_audio_exclusivelock_enter();
			psy_audio_sequencer_checkiterators(
				&workspace_player(self->workspace)->sequencer,
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
		stepsequencerbar_update(self);
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
		if (psy_audio_patternentry_front(entry)->mach != psy_audio_NOTECOMMANDS_psy_audio_EMPTY) {
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
	psy_audio_PatternCursor cursor;
	StepSequencerPosition position;

	cursor = workspace_patterncursor(self->workspace);
	position.line = offsettoline(workspace_player(self->workspace),
		(psy_dsp_big_beat_t) cursor.offset);
	position.steprow = position.line / 16;
	stepsequencerbar_setposition(self, position);
}

void stepsequencerbar_setpattern(StepsequencerBar* self,
	psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	stepsequencerposition_init(&self->position);
	stepsequencerbar_build(self);
	stepsequencerbar_update(self);	
}

void stepsequencerbar_onlpbchanged(StepsequencerBar* self,
	psy_audio_Player* sender, uintptr_t lpb)
{
	stepsequencerbar_update(self);	
}

// StepsequencerBarSelect
// prototypes
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
static void stepsequencerbarselect_onsize(StepsequencerBarSelect*,
	psy_ui_Size*);
static void stepsequencerbarselect_setposition(StepsequencerBarSelect*,
	StepSequencerPosition position);
static void stepsequencerbarselect_oneditpositionchanged(
	StepsequencerBarSelect*,
	Workspace* sender);


// barselect
static psy_ui_ComponentVtable stepsequencerbarselect_vtable;
static bool stepsequencerbarselect_vtable_initialized = FALSE;

static void stepsequencerbarselect_vtable_init(StepsequencerBarSelect* self)
{
	if (!stepsequencerbarselect_vtable_initialized) {
		stepsequencerbarselect_vtable = *(self->component.vtable);
		stepsequencerbarselect_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			stepsequencerbarselect_ondraw;
		stepsequencerbarselect_vtable.onsize =
			(psy_ui_fp_component_onsize)
			stepsequencerbarselect_onsize;
		stepsequencerbarselect_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			stepsequencerbarselect_onpreferredsize;				
		stepsequencerbarselect_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			stepsequencerbarselect_onmousedown;
	}
}

void stepsequencerbarselect_init(StepsequencerBarSelect* self,
	psy_ui_Component* parent,
	StepTimer* steptimer,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
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
	psy_signal_connect(&workspace->signal_patterncursorchanged, self,
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
	psy_audio_PatternCursor cursor;
	StepSequencerPosition position;

	cursor = workspace_patterncursor(self->workspace);
	position.line = offsettoline(workspace_player(self->workspace),
		(psy_dsp_big_beat_t) cursor.offset);
	position.steprow = position.line / 16;
	stepsequencerbarselect_setposition(self, position);	
}

void stepsequencerbarselect_ondraw(StepsequencerBarSelect* self, psy_ui_Graphics* g)
{
	int i;
	int cpx;
	int cpy;
	psy_ui_RealRectangle r_outter;
	psy_ui_RealRectangle r_inner;
	int numsteprows;

	cpx = 0;
	cpy = 0;
	numsteprows = 4;
	if (self->pattern) {
		numsteprows = (int)(psy_audio_pattern_length(self->pattern) *
			psy_audio_player_lpb(workspace_player(self->workspace)) / 16 + 0.5f);
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
			psy_ui_drawsolidrectangle(g, r_outter, psy_ui_colour_make(0x00444444));
			psy_ui_drawsolidrectangle(g, r_inner, psy_ui_colour_make(0x00D1E8D0));
		} else
		if ((i == self->position.steprow - 1) && ((self->position.line % 16)  < 1)) {
			psy_ui_drawsolidrectangle(g, r_outter, psy_ui_colour_make(0x00333333));
			psy_ui_drawsolidrectangle(g, r_inner, psy_ui_colour_make(0x00D1C5B6));
		} else {
			psy_ui_drawsolidrectangle(g, r_inner, psy_ui_colour_make(0x00A19586));
		}
		cpx += self->colwidth;
	}
}

void stepsequencerbarselect_onsize(StepsequencerBarSelect* self,
	psy_ui_Size* size)
{
	const psy_ui_TextMetric* tm;	

	tm = psy_ui_component_textmetric(&self->component);	
	self->colwidth = tm->tmAveCharWidth * 4;
	self->lineheight = (int)(0.8 * tm->tmHeight);	
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
	intptr_t row;
	intptr_t steprow;

	row = (intptr_t)(ev->pt.y / self->lineheight);
	steprow = (intptr_t)(row * 4 + (ev->pt.x / self->colwidth));
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

// StepSequencerView
// prototypes
static void stepsequencerview_ondestroy(StepsequencerView*, psy_ui_Component* sender);
static void stepsequencerview_ontimer(StepsequencerView*, uintptr_t timerid);
static void stepsequencerview_onsongchanged(StepsequencerView*,
	Workspace* sender, int flag, psy_audio_Song* song);
static void stepsequencerview_onsteprowselected(StepsequencerView*,
	psy_ui_Component* sender);
static void stepsequencerview_setpattern(StepsequencerView*,
	psy_audio_Pattern*);
static void stepsequencerview_onsequenceselectionchanged(StepsequencerView*,
	psy_audio_SequenceSelection* sender);
static void stepsequencerview_oneditpositionchanged(StepsequencerView*,
	Workspace* sender);
// vtable
static psy_ui_ComponentVtable stepsequencerview_vtable;
static bool stepsequencerview_vtable_initialized = FALSE;

static void stepsequencerview_vtable_init(StepsequencerView* self)
{
	if (!stepsequencerview_vtable_initialized) {
		stepsequencerview_vtable = *(self->component.vtable);
		stepsequencerview_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			stepsequencerview_ontimer;
		stepsequencerview_vtable_initialized = TRUE;
	}
}
// implementation
void stepsequencerview_init(StepsequencerView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all_em(&margin, 0.5, 0.0, 0.5, 2.0);		
	self->workspace = workspace;	
	psy_ui_component_init(&self->component, parent, NULL);
	stepsequencerview_vtable_init(self);
	self->component.vtable = &stepsequencerview_vtable;
	psy_ui_component_setstyletypes(&self->component,
		STYLE_STEPSEQUENCER, STYLE_STEPSEQUENCER, STYLE_STEPSEQUENCER);
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
	psy_signal_connect(&workspace->signal_patterncursorchanged, self,
		stepsequencerview_oneditpositionchanged);
	psy_signal_connect(&workspace->sequenceselection.signal_changed,
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
	if (psy_ui_component_visible(&self->component)) {
		steptimer_tick(&self->steptimer);
	}
}

void stepsequencerview_onsequenceselectionchanged(StepsequencerView* self,
	psy_audio_SequenceSelection* sender)
{	
	psy_audio_Pattern* pattern;
	psy_audio_SequenceEntry* entry;

	if (self->workspace->song) {
		pattern = psy_audio_sequence_pattern(&self->workspace->song->sequence,
			self->workspace->sequenceselection.editposition);
		entry = psy_audio_sequence_entry(&self->workspace->song->sequence,
			self->workspace->sequenceselection.editposition);
	} else {
		pattern = NULL;
		entry = NULL;
	}	
	stepsequencerview_setpattern(self, pattern);
	steptimer_reset(&self->steptimer, entry ? entry->offset : 0);	
}

void stepsequencerview_onsongchanged(StepsequencerView* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{	
	psy_audio_Pattern* pattern;
	
	pattern = (workspace->song)
		? psy_audio_sequence_pattern(&workspace->song->sequence,
			self->workspace->sequenceselection.editposition)
		: NULL;	
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

void stepsequencerview_oneditpositionchanged(StepsequencerView* self,
	Workspace* sender)
{
	psy_audio_PatternCursor cursor;
	StepSequencerPosition position;

	cursor = workspace_patterncursor(self->workspace);
	position.line = offsettoline(workspace_player(self->workspace),
		(psy_dsp_big_beat_t)cursor.offset);
	position.steprow = position.line / 16;
	stepsequencerbar_setposition(&self->stepsequencerbar, position);
	stepsequencerbarselect_setposition(&self->stepsequencerbarselect,
		position);
}

// StepSequencerPosition
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

