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
/* platform */
#include "../../detail/portable.h"

#define ISDIGIT TRUE

/* TrackerGrid */

/* prototypes */
static void trackergrid_on_destroyed(TrackerGrid*);
static void trackergrid_init_signals(TrackerGrid*);
static void trackergrid_dispose_signals(TrackerGrid*);
static void trackergrid_connect_input_handler(TrackerGrid*, InputHandler*);
static void trackergrid_on_draw(TrackerGrid*, psy_ui_Graphics*);
static void trackergrid_on_mouse_down(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_on_mouse_move(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_on_mouse_up(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_on_mouse_doubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static psy_ui_RealRectangle trackergrid_selection_bounds(TrackerGrid*);
static void trackergrid_drag_selection(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_on_scroll(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_clear_midline(TrackerGrid*);
static void trackergrid_input_value(TrackerGrid*, uintptr_t value,
	bool isdigit);
static void trackergrid_prev_track(TrackerGrid*);
static void trackergrid_next_track(TrackerGrid*);
static void trackergrid_prev_line(TrackerGrid*);
static void trackergrid_advance_line(TrackerGrid*);
static void trackergrid_prev_lines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_advance_lines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_home(TrackerGrid*);
static void trackergrid_end(TrackerGrid*);
static void trackergrid_row_delete(TrackerGrid*);
static void trackergrid_row_clear(TrackerGrid*);
static void trackergrid_prev_col(TrackerGrid*);
static void trackergrid_next_col(TrackerGrid*);
static void trackergrid_select_machine(TrackerGrid*);
static void trackergrid_set_default_event(TrackerGrid*,
	psy_audio_Pattern* defaultpattern, psy_audio_PatternEvent*);
static void trackergrid_enable_pattern_sync(TrackerGrid*);
static void trackergrid_prevent_pattern_sync(TrackerGrid*);
static void trackergrid_reset_pattern_sync(TrackerGrid*);
static bool trackergrid_on_tracker_cmds(TrackerGrid*, InputHandler*);
static bool trackergrid_on_note_cmds(TrackerGrid*, InputHandler* sender);
static bool trackergrid_insert_note(TrackerGrid*, intptr_t note_id);
static bool trackergrid_on_midi_cmds(TrackerGrid*, InputHandler* sender);
static void trackergrid_scroll_left(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_scroll_right(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_scroll_up(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_scroll_down(TrackerGrid*, psy_audio_SequenceCursor, bool set);
static void trackergrid_set_cursor(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_on_timer(TrackerGrid*, uintptr_t id);
static void trackergrid_on_focus(TrackerGrid*);
static void trackergrid_on_focus_lost(TrackerGrid*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(TrackerGrid* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			trackergrid_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackergrid_on_draw;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			trackergrid_on_mouse_down;
		vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			trackergrid_on_mouse_move;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			trackergrid_on_mouse_up;
		vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			trackergrid_on_mouse_doubleclick;		
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			trackergrid_on_timer;
		vtable.on_focus =
			(psy_ui_fp_component)
			trackergrid_on_focus;
		vtable.on_focuslost =
			(psy_ui_fp_component)
			trackergrid_on_focus_lost;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent,	
	TrackerState* state, InputHandler* input_handler, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(state);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_tab_index(&self->component, 0);
	self->workspace = workspace;	
	psy_table_init(&self->columns);	
	self->state = state;
	psy_audio_sequencecursor_init(&self->old_cursor);
	/* setup base component */			
	trackergrid_init_signals(self);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_scroll_step_height(trackergrid_base(self),
		self->state->line_height);
	/* init internal */	
	self->state->midline = FALSE;	
	self->state->show_empty_data = FALSE;	
	self->effcursor_always_down = FALSE;	
	self->prevent_event_driver = FALSE;
	self->down = FALSE;	
	trackergrid_connect_input_handler(self, input_handler);
	psy_audio_blockselection_init(&self->state->pv->selection);
	/* handle midline invalidation */
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_on_scroll);			
	psy_ui_component_start_timer(&self->component, 0, 50);	
}

void trackergrid_on_destroyed(TrackerGrid* self)
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

void trackergrid_connect_input_handler(TrackerGrid* self,
	InputHandler* input_handler)
{
	inputhandler_connect(input_handler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "tracker", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)trackergrid_on_tracker_cmds);
	inputhandler_connect(input_handler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "notes", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)trackergrid_on_note_cmds);
	inputhandler_connect(input_handler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_MIDI, "", VIEW_ID_PATTERNVIEW,
		self, (fp_inputhandler_input)trackergrid_on_midi_cmds);
}

void trackergrid_on_draw(TrackerGrid* self, psy_ui_Graphics* g)
{	
	psy_ui_RealRectangle g_clip;
		
	g_clip = psy_ui_graphics_cliprect(g);
	/* prepares patternentry draw done in trackergridcolumn */	
	trackerstate_update_clip_events(self->state, &g_clip,
		psy_ui_component_textmetric(&self->component));
	trackereventtable_prepare_selection(&self->state->track_events,
		self->state->pv->sequence, &self->state->pv->selection);
}

void trackergrid_prev_track(TrackerGrid* self)
{	
	PatternColNavigator navigator;
	psy_audio_SequenceCursor cursor;

	assert(self);

	patterncolnavigator_init(&navigator, self->state, TRUE);
	cursor = patterncolnavigator_prev_track(&navigator,
		patternviewstate_cursor(self->state->pv));
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
		patternviewstate_cursor(self->state->pv));
	if (patterncolnavigator_wrap(&navigator)) {
		trackergrid_scroll_left(self, cursor);
	} else {
		trackergrid_scroll_right(self, cursor);
	}
	trackergrid_set_cursor(self, cursor);
}

void trackergrid_scroll_up(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t topline;	
	double top;

	if (patternviewstate_single_mode(self->state->pv)) {
		line = psy_audio_sequencecursor_line(&cursor);
	} else {
		line = psy_audio_sequencecursor_line_abs(&cursor,
			self->state->pv->sequence);
	}
	top = self->state->beat_convert.line_px * line;	
	if (self->state->midline) {
		psy_ui_RealSize gridsize;		

		gridsize = psy_ui_component_scroll_size_px(&self->component);
		topline = (intptr_t)(gridsize.height /
			self->state->beat_convert.line_px / 2.0);
	} else {
		topline = 0;
	}
	if (psy_ui_component_scroll_top_px(&self->component) +
			topline * self->state->beat_convert.line_px > top) {
		intptr_t dlines;		
		
		dlines = (intptr_t)((psy_ui_component_scroll_top_px(&self->component) +
			topline * self->state->beat_convert.line_px - top) / (self->state->beat_convert.line_px));
		psy_ui_component_set_scroll_top_px(&self->component,			
			psy_ui_component_scroll_top_px(&self->component) -
			psy_ui_component_scroll_step_height_px(&self->component) * dlines);
	}
}

void trackergrid_scroll_down(TrackerGrid* self, psy_audio_SequenceCursor cursor,
	bool set)
{	
	intptr_t line;
	intptr_t visilines;	
	psy_ui_RealSize size;

	size = psy_ui_component_clientsize_px(&self->component);
	visilines = (intptr_t)(size.height / self->state->beat_convert.line_px);
	if (self->state->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	if (patternviewstate_single_mode(self->state->pv)) {
		line = psy_audio_sequencecursor_line(&cursor);
	} else {
		line = psy_audio_sequencecursor_line_abs(&cursor,
			self->state->pv->sequence);
	}	
	if (visilines < line - psy_ui_component_scroll_top_px(&self->component) /
			self->state->beat_convert.line_px) {
		intptr_t dlines;

		dlines = (intptr_t)
			(line - psy_ui_component_scroll_top_px(&self->component) /
			self->state->beat_convert.line_px - visilines);			
		self->component.blitscroll = TRUE;	
		if (set) {
			trackergrid_set_cursor(self, cursor);
		}
		psy_ui_component_set_scroll_top_px(&self->component,			
			psy_ui_component_scroll_top_px(&self->component) +
			psy_ui_component_scroll_step_height_px(&self->component) * dlines);
		self->component.blitscroll = FALSE;		
	}
	if (set) {
		trackergrid_set_cursor(self, cursor);		
	}	
}

void trackergrid_scroll_left(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{	
	psy_ui_Component* column;
	uintptr_t index;

	assert(self);
	
	column = psy_ui_component_intersect(trackergrid_base(self),
		psy_ui_realpoint_make(psy_ui_component_scroll_left_px(&self->component),
			0.0), &index);
	if ((index != psy_INDEX_INVALID) && (index > cursor.track)) {
			psy_ui_RealRectangle position;

			column = (psy_ui_Component*)psy_table_at(&self->columns,
				cursor.track);
			position = psy_ui_component_position(column);
			psy_ui_component_set_scroll_left(&self->component,
				psy_ui_value_make_px(position.left));			
	}	
}

void trackergrid_scroll_right(TrackerGrid* self, psy_audio_SequenceCursor
	cursor)
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
	trackleft = trackerstate_px_to_track(self->state,
		psy_ui_component_scroll_left_px(&self->component),
		psy_ui_component_textmetric(&self->component));
	trackright = trackerstate_px_to_track(self->state,
		size.width +
		psy_ui_component_scroll_left_px(&self->component),
		psy_ui_component_textmetric(&self->component));
	visitracks = trackright - trackleft;
	tracks = cursor.track + 1;
	if (tracks > trackleft + visitracks) {		
		psy_ui_Component* column;

		column = (psy_ui_Component*)psy_table_at(&self->columns,
			tracks - visitracks);
		if (column) {
			psy_ui_RealRectangle position;
			 
			position = psy_ui_component_position(column);
			psy_ui_component_set_scroll_left(&self->component,
				psy_ui_value_make_px(position.left));
		}		
	}	
}

void trackergrid_prev_line(TrackerGrid* self)
{	
	assert(self);

	trackergrid_prev_lines(self, 
		keyboardmiscconfig_cursor_step(self->state->pv->keymiscconfig),
		patternviewconfig_wrap_around(self->state->pv->patconfig));
}

void trackergrid_advance_line(TrackerGrid* self)
{
	assert(self);

	trackergrid_advance_lines(self,
		keyboardmiscconfig_cursor_step(self->state->pv->keymiscconfig),
		patternviewconfig_wrap_around(self->state->pv->patconfig));
}

void trackergrid_advance_lines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	PatternLineNavigator navigator;
	psy_audio_SequenceCursor cursor;

	assert(self);
		
	patternlinenavigator_init(&navigator, self->state->pv, wrap);		
	cursor = patternlinenavigator_down(&navigator, lines,
		patternviewstate_cursor(self->state->pv));	
	if (self->state->midline) {
		trackergrid_set_cursor(self, cursor);
		trackergrid_center_on_cursor(self);
	} else if (patternlinennavigator_wrap(&navigator)) {
		trackergrid_set_cursor(self, cursor);
		trackergrid_scroll_up(self, cursor);
	} else {
		trackergrid_scroll_down(self, cursor, TRUE);
	}
}

void trackergrid_prev_lines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);
	
	PatternLineNavigator navigator;
	psy_audio_SequenceCursor cursor;
		
	patternlinenavigator_init(&navigator, self->state->pv, wrap);		
	cursor = patternlinenavigator_up(&navigator, lines,
		patternviewstate_cursor(self->state->pv));
	if (self->state->midline) {
		trackergrid_set_cursor(self, cursor);
		trackergrid_center_on_cursor(self);
	} else if (!patternlinennavigator_wrap(&navigator)) {
		trackergrid_set_cursor(self, cursor);		
		trackergrid_scroll_up(self, cursor);
	} else {
		trackergrid_scroll_down(self, cursor, TRUE);	
	}
}

void trackergrid_home(TrackerGrid* self)
{
	assert(self);

	if (patternviewstate_ft2home(self->state->pv)) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinenavigator_init(&navigator, self->state->pv, FALSE);
		cursor = patternlinenavigator_home(&navigator,
			patternviewstate_cursor(self->state->pv));
		trackergrid_set_cursor(self, cursor);
		if (self->state->midline) {
			trackergrid_center_on_cursor(self);
		} else {
			trackergrid_scroll_up(self, cursor);
		}
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

	if (patternviewstate_ft2home(self->state->pv)) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinenavigator_init(&navigator, self->state->pv, FALSE);
		cursor = patternlinenavigator_end(&navigator,
			patternviewstate_cursor(self->state->pv));
		if (self->state->midline) {
			trackergrid_set_cursor(self, cursor);
			trackergrid_center_on_cursor(self);
		} else {
			trackergrid_scroll_down(self, cursor, TRUE);
		}
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
	psy_audio_Pattern* pattern;
	psy_audio_PatternEvent ev;

	assert(self);
	
	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return;
	}
	ev = psy_audio_pattern_event_at_cursor(pattern,
		*patternviewstate_cursor(self->state->pv));
	if (!psy_audio_patternevent_empty(&ev)) {
		psy_audio_machines_select(
			&workspace_song(self->workspace)->machines,
			ev.mach);
		psy_audio_instruments_select(
			&workspace_song(self->workspace)->instruments,
			psy_audio_instrumentindex_make(0, ev.inst));		
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
		if (defaultevent->mach != psy_audio_NOTECOMMANDS_EMPTY) {
			ev->mach = defaultevent->mach;
		}
		if (defaultevent->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			ev->vol = defaultevent->vol;
		}
	}
}

void trackergrid_row_delete(TrackerGrid* self)
{	
	psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return;
	}
	if (self->state->pv->cursor.offset - 
			psy_audio_sequencecursor_bpl(&self->state->pv->cursor) >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		if (patternviewstate_ft2delete(self->state->pv)) {
			trackergrid_prev_line(self);
		}
		node = psy_audio_pattern_findnode_cursor(pattern,
			self->state->pv->cursor, &prev);
		if (node) {
			psy_audio_pattern_remove(pattern, node);			
		}
		p = (prev)
			? prev->next
			: psy_audio_pattern_begin(pattern);
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->state->pv->cursor.track) {
				psy_audio_PatternEvent ev;
				psy_dsp_big_beat_t offset;
				psy_dsp_big_beat_t bpl;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				ev = *psy_audio_patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(pattern, p);
				bpl = psy_audio_sequencecursor_bpl(&self->state->pv->cursor);
				offset -= bpl;
				node = psy_audio_pattern_find_node(pattern, track, offset, bpl,
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*psy_audio_patternentry_front(entry) = ev;
				} else {
					psy_audio_pattern_insert(pattern, prev, track, offset, &ev);
				}
			}
		}
	}
}

void trackergrid_row_clear(TrackerGrid* self)
{
	psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return;
	}
	if (self->state->pv->cursor.column == PATTERNEVENT_COLUMN_NOTE) {
		self->prevent_cursor = TRUE;
		psy_undoredo_execute(&self->workspace->undoredo,
			&removecommand_allocinit(pattern, self->state->pv->cursor,
				self->state->pv->sequence)->command);
		self->prevent_cursor = FALSE;
		trackergrid_advance_line(self);
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackerconfig_trackdef(self->state->track_config,
			self->state->pv->cursor.track);
		columndef = trackdef_column_def(trackdef, self->state->pv->cursor.column);
		trackergrid_input_value(self, columndef->emptyvalue, !ISDIGIT);
	}
}

bool trackergrid_on_note_cmds(TrackerGrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;	
	bool worked;

	assert(self);
	
	worked = FALSE;
	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {
		worked = trackergrid_insert_note(self, cmd.id);
	}
	return worked;
}

bool trackergrid_insert_note(TrackerGrid* self, intptr_t note_id)
{
	psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return 0;
	}
	if (note_id != -1) {
		psy_audio_PatternEvent ev;
		bool insert;

		trackergrid_prevent_pattern_sync(self);
		insert = FALSE;
		if (note_id >= CMD_NOTE_OFF_C_0 && note_id < 255) {
			ev = psy_audio_player_pattern_event(&self->workspace->player, (uint8_t)note_id);
			ev.note = CMD_NOTE_STOP;
			psy_audio_player_playevent(&self->workspace->player, &ev);
		} else if (note_id == CMD_NOTE_CHORD_END) {
			if (self->state->pv->chord) {
				psy_audio_SequenceCursor cursor;

				cursor = self->state->pv->cursor;
				cursor.track = self->state->pv->chord_begin;
				trackergrid_scroll_left(self, cursor);
				trackergrid_set_cursor(self, cursor);
				trackergrid_advance_line(self);
			}
			self->state->pv->chord = FALSE;
			self->state->pv->chord_begin = 0;
		} else if (note_id >= CMD_NOTE_CHORD_C_0 && note_id < CMD_NOTE_STOP) {
			if (!self->state->pv->chord) {
				self->state->pv->chord_begin = self->state->pv->cursor.track;
				self->state->pv->chord = TRUE;
			}
			ev = psy_audio_player_pattern_event(&self->workspace->player,
				(uint8_t)note_id - (uint8_t)CMD_NOTE_CHORD_C_0);
			insert = TRUE;
		} else if (note_id < 256) {
			ev = psy_audio_player_pattern_event(&self->workspace->player, (uint8_t)note_id);
			psy_audio_player_playevent(&self->workspace->player, &ev);
			insert = TRUE;
		}
		if (insert) {			
			psy_undoredo_execute(&self->workspace->undoredo,
				&insertcommand_allocinit(pattern,
					*patternviewstate_cursor(self->state->pv), ev,
					patternviewstate_sequence(self->state->pv))->command);			
			if (self->state->pv->chord != FALSE) {
				trackergrid_next_track(self);
			} else {
				trackergrid_advance_line(self);
			}
			if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
				self->state->pv->cursor.key = ev.note;
				trackergrid_set_cursor(self, self->state->pv->cursor);
			}
		}
		trackergrid_enable_pattern_sync(self);
		return 1;
	}
	return 0;
}

bool trackergrid_on_midi_cmds(TrackerGrid* self, InputHandler* sender)
{
	psy_audio_Pattern* pattern;

	assert(self);

	if (self->prevent_event_driver) {
		return 0;
	}
	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
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
			psy_audio_midiinput_work_input(&self->workspace->player.midiinput,
				cmd.midi, &self->workspace->song->machines, &ev);
			if (ev.note != psy_audio_NOTECOMMANDS_RELEASE ||
					psy_audio_player_recording_noteoff(workspace_player(
						self->workspace))) {
				trackergrid_prevent_pattern_sync(self);
				psy_undoredo_execute(&self->workspace->undoredo,
					&insertcommand_allocinit(pattern,
						self->state->pv->cursor, ev,
						self->state->pv->sequence)->command);
				trackergrid_advance_line(self);
				if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
					self->state->pv->cursor.key = ev.note;
					trackergrid_set_cursor(self, self->state->pv->cursor);					
				}
				trackergrid_enable_pattern_sync(self);
			}
			return 1;			
		}
	}
	return 0;
}

void trackergrid_input_value(TrackerGrid* self, uintptr_t newvalue, bool isdigit)
{
	psy_audio_PatternEvent ev;
	psy_audio_Pattern* pattern;

	assert(self);	
	
	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return;
	}
	ev = psy_audio_pattern_event_at_cursor(pattern,
		*patternviewstate_cursor(self->state->pv));
	if (isdigit) {			
		TrackDef* trackdef;			

		trackdef = trackerconfig_trackdef(self->state->track_config,
			self->state->pv->cursor.track);			
		ev = trackdef_setevent_digit(trackdef, self->state->pv->cursor.column,
				self->state->pv->cursor.digit, &ev, newvalue);										
	} else {
		psy_audio_patternevent_setvalue(&ev, self->state->pv->cursor.column, newvalue);
	}
	trackergrid_prevent_pattern_sync(self);
	psy_undoredo_execute(&self->workspace->undoredo,
		&insertcommand_allocinit(pattern,
			self->state->pv->cursor, ev, 
			self->state->pv->sequence)->command);
	if (self->effcursor_always_down) {
		trackergrid_advance_line(self);
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;
		
		trackdef = trackerconfig_trackdef(self->state->track_config,
			self->state->pv->cursor.track);
		columndef = trackdef_column_def(trackdef, self->state->pv->cursor.column);
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
	trackergrid_invalidate_cursor(self);
	trackergrid_enable_pattern_sync(self);			
}

void trackergrid_invalidate_cursor(TrackerGrid* self)
{	
	assert(self);
	
	if (psy_ui_component_draw_visible(trackergrid_base(self))) {
		trackergrid_invalidate_internal_cursor(self, self->old_cursor);
		trackergrid_invalidate_internal_cursor(self, self->state->pv->cursor);
	}
	self->old_cursor = self->state->pv->cursor;
}

void trackergrid_invalidate_internal_cursor(TrackerGrid* self,
	psy_audio_SequenceCursor cursor)
{	
	psy_ui_Component* column;

	column = psy_ui_component_at(trackergrid_base(self), cursor.track);
	if (column) {
		psy_ui_RealSize size;

		size = psy_ui_component_scroll_size_px(column);
		psy_ui_component_invalidate_rect(column,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					0.0, trackerstate_beat_to_px(self->state,
						(patternviewstate_single_mode(self->state->pv))
						? psy_audio_sequencecursor_offset(&cursor)
						: psy_audio_sequencecursor_offset_abs(&cursor,
							self->state->pv->sequence))),
				psy_ui_realsize_make(
					trackerstate_trackwidth(self->state, cursor.track,
					psy_ui_component_textmetric(&self->component)),
					self->state->beat_convert.line_px)));
	}	
}

void trackergrid_invalidate_playbar(TrackerGrid* self)
{		
	double last;
	double curr;
	double minval;
	double maxval;
	psy_ui_RealSize size;

	if (!self->state->pv->sequence) {
		return;
	}	
	last = self->workspace->player.sequencer.hostseqtime.lastplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		last += psy_audio_sequencecursor_seqoffset(
			&self->workspace->player.sequencer.hostseqtime.lastplaycursor,
			self->state->pv->sequence);
	}
	last = trackerstate_beat_to_px(self->state, last);
	curr = self->workspace->player.sequencer.hostseqtime.currplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		curr += psy_audio_sequencecursor_seqoffset(
			&self->workspace->player.sequencer.hostseqtime.currplaycursor,
			self->state->pv->sequence);
	}
	curr = trackerstate_beat_to_px(self->state, curr);		
	minval = psy_min(last, curr);
	maxval = psy_max(last, curr);
	size = psy_ui_component_scroll_size_px(&self->component);
	psy_ui_component_invalidate_rect(&self->component, 
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(				
				psy_ui_component_scroll_left_px(&self->component),
				minval),
			psy_ui_realsize_make(
				size.width,
				maxval - minval + self->state->beat_convert.line_px)));	
}

void trackergrid_on_scroll(TrackerGrid* self, psy_ui_Component* sender)
{
	assert(self);

	if (self->state->midline) {
		trackergrid_clear_midline(self);
	}
}

void trackergrid_clear_midline(TrackerGrid* self)
{
	psy_ui_RealSize size;
	intptr_t visilines;

	assert(self);

	size = psy_ui_component_clientsize_px(&self->component);	
	self->state->midline = FALSE;
	visilines = (intptr_t)(size.height / self->state->beat_convert.line_px);
	psy_ui_component_invalidate_rect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scroll_left_px(&self->component),
				((visilines) / 2 - 1) * self->state->beat_convert.line_px +
					psy_ui_component_scroll_top_px(&self->component)),
			psy_ui_realsize_make(size.width,
			self->state->beat_convert.line_px)));
	psy_ui_component_update(&self->component);
	self->state->midline = TRUE;
	psy_ui_component_invalidate_rect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scroll_left_px(&self->component),
				(visilines / 2 - 2) * self->state->beat_convert.line_px +
				psy_ui_component_scroll_top_px(&self->component)),
			psy_ui_realsize_make(size.width,
				self->state->beat_convert.line_px * 4)));
}

void trackergrid_center_on_cursor(TrackerGrid* self)
{	
	intptr_t line;
	intptr_t visilines;
	psy_ui_RealSize size;	

	assert(self);

	if (patternviewstate_single_mode(self->state->pv)) {
		line = beatline_beat_to_line(&self->state->pv->beat_line,
			psy_audio_sequencecursor_offset(&self->state->pv->cursor));			
	} else {
		line = beatline_beat_to_line(&self->state->pv->beat_line,
			self->state->pv->cursor.offset);
	}
	size = psy_ui_component_scroll_size_px(psy_ui_component_parent(&self->component));
	visilines = (intptr_t)(size.height / self->state->beat_convert.line_px);
	psy_ui_component_set_scroll_top_px(&self->component,		
		-(visilines / 2 - line) * self->state->beat_convert.line_px);
}

void trackergrid_set_center_mode(TrackerGrid* self, intptr_t mode)
{
	assert(self);

	self->state->midline = (mode != FALSE);
	if (mode) {
		psy_ui_component_set_overflow(&self->component, (psy_ui_Overflow)
			(psy_ui_OVERFLOW_SCROLL | psy_ui_OVERFLOW_VSCROLLCENTER));
		trackergrid_center_on_cursor(self);
	} else {
		psy_ui_component_set_overflow(&self->component,
			psy_ui_OVERFLOW_SCROLL);
		psy_ui_component_set_scroll_top_px(&self->component, 0.0);
	}
}

void trackergrid_on_mouse_down(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	self->down = TRUE;
	if (trackdrag_active(&self->state->track_config->resize)) {
		psy_signal_emit(&self->signal_colresize, self, 0);
	} else if (patternviewstate_sequence(self->state->pv) && psy_ui_mouseevent_button(ev) == 1) {		
		self->last_drag_cursor = self->state->pv->selection.drag_base;
		psy_audio_blockselection_init_all(&self->state->pv->selection,
			self->last_drag_cursor, self->last_drag_cursor);
		psy_audio_blockselection_disable(&self->state->pv->selection);
		if (!psy_ui_component_has_focus(&self->component)) {
			psy_ui_component_set_focus(&self->component);
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
	if (trackdrag_active(&self->state->track_config->resize)) {
		psy_signal_emit(&self->signal_colresize, self, 0);
		psy_ui_mouseevent_stop_propagation(ev);
	} else {
		psy_audio_SequenceCursor cursor;
		TrackerColumn* column;
		uintptr_t index;

		column = (TrackerColumn*)psy_ui_component_intersect(
			&self->component, psy_ui_mouseevent_pt(ev), &index);
		if (column) {				
			cursor = trackerstate_make_cursor(self->state,
				psy_ui_mouseevent_pt(ev), column->track,
				self->state->beat_convert.line_px,
				psy_ui_component_textmetric(&self->component));
			if (!psy_audio_sequencecursor_equal(&cursor,
					&self->last_drag_cursor)) {
				psy_ui_RealRectangle rc;
				psy_ui_RealRectangle rc_curr;
				
				rc = trackergrid_selection_bounds(self);
				if (!psy_audio_blockselection_valid(
						&self->state->pv->selection)) {
					psy_audio_blockselection_startdrag(
						&self->state->pv->selection, cursor);
				} else {
					trackergrid_drag_selection(self, cursor);
				}					
				rc_curr = trackergrid_selection_bounds(self);
				psy_ui_realrectangle_union(&rc, &rc_curr);
				psy_ui_component_invalidate_rect(&self->component, rc);
				self->last_drag_cursor = cursor;
			}
		}
	}
}

void trackergrid_on_mouse_up(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_release_capture(&self->component);
	if (!self->down || psy_ui_mouseevent_button(ev) != 1) {
		self->down = FALSE;
		return;
	}
	self->down = FALSE;
	if (trackdrag_active(&self->state->track_config->resize)) {
		/* end track resize */
		trackdrag_stop(&self->state->track_config->resize);		
		psy_signal_emit(&self->signal_colresize, self, 0);		
	} else if (!psy_audio_blockselection_valid(&self->state->pv->selection)) {
		/* set cursor if no selection was made */
		trackergrid_set_cursor(self, self->state->pv->selection.drag_base);		
	}	
}

psy_ui_RealRectangle trackergrid_selection_bounds(TrackerGrid* self)
{
	psy_ui_RealRectangle rv;
	psy_audio_Sequence* sequence;

	psy_ui_realrectangle_init(&rv);
	
	sequence = self->state->pv->sequence;
	if (!sequence) {
		return rv;
	}			
	if (self->state->pv->selection.valid) {
		psy_audio_SequenceEntry* top_entry;
		psy_audio_SequenceEntry* bottom_entry;
		
		psy_dsp_big_beat_t top_abs;
		psy_dsp_big_beat_t bottom_abs;		
				
		top_entry = psy_audio_sequence_entry(sequence,
			self->state->pv->selection.topleft.order_index);
		if (!top_entry) {
			return rv;
		}
		bottom_entry = psy_audio_sequence_entry(sequence,
			self->state->pv->selection.bottomright.order_index);
		if (!bottom_entry) {
			return rv;
		}		
		top_abs = psy_audio_sequenceentry_offset(top_entry) +
			psy_audio_sequencecursor_offset(
				&self->state->pv->selection.topleft);
		bottom_abs = psy_audio_sequenceentry_offset(bottom_entry) +
			psy_audio_sequencecursor_offset(
				&self->state->pv->selection.bottomright);		
		if (patternviewstate_single_mode(self->state->pv)) {
			psy_audio_SequenceEntry* cursor_entry;
		
			cursor_entry = psy_audio_sequence_entry(sequence,
				self->state->pv->cursor.order_index);
			if (!cursor_entry) {
				return rv;
			}			
			rv.top = trackerstate_beat_to_px(self->state,
				top_abs - psy_audio_sequenceentry_offset(cursor_entry));
			rv.bottom = trackerstate_beat_to_px(self->state,
				bottom_abs - psy_audio_sequenceentry_offset(cursor_entry));			
		} else {			
			rv.top = trackerstate_beat_to_px(self->state, top_abs);
			rv.bottom = trackerstate_beat_to_px(self->state, bottom_abs);
		}
		rv.top = psy_max(0.0, rv.top);
		rv.bottom = psy_max(0.0, rv.bottom);
		if (rv.bottom < rv.top) {
			rv.bottom = rv.top;
		}
		if (rv.bottom > rv.top) {
			TrackerColumn* column;
			
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
	}
	return rv;
}

void trackergrid_on_mouse_doubleclick(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 1) {
		PatternSelect select;
		
		patternselect_init(&select, self->state->pv);
		patternselect_select_col(&select);
	}
}

void trackergrid_drag_selection(TrackerGrid* self,
	psy_audio_SequenceCursor cursor)
{
	bool restore_midline;
	
	restore_midline = self->state->midline;	
	self->state->midline = FALSE;	
	psy_audio_blockselection_drag(&self->state->pv->selection,
		cursor, self->state->pv->sequence);
	if (psy_audio_sequencecursor_offset(&cursor) <
			psy_audio_sequencecursor_offset(&self->last_drag_cursor)) {
		trackergrid_scroll_up(self, cursor);
	} else {
		trackergrid_scroll_down(self, cursor, FALSE);
	}
	if (psy_audio_sequencecursor_track(&cursor) <
			psy_audio_sequencecursor_offset(&self->last_drag_cursor)) {
		trackergrid_scroll_left(self, cursor);
	} else {
		trackergrid_scroll_right(self, cursor);
	}
	self->state->midline = restore_midline;
}

void trackergrid_enable_pattern_sync(TrackerGrid* self)
{
	assert(self);
	
	trackergrid_reset_pattern_sync(self);	
}

void trackergrid_prevent_pattern_sync(TrackerGrid* self)
{
	assert(self);

	trackergrid_reset_pattern_sync(self);	
}

void trackergrid_reset_pattern_sync(TrackerGrid* self)
{
	psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (pattern) {
		self->component.opcount = pattern->opcount;
	} else {
		self->component.opcount = 0;
	}
}

void trackergrid_block_start(TrackerGrid* self)
{
	assert(self);

	self->state->pv->selection = psy_audio_blockselection_make(		
		*patternviewstate_cursor(self->state->pv),
		*patternviewstate_cursor(self->state->pv));
	psy_audio_blockselection_startdrag(&self->state->pv->selection,
		*patternviewstate_cursor(self->state->pv));		
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_block_end(TrackerGrid* self)
{
	assert(self);

	psy_audio_blockselection_drag(&self->state->pv->selection,
		self->state->pv->cursor, self->state->pv->sequence);	
	psy_ui_component_invalidate(&self->component);
}

bool trackergrid_on_tracker_cmds(TrackerGrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);	
	return trackergrid_handle_command(self, cmd.id);
}

bool trackergrid_handle_command(TrackerGrid* self, intptr_t cmd)
{	
	assert(self);

	switch (cmd) {
	case CMD_NAVUP:
		if (patternviewstate_move_cursor_one_step(self->state->pv)) {
			trackergrid_prev_lines(self, 1, 0);
		} else {
			trackergrid_prev_line(self);
		}
		return TRUE;
	case CMD_NAVPAGEUP:
		trackergrid_prev_lines(self, patternviewstate_curr_pgup_down_step(
			self->state->pv), FALSE);
		return TRUE;
	case CMD_NAVDOWN:
		if (patternviewstate_move_cursor_one_step(self->state->pv)) {
			trackergrid_advance_lines(self, 1, FALSE);
		} else {
			trackergrid_advance_line(self);
		}
		return TRUE;
	case CMD_NAVPAGEDOWN:
		trackergrid_advance_lines(self, patternviewstate_curr_pgup_down_step(
			self->state->pv), FALSE);
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
		trackergrid_block_start(self);
		return TRUE;
	case CMD_BLOCKEND:
		trackergrid_block_end(self);
		return TRUE;
	case CMD_BLOCKUNMARK:
		patternviewstate_block_unmark(self->state->pv);
		return TRUE;
	case CMD_BLOCKCUT:
		patternviewstate_block_cut(self->state->pv);
		return TRUE;
	case CMD_BLOCKCOPY:
		patternviewstate_block_copy(self->state->pv);
		return TRUE;
	case CMD_BLOCKPASTE:
		patternviewstate_block_paste(self->state->pv);		
		return TRUE;
	case CMD_BLOCKMIX:
		patternviewstate_block_mixpaste(self->state->pv);
		return TRUE;
	case CMD_BLOCKDELETE:
		patternviewstate_block_delete(self->state->pv);
		return TRUE;
	case CMD_TRANSPOSEBLOCKINC:
		patternviewstate_block_transpose(self->state->pv, 1);
		return TRUE;
	case CMD_TRANSPOSEBLOCKDEC:
		patternviewstate_block_transpose(self->state->pv, -1);
		return TRUE;
	case CMD_TRANSPOSEBLOCKINC12:
		patternviewstate_block_transpose(self->state->pv, 12);
		return TRUE;
	case CMD_TRANSPOSEBLOCKDEC12:
		patternviewstate_block_transpose(self->state->pv, -12);
		return TRUE;
	case CMD_ROWDELETE:
		trackergrid_row_delete(self);			
		return TRUE;
	case CMD_ROWCLEAR:
		trackergrid_row_clear(self);			
		return TRUE;
	case CMD_SELECTALL: {
		PatternSelect select;
		
		patternselect_init(&select, self->state->pv);
		patternselect_select_all(&select);
		return TRUE; }
	case CMD_SELECTCOL: {
		PatternSelect select;
		
		patternselect_init(&select, self->state->pv);
		patternselect_select_col(&select);		
		return TRUE; }
	case CMD_SELECTBAR: {
		PatternSelect select;
		
		patternselect_init(&select, self->state->pv);
		patternselect_select_bar(&select);		
		return TRUE; }
	case CMD_SELECTMACHINE:
		trackergrid_select_machine(self);
		psy_ui_component_set_focus(&self->component);
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
				trackergrid_input_value(self, (uint8_t)digit, ISDIGIT);
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

	if (psy_property_at_bool(self->workspace->config.misc.misc, "record-tweaks", FALSE)) {
		psy_audio_PatternEvent event;
		psy_audio_Machine* machine;
		int value;
		psy_audio_Pattern* pattern;		

		pattern = patternviewstate_pattern(self->state->pv);
		if (!pattern) {
			return;
		}
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
		trackergrid_prevent_pattern_sync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(pattern, self->state->pv->cursor, event,
				&self->workspace->song->sequence)->command);		
		if (keyboardmiscconfig_advancelineonrecordtweak(&self->workspace->config.misc) &&
			!(keyboardmiscconfig_following_song(&self->workspace->config.misc) &&
				psy_audio_player_playing(workspace_player(self->workspace)))) {
			trackergrid_advance_line(self);
		}
		trackergrid_enable_pattern_sync(self);
	}
}

void trackergrid_on_client_align(TrackerGrid* self, psy_ui_Component* sender)
{	
	assert(self);
		
	if (trackergrid_midline(self)) {		
		trackergrid_center_on_cursor(self);				
	} 
}

void trackergrid_show_empty_data(TrackerGrid* self, int showstate)
{
	assert(self);

	self->state->show_empty_data = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_build(TrackerGrid* self)
{
	uintptr_t t;
		
	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->columns);
	for (t = 0; t < patternviewstate_num_song_tracks(self->state->pv); ++t) {
		TrackerColumn* column;

		column = trackercolumn_allocinit(&self->component, t, self->state,
			self->workspace);
		if (column) {
			psy_table_insert(&self->columns, t, (void*)column);
		}
	}
	psy_ui_component_align(&self->component);
}

void trackergrid_set_cursor(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	if (patternviewstate_sequence(self->state->pv)) {		
		psy_audio_sequence_set_cursor(patternviewstate_sequence(
			self->state->pv), cursor);		
	}
}

void trackergrid_on_timer(TrackerGrid* self, uintptr_t id)
{
	if (trackergrid_check_update(self)) {
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_on_focus(TrackerGrid* self)
{
	psy_ui_component_add_style_state(
		psy_ui_component_parent(psy_ui_component_parent(psy_ui_component_parent(&self->component))),
		psy_ui_STYLESTATE_SELECT);
}

void trackergrid_on_focus_lost(TrackerGrid* self)
{
	psy_ui_component_remove_style_state(
		psy_ui_component_parent(psy_ui_component_parent(psy_ui_component_parent(&self->component))),
		psy_ui_STYLESTATE_SELECT);
}

/* TrackerView */

/* prototypes */
static void trackerview_on_song_changed(TrackerView*, psy_audio_Player* sender);
static void trackerview_connect_song(TrackerView*);
static void trackerview_on_cursor_changed(TrackerView*, psy_audio_Sequence*
	sender);
static void trackerview_configure(TrackerView*);
static void trackerview_on_draw_empty_data(TrackerView*, psy_Property* sender);
static void trackerview_on_center_cursor(TrackerView*, psy_Property* sender);
static void trackerview_on_line_numbers(TrackerView*, psy_Property* sender);
static void trackerview_on_single_display(TrackerView*, psy_Property* sender);
static void trackerview_on_play_line_changed(TrackerView*, Workspace* sender);
static void trackerview_on_play_status_changed(TrackerView*, Workspace* sender);
static void trackerview_on_grid_scroll(TrackerView*, psy_ui_Component*);
static bool trackerview_playing_following_song(const TrackerView*);
static void trackerview_on_mouse_down(TrackerView*, psy_ui_MouseEvent*);
static void trackerview_on_draw(TrackerView*, psy_ui_Graphics*);
static void trackerview_on_sequence_tweak(TrackerView*,
	psy_audio_Sequence* sender);


/* vtable */
static psy_ui_ComponentVtable trackerview_vtable;
static bool trackerview_vtable_initialized = FALSE;

static void trackerview_vtable_init(TrackerView* self)
{
	if (!trackerview_vtable_initialized) {
		trackerview_vtable = *(self->component.vtable);
		trackerview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackerview_on_draw;
		trackerview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			trackerview_on_mouse_down;
		trackerview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &trackerview_vtable);
}

/* implementation */
void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerview_vtable_init(self);
	psy_ui_component_set_style_type(&self->component, STYLE_PV_TRACK_VIEW);
	psy_ui_component_set_style_type_select(&self->component,
		STYLE_PV_TRACK_VIEW_SELECT);
	self->workspace = workspace;
	/* hscroll */
	psy_ui_scrollbar_init(&self->hscroll, &self->component);
	psy_ui_component_set_align(&self->hscroll.component, psy_ui_ALIGN_BOTTOM);
	/* lines */
	trackerlinenumberview_init(&self->lines, &self->component, state, workspace);
	psy_ui_component_set_align(&self->lines.component, psy_ui_ALIGN_LEFT);
	/* grid */
	trackergrid_init(&self->grid, &self->component, state, &workspace->inputhandler,
		workspace);	
	psy_ui_component_set_wheel_scroll(&self->grid.component, 4);
	psy_ui_component_set_overflow(trackergrid_base(&self->grid),
		psy_ui_OVERFLOW_SCROLL);
	/* scroll */
	psy_ui_scroller_init(&self->scroller, &self->component, &self->hscroll,
		NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->grid.component);	
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->grid.component, psy_ui_ALIGN_FIXED);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		trackerview_on_song_changed);
	psy_signal_connect(&self->grid.component.signal_scrolled, self,
		trackerview_on_grid_scroll);
	psy_signal_connect(&self->workspace->player.sequencer.signal_play_line_changed, self,
		trackerview_on_play_line_changed);
	psy_signal_connect(&self->workspace->player.sequencer.signal_play_status_changed, self,
		trackerview_on_play_status_changed);
	trackerview_connect_song(self);	
	/* configuration */
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"drawemptydata", self, trackerview_on_draw_empty_data);
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"linenumbers", self, trackerview_on_line_numbers);
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"centercursoronscreen", self, trackerview_on_center_cursor);
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"displaysinglepattern", self, trackerview_on_single_display);
	trackerview_configure(self);
}

void trackerview_on_song_changed(TrackerView* self, psy_audio_Player* sender)
{
	psy_audio_sequencecursor_init(&self->grid.old_cursor);
	trackerview_connect_song(self);
}

void trackerview_connect_song(TrackerView* self)
{
	if (patternviewstate_sequence(self->grid.state->pv)) {		
		psy_signal_connect(&patternviewstate_sequence(
			self->grid.state->pv)->signal_cursorchanged,
			self, trackerview_on_cursor_changed);
		psy_signal_connect(&patternviewstate_sequence(
			self->grid.state->pv)->signal_tweak,
			self, trackerview_on_sequence_tweak);
	}
}

void trackerview_on_cursor_changed(TrackerView* self,
	psy_audio_Sequence* sender)
{		
	bool invalidate_cursor;
	bool invalidate_align;
			
	if (psy_audio_sequence_lpb_changed(sender)) {		
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
	invalidate_cursor = TRUE;
	invalidate_align = FALSE;
	if (patternviewstate_single_mode(self->grid.state->pv)) {		
		if (!psy_audio_orderindex_equal(
				&self->grid.state->pv->sequence->cursor.order_index,
				self->grid.state->pv->sequence->lastcursor.order_index)) {
			if (trackerview_playing_following_song(self)) {						
				psy_ui_component_set_scroll_top_px(&self->grid.component, 0.0);
			}
			invalidate_align = TRUE;
			invalidate_cursor = FALSE;
		}			
	} else if (trackerview_playing_following_song(self)) {		
		if (self->grid.state->pv->sequence->cursor.order_index.order <=
				self->grid.state->pv->sequence->lastcursor.order_index.order) {
			psy_ui_component_set_scroll_top_px(&self->grid.component, 0.0);
			invalidate_align = TRUE;
			invalidate_cursor = FALSE;		
		}
	}
	if (invalidate_cursor && !(trackerview_playing_following_song(self))) {
		trackerlinenumbers_invalidate_cursor(&self->lines.linenumbers);
		trackergrid_invalidate_cursor(&self->grid);
	}
	if (invalidate_align) {
		psy_ui_component_align(&self->grid.component);			
		psy_ui_component_align(&self->lines.pane);
		psy_ui_component_invalidate(&self->grid.component);
		psy_ui_component_invalidate(&self->lines.pane);				
	}
}

void trackerview_on_play_line_changed(TrackerView* self, Workspace* sender)
{
	if (!psy_ui_component_draw_visible(trackerview_base(self))) {
		return;
	}
	if (trackerview_playing_following_song(self)) {		
		trackergrid_scroll_down(&self->grid, *patternviewstate_cursor(
			self->grid.state->pv), FALSE);		
	}
	trackergrid_invalidate_playbar(&self->grid);
	trackerlinenumbers_invalidate_playbar(&self->lines.linenumbers);	
}

void trackerview_on_play_status_changed(TrackerView* self, Workspace* sender)
{	
	trackerlinenumbers_invalidate_playbar(&self->lines.linenumbers);
	trackergrid_invalidate_playbar(&self->grid);
	trackerlinenumbers_invalidate_cursor(&self->lines.linenumbers);
	trackergrid_invalidate_cursor(&self->grid);
}

bool trackerview_playing_following_song(const TrackerView* self)
{
	return self->workspace->player.sequencer.hostseqtime.currplaying &&
		keyboardmiscconfig_following_song(&self->workspace->config.misc);
}

void trackerview_on_grid_scroll(TrackerView* self, psy_ui_Component* sender)
{
	assert(self);

	trackerlinenumberview_set_scroll_top(&self->lines,
		psy_ui_component_scroll_top(&self->grid.component));
}

void trackerview_configure(TrackerView* self)
{
	PatternViewConfig* config;

	config = &self->workspace->config.visual.patview;
	if (patternviewconfig_is_smooth_scrolling(config)) {
		psy_ui_scroller_scroll_smooth(&self->scroller);
	} else {
		psy_ui_scroller_scroll_fast(&self->scroller);
	}
	if (patternviewconfig_center_cursor_on_screen(config)) {
		trackergrid_center_on_cursor(&self->grid);
	}
	trackergrid_show_empty_data(&self->grid,
		patternviewconfig_draw_empty_data(config));
	trackergrid_set_center_mode(&self->grid,
		patternviewconfig_center_cursor_on_screen(config));
	if (patternviewconfig_line_numbers(config)) {
		psy_ui_component_show(trackerlinenumberview_base(&self->lines));
	} else {
		psy_ui_component_hide(trackerlinenumberview_base(&self->lines));
	}
}

void trackerview_on_draw_empty_data(TrackerView* self, psy_Property* sender)
{
	trackergrid_show_empty_data(&self->grid, psy_property_item_bool(
		sender));
}

void trackerview_on_center_cursor(TrackerView* self, psy_Property* sender)
{
	trackergrid_set_center_mode(&self->grid, psy_property_item_int(sender));
}

void trackerview_on_line_numbers(TrackerView* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(trackerlinenumberview_base(&self->lines));		
	} else {
		psy_ui_component_hide(trackerlinenumberview_base(&self->lines));
	}	
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void trackerview_on_single_display(TrackerView* self, psy_Property* sender)
{
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void trackerview_on_mouse_down(TrackerView* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 1) {
		psy_ui_component_set_focus(&self->grid.component);
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void trackerview_on_draw(TrackerView* self, psy_ui_Graphics* g)
{
	trackerstate_update_abs_positions(self->grid.state,
		&self->workspace->player);
}

void trackerview_on_sequence_tweak(TrackerView* self,
	psy_audio_Sequence* sender)
{
	trackergrid_invalidate_internal_cursor(&self->grid,
		self->grid.state->pv->cursor);
}
