/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogrid.h"
/* local */
#include "trackergridstate.h"
#include "patterncmds.h"
#include "patternnavigator.h"
#include "cmdsnotes.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define PIANOROLL_REFRESHRATE 50
#define CMD_ENTER 1100

static void setcmdall(psy_Property* cmds, uintptr_t cmd, uint32_t keycode,
	bool shift, bool ctrl, const char* key, const char* shorttext);

/* Pianogrid */

/* prototypes */
void pianogrid_connect_input_handler(Pianogrid*,
	InputHandler* input_handler);
static void pianogrid_on_draw(Pianogrid*, psy_ui_Graphics*);
static psy_audio_BlockSelection pianogrid_clipselection(Pianogrid*,
	psy_ui_RealRectangle clip);
static void pianogrid_draw_background(Pianogrid*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogrid_drawuncoveredrightbackground(Pianogrid*,
	psy_ui_Graphics* g, psy_ui_RealSize);
static void pianogrid_drawuncoveredbottombackground(Pianogrid*,
	psy_ui_Graphics*, psy_ui_RealSize);
static void pianogrid_on_preferred_size(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_on_mouse_down(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_on_mouse_move(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_on_mouse_up(Pianogrid*, psy_ui_MouseEvent*);
static psy_audio_SequenceCursor pianogrid_make_cursor(Pianogrid* self, double x, double y);
static void pianogrid_on_focus(Pianogrid*);
static void pianogrid_on_focus_lost(Pianogrid*);
static bool pianogrid_on_roll_cmds(Pianogrid*, InputHandler* sender);
static bool pianogrid_on_note_cmds(Pianogrid*, InputHandler* sender);
static bool pianogrid_handle_command(Pianogrid*, uintptr_t cmd);
static void pianogrid_navup(Pianogrid*);
static void pianogrid_navdown(Pianogrid*);
/* block operations */
static void pianogrid_enter(Pianogrid*);
static void pianogrid_rowclear(Pianogrid*);
static void pianogrid_blockstart(Pianogrid*);
static void pianogrid_blockend(Pianogrid*);

/* vtable */
static psy_ui_ComponentVtable pianogrid_vtable;
static bool pianogrid_vtable_initialized = FALSE;

static void pianogrid_vtable_init(Pianogrid* self)
{
	assert(self);

	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pianogrid_on_draw;
		pianogrid_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_down;
		pianogrid_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_up;
		pianogrid_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_move;
		pianogrid_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			pianogrid_on_preferred_size;
		pianogrid_vtable.on_focus =
			(psy_ui_fp_component_event)
			pianogrid_on_focus;
		pianogrid_vtable.on_focuslost =
			(psy_ui_fp_component_event)
			pianogrid_on_focus_lost;
		pianogrid_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pianogrid_base(self), &pianogrid_vtable);
}

/* implementation */
void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, PianoGridState* gridstate,
	InputHandler* input_handler, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(pianogrid_base(self), parent, NULL);	
	pianogrid_vtable_init(self);
	psy_ui_component_set_tab_index(&self->component, 0);
	psy_ui_component_set_style_type(&self->component, STYLE_PV_TRACK_VIEW_SELECT);
	psy_ui_component_set_wheel_scroll(pianogrid_base(self), 4);
	self->workspace = workspace;
	self->gridstate = gridstate;
	self->keyboardstate = keyboardstate;
	self->trackdisplay = PIANOROLL_TRACK_DISPLAY_ALL;	
	self->prevent_context_menu = FALSE;
	self->hoverpatternentry = NULL;
	self->cursoronnoterelease = FALSE;
	self->edit_mode = TRUE;
	self->preventscrollleft = FALSE;
	pianogrid_connect_input_handler(self, input_handler);
	psy_audio_sequencecursor_init(&self->old_cursor);
	psy_ui_component_set_overflow(pianogrid_base(self),
		psy_ui_OVERFLOW_SCROLL);	
}

void pianogrid_connect_input_handler(Pianogrid* self,
	InputHandler* input_handler)
{
	inputhandler_connect(&self->workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "pianoroll", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)pianogrid_on_roll_cmds);
	inputhandler_connect(&self->workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "notes", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)pianogrid_on_note_cmds);
}

void pianogrid_onpatternchange(Pianogrid* self)
{	
	assert(self);

	self->hoverpatternentry = NULL;	
	psy_ui_component_updateoverflow(pianogrid_base(self));
}

void pianogrid_settrackdisplay(Pianogrid* self, PianoTrackDisplay display)
{
	assert(self);

	self->trackdisplay = display;
	psy_ui_component_invalidate(pianogrid_base(self));
}

void pianogrid_on_draw(Pianogrid* self, psy_ui_Graphics* g)
{
	PianoGridDraw griddraw;	

	assert(self);

	if (!patternviewstate_sequence(self->gridstate->pv)) {		
		return;
	}	
	pianogrid_draw_background(self, g, 
		pianogrid_clipselection(self, psy_ui_graphics_cliprect(g)));
	pianogriddraw_init(&griddraw,
		self->keyboardstate, self->gridstate,
		self->gridstate->pv->cursor.seqoffset,
		self->hoverpatternentry,		
		self->trackdisplay,
		self->cursoronnoterelease,
		psy_ui_component_scroll_size_px(pianogrid_base(self)),		
		self->workspace);
	pianogriddraw_ondraw(&griddraw, g);
}

psy_audio_BlockSelection pianogrid_clipselection(Pianogrid* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_BlockSelection rv;

	assert(self);

	pianogridstate_clip(self->gridstate, clip.left, clip.right,
		&rv.topleft.absoffset, &rv.bottomright.absoffset);
	pianokeyboardstate_clip(self->keyboardstate,
		clip.top, clip.bottom,
		&rv.bottomright.key, &rv.topleft.key);
	return rv;
}

void pianogrid_draw_background(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{	
	psy_ui_RealSize size;
		
	assert(self);	

	size = psy_ui_component_scroll_size_px(pianogrid_base(self));
	pianogrid_drawuncoveredrightbackground(self, g, size);
	pianogrid_drawuncoveredbottombackground(self, g, size);
}

void pianogrid_drawuncoveredrightbackground(Pianogrid* self,
	psy_ui_Graphics* g, psy_ui_RealSize size)
{
	double blankstart;

	assert(self);	

	blankstart = pianogridstate_beattopx(self->gridstate,
		patternviewstate_length(self->gridstate->pv));
	if (blankstart - psy_ui_component_scroll_left_px(&self->component) <
			size.width) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				blankstart,
				psy_ui_component_scroll_top_px(&self->component)),
			psy_ui_realsize_make(size.width -
				(blankstart - psy_ui_component_scroll_left_px(
					pianogrid_base(self))),
				size.height)),
			psy_ui_component_background_colour(&self->component));
			// patternviewskin_separatorcolour(patternviewstate_skin(
			//	self->gridstate->pv), 1, 2));
	}
}

void pianogrid_drawuncoveredbottombackground(Pianogrid* self,
	psy_ui_Graphics* g, psy_ui_RealSize size)
{
	double blankstart;

	assert(self);	

	blankstart = self->keyboardstate->keyboard_extent_px;
	if (blankstart - psy_ui_component_scroll_top_px(&self->component) <
			size.height) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scroll_left_px(pianogrid_base(self)),
				blankstart),
			psy_ui_realsize_make(size.width,
				size.height - (blankstart - psy_ui_component_scroll_top_px(
					pianogrid_base(self))))),
			psy_ui_component_background_colour(&self->component));
			// patternviewskin_separatorcolour(patternviewstate_skin(
			//     self->gridstate->pv), 1, 2));
	}
}

void pianogrid_on_preferred_size(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	assert(self);
	
	rv->height = psy_ui_value_make_px((self->keyboardstate->keymax -
		self->keyboardstate->keymin) * self->keyboardstate->key_extent_px);	
	rv->width = psy_ui_value_make_px(pianogridstate_beattopx(self->gridstate,
		patternviewstate_length(self->gridstate->pv)));		
}

void pianogrid_on_mouse_down(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	self->dragcursor = pianogrid_make_cursor(self, psy_ui_mouseevent_pt(ev).x,
		psy_ui_mouseevent_pt(ev).y);	
	self->gridstate->pv->dragselectionbase = self->dragcursor;
	self->last_drag_cursor = self->dragcursor;
	if (psy_ui_mouseevent_button(ev) != 2) {
		psy_audio_blockselection_disable(&self->gridstate->pv->selection);
		self->gridstate->pv->selection.topleft = self->dragcursor;		
	}
	if (psy_ui_mouseevent_button(ev) == 2) {
		/* right button */
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_Pattern* pattern;
		
		pattern = patternviewstate_pattern(self->gridstate->pv);
		if (!pattern) {
			return;
		}
		node = psy_audio_pattern_find_node(
			pattern,
			self->gridstate->pv->cursor.track,
			self->dragcursor.absoffset - self->dragcursor.seqoffset,
			pianogridstate_step(self->gridstate), &prev);
		if (!node) {
			if (prev) {
				psy_audio_PatternEntry* preventry;

				preventry = psy_audio_patternnode_entry(prev);
				if (preventry->track != self->dragcursor.track) {
					prev = psy_audio_patternnode_prev_track(prev,
						self->dragcursor.track);
				}
			}
			node = prev;
		}
		if (node) {
			psy_audio_PatternNode* next;
			psy_audio_PatternEntry* patternentry;

			patternentry = psy_audio_patternnode_entry(node);
			if (!pianogrid_keyhittest(self, node,
					self->dragcursor.track, self->dragcursor.key)) {
				pianogrid_invalidate_cursor(self);
				return;
			}
			if (patternentry->offset == self->dragcursor.absoffset - self->dragcursor.seqoffset) {
				next = psy_audio_patternnode_next_track(node, self->dragcursor.track);
				if (self->hoverpatternentry == psy_audio_patternnode_entry(node)) {
					self->hoverpatternentry = NULL;
				}
				self->preventscrollleft = TRUE;
				psy_undoredo_execute(&self->workspace->undoredo,
					&removecommand_allocinit(pattern,
						self->dragcursor, &self->workspace->song->sequence)->command);
				self->prevent_context_menu = TRUE;
				self->preventscrollleft = FALSE;
				if (next) {
					psy_audio_PatternEntry* nextentry;

					nextentry = psy_audio_patternnode_entry(next);
					if (psy_audio_patternentry_front(nextentry)->note
						== psy_audio_NOTECOMMANDS_RELEASE) {
						if (self->hoverpatternentry == psy_audio_patternnode_entry(next)) {
							self->hoverpatternentry = NULL;
						}
						self->dragcursor.absoffset = nextentry->offset + self->dragcursor.seqoffset;
						self->preventscrollleft = TRUE;
						psy_undoredo_execute(&self->workspace->undoredo,
							&removecommand_allocinit(pattern,
								self->dragcursor, &self->workspace->song->sequence)->command);
						self->preventscrollleft = FALSE;
					}
				}
				pianogrid_set_cursor(self, self->dragcursor);
				psy_ui_component_invalidate(&self->component);
			} else {
				if (psy_audio_patternentry_front(patternentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
					pianogrid_set_cursor(self, self->dragcursor);					
					return;
				} else {
					psy_audio_PatternEvent release;

					next = psy_audio_patternnode_next_track(node, self->dragcursor.track);
					if (next) {
						psy_audio_PatternEntry* nextentry;

						nextentry = psy_audio_patternnode_entry(next);
						if (psy_audio_patternentry_front(nextentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
							psy_audio_SequenceCursor release;

							release = self->dragcursor;
							release.absoffset = nextentry->offset + self->dragcursor.seqoffset;
							self->preventscrollleft = TRUE;
							psy_undoredo_execute(&self->workspace->undoredo,
								&removecommand_allocinit(pattern,
									release, &self->workspace->song->sequence)->command);
							self->preventscrollleft = FALSE;
						}
					}
					psy_audio_patternevent_clear(&release);
					release.note = psy_audio_NOTECOMMANDS_RELEASE;
					self->preventscrollleft = TRUE;
					psy_undoredo_execute(&self->workspace->undoredo,
						&insertcommand_allocinit(patternviewstate_pattern(self->gridstate->pv),
							self->dragcursor, release, &self->workspace->song->sequence)->command);
					self->preventscrollleft = FALSE;
					self->prevent_context_menu = TRUE;
					self->dragcursor.key = psy_audio_patternentry_front(patternentry)->note;
					pianogrid_set_cursor(self, self->dragcursor);					
				}
			}
			psy_ui_mouseevent_stop_propagation(ev);
			self->gridstate->pv->selection.valid = FALSE;
		}
	} else if (!psy_ui_mouseevent_ctrl_key(ev)  &&	
			(psy_ui_mouseevent_button(ev) == 1)) {
		if (self->edit_mode) {
			psy_audio_PatternEvent patternevent;			

			patternevent = psy_audio_player_pattern_event(&self->workspace->player,
				self->dragcursor.key);
			patternevent.note = self->dragcursor.key;
			self->preventscrollleft = TRUE;
			psy_undoredo_execute(&self->workspace->undoredo,
				&insertcommand_allocinit(
					patternviewstate_pattern(self->gridstate->pv),
					self->dragcursor, patternevent, &self->workspace->song->sequence)->command);
			self->play_event = patternevent;
			psy_audio_player_playevent(&self->workspace->player, &self->play_event);
			self->preventscrollleft = FALSE;
			pianogrid_set_cursor(self, self->dragcursor);
		}
		psy_ui_component_invalidate(&self->component);
	}	
}

void pianogrid_on_mouse_move(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_SequenceCursor cursor;

	assert(self);
	
	
	if (patternviewstate_sequence(self->gridstate->pv)) {
		psy_audio_PatternEntry* oldhover;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_Pattern* pattern;

		oldhover = self->hoverpatternentry;
		pattern = patternviewstate_pattern(self->gridstate->pv);
		if (!pattern) {
			return;
		}
		node = psy_audio_pattern_find_node(pattern,
			self->gridstate->pv->cursor.track,
				pianogridstate_quantize(self->gridstate,
					pianogridstate_pxtobeat(self->gridstate,
						psy_ui_mouseevent_pt(ev).x -
						psy_ui_component_scroll_left_px(&self->component)) +
					(patternviewstate_single_mode(self->gridstate->pv))
					? self->gridstate->pv->cursor.seqoffset
					: 0.0),
				pianogridstate_step(self->gridstate), &prev);
		if (!node) {
			if (prev) {
				psy_audio_PatternEntry* preventry;

				preventry = psy_audio_patternnode_entry(prev);
				if (preventry->track != self->gridstate->pv->cursor.track) {
					prev = psy_audio_patternnode_prev_track(prev,
						self->gridstate->pv->cursor.track);
				}
				if (psy_audio_patternentry_front(preventry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
					prev = NULL;
				}
			}
			node = prev;
		}
		if (node && pianogrid_keyhittest(self, node, self->gridstate->pv->cursor.track,
			keyboardstate_screen_to_key(self->keyboardstate, psy_ui_mouseevent_pt(ev), 0.0))) {
			self->hoverpatternentry = psy_audio_patternnode_entry(node);
		} else {
			self->hoverpatternentry = NULL;
		}
		if (self->hoverpatternentry != oldhover) {
			psy_ui_component_invalidate(&self->component);
		}
		if (((psy_ui_mouseevent_button(ev) == 1) && (!self->edit_mode || psy_ui_mouseevent_ctrl_key(ev)))) {
			cursor = pianogrid_make_cursor(self, psy_ui_mouseevent_pt(ev).x,
				psy_ui_mouseevent_pt(ev).y);
			if (cursor.key != self->last_drag_cursor.key ||
				cursor.absoffset != self->last_drag_cursor.absoffset) {
				if (!self->gridstate->pv->selection.valid) {
					pianogrid_startdragselection(self, cursor);
				} else {
					pianogrid_dragselection(self, cursor);
				}
				psy_ui_component_invalidate(&self->component);
				self->last_drag_cursor = cursor;
			}
		}
	}
}

void pianogrid_startdragselection(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	psy_dsp_big_beat_t bpl;

	self->gridstate->pv->selection.valid = TRUE;
	bpl = 1.0 / psy_audio_player_lpb(workspace_player(self->workspace));	
	self->gridstate->pv->selection.topleft.track = cursor.track;
	self->gridstate->pv->selection.bottomright.track = cursor.track + 1;
	if (cursor.key >= self->gridstate->pv->dragselectionbase.key) {
		self->gridstate->pv->selection.topleft.key = self->gridstate->pv->dragselectionbase.key;
		self->gridstate->pv->selection.bottomright.key = cursor.key;
	} else {
		self->gridstate->pv->selection.topleft.key = cursor.key;
		self->gridstate->pv->selection.bottomright.key = self->gridstate->pv->dragselectionbase.key;
	}
	if (cursor.absoffset >= self->gridstate->pv->dragselectionbase.absoffset) {
		self->gridstate->pv->selection.topleft.absoffset = self->gridstate->pv->dragselectionbase.absoffset;
		self->gridstate->pv->selection.bottomright.absoffset = cursor.absoffset + bpl;
	} else {
		self->gridstate->pv->selection.topleft.absoffset = cursor.absoffset;
		self->gridstate->pv->selection.bottomright.absoffset = self->gridstate->pv->dragselectionbase.absoffset + bpl;
	}
	self->gridstate->pv->selection.bottomright.key += 1;
}

void pianogrid_dragselection(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	psy_dsp_big_beat_t bpl;

	bpl = 1.0 / psy_audio_player_lpb(workspace_player(self->workspace));
	/* intptr_t restoremidline = self->midline; */
	if (cursor.key >= self->gridstate->pv->dragselectionbase.key) {
		self->gridstate->pv->selection.topleft.key = self->gridstate->pv->dragselectionbase.key;
		self->gridstate->pv->selection.bottomright.key = cursor.key + 1;
	} else {
		self->gridstate->pv->selection.topleft.key = cursor.key;
		self->gridstate->pv->selection.bottomright.key = self->gridstate->pv->dragselectionbase.key + 1;
	}
	if (cursor.absoffset >= self->gridstate->pv->dragselectionbase.absoffset) {
		self->gridstate->pv->selection.topleft.absoffset = self->gridstate->pv->dragselectionbase.absoffset;
		self->gridstate->pv->selection.bottomright.absoffset = cursor.absoffset + bpl;
	} else {
		self->gridstate->pv->selection.topleft.absoffset = cursor.absoffset;
		self->gridstate->pv->selection.bottomright.absoffset = self->gridstate->pv->dragselectionbase.absoffset + bpl;
	}
	/* self->midline = 0; */
	if (cursor.absoffset < self->last_drag_cursor.absoffset) {
		/* pianogrid_scrollleft(self, cursor); */
	} else {	
		/* pianogrid_scrollright(self, cursor); */
	}
	if (cursor.key < self->last_drag_cursor.key) {
		/* pianogrid_scrollup(self, cursor); */
	} else {
		/* pianogrid_scrolldown(self, cursor); */
	}
	/* self->midline = restoremidline; */
}


void pianogrid_on_mouse_up(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);
		
	if (self->prevent_context_menu) {				
		self->prevent_context_menu = FALSE;			
	} else if (!self->gridstate->pv->selection.valid) {		
		if (!self->edit_mode || ((psy_ui_mouseevent_button(ev) == 1 && psy_ui_mouseevent_ctrl_key(ev)))) {
			pianogrid_set_cursor(self, self->dragcursor);			
		}
	}
	self->play_event.note = psy_audio_NOTECOMMANDS_RELEASE;
	psy_audio_player_playevent(&self->workspace->player, &self->play_event);
	psy_ui_mouseevent_stop_propagation(ev);
}

psy_audio_SequenceCursor pianogrid_make_cursor(Pianogrid* self, double x, double y)
{
	psy_audio_SequenceCursor rv;

	rv = self->gridstate->pv->cursor;
	psy_audio_sequencecursor_updateseqoffset(&rv,
		&self->workspace->song->sequence);	
	rv.absoffset = pianogridstate_quantize(self->gridstate,
		pianogridstate_pxtobeat(self->gridstate, x)) +
		((patternviewstate_single_mode(self->gridstate->pv))
		? rv.seqoffset
		: 0);
	rv.key = keyboardstate_screen_to_key(self->keyboardstate, psy_ui_realpoint_make(0, y), 0.0);	
	return rv;
}

bool pianogrid_keyhittest(Pianogrid* self, psy_audio_PatternNode* node,
	uintptr_t track, uint8_t cursorkey)
{
	psy_audio_PatternEntry* patternentry;
	bool noteoff;

	assert(self);

	noteoff = FALSE;
	patternentry = psy_audio_patternnode_entry(node);
	if (psy_audio_patternentry_front(patternentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
		psy_audio_PatternNode* prevtrack;

		/*
		** determine noterelease key
		** (note: noterelease event has no key but is drawn next to
		** the prev note or if there is none with middlec as key)
		*/
		prevtrack = psy_audio_patternnode_prev_track(node, track);
		if (prevtrack) {
			/* compare cursor key with the previous note key */
			if (cursorkey == psy_audio_patternentry_front(
				psy_audio_patternnode_entry(prevtrack))->note) {
				noteoff = TRUE;
			}
		} else if (cursorkey == psy_audio_NOTECOMMANDS_MIDDLEC) {
			/* compare cursor key with middlec key */
			noteoff = TRUE;
		}
	}
	return !(psy_audio_patternentry_front(patternentry)->note != cursorkey && !noteoff);
}

void pianogrid_update_cursor(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	assert(self);

	self->gridstate->pv->cursor = cursor;
	if (!psy_ui_component_draw_visible(&self->component)) {
		self->old_cursor = self->gridstate->pv->cursor;
		return;
	}
	pianogrid_invalidate_cursor(self);	
}

void pianogrid_navup(Pianogrid* self)
{
	if (self->gridstate->pv->cursor.key < self->keyboardstate->keymax - 1) {
		psy_audio_SequenceCursor cursor;

		cursor = pianogridstate_cursor(self->gridstate);
		++cursor.key;
		pianogrid_scrollup(self, self->gridstate->pv->cursor);
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_navdown(Pianogrid* self)
{
	if (self->gridstate->pv->cursor.key > self->keyboardstate->keymin) {
		psy_audio_SequenceCursor cursor;

		cursor = pianogridstate_cursor(self->gridstate);
		--cursor.key;
		pianogrid_scrolldown(self, self->gridstate->pv->cursor);
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_enter(Pianogrid* self)
{
	psy_audio_PatternEvent patternevent;

	psy_audio_patternevent_clear(&patternevent);
	patternevent.note = self->gridstate->pv->cursor.key;
	psy_undoredo_execute(&self->workspace->undoredo,
		&insertcommand_allocinit(patternviewstate_pattern(self->gridstate->pv),
			self->gridstate->pv->cursor, patternevent,
			&self->workspace->song->sequence)->command);
	pianogrid_advanceline(self);
}

void pianogrid_rowclear(Pianogrid* self)
{
	psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->gridstate->pv);
	if (!pattern) {
		return;
	}
	psy_undoredo_execute(&self->workspace->undoredo,
		&removecommand_allocinit(pattern,
			self->gridstate->pv->cursor, &self->workspace->song->sequence)->command);
	pianogrid_advanceline(self);
}

void pianogrid_blockstart(Pianogrid* self)
{
	self->gridstate->pv->dragselectionbase = self->gridstate->pv->cursor;
	pianogrid_startdragselection(self, self->gridstate->pv->cursor);
	psy_ui_component_invalidate(&self->component);
}

void pianogrid_blockend(Pianogrid* self)
{
	pianogrid_dragselection(self, self->gridstate->pv->cursor);
	psy_ui_component_invalidate(&self->component);
}

void pianogrid_prevline(Pianogrid* self)
{
	assert(self);

	pianogrid_prevlines(self, 
		keyboardmiscconfig_cursor_step(self->gridstate->pv->keymiscconfig),
		TRUE);
}

void pianogrid_prevlines(Pianogrid* self, uintptr_t lines, bool wrap)
{	
	assert(self);

	if (patternviewstate_pattern(self->gridstate->pv) &&
		patternviewstate_sequence(self->gridstate->pv)) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinennavigator_init(&navigator, self->gridstate->pv, wrap);
		cursor = patternlinennavigator_up(&navigator, lines,
			self->gridstate->pv->cursor);
		if (!patternlinennavigator_wrap(&navigator)) {
			pianogrid_scrollleft(self, cursor);
		} else {
			pianogrid_scrollright(self, cursor);
		}
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_prevkeys(Pianogrid* self, uint8_t lines, bool wrap)
{
	assert(self);

	if (patternviewstate_pattern(self->gridstate->pv)) {
		psy_audio_SequenceCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->pv->cursor,
			patternviewstate_pattern(self->gridstate->pv),
			pianogridstate_step(self->gridstate), wrap,
			self->keyboardstate->keymax);		
		if (psy_audio_patterncursornavigator_prevkeys(&cursornavigator,
			lines)) {
			pianogrid_scrollup(self, self->gridstate->pv->cursor);
		} else {
			pianogrid_scrolldown(self, self->gridstate->pv->cursor);
		}
		pianogrid_set_cursor(self, self->gridstate->pv->cursor);
	}
}

void pianogrid_advanceline(Pianogrid* self)
{
	assert(self);

	pianogrid_advancelines(self, keyboardmiscconfig_cursor_step(
		self->gridstate->pv->keymiscconfig), TRUE);
}

void pianogrid_advancelines(Pianogrid* self, uintptr_t lines, bool wrap)
{
	assert(self);
	assert(self->workspace);

	if (patternviewstate_pattern(self->gridstate->pv) &&
		patternviewstate_sequence(self->gridstate->pv)) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinennavigator_init(&navigator, self->gridstate->pv, wrap);
		cursor = patternlinennavigator_down(&navigator, lines,
			self->gridstate->pv->cursor);
		if (patternlinennavigator_wrap(&navigator)) {
			pianogrid_scrollleft(self, cursor);
		} else {
			pianogrid_scrollright(self, cursor);
		}
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_advancekeys(Pianogrid* self, uint8_t lines, bool wrap)
{
	assert(self);

	if (patternviewstate_pattern(self->gridstate->pv)) {
		psy_audio_SequenceCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->pv->cursor,
			patternviewstate_pattern(self->gridstate->pv),
			pianogridstate_step(self->gridstate), wrap,
			self->keyboardstate->keymax);		
		if (psy_audio_patterncursornavigator_advancekeys(&cursornavigator, lines)) {
			pianogrid_scrollup(self, self->gridstate->pv->cursor);
		} else {
			pianogrid_scrolldown(self, self->gridstate->pv->cursor);
		}
		pianogrid_set_cursor(self, self->gridstate->pv->cursor);
	}
}

bool pianogrid_scrollleft(Pianogrid* self, psy_audio_SequenceCursor cursor)
{		
	assert(self);
	
	if (pianogridstate_pxtobeat(self->gridstate,
		psy_ui_component_scroll_left_px(&self->component)) > patternviewstate_draw_offset(
			self->gridstate->pv, cursor.absoffset)) {
		psy_ui_component_set_scroll_left(&self->component,
			psy_ui_value_make_px(pianogridstate_quantizebeattopx(
				self->gridstate,
				patternviewstate_draw_offset(self->gridstate->pv,
					cursor.absoffset))));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scrollright(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t visilines;
	psy_ui_RealSize clientsize;

	assert(self);

	clientsize = psy_ui_component_clientsize_px(&self->component);
	visilines = (intptr_t)((clientsize.width /
		(psy_dsp_big_beat_t)self->gridstate->pxperbeat) * cursor.lpb);	
	--visilines;	
	line = pianogridstate_beattosteps(self->gridstate,
		patternviewstate_draw_offset(self->gridstate->pv,
		cursor.absoffset));
	if (visilines < line - psy_ui_component_scroll_left_px(&self->component) /
		pianogridstate_steppx(self->gridstate) + 2) {
		intptr_t dlines;
		
		dlines = (intptr_t)((line - psy_ui_component_scroll_left_px(&self->component) /
			pianogridstate_steppx(self->gridstate) - visilines + 16));
		self->component.blitscroll = TRUE;
		psy_ui_component_set_scroll_left(&self->component,		
			psy_ui_value_make_px(
				psy_max(0.0, psy_ui_component_scroll_left_px(&self->component) +
					psy_ui_component_scrollstep_width_px(&self->component) * dlines)));
		self->component.blitscroll = FALSE;
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scrollup(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t topline;
	intptr_t dlines;	
	double linepx;

	assert(self);
	
	line = self->keyboardstate->keymax - cursor.key - 2;
	linepx = self->keyboardstate->key_extent_px * line;
	topline = 0;
	dlines = (intptr_t)((psy_ui_component_scroll_top_px(&self->component) - linepx) /
		(self->keyboardstate->key_extent_px));
	if (dlines > 0) {		
		psy_ui_component_set_scroll_top(&self->component,
			psy_ui_value_make_px(
			psy_max(0, psy_ui_component_scroll_top_px(&self->component) -
				psy_ui_component_scroll_step_height_px(&self->component) * dlines)));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scrolldown(Pianogrid* self, psy_audio_SequenceCursor cursor)
{	
	psy_ui_RealSize clientsize;	
	intptr_t topline;
	intptr_t line;
	intptr_t visilines;	
	intptr_t dlines;

	assert(self);

	clientsize = psy_ui_component_clientsize_px(&self->component);		
	visilines = (intptr_t)floor(clientsize.height / self->keyboardstate->key_extent_px);
	topline = (intptr_t)ceil(psy_ui_component_scroll_top_px(&self->component) /
		self->keyboardstate->key_extent_px);
	line = self->keyboardstate->keymax - cursor.key + 1;
	dlines = (intptr_t)(line - topline - visilines);
	if (dlines > 0) {		
		psy_ui_component_set_scroll_top(&self->component,
			psy_ui_value_make_px(
				psy_ui_component_scroll_top_px(&self->component) +
				psy_ui_component_scroll_step_height_px(&self->component) *
				dlines));
		return FALSE;
	}
	return TRUE;

}

void pianogrid_invalidate_line(Pianogrid* self, intptr_t line)
{	
	double scrolltoppx;
	intptr_t seqline;
	psy_ui_RealRectangle r1;
	psy_ui_RealSize size;
	psy_ui_RealSize stepsize;

	assert(self);

	scrolltoppx = psy_ui_component_scroll_top_px(pianogrid_base(self));
	size = psy_ui_component_clientsize_px(&self->component);
	stepsize = psy_ui_realsize_make(pianogridstate_steppx(self->gridstate),
		size.height);
	seqline = 0;
	if (patternviewstate_single_mode(self->gridstate->pv)) {
		seqline = (intptr_t)(self->gridstate->pv->cursor.seqoffset *
			self->gridstate->pv->cursor.lpb);
	}
	r1 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line - seqline) * stepsize.width, scrolltoppx), stepsize);
	psy_ui_component_invalidate_rect(pianogrid_base(self), r1);	
}

void pianogrid_invalidate_lines(Pianogrid* self, intptr_t line1, intptr_t line2)
{
	psy_ui_RealSize size;
	psy_ui_RealSize stepsize;
	psy_ui_RealRectangle r1;
	psy_ui_RealRectangle r2;	
	double scrolltoppx;
	intptr_t seqline;

	assert(self);

	scrolltoppx = psy_ui_component_scroll_top_px(pianogrid_base(self));	
	size = psy_ui_component_clientsize_px(&self->component);
	stepsize = psy_ui_realsize_make(pianogridstate_steppx(self->gridstate),
		size.height);
	seqline = 0;
	if (patternviewstate_single_mode(self->gridstate->pv)) {
		seqline = (intptr_t)(self->gridstate->pv->cursor.seqoffset *
			self->gridstate->pv->cursor.lpb);
	}
	r1 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line1 - seqline) * stepsize.width, scrolltoppx), stepsize);
	r2 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line2 - seqline) * stepsize.width, scrolltoppx), stepsize);
	psy_ui_realrectangle_union(&r1, &r2);
	psy_ui_component_invalidate_rect(pianogrid_base(self), r1);
}

void pianogrid_invalidate_cursor(Pianogrid* self)
{
	assert(self);

	if (psy_ui_component_draw_visible(pianogrid_base(self))) {
		pianogrid_invalidate_line(self, self->old_cursor.linecache);
		pianogrid_invalidate_line(self, self->gridstate->pv->cursor.linecache);
	}	
	self->old_cursor = self->gridstate->pv->cursor;
}

void pianogrid_set_cursor(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	if (patternviewstate_sequence(self->gridstate->pv)) {
		bool restore;

		restore = self->preventscrollleft;
		self->preventscrollleft = TRUE;
		psy_audio_sequence_set_cursor(patternviewstate_sequence(
			self->gridstate->pv), cursor);
		self->preventscrollleft = restore;
	}
}

void pianogrid_on_focus(Pianogrid* self)
{
	psy_ui_component_add_style_state(
		psy_ui_component_parent(psy_ui_component_parent(psy_ui_component_parent(&self->component))),
		psy_ui_STYLESTATE_SELECT);
}

void pianogrid_on_focus_lost(Pianogrid* self)
{
	psy_ui_component_remove_style_state(
		psy_ui_component_parent(psy_ui_component_parent(psy_ui_component_parent(&self->component))),
		psy_ui_STYLESTATE_SELECT);
}

bool pianogrid_on_note_cmds(Pianogrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	assert(self);

	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {
		psy_audio_PatternEvent ev;
		bool chord;

		chord = FALSE;
		if (cmd.id >= CMD_NOTE_OFF_C_0 && cmd.id < 255) {
			ev = psy_audio_player_pattern_event(&self->workspace->player, (uint8_t)cmd.id);
			ev.note = CMD_NOTE_STOP;
			psy_audio_player_playevent(&self->workspace->player, &ev);
			return 1;
		} else if (cmd.id == CMD_NOTE_CHORD_END) {
			self->gridstate->pv->cursor.track = self->gridstate->pv->chord_begin;
			return 1;
		} else if (cmd.id >= CMD_NOTE_CHORD_C_0 && cmd.id < CMD_NOTE_STOP) {
			chord = TRUE;
			ev = psy_audio_player_pattern_event(&self->workspace->player,
				(uint8_t)cmd.id - (uint8_t)CMD_NOTE_CHORD_C_0);
		} else if (cmd.id < 256) {
			chord = FALSE;
			ev = psy_audio_player_pattern_event(&self->workspace->player, (uint8_t)cmd.id);
		}
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(patternviewstate_pattern(self->gridstate->pv),
				self->gridstate->pv->cursor, ev,
				&self->workspace->song->sequence)->command);
		if (chord != FALSE) {
			++self->gridstate->pv->cursor.track;
		} else {
			self->gridstate->pv->cursor.track = self->gridstate->pv->chord_begin;
		}
		if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
			self->gridstate->pv->cursor.key = ev.note;
		}
		pianogrid_set_cursor(self, pianogridstate_cursor(self->gridstate));
		return 1;
	}
	return 0;
}


bool pianogrid_on_roll_cmds(Pianogrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);
	return pianogrid_handle_command(self, cmd.id);
}

bool pianogrid_handle_command(Pianogrid* self, uintptr_t cmd)
{
	bool handled;

	assert(self);

	handled = TRUE;
	switch (cmd) {
	case CMD_NAVUP:
		pianogrid_navup(self);
		break;
	case CMD_NAVPAGEUP:
		pianogrid_prevlines(self,
			patternviewstate_curr_pgup_down_step(self->gridstate->pv), 0);
		break;
	case CMD_NAVPAGEUPKEYBOARD:
		pianogrid_advancekeys(self, 12, 0);
		break;
	case CMD_NAVDOWN:
		pianogrid_navdown(self);
		break;
	case CMD_NAVPAGEDOWN:
		pianogrid_advancelines(self,
			patternviewstate_curr_pgup_down_step(self->gridstate->pv), 0);
		break;
	case CMD_NAVPAGEDOWNKEYBOARD:
		pianogrid_prevkeys(self, 12, 0);
		break;
	case CMD_NAVLEFT:
		pianogrid_prevline(self);
		break;
	case CMD_NAVRIGHT:
		pianogrid_advanceline(self);
		break;
	case CMD_BLOCKSTART:
		pianogrid_blockstart(self);
		break;
	case CMD_BLOCKEND:
		pianogrid_blockend(self);
		break;
	case CMD_ENTER: {
		pianogrid_enter(self);
		break; }
	case CMD_ROWCLEAR:
		pianogrid_rowclear(self);
		break;
	case CMD_BLOCKPASTE:
		patternviewstate_block_paste(self->gridstate->pv);
		break;
	case CMD_BLOCKCOPY:
		patternviewstate_block_copy(self->gridstate->pv);
		break;
	case CMD_BLOCKCUT:
		patternviewstate_block_cut(self->gridstate->pv);
		break;
	case CMD_BLOCKDELETE:
		patternviewstate_block_delete(self->gridstate->pv);
		break;
	case CMD_SELECTALL:
		patternviewstate_select_all(self->gridstate->pv);
		break;
	case CMD_SELECTBAR:
		patternviewstate_select_bar(self->gridstate->pv);
		break;
	case CMD_BLOCKUNMARK:
		patternviewstate_block_unmark(self->gridstate->pv);
		break;
	default:
		handled = FALSE;
		break;
	}
	return handled;
}
