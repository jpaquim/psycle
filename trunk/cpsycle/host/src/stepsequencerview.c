/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "stepsequencerview.h"
#include "patterncmds.h"
#include "styles.h"
/* audio */
#include <songio.h>
#include <exclusivelock.h>

/* StepSequencerState */

/* prototypes */
static bool stepsequencerstate_updatepattern(StepSequencerState*);

/* implementation */
void stepsequencerstate_init(StepSequencerState* self, Workspace* workspace)
{
	assert(self);
	assert(workspace);

	self->numtiles = 16;	
	self->workspace = workspace;
	self->barbuttonindex = psy_INDEX_INVALID;	
}

psy_audio_Pattern* stepsequencerstate_pattern(StepSequencerState* self)
{
	assert(self);

	if (!self->workspace->song) {
		return NULL;
	}
	return psy_audio_sequence_pattern(&self->workspace->song->sequence,
		self->workspace->song->sequence.cursor.orderindex);	
}

/*
** Checks and updates the current pattern.
** Called by stepsequencerstate_update_positions
** return true if current pattern changed
*/
bool stepsequencerstate_updatepattern(StepSequencerState* self)
{
	psy_audio_Song* song;

	song = self->workspace->song;
	if (!song) {
		return FALSE;
	}
	return !psy_audio_orderindex_equal(
		&song->sequence.cursor.orderindex,
		song->sequence.lastcursor.orderindex);	
}

/* StepSequencerTile */

/* implementation */
void stepsequencertile_init(StepSequencerTile* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);	
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

StepSequencerTile* stepsequencertile_allocinit(psy_ui_Component* parent)
{
	StepSequencerTile* rv;

	rv = stepsequencertile_alloc();
	if (rv) {
		stepsequencertile_init(rv, parent);
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
static void stepsequencerbar_update_playline(StepsequencerBar* self);
static void stepsequencerbar_onmousedown(StepsequencerBar*,
psy_ui_MouseEvent*);
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
		psy_table_insert(&self->tiles, i, (void*)
			stepsequencertile_allocinit(&self->component));
	}
	psy_ui_component_align(&self->component);
}

void stepsequencerbar_update(StepsequencerBar* self)
{		
	psy_audio_Song* song;

	assert(self);

	song = workspace_song(self->state->workspace);
	if (song) {	
		psy_audio_Pattern* pattern;
		psy_audio_SequenceCursor cursor;
		psy_audio_PatternNode* curr;		
		uintptr_t i;
		uintptr_t line;
		uintptr_t steprow;
		HostSequencerTime host_time;

		host_time = self->state->workspace->host_sequencer_time;
		cursor = song->sequence.cursor;		
		steprow = psy_audio_sequencecursor_line_pattern(&cursor) / self->state->numtiles;
		line = steprow * self->state->numtiles;
		for (i = 0; i < self->state->numtiles; ++i, ++line) {
			StepSequencerTile* tile;

			tile = (StepSequencerTile*)psy_table_at(&self->tiles, i);
			if (tile) {
				if (host_time.currplaying && line == host_time.currplayline) {
					stepsequencertile_play(tile);
				} else {
					stepsequencertile_resetplay(tile);
				}
				stepsequencertile_turnoff(tile);
			}
		}		
		pattern = psy_audio_sequence_pattern(&song->sequence, cursor.orderindex);
		if (!pattern) {
			return;
		}
		curr = psy_audio_pattern_begin(pattern);
		while (curr) {						
			StepSequencerTile* tile;
			psy_audio_PatternEntry* entry;			
			uintptr_t currstep;		
			
			entry = psy_audio_patternnode_entry(curr);
			line = (uintptr_t)(cursor.lpb * entry->offset);
			currstep = line % self->state->numtiles;
			tile = NULL;
			if (entry->track == cursor.track) {
				uintptr_t start;

				start = steprow * self->state->numtiles;
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

void stepsequencerbar_update_playline(StepsequencerBar* self)
{
	psy_audio_Song* song;

	assert(self);

	song = workspace_song(self->state->workspace);
	if (song) {		
		psy_audio_SequenceCursor cursor;		
		uintptr_t linestart;
		uintptr_t steprow;
		HostSequencerTime host_time;
		StepSequencerTile* tile;

		host_time = self->state->workspace->host_sequencer_time;
		cursor = song->sequence.cursor;		
		steprow = psy_audio_sequencecursor_line_pattern(&cursor) / self->state->numtiles;
		linestart = steprow * self->state->numtiles;
		tile = (StepSequencerTile*)psy_table_at(&self->tiles,
			host_time.currplayline -
			psy_audio_sequencecursor_seqline(&song->sequence.cursor) -
			linestart);
		if (tile) {
			stepsequencertile_play(tile);
		}
		tile = (StepSequencerTile*)psy_table_at(&self->tiles,
			host_time.lastplayline -
			psy_audio_sequencecursor_seqline(&song->sequence.cursor) -
			linestart);
		if (tile) {
			stepsequencertile_resetplay(tile);
		}
	}
}

void stepsequencerbar_onmousedown(StepsequencerBar* self,
	psy_ui_MouseEvent* ev)
{	
	psy_audio_Song* song;

	assert(self);

	song = workspace_song(self->state->workspace);
	if (song) {
		intptr_t step;
		psy_audio_Pattern* pattern;		
		psy_audio_SequenceCursor cursor;
		psy_audio_PatternEvent patternevent;		
		psy_ui_Value width;
		uintptr_t steprow;
		double stepwidth;		
		
		width = psy_ui_value_make_ew(4.0);
		stepwidth = psy_ui_value_px(&width,
			psy_ui_component_textmetric(&self->component), NULL);		
		cursor = song->sequence.cursor;		
		steprow = psy_audio_sequencecursor_line_pattern(&cursor) / self->state->numtiles;
		step = (intptr_t)(psy_ui_mouseevent_pt(ev).x / stepwidth +
			steprow * self->state->numtiles);
		cursor.column = 0;
		cursor.absoffset = step / (psy_dsp_big_beat_t)cursor.lpb;
		pattern = psy_audio_sequence_pattern(&song->sequence, cursor.orderindex);
		if (!pattern) {
			return;
		}
		patternevent = psy_audio_pattern_event_at_cursor(pattern, cursor);		
		if (psy_audio_patternevent_empty(&patternevent)) {						
			psy_audio_PatternEvent event;

			psy_audio_patternevent_clear(&event);
			event.note = 48;
			event.inst = (uint16_t)psy_audio_instruments_selected(
				&song->instruments).subslot;
			event.mach = (uint8_t)psy_audio_machines_selected(&song->machines);
			event = psy_audio_patterndefaults_fill_event(
				&workspace_player(self->state->workspace)->patterndefaults,
				cursor.track, event);
			psy_undoredo_execute(&self->state->workspace->undoredo,
				&insertcommand_allocinit(pattern, cursor, event,
					&song->sequence)->command);
		} else {
			psy_undoredo_execute(&self->state->workspace->undoredo,
				&removecommand_allocinit(pattern, cursor,
				&song->sequence)->command);
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
	StepSequencerState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
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
	psy_ui_Component* parent, StepSequencerState* state)
{
	StepSequencerBarButton* rv;

	rv = stepsequencerbarbutton_alloc();
	if (rv) {
		stepsequencerbarbutton_init(rv, parent, state);
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
	uintptr_t steprow;
	psy_audio_SequenceCursor cursor;

	assert(self);
	
	size = psy_ui_component_size_px(&self->component);	
	r_inner = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(	
			size.width / 2 - (int)(size.width * 0.2),
			size.height / 2 - (int)(size.height * 0.2)),
		psy_ui_realsize_make(
			(int)(size.width * 0.4),
			(int)(size.height * 0.4)));
	r_outter = r_inner;
	r_outter.left -= 3;
	r_outter.right += 3;
	r_outter.top -= 3;
	r_outter.bottom += 3;
	cursor = self->state->workspace->song->sequence.cursor;
	steprow = psy_audio_sequencecursor_line_pattern(&cursor) /
		self->state->numtiles;
	if (self->index == steprow) {
		psy_ui_drawsolidrectangle(g, r_outter,
			psy_ui_colour_make(0x00444444));
		psy_ui_drawsolidrectangle(g, r_inner,
			psy_ui_colour_make(0x00D1E8D0));
	} else if ((self->index + 1 == steprow) &&
		((psy_audio_sequencecursor_line_pattern(&cursor) % self->state->numtiles) < 1)) {
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

	psy_ui_component_init(&self->component, parent, NULL);	
	stepsequencerbarselect_vtable_init(self);
	self->state = state;	
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
	uintptr_t numsteprows;
	psy_ui_Component* row;
	psy_audio_Pattern* pattern;
	
	assert(self);
	
	pattern = stepsequencerstate_pattern(self->state);
	if (!pattern) {
		return;
	}		
	numsteprows = 4;
	if (pattern) {
		numsteprows = (uintptr_t)(psy_audio_pattern_length(pattern) *
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
			button = stepsequencerbarbutton_allocinit(row, self->state);
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
		cursor.absoffset =
			(double)(self->state->barbuttonindex * self->state->numtiles) /
			(double)cursor.lpb +
			cursor.seqoffset;
		if (self->state->workspace && workspace_song(self->state->workspace)) {
			psy_audio_sequence_set_cursor(
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
	Workspace* sender);
static void stepsequencerview_oncursorchanged(StepsequencerView*,
	psy_audio_Sequence* sender);
static void stepsequencerview_updatepattern(StepsequencerView*);
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
	psy_ui_component_init(&self->tilerow, &self->component, NULL);
	stepsequencerbar_init(&self->stepsequencerbar, &self->tilerow, &self->state);
	psy_ui_component_setalign(&self->stepsequencerbar.component,
		psy_ui_ALIGN_TOP);
	stepsequencerview_connectworkspace(self, workspace);
	stepsequencerview_connectpattern(self);
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
		stepsequencerbar_update_playline(&self->stepsequencerbar);
	}
}

void stepsequencerview_oncursorchanged(StepsequencerView* self,
	psy_audio_Sequence* sender)
{
	assert(self);
	
	stepsequencerview_updatepattern(self);
	stepsequencerview_update(self);
}

void stepsequencerview_updatepattern(StepsequencerView* self)
{
	if (stepsequencerstate_updatepattern(&self->state)) {
		stepsequencerview_connectpattern(self);
		stepsequencerbarselect_build(&self->stepsequencerbarselect);
		psy_ui_component_align(psy_ui_component_parent(&self->component));		
	}	
}

void stepsequencerview_onsongchanged(StepsequencerView* self,
	Workspace* sender)
{	
	assert(self);

	if (sender->song) {
		psy_signal_connect(&sender->song->sequence.signal_cursorchanged, self,
			stepsequencerview_oncursorchanged);
	}
	stepsequencerview_connectpattern(self);
	stepsequencerview_update(self);
}

void stepsequencerview_update(StepsequencerView* self)
{		
	assert(self);
	
	stepsequencerbar_update(&self->stepsequencerbar);
	psy_ui_component_invalidate(&self->stepsequencerbarselect.component);
}

void stepsequencerview_connectpattern(StepsequencerView* self)
{
	psy_audio_Pattern* pattern;

	assert(self);
	
	pattern = stepsequencerstate_pattern(&self->state);
	if (pattern) {
		psy_signal_connect(&pattern->signal_lengthchanged,
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
