/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "stepsequencerview.h"
#include "patterncmds.h"

/* host */
#include "styles.h"
/* audio */
#include <songio.h>
#include <exclusivelock.h>

static uintptr_t offsettoline(psy_audio_Player* player, psy_dsp_big_beat_t offset)
{
	return (uintptr_t)(offset * (psy_dsp_big_beat_t)psy_audio_player_lpb(player));
}

/* StepSequencerPosition */
void stepsequencerposition_init(StepSequencerPosition* self)
{
	assert(self);

	self->line = 0;
	self->steprow = 0;	
	self->seqentryline = 0;
}

StepSequencerPosition stepsequencerposition_make(uintptr_t line,
	uintptr_t seqentryline, uintptr_t numtiles)
{
	StepSequencerPosition rv;

	rv.line = line;
	rv.steprow = line / numtiles;
	rv.seqentryline = seqentryline;
	return rv;
}

/* StepSequencerState */

/* prototypes */
static bool stepsequencerstate_updatepattern(StepSequencerState*);

/* implementation */
void stepsequencerstate_init(StepSequencerState* self, Workspace* workspace)
{
	assert(self);
	assert(workspace);

	self->numtiles = 16;
	stepsequencerposition_init(&self->editposition);
	stepsequencerposition_init(&self->playposition);
	self->pattern = NULL;
	self->workspace = workspace;
	self->barbuttonindex = psy_INDEX_INVALID;	
}

/*
** Updates the stepsequencer state with the workspace cursor and the player.
** Called by the stepsequencerview when song, cursor or playline changes
*/
bool stepsequencerstate_update_positions(StepSequencerState* self)
{
	const psy_audio_SequencerTime* seqtime;
	psy_audio_SequenceCursor cursor;
	psy_dsp_big_beat_t seqoffset;

	assert(self);
	
	/* update stepsequencer editposition */
	cursor = self->workspace->song->sequence.cursor;
	seqoffset = 0.0;
	if (workspace_song(self->workspace)) {
		psy_audio_sequencecursor_updateseqoffset(&cursor,
			&self->workspace->song->sequence);
		seqoffset = psy_audio_sequencecursor_seqoffset(&cursor);		
	}
	self->editposition = stepsequencerposition_make(
		offsettoline(workspace_player(self->workspace),
			cursor.offset),
		offsettoline(workspace_player(self->workspace), seqoffset),
		self->numtiles);
	seqtime = psy_audio_player_sequencertime(workspace_player(
		self->workspace));
	/* update stepsequencer playposition */
	self->playposition = stepsequencerposition_make(
		seqtime->linecounter - self->editposition.seqentryline,
		self->editposition.seqentryline, self->numtiles);	
	return stepsequencerstate_updatepattern(self);
}

/*
** Checks and updates the current pattern.
** Called by stepsequencerstate_update_positions
** return true if current pattern changed
*/
bool stepsequencerstate_updatepattern(StepSequencerState* self)
{
	psy_audio_Pattern* pattern;

	assert(self);

	pattern = NULL;
	if (workspace_song(self->workspace)) {
		psy_audio_SequenceCursor cursor;

		cursor = self->workspace->song->sequence.cursor;
		pattern = psy_audio_sequence_pattern(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			psy_audio_sequencecursor_orderindex(&cursor));
	}
	if (pattern != self->pattern) {
		self->pattern = pattern;
		return TRUE;
	}
	return FALSE;
}

/* StepSequencerTile */

/* implementation */
void stepsequencertile_init(StepSequencerTile* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);	
	psy_ui_component_setstyletype(&self->component, STYLE_STEPSEQUENCER_TILE);
	psy_ui_component_setstyletype_select(&self->component,
		STYLE_STEPSEQUENCER_TILE_SELECT);		
	psy_ui_component_setstyletype_active(&self->component,
		STYLE_STEPSEQUENCER_TILE_ACTIVE);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(3.0, 2.0));	
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
		psy_ui_component_deallocateafterdestroyed(
			stepsequencertile_base(rv));		
	}
	return rv;
}

void stepsequencertile_turnon(StepSequencerTile* self)
{
	assert(self);

	psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
}

void stepsequencertile_turnoff(StepSequencerTile* self)
{
	assert(self);

	psy_ui_component_removestylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);	
}

void stepsequencertile_play(StepSequencerTile* self)
{
	assert(self);

	psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_ACTIVE);
}

void stepsequencertile_resetplay(StepSequencerTile* self)
{
	assert(self);

	psy_ui_component_removestylestate(&self->component,
		psy_ui_STYLESTATE_ACTIVE);
}

/* StepsequencerBar */

/* prototypes */
static void stepsequencerbar_ondestroy(StepsequencerBar*);
static void stepsequencerbar_build(StepsequencerBar*);
static void stepsequencerbar_update(StepsequencerBar*);
static void stepsequencerbar_onmousedown(StepsequencerBar*,
psy_ui_MouseEvent*);
static void stepsequencerbar_setdefaultevent(StepsequencerBar*,
	uintptr_t track, psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent*);
static void stepsequencerbar_onlpbchanged(StepsequencerBar*,
	psy_audio_Player* sender, uintptr_t lpb);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(StepsequencerBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			stepsequencerbar_ondestroy;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			stepsequencerbar_onmousedown;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void stepsequencerbar_init(StepsequencerBar* self, psy_ui_Component* parent,
	StepSequencerState* state)
{	
	assert(self);

	self->state = state;	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_signal_connect(&workspace_player(state->workspace)->signal_lpbchanged,
		self, stepsequencerbar_onlpbchanged);
	psy_table_init(&self->tiles);
	stepsequencerbar_build(self);
}

void stepsequencerbar_ondestroy(StepsequencerBar* self)
{
	assert(self);

	psy_table_dispose(&self->tiles);
}

void stepsequencerbar_build(StepsequencerBar* self)
{	
	uintptr_t i;

	assert(self);

	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->tiles);
	for (i = 0; i < self->state->numtiles; ++i) {
		StepSequencerTile* tile;

		tile = stepsequencertile_allocinit(&self->component, &self->component);
		psy_table_insert(&self->tiles, i, (void*)tile);
	}
	psy_ui_component_align(&self->component);
}

void stepsequencerbar_update(StepsequencerBar* self)
{	
	assert(self);

	if (self->state->pattern) {				
		psy_audio_SequenceCursor cursor;
		psy_audio_PatternNode* curr;
		const psy_audio_SequencerTime* seqtime;
		uintptr_t playline;
		uintptr_t i;
		uintptr_t line;
				
		cursor = self->state->workspace->song->sequence.cursor;
		curr = psy_audio_pattern_begin(self->state->pattern);
		seqtime = psy_audio_player_sequencertime(workspace_player(
			self->state->workspace));
		playline = seqtime->linecounter - self->state->editposition.seqentryline;
		line = self->state->editposition.steprow * self->state->numtiles;
		for (i = 0; i < self->state->numtiles; ++i, ++line) {
			StepSequencerTile* tile;

			tile = (StepSequencerTile*)psy_table_at(&self->tiles, i);
			if (tile) {
				if (psy_audio_player_playing(
						workspace_player(self->state->workspace)) &&
					(line == playline)) {
					stepsequencertile_play(tile);
				} else {
					stepsequencertile_resetplay(tile);
				}
				stepsequencertile_turnoff(tile);
			}
		}
		while (curr) {						
			StepSequencerTile* tile;
			psy_audio_PatternEntry* entry;			
			uintptr_t currstep;		
			
			entry = psy_audio_patternnode_entry(curr);
			line = offsettoline(workspace_player(self->state->workspace),
				entry->offset);
			currstep = line % self->state->numtiles;
			tile = NULL;
			if (entry->track == cursor.track) {
				uintptr_t start;

				start = self->state->editposition.steprow * self->state->numtiles;
				if (line >= start && line < start + self->state->numtiles) {
					tile = (StepSequencerTile*)psy_table_at(&self->tiles,
						currstep);
					if (tile) {
						stepsequencertile_turnon(tile);
					}
				}
			}
			psy_audio_patternnode_next(&curr);
		}							
	}	
}

void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_MouseEvent* ev)
{	
	assert(self);

	if (self->state->pattern && workspace_song(self->state->workspace)) {
		intptr_t step;		
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_SequenceCursor cursor;
		psy_dsp_big_beat_t bpl;
		psy_ui_Value width;
		double stepwidth;

		width = psy_ui_value_make_ew(4.0);
		stepwidth = psy_ui_value_px(&width,
			psy_ui_component_textmetric(&self->component), NULL);
		bpl = (psy_dsp_big_beat_t) 1 / psy_audio_player_lpb(workspace_player(
			self->state->workspace));
		step = (intptr_t)(ev->pt.x / stepwidth + self->state->editposition.steprow * self->state->numtiles);
		cursor = self->state->workspace->song->sequence.cursor;
		cursor.column = 0;	
		cursor.offset = step / (psy_dsp_big_beat_t) psy_audio_player_lpb(
			workspace_player(self->state->workspace));		
		node = psy_audio_pattern_findnode(self->state->pattern,
			cursor.track, cursor.offset, bpl, &prev);
		psy_audio_exclusivelock_enter();
		if (node) {			
			psy_undoredo_execute(&self->state->workspace->undoredo,
				&removecommand_alloc(self->state->pattern,
					1.0 / (double)cursor.lpb,
					cursor, self->state->workspace)->command);
		} else {
			psy_audio_PatternEvent event;

			psy_audio_patternevent_clear(&event);
			event.note = 48;
			event.inst = (uint16_t)psy_audio_instruments_selected(
				&workspace_song(self->state->workspace)->instruments).subslot;
			event.mach = (uint8_t)psy_audio_machines_selected(&workspace_song(
				self->state->workspace)->machines);
			/* event.cmd = psy_audio_PATTERNCMD_GATE;
			   event.parameter = 0x80; */
			stepsequencerbar_setdefaultevent(self,
				cursor.track,
				&workspace_player(self->state->workspace)->patterndefaults, &event);
			psy_undoredo_execute(&self->state->workspace->undoredo,
				&insertcommand_alloc(self->state->pattern,
					1.0 / (double)cursor.lpb,					
					cursor, event, self->state->workspace)->command);			
		}
		psy_audio_exclusivelock_leave();		
	}
}

void stepsequencerbar_setdefaultevent(StepsequencerBar* self, uintptr_t track,
	psy_audio_Pattern* patterndefaults, psy_audio_PatternEvent* rv)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;	

	assert(self);
				
	node = psy_audio_pattern_findnode(patterndefaults, track, 0,
		(psy_dsp_big_beat_t)0.25f, &prev);
	if (node) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent defaultevent;

		entry = (psy_audio_PatternEntry*)node->entry;
		assert(psy_audio_patternentry_front(entry));
		defaultevent = *psy_audio_patternentry_front(entry);
		if (defaultevent.note != psy_audio_NOTECOMMANDS_EMPTY) {
			rv->note = defaultevent.note;
		}
		if (defaultevent.inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
			rv->inst = defaultevent.inst;
		}
		if (defaultevent.mach != psy_audio_NOTECOMMANDS_psy_audio_EMPTY) {
			rv->mach = defaultevent.mach;
		}
		if (defaultevent.vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			rv->vol = defaultevent.vol;
		}
		if (defaultevent.cmd != 0) {
			rv->cmd = defaultevent.cmd;
		}
		if (defaultevent.parameter != 0) {
			rv->parameter = defaultevent.parameter;
		}	
	}
}

void stepsequencerbar_onlpbchanged(StepsequencerBar* self,
	psy_audio_Player* sender, uintptr_t lpb)
{
	assert(self);

	stepsequencerbar_update(self);	
}


/* StepSequencerBarButton */

/* prototypes */

static void stepsequencerbarbutton_onmousedown(StepSequencerBarButton*,
	psy_ui_MouseEvent*);
static void stepsequencerbarbutton_ondraw(StepSequencerBarButton*,
	psy_ui_Graphics*);

/* vtable */
static psy_ui_ComponentVtable stepsequencerbarbutton_vtable;
static bool stepsequencerbarbutton_vtable_initialized = FALSE;

static void stepsequencerbarbutton_vtable_init(StepSequencerBarButton* self)
{
	if (!stepsequencerbarbutton_vtable_initialized) {
		stepsequencerbarbutton_vtable = *self->component.vtable;
		stepsequencerbarbutton_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			stepsequencerbarbutton_onmousedown;
		stepsequencerbarbutton_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			stepsequencerbarbutton_ondraw;
		stepsequencerbarbutton_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &stepsequencerbarbutton_vtable);
}

/* implementation */
void stepsequencerbarbutton_init(StepSequencerBarButton* self, psy_ui_Component* parent,
	psy_ui_Component* view, StepSequencerState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);
	stepsequencerbarbutton_vtable_init(self);
	self->state = state;
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(3.0, 1.0));
}

StepSequencerBarButton* stepsequencerbarbutton_alloc(void)
{
	return (StepSequencerBarButton*)malloc(sizeof(StepSequencerBarButton));
}

StepSequencerBarButton* stepsequencerbarbutton_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	StepSequencerState* state)
{
	StepSequencerBarButton* rv;

	rv = stepsequencerbarbutton_alloc();
	if (rv) {
		stepsequencerbarbutton_init(rv, parent, view, state);
		psy_ui_component_deallocateafterdestroyed(
			stepsequencerbarbutton_base(rv));
	}
	return rv;
}

void stepsequencerbarbutton_onmousedown(StepSequencerBarButton* self,
	psy_ui_MouseEvent* ev)
{
	self->state->barbuttonindex = self->index;
}

void stepsequencerbarbutton_ondraw(StepSequencerBarButton* self,
	psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;	
	psy_ui_RealRectangle r_outter;
	psy_ui_RealRectangle r_inner;	

	assert(self);
	
	size = psy_ui_component_size_px(&self->component);		
	psy_ui_setrectangle(&r_inner,
		size.width / 2 - (int)(size.width * 0.2),
		size.height / 2 - (int)(size.height * 0.2),
		(int)(size.width * 0.4),
		(int)(size.height * 0.4));
	r_outter = r_inner;
	r_outter.left -= 3;
	r_outter.right += 3;
	r_outter.top -= 3;
	r_outter.bottom += 3;
	if (self->index == self->state->editposition.steprow) {
		psy_ui_drawsolidrectangle(g, r_outter,
			psy_ui_colour_make(0x00444444));
		psy_ui_drawsolidrectangle(g, r_inner,
			psy_ui_colour_make(0x00D1E8D0));
	} else if ((self->index == self->state->editposition.steprow - 1) &&
		((self->state->editposition.line % self->state->numtiles) < 1)) {
		psy_ui_drawsolidrectangle(g, r_outter,
			psy_ui_colour_make(0x00333333));
		psy_ui_drawsolidrectangle(g, r_inner,
			psy_ui_colour_make(0x00D1C5B6));
	} else {
		psy_ui_drawsolidrectangle(g, r_inner,
			psy_ui_colour_make(0x00A19586));
	}	
}

/* StepsequencerBarSelect */
/* prototypes */
static void stepsequencerbarselect_ondestroy(StepsequencerBarSelect*);
static void stepsequencerbarselect_onmousedown(StepsequencerBarSelect*,
	psy_ui_MouseEvent*);
static void stepsequencerbarselect_build(StepsequencerBarSelect*);

/* vtable */
static psy_ui_ComponentVtable stepsequencerbarselect_vtable;
static bool stepsequencerbarselect_vtable_initialized = FALSE;

static void stepsequencerbarselect_vtable_init(StepsequencerBarSelect* self)
{
	if (!stepsequencerbarselect_vtable_initialized) {
		stepsequencerbarselect_vtable = *(self->component.vtable);
		stepsequencerbarselect_vtable.ondestroy =
			(psy_ui_fp_component_event)
			stepsequencerbarselect_ondestroy;
		stepsequencerbarselect_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			stepsequencerbarselect_onmousedown;
		stepsequencerbarselect_vtable_initialized = TRUE;
	}
	self->component.vtable = &stepsequencerbarselect_vtable;
}

/* implementation */
void stepsequencerbarselect_init(StepsequencerBarSelect* self,
	psy_ui_Component* parent, StepSequencerState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, parent);	
	stepsequencerbarselect_vtable_init(self);
	self->state = state;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	 psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());	
	psy_table_init(&self->buttons);
	stepsequencerbarselect_build(self);
}

void stepsequencerbarselect_ondestroy(StepsequencerBarSelect* self)
{
	assert(self);

	psy_table_dispose(&self->buttons);
}

void stepsequencerbarselect_build(StepsequencerBarSelect* self)
{
	uintptr_t i;
	int numsteprows;
	psy_ui_Component* row;

	assert(self);
		
	numsteprows = 4;
	if (self->state->pattern) {
		numsteprows = (int)(psy_audio_pattern_length(self->state->pattern) *
			psy_audio_player_lpb(workspace_player(self->state->workspace)) /
			self->state->numtiles + 0.5f);
	}	
	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->buttons);
	row = NULL;
	for (i = 0; i < numsteprows; ++i) {
		StepSequencerBarButton* button;
		
		if ((i % 4) == 0) {
			row = psy_ui_component_allocinit(&self->component, NULL);
			psy_ui_component_setalignexpand(row, psy_ui_HEXPAND);
			psy_ui_component_setdefaultalign(row, psy_ui_ALIGN_LEFT,
				psy_ui_margin_make_em(0.0, 1.0, 0.1, 0.0));
		}
		if (row) {
			button = stepsequencerbarbutton_allocinit(row, NULL, self->state);
			button->index = i;
			psy_ui_component_setalign(&button->component, psy_ui_ALIGN_LEFT);
			psy_table_insert(&self->buttons, i, (void*)button);
		}
	}
	psy_ui_component_align(&self->component);	
}

void stepsequencerbarselect_onmousedown(StepsequencerBarSelect* self,
	psy_ui_MouseEvent* ev)
{	
	assert(self);

	if (self->state->barbuttonindex != psy_INDEX_INVALID) {
		psy_audio_SequenceCursor cursor;		
		
		cursor = self->state->workspace->song->sequence.cursor;
		cursor.offset =
			(double)(self->state->barbuttonindex * self->state->numtiles) /
			(double)cursor.lpb +
			(double)self->state->editposition.seqentryline /
			(double)cursor.lpb;
		if (self->state->workspace && workspace_song(self->state->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->state->workspace)),
				cursor);
		}
	}
	self->state->barbuttonindex = psy_INDEX_INVALID;
}

/* StepSequencerView */

/* prototypes */
static void stepsequencerview_connectworkspace(StepsequencerView*, Workspace*);
static void stepsequencerview_onplaylinechanged(StepsequencerView*,
	Workspace* sender);
static void stepsequencerview_onsongchanged(StepsequencerView*,
	Workspace* sender, int flag);
static void stepsequencerview_oncursorchanged(StepsequencerView*,
	psy_audio_Sequence* sender);
static void stepsequencerview_update(StepsequencerView*);
static void stepsequencerview_connectpattern(StepsequencerView*);
static void stepsequencerview_onpatternlengthchanged(StepsequencerView*,
	psy_audio_Pattern* sender);

/* implementation */
void stepsequencerview_init(StepsequencerView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);

	psy_ui_component_init(&self->component, parent, NULL);		
	psy_ui_component_setstyletype(&self->component, STYLE_STEPSEQUENCER);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 2.0));
	stepsequencerstate_init(&self->state, workspace);
	stepsequencerbarselect_init(&self->stepsequencerbarselect,
		&self->component, &self->state);	
	psy_ui_component_init(&self->tilerow, &self->component, &self->component);
	stepsequencerbar_init(&self->stepsequencerbar, &self->tilerow,
		&self->state);
	psy_ui_component_setalign(&self->stepsequencerbar.component,
		psy_ui_ALIGN_TOP);
	stepsequencerview_connectworkspace(self, workspace);
	stepsequencerview_update(self);
}

void stepsequencerview_connectworkspace(StepsequencerView* self,
	Workspace* workspace)
{
	assert(self);
	assert(workspace);

	psy_signal_connect(&workspace->signal_songchanged, self,
		stepsequencerview_onsongchanged);
	if (workspace->song) {
		psy_signal_connect(&workspace->song->sequence.signal_cursorchanged, self,
			stepsequencerview_oncursorchanged);
	}
	psy_signal_connect(&workspace->signal_playlinechanged,
		self, stepsequencerview_onplaylinechanged);	
}

void stepsequencerview_onplaylinechanged(StepsequencerView* self,
	Workspace* sender)
{
	assert(self);

	if (psy_ui_component_visible(&self->component)) {
		stepsequencerview_update(self);
	}
}

void stepsequencerview_oncursorchanged(StepsequencerView* self,
	psy_audio_Sequence* sender)
{
	assert(self);

	stepsequencerview_update(self);
}

void stepsequencerview_onsongchanged(StepsequencerView* self, Workspace*
	workspace, int flag)
{	
	assert(self);

	if (workspace->song) {
		psy_signal_connect(&workspace->song->sequence.signal_cursorchanged, self,
			stepsequencerview_oncursorchanged);
	}
	stepsequencerview_update(self);
}

void stepsequencerview_update(StepsequencerView* self)
{		
	assert(self);

	if (stepsequencerstate_update_positions(&self->state)) {
		stepsequencerview_connectpattern(self);
		stepsequencerbarselect_build(&self->stepsequencerbarselect);
		psy_ui_component_align(psy_ui_component_parent(&self->component));
	}
	stepsequencerbar_update(&self->stepsequencerbar);
	psy_ui_component_invalidate(&self->stepsequencerbarselect.component);
}

void stepsequencerview_connectpattern(StepsequencerView* self)
{
	assert(self);

	if (self->state.pattern && workspace_song(self->state.workspace) &&
			!psy_signal_connected(&self->state.pattern->signal_lengthchanged,
				self, stepsequencerview_connectpattern)) {
		psy_signal_connect(&self->state.pattern->signal_lengthchanged,
			self, stepsequencerview_onpatternlengthchanged);
	}
}

void stepsequencerview_onpatternlengthchanged(StepsequencerView* self,
	psy_audio_Pattern* sender)
{
	assert(self);

	stepsequencerbarselect_build(&self->stepsequencerbarselect);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}
