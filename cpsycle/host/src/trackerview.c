/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerview.h"
/* local */
#include "cmdsnotes.h"
#include "patterncmds.h"
#include "patternnavigator.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define ISDIGIT TRUE

/* TrackerGrid */

/* prototypes */
static void trackergrid_on_destroy(TrackerGrid*);
static void trackergrid_init_signals(TrackerGrid*);
static void trackergrid_dispose_signals(TrackerGrid*);
static void trackergrid_on_draw(TrackerGrid*, psy_ui_Graphics*);
static void trackergrid_on_mouse_down(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_on_mouse_move(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_on_mouse_up(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_on_mouse_doubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_dragselection(TrackerGrid*, psy_audio_SequenceCursor);
static psy_ui_RealRectangle trackergrid_selection_bounds(TrackerGrid*);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_clearmidline(TrackerGrid*);
static void trackergrid_inputvalue(TrackerGrid*, uintptr_t value, bool isdigit);
static void trackergrid_prev_track(TrackerGrid*);
static void trackergrid_next_track(TrackerGrid*);
static void trackergrid_prev_line(TrackerGrid*);
static void trackergrid_advance_line(TrackerGrid*);
static void trackergrid_prev_lines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_advance_lines(TrackerGrid*, uintptr_t lines,
	bool wrap);
static void trackergrid_home(TrackerGrid*);
static void trackergrid_end(TrackerGrid*);
static void trackergrid_row_delete(TrackerGrid*);
static void trackergrid_row_clear(TrackerGrid*);
static void trackergrid_prev_col(TrackerGrid*);
static void trackergrid_next_col(TrackerGrid*);
static void trackergrid_select_machine(TrackerGrid*);
static void trackergrid_set_default_event(TrackerGrid*,
	psy_audio_Pattern* defaultpattern, psy_audio_PatternEvent*);
static void trackergrid_enablepatternsync(TrackerGrid*);
static void trackergrid_preventpatternsync(TrackerGrid*);
static void trackergrid_resetpatternsync(TrackerGrid*);
static void trackergrid_on_goto_cursor(TrackerGrid*, Workspace* sender,
	psy_audio_SequenceCursor*);
static bool trackergrid_on_tracker_cmds(TrackerGrid*, InputHandler*);
static bool trackergrid_on_note_cmds(TrackerGrid*, InputHandler* sender);
static bool trackergrid_on_midi_cmds(TrackerGrid*, InputHandler* sender);
static bool trackergrid_scroll_left(TrackerGrid*, psy_audio_SequenceCursor);
static bool trackergrid_scroll_right(TrackerGrid*, psy_audio_SequenceCursor);
static bool trackergrid_scroll_up(TrackerGrid*, psy_audio_SequenceCursor);
static bool trackergrid_scroll_down(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_set_cursor(TrackerGrid*, psy_audio_SequenceCursor);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(TrackerGrid* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			trackergrid_on_destroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackergrid_on_draw;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_on_mouse_down;
		vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_on_mouse_move;
		vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_on_mouse_up;
		vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_on_mouse_doubleclick;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &vtable);
}

/* implementation */
void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent,
	TrackerState* state, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(state);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->workspace = workspace;	
	psy_table_init(&self->columns);	
	self->state = state;
	psy_audio_sequencecursor_init(&self->oldcursor);
	/* setup base component */			
	trackergrid_init_signals(self);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_setscrollstep_height(trackergrid_base(self),
		self->state->lineheight);
	/* init internal */
	self->chord = FALSE;
	self->syncpattern = TRUE;	
	self->state->midline = FALSE;
	self->chordbegin = 0;	
	self->state->showemptydata = FALSE;	
	self->effcursoralwaysdown = FALSE;	
	self->preventscrolltop = FALSE;
	self->preventeventdriver = FALSE;
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "tracker", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)trackergrid_on_tracker_cmds);
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "notes", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)trackergrid_on_note_cmds);
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_VIEW,
		psy_EVENTDRIVER_MIDI, "", VIEW_ID_PATTERNVIEW, 
		self, (fp_inputhandler_input)trackergrid_on_midi_cmds);
	psy_audio_blockselection_init(&self->state->pv->selection);
	/* handle midline invalidation */
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);		
	psy_signal_connect(&self->workspace->signal_gotocursor, self,
		trackergrid_on_goto_cursor);	
}

void trackergrid_on_destroy(TrackerGrid* self)
{
	assert(self);

	trackergrid_dispose_signals(self);
	psy_table_dispose(&self->columns);
}

void trackergrid_init_signals(TrackerGrid* self)
{
	assert(self);
	
	psy_signal_init(&self->signal_colresize);
}

void trackergrid_dispose_signals(TrackerGrid* self)
{
	assert(self);
	
	psy_signal_dispose(&self->signal_colresize);
}

void trackergrid_on_draw(TrackerGrid* self, psy_ui_Graphics* g)
{
	psy_audio_BlockSelection clip;
	psy_ui_RealRectangle g_clip;
		
	g_clip = psy_ui_cliprect(g);
	trackerstate_lineclip(self->state, &g_clip, &clip);
	trackerstate_clip(self->state, &g_clip, &clip);
	/* prepares entry draw done in trackergridcolumn */	
	trackerstate_update_clip_events(self->state, &clip);			
}

void trackergrid_prev_track(TrackerGrid* self)
{	
	PatternColNavigator navigator;
	psy_audio_SequenceCursor cursor;

	assert(self);

	patterncolnavigator_init(&navigator, self->state, TRUE);
	cursor = patterncolnavigator_prev_track(&navigator,
		self->state->pv->cursor);
	if (patterncolnavigator_wrap(&navigator)) {
		trackergrid_scroll_right(self, cursor);
	} else {
		trackergrid_scroll_left(self, cursor);
	}
	trackergrid_set_cursor(self, cursor);
}

void trackergrid_next_track(TrackerGrid* self)
{	
	PatternColNavigator navigator;
	psy_audio_SequenceCursor cursor;

	assert(self);

	patterncolnavigator_init(&navigator, self->state, TRUE);
	cursor = patterncolnavigator_next_track(&navigator,
		self->state->pv->cursor);
	if (patterncolnavigator_wrap(&navigator)) {
		trackergrid_scroll_left(self, cursor);
	} else {
		trackergrid_scroll_right(self, cursor);
	}
	trackergrid_set_cursor(self, cursor);
}

bool trackergrid_scroll_up(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t topline;	
	double top;

	line = patternviewstate_beattoline(self->state->pv,
		psy_audio_sequencecursor_offset(&cursor));
	top = self->state->lineheightpx * line;	
	if (self->state->midline) {
		psy_ui_RealSize gridsize;		

		gridsize = psy_ui_component_scrollsize_px(&self->component);
		topline = (intptr_t)(gridsize.height / self->state->lineheightpx / 2.0);
	} else {
		topline = 0;
	}
	if (psy_ui_component_scrolltop_px(&self->component) +
			topline * self->state->lineheightpx > top) {
		intptr_t dlines;		
		
		dlines = (intptr_t)((psy_ui_component_scrolltop_px(&self->component) +
			topline * self->state->lineheightpx - top) /
			(self->state->lineheightpx));		
		psy_ui_component_setscrolltop_px(&self->component,			
			psy_ui_component_scrolltop_px(&self->component) -
			psy_ui_component_scrollstep_height_px(&self->component) * dlines);
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scroll_down(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t visilines;	
	
	visilines = self->state->visilines;
	if (self->state->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}		
	line = patternviewstate_beattoline(self->state->pv, 
		psy_audio_sequencecursor_offset(&cursor));
	if (visilines < line - psy_ui_component_scrolltop_px(&self->component) /
			self->state->lineheightpx) {
		intptr_t dlines;

		dlines = (intptr_t)
			(line - psy_ui_component_scrolltop_px(&self->component) /
			self->state->lineheightpx - visilines);	
		self->component.blitscroll = TRUE;
		psy_ui_component_setscrolltop_px(&self->component,			
			psy_ui_component_scrolltop_px(&self->component) +
			psy_ui_component_scrollstep_height_px(&self->component) * dlines);
		self->component.blitscroll = FALSE;
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scroll_left(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{	
	psy_ui_Component* column;
	uintptr_t index;

	assert(self);
	
	column = psy_ui_component_intersect(trackergrid_base(self),
		psy_ui_realpoint_make(psy_ui_component_scrollleft_px(&self->component),
			0.0), &index);
	if ((index != psy_INDEX_INVALID) && (index > cursor.track)) {
			psy_ui_RealRectangle position;

			column = (psy_ui_Component*)psy_table_at(&self->columns, cursor.track);
			position = psy_ui_component_position(column);
			psy_ui_component_setscrollleft(&self->component,
				psy_ui_value_make_px(position.left));
			return FALSE;
	}
	return TRUE;
}

bool trackergrid_scroll_right(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	uintptr_t visitracks;
	uintptr_t tracks;
	psy_ui_RealSize size;	
	const psy_ui_TextMetric* tm;	
	intptr_t trackright;
	intptr_t trackleft;

	assert(self);

	size = psy_ui_component_clientsize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	trackleft = trackerstate_pxtotrack(self->state,
		psy_ui_component_scrollleft_px(&self->component));
	trackright = trackerstate_pxtotrack(self->state,
		size.width +
		psy_ui_component_scrollleft_px(&self->component));
	visitracks = trackright - trackleft;
	tracks = cursor.track + 1;
	if (tracks > trackleft + visitracks) {		
		psy_ui_Component* column;

		column = (psy_ui_Component*)psy_table_at(&self->columns, tracks - visitracks);
		if (column) {
			psy_ui_RealRectangle position;
			 
			position = psy_ui_component_position(column);
			psy_ui_component_setscrollleft(&self->component,
				psy_ui_value_make_px(position.left));
		}
		return FALSE;
	}
	return TRUE;
}

void trackergrid_prev_line(TrackerGrid* self)
{	
	assert(self);

	trackergrid_prev_lines(self, workspace_cursorstep(self->workspace),
		self->state->pv->wraparound);
}

void trackergrid_advance_line(TrackerGrid* self)
{
	assert(self);

	trackergrid_advance_lines(self, workspace_cursorstep(self->workspace),
		self->state->pv->wraparound);
}

void trackergrid_advance_lines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	PatternLineNavigator navigator;
	psy_audio_SequenceCursor cursor;

	assert(self);
		
	patternlinennavigator_init(&navigator, self->state->pv, wrap);		
	cursor = patternlinennavigator_down(&navigator, lines,
		patternviewstate_cursor(self->state->pv));		
	trackergrid_set_cursor(self, cursor);
	if (patternlinennavigator_wrap(&navigator)) {
		trackergrid_scroll_up(self, cursor);
	} else {
		trackergrid_scroll_down(self, cursor);
	}
}

void trackergrid_prev_lines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);
	
	PatternLineNavigator navigator;
	psy_audio_SequenceCursor cursor;
		
	patternlinennavigator_init(&navigator, self->state->pv, wrap);		
	cursor = patternlinennavigator_up(&navigator, lines,
		patternviewstate_cursor(self->state->pv));		
	trackergrid_set_cursor(self, cursor);		
	if (!patternlinennavigator_wrap(&navigator)) {
		trackergrid_scroll_up(self, cursor);
	} else {
		trackergrid_scroll_down(self, cursor);	
	}
}

void trackergrid_home(TrackerGrid* self)
{
	assert(self);

	if (self->state->pv->ft2home) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinennavigator_init(&navigator, self->state->pv, FALSE);
		cursor = patternlinennavigator_home(&navigator,
			patternviewstate_cursor(self->state->pv));
		trackergrid_set_cursor(self, cursor);
		trackergrid_scroll_up(self, cursor);
	} else {
		PatternColNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patterncolnavigator_init(&navigator, self->state, FALSE);
		cursor = patterncolnavigator_home(&navigator,
			patternviewstate_cursor(self->state->pv));
		trackergrid_set_cursor(self, cursor);
		trackergrid_scroll_left(self, cursor);		
	}
}

void trackergrid_end(TrackerGrid* self)
{
	assert(self);

	if (self->state->pv->ft2home) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinennavigator_init(&navigator, self->state->pv, FALSE);
		cursor = patternlinennavigator_end(&navigator,
			patternviewstate_cursor(self->state->pv));
		trackergrid_set_cursor(self, cursor);
		trackergrid_scroll_down(self, cursor);
	} else {
		PatternColNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patterncolnavigator_init(&navigator, self->state, FALSE);
		cursor = patterncolnavigator_end(&navigator,
			patternviewstate_cursor(self->state->pv));
		trackergrid_set_cursor(self, cursor);
		trackergrid_scroll_right(self, cursor);
	}
}

void trackergrid_prev_col(TrackerGrid* self)
{
	PatternColNavigator navigator;
	psy_audio_SequenceCursor cursor;

	patterncolnavigator_init(&navigator, self->state, TRUE);

	cursor = patterncolnavigator_prev_col(&navigator,
		patternviewstate_cursor(self->state->pv));
	if (patterncolnavigator_wrap(&navigator)) {
		trackergrid_scroll_right(self, cursor);
	} else {
		trackergrid_scroll_left(self, cursor);
	}
	trackergrid_set_cursor(self, cursor);
}

void trackergrid_next_col(TrackerGrid* self)
{
	PatternColNavigator navigator;
	psy_audio_SequenceCursor cursor;

	patterncolnavigator_init(&navigator, self->state, TRUE);

	cursor = patterncolnavigator_next_col(&navigator,
		patternviewstate_cursor(self->state->pv));
	if (patterncolnavigator_wrap(&navigator)) {
		trackergrid_scroll_left(self, cursor);
	} else {
		trackergrid_scroll_right(self, cursor);
	}	
	trackergrid_set_cursor(self, cursor);
}

void trackergrid_select_machine(TrackerGrid* self)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		
		node = psy_audio_pattern_findnode_cursor(
			patternviewstate_pattern(self->state->pv),
			self->state->pv->cursor, &prev);
		if (node) {
			psy_audio_PatternEvent* ev;

			entry = (psy_audio_PatternEntry*)node->entry;
			ev = psy_audio_patternentry_front(entry);
			psy_audio_machines_select(&workspace_song(self->workspace)->machines,
				ev->mach);
			psy_audio_instruments_select(&workspace_song(self->workspace)->instruments,
				psy_audio_instrumentindex_make(0, ev->inst));			
		}		
	}
}

void trackergrid_set_default_event(TrackerGrid* self,
	psy_audio_Pattern* defaults, psy_audio_PatternEvent* ev)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	assert(self);

	node = psy_audio_pattern_findnode_cursor(defaults, self->state->pv->cursor,
		&prev);
	if (node) {
		psy_audio_PatternEvent* defaultevent;

		defaultevent = psy_audio_patternentry_front(psy_audio_patternnode_entry(node));
		if (defaultevent->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
			ev->inst = defaultevent->inst;
		}
		if (defaultevent->mach != psy_audio_NOTECOMMANDS_psy_audio_EMPTY) {
			ev->mach = defaultevent->mach;
		}
		if (defaultevent->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			ev->vol = defaultevent->vol;
		}
	}
}

void trackergrid_row_delete(TrackerGrid* self)
{
	assert(self);

	if (self->state->pv->cursor.offset - patternviewstate_bpl(
			self->state->pv) >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		if (self->state->pv->ft2delete) {
			trackergrid_prev_line(self);
		}
		node = psy_audio_pattern_findnode_cursor(
			patternviewstate_pattern(self->state->pv),
			self->state->pv->cursor, &prev);
		if (node) {
			psy_audio_pattern_remove(patternviewstate_pattern(self->state->pv), node);			
		}
		p = (prev)
			? prev->next
			: psy_audio_pattern_begin(patternviewstate_pattern(self->state->pv));
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->state->pv->cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_big_beat_t offset;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				event = *psy_audio_patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(patternviewstate_pattern(self->state->pv), p);				
				offset -= (psy_dsp_big_beat_t)patternviewstate_bpl(
					self->state->pv);
				node = psy_audio_pattern_findnode(
					patternviewstate_pattern(self->state->pv), track, offset,
					(psy_dsp_big_beat_t)patternviewstate_bpl(self->state->pv),
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*psy_audio_patternentry_front(entry) = event;
				} else {
					psy_audio_pattern_insert(
						patternviewstate_pattern(self->state->pv), prev, track,
						offset, &event);
				}
			}
		}
	}
}

void trackergrid_row_clear(TrackerGrid* self)
{
	assert(self);

	if (self->state->pv->cursor.column == PATTERNEVENT_COLUMN_NOTE) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&removecommand_allocinit(
				patternviewstate_pattern(self->state->pv),
			self->state->pv->cursor, self->state->pv->sequence)->command);
		trackergrid_advance_line(self);		
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackerstate_trackdef(self->state, self->state->pv->cursor.track);
		columndef = trackdef_columndef(trackdef, self->state->pv->cursor.column);
		trackergrid_inputvalue(self, columndef->emptyvalue, !ISDIGIT);
	}
}

bool trackergrid_on_note_cmds(TrackerGrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	assert(self);

	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {		
		psy_audio_PatternEvent ev;
		
		trackergrid_preventpatternsync(self);
		if (cmd.id >= CMD_NOTE_OFF_C_0 && cmd.id < 255) {
			ev = psy_audio_player_patternevent(&self->workspace->player, (uint8_t)cmd.id);
			ev.note = CMD_NOTE_STOP;
			psy_audio_player_playevent(&self->workspace->player, &ev);
			return 1;
		} else if (cmd.id == CMD_NOTE_CHORD_END) {
			if (self->chord) {
				psy_audio_SequenceCursor cursor;

				cursor = self->state->pv->cursor;
				cursor.track = self->chordbegin;		
				trackergrid_scroll_left(self, cursor);
				trackergrid_set_cursor(self, cursor);
				trackergrid_advance_line(self);
			}
			self->chord = FALSE;
			self->chordbegin = 0;
			return 1;
		} else if (cmd.id >= CMD_NOTE_CHORD_C_0 && cmd.id < CMD_NOTE_STOP) {
			if (!self->chord) {
				self->chordbegin = self->state->pv->cursor.track;
				self->chord = TRUE;
			}
			ev = psy_audio_player_patternevent(&self->workspace->player,
				(uint8_t)cmd.id - (uint8_t)CMD_NOTE_CHORD_C_0);
		} else if (cmd.id < 256) {			
			ev = psy_audio_player_patternevent(&self->workspace->player, (uint8_t)cmd.id);
			psy_audio_player_playevent(&self->workspace->player, &ev);
		}
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(patternviewstate_pattern(self->state->pv),
				self->state->pv->cursor, ev,
				self->state->pv->sequence)->command);
		if (self->chord != FALSE) {
			trackergrid_next_track(self);
		} else {
			trackergrid_advance_line(self);
		}
		if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
			self->state->pv->cursor.key = ev.note;			
			trackergrid_set_cursor(self, self->state->pv->cursor);			
		}
		trackergrid_enablepatternsync(self);
		return 1;
	}
	return 0;
}

bool trackergrid_on_midi_cmds(TrackerGrid* self, InputHandler* sender)
{
	if (self->preventeventdriver) {
		return 0;
	}
	if (!workspace_song(self->workspace)) {
		return 0;
	}
	if (!psy_audio_player_playing(&self->workspace->player) && 
			psy_audio_player_recordingnotes(&self->workspace->player)) {
		psy_EventDriverCmd cmd;				

		assert(self);

		cmd = inputhandler_cmd(sender);
		if (cmd.type == psy_EVENTDRIVER_MIDI) {
			psy_audio_PatternEvent ev;
			
			psy_audio_patternevent_clear(&ev);
			psy_audio_midiinput_workinput(&self->workspace->player.midiinput, cmd.midi,
				&self->workspace->song->machines, &ev);
			if (ev.note != psy_audio_NOTECOMMANDS_RELEASE ||
					psy_audio_player_recordingnoteoff(workspace_player(
						self->workspace))) {
				trackergrid_preventpatternsync(self);
				psy_undoredo_execute(&self->workspace->undoredo,
					&insertcommand_allocinit(patternviewstate_pattern(self->state->pv),			
						self->state->pv->cursor, ev, self->state->pv->sequence)->command);
				trackergrid_advance_line(self);
				if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
					self->state->pv->cursor.key = ev.note;
					trackergrid_set_cursor(self, self->state->pv->cursor);					
				}
				trackergrid_enablepatternsync(self);
			}
			return 1;			
		}
	}
	return 0;
}

void trackergrid_inputvalue(TrackerGrid* self, uintptr_t newvalue, bool isdigit)
{
	assert(self);

	if (patternviewstate_pattern(self->state->pv) && newvalue != -1) {
		psy_audio_PatternEvent ev;		

		ev = psy_audio_pattern_event_at_cursor(patternviewstate_pattern(
			self->state->pv), self->state->pv->cursor);
		if (isdigit) {			
			TrackDef* trackdef;			

			trackdef = trackerstate_trackdef(self->state, self->state->pv->cursor.track);			
			ev = trackdef_setevent_digit(trackdef, self->state->pv->cursor.column,
					self->state->pv->cursor.digit, &ev, newvalue);										
		} else {
			psy_audio_patternevent_setvalue(&ev, self->state->pv->cursor.column, newvalue);
		}
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(patternviewstate_pattern(self->state->pv),				
				self->state->pv->cursor, ev, 
				self->state->pv->sequence)->command);
		if (self->effcursoralwaysdown) {
			trackergrid_advance_line(self);
		} else {
			TrackDef* trackdef;
			TrackColumnDef* columndef;

			trackdef = trackerstate_trackdef(self->state, self->state->pv->cursor.track);
			columndef = trackdef_columndef(trackdef, self->state->pv->cursor.column);
			if (!isdigit) {
				if (columndef->wrapclearcolumn == PATTERNEVENT_COLUMN_NONE) {
					trackergrid_next_col(self);
				} else {
					self->state->pv->cursor.digit = 0;
					self->state->pv->cursor.column = columndef->wrapclearcolumn;
					trackergrid_advance_line(self);
				}
			} else if (self->state->pv->cursor.digit + 1 >= columndef->numdigits) {
				if (columndef->wrapeditcolumn == PATTERNEVENT_COLUMN_NONE) {
					trackergrid_next_col(self);
				} else {
					self->state->pv->cursor.digit = 0;
					self->state->pv->cursor.column = columndef->wrapeditcolumn;
					trackergrid_advance_line(self);
				}
			} else {
				trackergrid_next_col(self);
			}
		}
		trackergrid_invalidatecursor(self);
		trackergrid_enablepatternsync(self);		
	}
}

void trackergrid_invalidatecursor(TrackerGrid* self)
{	
	assert(self);
	
	if (psy_ui_component_drawvisible(trackergrid_base(self))) {
		trackergrid_invalidateinternalcursor(self, self->oldcursor);
		trackergrid_invalidateinternalcursor(self, self->state->pv->cursor);
	}
	self->oldcursor = self->state->pv->cursor;
}

void trackergrid_invalidateinternalcursor(TrackerGrid* self,
	psy_audio_SequenceCursor cursor)
{	
	psy_ui_Component* column;

	column = psy_ui_component_at(trackergrid_base(self), cursor.track);
	if (column) {
		psy_ui_RealSize size;

		size = psy_ui_component_scrollsize_px(column);
		psy_ui_component_invalidaterect(column,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					0.0, trackerstate_beattopx(self->state,
						psy_audio_sequencecursor_offset(&cursor))),
				psy_ui_realsize_make(
					size.width, trackerstate_lineheight(self->state))));
	}	
}

void trackergrid_invalidateline(TrackerGrid* self, intptr_t line)
{
	psy_ui_RealSize size;
	intptr_t seqstartline;

	assert(self);

	if (!patternviewstate_pattern(self->state->pv)) {
		return;
	}		
	seqstartline = patternviewstate_beattoline(self->state->pv,
		((self->state->pv->singlemode)
			? self->state->pv->cursor.seqoffset
			: 0.0));
	size = psy_ui_component_scrollsize_px(&self->component);		
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(					
				psy_ui_component_scrollleft_px(&self->component),
				self->state->lineheightpx * (line - seqstartline)),					
			psy_ui_realsize_make(size.width, self->state->lineheightpx)));
	if (patternviewstate_pattern(self->state->pv)) {
		trackergrid_preventpatternsync(self);
		patternviewstate_pattern(self->state->pv)->opcount++;
		trackergrid_resetpatternsync(self);
	}	
}

void trackergrid_invalidate_playbar(TrackerGrid* self)
{
	trackergrid_invalidatelines(self, self->workspace->host_sequencer_time.lastplayline,
		self->workspace->host_sequencer_time.currplayline);
}

void trackergrid_invalidatelines(TrackerGrid* self, intptr_t line1, intptr_t line2)
{
	psy_ui_RealSize size;
	intptr_t seqstartline;
	psy_ui_RealRectangle r1;
	psy_ui_RealRectangle r2;

	assert(self);

	if (!patternviewstate_pattern(self->state->pv)) {
		return;
	}
	seqstartline = patternviewstate_beattoline(self->state->pv,
		((self->state->pv->singlemode)
			? self->state->pv->cursor.seqoffset
			: 0.0));
	size = psy_ui_component_scrollsize_px(&self->component);
	r1 = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_component_scrollleft_px(&self->component),
			self->state->lineheightpx * (line1 - seqstartline)),
		psy_ui_realsize_make(size.width, self->state->lineheightpx));
	r2 = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_component_scrollleft_px(&self->component),
			self->state->lineheightpx * (line2 - seqstartline)),
		psy_ui_realsize_make(size.width, self->state->lineheightpx));
	psy_ui_realrectangle_union(&r1, &r2);
	psy_ui_component_invalidaterect(&self->component, r1);
	if (patternviewstate_pattern(self->state->pv)) {
		trackergrid_preventpatternsync(self);
		patternviewstate_pattern(self->state->pv)->opcount++;
		trackergrid_resetpatternsync(self);
	}
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender)
{
	assert(self);

	if (self->state->midline) {
		trackergrid_clearmidline(self);
	}
}

void trackergrid_clearmidline(TrackerGrid* self)
{
	psy_ui_RealSize size;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);	
	self->state->midline = FALSE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleft_px(&self->component),
				((self->state->visilines) / 2 - 1) * self->state->lineheightpx +
					psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width, self->state->lineheightpx)));
	psy_ui_component_update(&self->component);
	self->state->midline = TRUE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleft_px(&self->component),
				(self->state->visilines / 2 - 2) * self->state->lineheightpx +
				psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width, self->state->lineheightpx * 4)));		
}

void trackergrid_centeroncursor(TrackerGrid* self)
{
	intptr_t line;

	assert(self);

	line = patternviewstate_beattoline(self->state->pv,
		self->state->pv->cursor.offset);
	psy_ui_component_setscrolltop_px(&self->component,		
		-(self->state->visilines / 2 - line) *
			self->state->lineheightpx);
}

void trackergrid_setcentermode(TrackerGrid* self, int mode)
{
	assert(self);

	self->state->midline = mode;
	if (mode) {
		psy_ui_component_setoverflow(&self->component,
			(psy_ui_Overflow)(psy_ui_OVERFLOW_SCROLL | psy_ui_OVERFLOW_VSCROLLCENTER));
		trackergrid_centeroncursor(self);
	} else {
		psy_ui_component_setoverflow(&self->component,
			psy_ui_OVERFLOW_SCROLL);
		psy_ui_component_setscrolltop_px(&self->component, 0.0);
	}
}

void trackergrid_on_mouse_down(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	if (trackdrag_active(&self->state->trackconfig->resize)) {
		psy_signal_emit(&self->signal_colresize, self, 0);
	} else if (patternviewstate_pattern(self->state->pv) && psy_ui_mouseevent_button(ev) == 1) {		
		self->lastdragcursor = self->state->pv->dragselectionbase;
		psy_audio_blockselection_init_all(&self->state->pv->selection,
			self->state->pv->dragselectionbase, self->state->pv->dragselectionbase);
		psy_audio_blockselection_disable(&self->state->pv->selection);
		if (!psy_ui_component_hasfocus(&self->component)) {
			psy_ui_component_setfocus(&self->component);
		}
		psy_ui_component_capture(&self->component);		
	}
}

void trackergrid_on_mouse_move(TrackerGrid* self, psy_ui_MouseEvent* ev)
{	
	assert(self);	
	
	if (psy_ui_mouseevent_button(ev) != 1) {
		return;
	}		
	if (trackdrag_active(&self->state->trackconfig->resize)) {
		psy_signal_emit(&self->signal_colresize, self, 0);
		psy_ui_mouseevent_stop_propagation(ev);
	} else {
		psy_audio_SequenceCursor cursor;
		TrackerColumn* column;
		uintptr_t index;

		column = (TrackerColumn*)psy_ui_component_intersect(
			&self->component, psy_ui_mouseevent_pt(ev), &index);
		if (column) {				
			cursor = trackerstate_makecursor(self->state,
				psy_ui_mouseevent_pt(ev), column->track);
			if (!psy_audio_sequencecursor_equal(&cursor,
					&self->lastdragcursor)) {					
				psy_ui_RealRectangle rc;
				psy_ui_RealRectangle rc_curr;
				
				rc = trackergrid_selection_bounds(self);
				if (!psy_audio_blockselection_valid(&self->state->pv->selection)) {						
					trackerstate_startdragselection(self->state, cursor);
				} else {
					trackergrid_dragselection(self, cursor);
				}					
				rc_curr = trackergrid_selection_bounds(self);
				psy_ui_realrectangle_union(&rc, &rc_curr);
				psy_ui_component_invalidaterect(&self->component, rc);
				self->lastdragcursor = cursor;
			}
		}
	}
}

void trackergrid_dragselection(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	int restoremidline = self->state->midline;
	
	self->state->midline = FALSE;
	trackerstate_dragselection(self->state, cursor);
	if (cursor.offset < self->lastdragcursor.offset) {
		trackergrid_scroll_up(self, cursor);
	} else {
		trackergrid_scroll_down(self, cursor);
	}
	if (cursor.track < self->lastdragcursor.track) {
		trackergrid_scroll_left(self, cursor);
	} else {
		trackergrid_scroll_right(self, cursor);
	}
	self->state->midline = restoremidline;
}

void trackergrid_on_mouse_up(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_releasecapture(&self->component);
	if (psy_ui_mouseevent_button(ev) != 1) {
		return;
	}		
	if (trackdrag_active(&self->state->trackconfig->resize)) {
		/* End track resize */
		trackdrag_stop(&self->state->trackconfig->resize);		
		psy_signal_emit(&self->signal_colresize, self, 0);		
	} else if (!psy_audio_blockselection_valid(&self->state->pv->selection)) {
		/* set cursor if no selection was made */
		self->preventscrolltop = TRUE;
		trackergrid_set_cursor(self, self->state->pv->dragselectionbase);
		self->preventscrolltop = FALSE;		
	}	
}

psy_ui_RealRectangle trackergrid_selection_bounds(TrackerGrid* self)
{
	psy_ui_RealRectangle rv;

	psy_ui_realrectangle_init(&rv);
	if (self->state->pv->selection.valid) {
		TrackerColumn* column;

		rv.top = trackerstate_beattopx(self->state,
			self->state->pv->selection.topleft.offset);
		rv.bottom = trackerstate_beattopx(self->state,
			self->state->pv->selection.bottomright.offset);
		/* left */
		column = (TrackerColumn*)psy_table_at(&self->columns,
			self->state->pv->selection.topleft.track);
		if (column) {
			psy_ui_RealRectangle position;

			position = psy_ui_component_position(trackercolumn_base(column));
			rv.left = position.left;
		}
		/* right */
		column = (TrackerColumn*)psy_table_at(&self->columns,
			self->state->pv->selection.bottomright.track);
		if (column) {
			psy_ui_RealRectangle position;

			position = psy_ui_component_position(trackercolumn_base(column));
			rv.right = position.left;
		}
	}
	return rv;
}

void trackergrid_on_mouse_doubleclick(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 1) {
		patternviewstate_selectcol(self->state->pv);		
	}
}

void trackergrid_setpattern(TrackerGrid* self, psy_audio_Pattern* pattern)
{
	assert(self);

	patternviewstate_setpattern(self->state->pv, pattern);
	trackergrid_resetpatternsync(self);
	psy_ui_component_updateoverflow(trackergrid_base(self));	
	if (!self->preventscrolltop) {
		if (self->state->pv->singlemode) {
			if ((self->workspace->host_sequencer_time.currplaying && workspace_followingsong(self->workspace)) ||
				!self->workspace->host_sequencer_time.currplaying) {
					psy_ui_component_setscrolltop_px(&self->component, 0.0);
			}
		} else {	
			if ((self->workspace->host_sequencer_time.currplaying && workspace_followingsong(self->workspace)) ||
					!self->workspace->host_sequencer_time.currplaying && self->state->pv->cursor.orderindex.order == 0) {
				psy_ui_component_setscrolltop_px(&self->component, 0.0);
			} else if (!self->workspace->host_sequencer_time.currplaying) {
				psy_ui_component_setscrolltop_px(&self->component,
					trackerstate_beattopx(self->state,
						psy_audio_sequencecursor_offset_abs(&self->state->pv->cursor)));
			}
		}
	}
	if (self->state->midline) {
		trackergrid_centeroncursor(self);
	}
	psy_ui_component_align(&self->component);
}

void trackergrid_enablepatternsync(TrackerGrid* self)
{
	assert(self);
	
	trackergrid_resetpatternsync(self);
	self->syncpattern = TRUE;
}

void trackergrid_preventpatternsync(TrackerGrid* self)
{
	assert(self);

	trackergrid_resetpatternsync(self);
	self->syncpattern = FALSE;
}

void trackergrid_resetpatternsync(TrackerGrid* self)
{
	assert(self);

	if (patternviewstate_pattern(self->state->pv)) {
		self->component.opcount = patternviewstate_pattern(
			self->state->pv)->opcount;
	} else {
		self->component.opcount = 0;
	}
}

void trackergrid_changegenerator(TrackerGrid* self)
{
	assert(self);

	patterncmds_changemachine(self->state->pv->cmds, self->state->pv->selection);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_changeinstrument(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_changeinstrument(self->state->pv->cmds, self->state->pv->selection);
	psy_ui_component_invalidate(&self->component);	
}

void trackergrid_blockstart(TrackerGrid* self)
{
	assert(self);

	self->state->pv->dragselectionbase = patternviewstate_cursor(self->state->pv);
	trackerstate_startdragselection(self->state, self->state->pv->cursor);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockend(TrackerGrid* self)
{
	assert(self);

	trackergrid_dragselection(self, self->state->pv->cursor);
	psy_ui_component_invalidate(&self->component);
}

bool trackergrid_on_tracker_cmds(TrackerGrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);	
	return trackergrid_handlecommand(self, cmd.id);
}

bool trackergrid_handlecommand(TrackerGrid* self, intptr_t cmd)
{	
	assert(self);

	switch (cmd) {
	case CMD_NAVUP:
		if (self->state->pv->movecursoronestep) {
			trackergrid_prev_lines(self, 1, 0);
		} else {
			trackergrid_prev_line(self);
		}
		return TRUE;
	case CMD_NAVPAGEUP:
		trackergrid_prev_lines(self, self->state->pv->pgupdownstep, FALSE);
		return TRUE;
	case CMD_NAVDOWN:
		if (self->state->pv->movecursoronestep) {
			trackergrid_advance_lines(self, 1, 0);
		} else {
			trackergrid_advance_line(self);
		}
		return TRUE;
	case CMD_NAVPAGEDOWN:
		trackergrid_advance_lines(self, self->state->pv->pgupdownstep, FALSE);
		return TRUE;
	case CMD_NAVLEFT:
		trackergrid_prev_col(self);
		return TRUE;
	case CMD_NAVRIGHT:
		trackergrid_next_col(self);
		return TRUE;
	case CMD_NAVTOP:
		trackergrid_home(self);
		return TRUE;
	case CMD_NAVBOTTOM:
		trackergrid_end(self);
		return TRUE;
	case CMD_COLUMNPREV:
		trackergrid_prev_track(self);
		return TRUE;
	case CMD_COLUMNNEXT:
		trackergrid_next_track(self);
		return TRUE;
	case CMD_BLOCKSTART:
		trackergrid_blockstart(self);
		return TRUE;
	case CMD_BLOCKEND:
		trackergrid_blockend(self);
		return TRUE;
	case CMD_BLOCKUNMARK:
		patternviewstate_blockunmark(self->state->pv);
		return TRUE;
	case CMD_BLOCKCUT:
		patternviewstate_blockcut(self->state->pv);
		return TRUE;
	case CMD_BLOCKCOPY:
		patternviewstate_blockcopy(self->state->pv);
		return TRUE;
	case CMD_BLOCKPASTE:
		patternviewstate_blockpaste(self->state->pv);		
		return TRUE;
	case CMD_BLOCKMIX:
		patternviewstate_blockmixpaste(self->state->pv);
		return TRUE;
	case CMD_BLOCKDELETE:
		patternviewstate_blockdelete(self->state->pv);
		return TRUE;
	case CMD_TRANSPOSEBLOCKINC:
		patternviewstate_blocktranspose(self->state->pv, 1);
		return TRUE;
	case CMD_TRANSPOSEBLOCKDEC:
		patternviewstate_blocktranspose(self->state->pv, -1);
		return TRUE;
	case CMD_TRANSPOSEBLOCKINC12:
		patternviewstate_blocktranspose(self->state->pv, 12);
		return TRUE;
	case CMD_TRANSPOSEBLOCKDEC12:
		patternviewstate_blocktranspose(self->state->pv, -12);
		return TRUE;
	case CMD_ROWDELETE:
		trackergrid_row_delete(self);			
		return TRUE;
	case CMD_ROWCLEAR:
		trackergrid_row_clear(self);			
		return TRUE;
	case CMD_SELECTALL:
		patternviewstate_selectall(self->state->pv);
		return TRUE;
	case CMD_SELECTCOL:
		patternviewstate_selectcol(self->state->pv);		
		return TRUE;
	case CMD_SELECTBAR:
		patternviewstate_selectbar(self->state->pv);		
		return TRUE;
	case CMD_SELECTMACHINE:
		trackergrid_select_machine(self);
		psy_ui_component_setfocus(&self->component);
		return TRUE;
	case CMD_UNDO:
		workspace_undo(self->workspace);
		return TRUE;
	case CMD_REDO:
		workspace_redo(self->workspace);
		return TRUE;
	case CMD_DIGIT0:
	case CMD_DIGIT1:
	case CMD_DIGIT2:
	case CMD_DIGIT3:
	case CMD_DIGIT4:
	case CMD_DIGIT5:
	case CMD_DIGIT6:
	case CMD_DIGIT7:
	case CMD_DIGIT8:
	case CMD_DIGIT9:
	case CMD_DIGITA:
	case CMD_DIGITB:
	case CMD_DIGITC:
	case CMD_DIGITD:
	case CMD_DIGITE:
	case CMD_DIGITF:
		if (self->state->pv->cursor.column != PATTERNEVENT_COLUMN_NOTE) {
			int digit = (int)cmd - CMD_DIGIT0;
			if (digit >= 0) {
				trackergrid_inputvalue(self, (uint8_t)digit, ISDIGIT);
			}
			return TRUE;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void trackergrid_tweak(TrackerGrid* self, int slot, uintptr_t tweak,
	float normvalue)
{
	assert(self);

	if (workspace_recordingtweaks(self->workspace)) {
		psy_audio_PatternEvent event;
		psy_audio_Machine* machine;
		int value;

		machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, slot);
		assert(machine);
		value = 0; /* machine_parametervalue_scaled(machine, tweak, normvalue); */
		psy_audio_patternevent_init_all(&event,
			(unsigned char)(
				(keyboardmiscconfig_recordtweaksastws(&self->workspace->config.misc))
				? psy_audio_NOTECOMMANDS_TWEAKSLIDE
				: psy_audio_NOTECOMMANDS_TWEAK),
			psy_audio_NOTECOMMANDS_INST_EMPTY,
			(unsigned char)psy_audio_machines_selected(&workspace_song(self->workspace)->machines),
			psy_audio_NOTECOMMANDS_VOL_EMPTY,
			(unsigned char)((value & 0xFF00) >> 8),
			(unsigned char)(value & 0xFF));
		event.inst = (unsigned char)tweak;
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(patternviewstate_pattern(self->state->pv),				
				self->state->pv->cursor, event,
				&self->workspace->song->sequence)->command);
		if (keyboardmiscconfig_advancelineonrecordtweak(&self->workspace->config.misc) &&
			!(workspace_followingsong(self->workspace) &&
				psy_audio_player_playing(workspace_player(self->workspace)))) {
			trackergrid_advance_line(self);
		}
		trackergrid_enablepatternsync(self);
	}
}

void trackergrid_onclientalign(TrackerGrid* self, psy_ui_Component* sender)
{	
	assert(self);
		
	if (trackergrid_midline(self)) {		
		trackergrid_centeroncursor(self);				
	} 
}

void trackergrid_showemptydata(TrackerGrid* self, int showstate)
{
	assert(self);

	self->state->showemptydata = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_on_goto_cursor(TrackerGrid* self, Workspace* sender,
	psy_audio_SequenceCursor* cursor)
{			
	double y;
	psy_ui_RealSize clientsize;
	
	y = trackerstate_beattopx(self->state, psy_audio_sequencecursor_offset(
			cursor));
	clientsize = psy_ui_component_clientsize_px(&self->component);
	if (y < psy_ui_component_scrolltop_px(&self->component) ||
		y > psy_ui_component_scrolltop_px(&self->component) + clientsize.height) {
		psy_ui_component_setscrolltop_px(&self->component,
			trackerstate_beattopx(self->state,
				psy_audio_sequencecursor_offset(cursor)));
	}
}

void trackergrid_build(TrackerGrid* self)
{
	uintptr_t t;
	
	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->columns);
	for (t = 0; t < patternviewstate_numsongtracks(self->state->pv); ++t) {
		TrackerColumn* column;

		column = trackercolumn_allocinit(&self->component, t, self->state,
			self->workspace);
		if (column) {
			psy_table_insert(&self->columns, t, (void*)column);
		}
	}
	psy_ui_component_align(&self->component);
}

void trackergrid_update_follow_song(TrackerGrid* self)
{			
	if (self->workspace->host_sequencer_time.lastplayline < self->workspace->host_sequencer_time.currplayline) {
		trackergrid_invalidateline(self, self->workspace->host_sequencer_time.lastplayline);
		trackergrid_scroll_down(self, trackerstate_checkcursorbounds(self->state,
			patternviewstate_cursor(self->state->pv)));
		trackergrid_invalidateline(self, self->workspace->host_sequencer_time.currplayline);
	} else {
		psy_ui_component_setscrolltop_px(&self->component, 0.0);
	}
}

void trackergrid_set_cursor(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	if (patternviewstate_sequence(self->state->pv)) {
		bool restore;

		restore = self->preventscrolltop;
		self->preventscrolltop = TRUE;
		psy_audio_sequence_set_cursor(patternviewstate_sequence(
			self->state->pv), cursor);
		self->preventscrolltop = restore;
	}
}

/* TrackerView */

/* prototypes */
static void trackerview_onscrollpanealign(TrackerView*,
	psy_ui_Component* sender);
static void trackerview_onsongchanged(TrackerView*, Workspace* sender);
static void trackerview_connectsong(TrackerView*);
static void trackerview_oncursorchanged(TrackerView*, psy_audio_Sequence*
	sender);
static void trackerview_onconfigure(TrackerView*, PatternViewConfig*,
	psy_Property*);
static void trackerview_onplaylinechanged(TrackerView*, Workspace* sender);
static void trackerview_onplaystatuschanged(TrackerView*, Workspace* sender);


/* implementation */
void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	TrackerState* state, Workspace* workspace)
{
	trackergrid_init(&self->grid, parent, state, workspace);
	self->workspace = workspace;	
	psy_ui_component_setwheelscroll(&self->grid.component, 4);
	psy_ui_component_setoverflow(trackergrid_base(&self->grid),
		psy_ui_OVERFLOW_SCROLL);
	psy_ui_scroller_init(&self->scroller, &self->grid.component, parent);	
	psy_signal_connect(&self->scroller.pane.signal_align, self,
		trackerview_onscrollpanealign);	
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_FIXED);
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerview_onsongchanged);
	trackerview_connectsong(self);
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		trackerview_onconfigure);
}

void trackerview_onscrollpanealign(TrackerView* self, psy_ui_Component* sender)
{
	psy_ui_RealSize size;

	size = psy_ui_component_scrollsize_px(&self->scroller.pane);
	self->grid.state->visilines = (intptr_t)(size.height /
		self->grid.state->lineheightpx);
}

void trackerview_onsongchanged(TrackerView* self, Workspace* sender)
{
	psy_audio_sequencecursor_init(&self->grid.oldcursor);
	trackerview_connectsong(self);	
}

void trackerview_connectsong(TrackerView* self)
{
	if (patternviewstate_sequence(self->grid.state->pv)) {
		psy_signal_connect(&patternviewstate_sequence(
			self->grid.state->pv)->signal_cursorchanged,
		self, trackerview_oncursorchanged);
		psy_signal_connect(&self->workspace->signal_playlinechanged, self,
			trackerview_onplaylinechanged);
		psy_signal_connect(&self->workspace->signal_playstatuschanged, self,
			trackerview_onplaystatuschanged);

	}
}

void trackerview_oncursorchanged(TrackerView* self, psy_audio_Sequence* sender)
{
	assert(self);
				
	if (sender->cursor.lpb != sender->lastcursor.lpb) {		
		psy_ui_component_align(&self->scroller.pane);		
		psy_ui_component_invalidate(trackergrid_base(&self->grid));				
	} else if (psy_audio_player_playing(&self->workspace->player) &&
			workspace_followingsong(self->workspace)) {
		if (psy_ui_component_drawvisible(trackerview_base(self))) {
			trackergrid_update_follow_song(&self->grid);
		}
	} else if (self->grid.state->midline) {
		trackergrid_centeroncursor(&self->grid);
	} else {		
		trackergrid_invalidatecursor(&self->grid);
	}	
}

void trackerview_onplaylinechanged(TrackerView* self, Workspace* sender)
{
	if (!workspace_followingsong(sender) && psy_ui_component_drawvisible(
			trackerview_base(self))) {
		trackergrid_invalidate_playbar(&self->grid);		
	}
}

void trackerview_onplaystatuschanged(TrackerView* self, Workspace* sender)
{
	self->grid.state->prevent_cursor = workspace_followingsong(sender) &&
		sender->host_sequencer_time.currplaying;
	trackergrid_invalidate_playbar(&self->grid);	
	self->grid.oldcursor = patternviewstate_cursor(self->grid.state->pv);
}

void trackerview_onconfigure(TrackerView* self, PatternViewConfig* config,
	psy_Property* property)
{
	if (patternviewconfig_issmoothscrolling(config)) {
		psy_ui_scroller_scrollsmooth(&self->scroller);
	} else {
		psy_ui_scroller_scrollfast(&self->scroller);
	}
	if (patternviewconfig_centercursoronscreen(config)) {
		trackergrid_centeroncursor(&self->grid);
	}
	self->grid.notestabmode = patternviewconfig_notetabmode(config);
	trackergrid_showemptydata(&self->grid,
		patternviewconfig_drawemptydata(config));
	trackergrid_setcentermode(&self->grid,
		patternviewconfig_centercursoronscreen(config));
}
