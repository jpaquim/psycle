/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogriddraw.h"
/* platform */
#include "../../detail/portable.h"


/* PianogridDraw */

/* prototypes */
static void pianogriddraw_draw_grid(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection);
static psy_audio_BlockSelection pianogriddraw_clip_selection(PianoGridDraw*,
	psy_ui_RealRectangle clip);
static void pianogriddraw_draw_cells(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_draw_step_separators(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_draw_key_separators(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_draw_playbar(PianoGridDraw*, psy_ui_Graphics*);
static void pianogriddraw_draw_entries(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection);
static bool pianogriddraw_has_track_display(PianoGridDraw*, uintptr_t track);
static PianogridTrackEvent* pianogriddraw_lasttrackevent_at(PianoGridDraw*,
	uintptr_t track, psy_Table* lasttrackevent);
static void pianogriddraw_lasttrackevent_dispose(PianoGridDraw*,
	psy_Table* lasttrackevent);
static void pianogriddraw_draw_event(PianoGridDraw*, psy_ui_Graphics*,
	PianogridTrackEvent*, psy_dsp_big_beat_t length);
static void pianogriddraw_draw_cursor(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection);
static psy_ui_Colour pianogriddraw_cellcolour(PianoGridDraw*,
	uintptr_t step, uint8_t key, bool sel);
static bool pianogriddraw_in_selection(PianoGridDraw*, uint8_t key,
	psy_dsp_big_beat_t offset_abs);
static void pianogriddraw_prepare_selection(PianoGridDraw* self,
	psy_audio_Sequence* sequence, const psy_audio_BlockSelection* selection);

/* implementation */
void pianogriddraw_init(PianoGridDraw* self,
	KeyboardState* keyboardstate, PianoGridState* gridstate,	
	psy_audio_PatternEntry* hoverpatternentry,	
	psy_ui_RealSize size, Workspace* workspace)
{	
	assert(self);
	assert(gridstate);
	
	self->workspace = workspace;
	self->keyboardstate = keyboardstate;
	self->gridstate = gridstate;
	self->size = size;	
	self->hoverpatternentry =hoverpatternentry;	
	self->trackdisplay = pianogridstate_track_display(self->gridstate);	
	self->drawgrid = TRUE;
	self->drawentries = TRUE;
	self->drawcursor = TRUE;
	self->drawplaybar = TRUE;	
}

psy_audio_BlockSelection pianogriddraw_clip_selection(PianoGridDraw* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_BlockSelection rv;

	assert(self);
	
	pianogridstate_clip(self->gridstate, clip.left, clip.right,
		&rv.topleft.offset, &rv.bottomright.offset);
	pianokeyboardstate_clip(self->keyboardstate,
		clip.top, clip.bottom,
		&rv.bottomright.key, &rv.topleft.key);
	return rv;
}

void pianogriddraw_on_draw(PianoGridDraw* self, psy_ui_Graphics* g)
{
	psy_audio_BlockSelection clip;

	assert(self);

	if (!patternviewstate_sequence(self->gridstate->pv)) {		
		return;
	}
	clip = pianogriddraw_clip_selection(self, psy_ui_graphics_cliprect(g));
	if (self->drawgrid) {
		pianogriddraw_draw_grid(self, g, clip);
	}
	if (self->drawplaybar) {
		pianogriddraw_draw_playbar(self, g);
	}
	if (self->drawentries) {
		pianogriddraw_draw_entries(self, g, clip);
	}
	if (self->drawcursor) {
		pianogriddraw_draw_cursor(self, g, clip);
	}
}

void pianogriddraw_draw_grid(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	assert(self);

	pianogriddraw_draw_cells(self, g, clip);
	pianogriddraw_draw_step_separators(self, g, clip);
	pianogriddraw_draw_key_separators(self, g, clip);
}

void pianogriddraw_draw_cells(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	uint8_t key;
	uintptr_t seqline;
	double seqoffset;

	assert(self);
	
	pianogriddraw_prepare_selection(self,
		self->gridstate->pv->sequence,
		&self->gridstate->pv->selection);
	if (patternviewstate_single_mode(self->gridstate->pv)) {
		seqoffset = psy_audio_sequencecursor_seqoffset(
			&self->gridstate->pv->cursor, self->gridstate->pv->sequence);
		seqline = (uintptr_t)(seqoffset *
			(double)self->gridstate->pv->cursor.lpb);
	} else {
		seqoffset = 0;
		seqline = 0;
	}
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {
		double cpy;
		uintptr_t line;
		psy_dsp_big_beat_t c;

		cpy = keyboardstate_key_to_px(self->keyboardstate, key);
		c = clip.topleft.offset;
		line = pianogridstate_beat_to_line(self->gridstate, c);
		for (; c <= clip.bottomright.offset;
			c += pianogridstate_step(self->gridstate), ++line) {
			psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					pianogridstate_beat_to_px(self->gridstate,
						patternviewstate_draw_offset(self->gridstate->pv, c)),
					cpy),
				psy_ui_realsize_make(
					pianogridstate_steppx(self->gridstate) + 1,
					self->keyboardstate->key_extent_px + 1)),									
				pianogriddraw_cellcolour(self, line + seqline, key,
					pianogriddraw_in_selection(self, key, c)));
		}
	}
}

bool pianogriddraw_in_selection(PianoGridDraw* self, uint8_t key,
	psy_dsp_big_beat_t offset_abs)
{	
	if (!psy_audio_blockselection_valid(&self->gridstate->pv->selection)) {
		return FALSE;
	}		
	if (!(key >= self->gridstate->pv->selection.topleft.key &&
			key < self->gridstate->pv->selection.bottomright.key)) {
		return FALSE;
	}					
	if (offset_abs >= self->selection_top_abs &&
		offset_abs < self->selection_bottom_abs) {
		return TRUE;
	}
	return FALSE;
}

psy_ui_Colour pianogriddraw_cellcolour(PianoGridDraw* self, uintptr_t step,
	uint8_t key, bool sel)
{
	psy_ui_Colour rv;
	psy_ui_Style* style;

	assert(self);

	if ((step % (self->gridstate->pv->cursor.lpb * 4)) == 0) {
		if (sel) {
			style = psy_ui_style(STYLE_PV_ROW4BEAT_SELECT);
			rv = style->background.colour;
			// patternviewskin_selection4beatcolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
		} else {
			style = psy_ui_style(STYLE_PV_ROW4BEAT);
			rv = style->background.colour;
			// rv = patternviewskin_row4beatcolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
		}
	} else if ((step % self->gridstate->pv->cursor.lpb) == 0) {
		if (sel) {
			style = psy_ui_style(STYLE_PV_ROWBEAT_SELECT);
			rv = style->background.colour;
			// rv = patternviewskin_selectionbeatcolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
		} else {
			style = psy_ui_style(STYLE_PV_ROWBEAT);
			rv = style->background.colour;
			//rv = patternviewskin_rowbeatcolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
		}
	} else {
		if (sel) {
			style = psy_ui_style(STYLE_PV_ROW_SELECT);
			rv = style->background.colour;
			// rv = patternviewskin_selectioncolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
		} else {
			style = psy_ui_style(STYLE_PV_ROW);
			rv = style->background.colour;
			// rv = patternviewskin_rowcolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
		}
	}
	if (psy_dsp_isblack(key)) {
		psy_ui_colour_add_rgb(&rv, -4, -4, -4);
	}
	return rv;
}

void pianogriddraw_prepare_selection(PianoGridDraw* self,
	psy_audio_Sequence* sequence, const psy_audio_BlockSelection* selection)
{	
	psy_audio_SequenceEntry* top_entry;
	psy_audio_SequenceEntry* bottom_entry;

	if (!psy_audio_blockselection_valid(&self->gridstate->pv->selection)) {
		return;
	}	
	self->selection_top_abs = self->selection_bottom_abs = 0.0;	
	if (!psy_audio_blockselection_valid(selection)) {
		return;
	}			
	top_entry = psy_audio_sequence_entry(sequence,
		selection->topleft.order_index);
	if (!top_entry) {
		return;
	}
	bottom_entry = psy_audio_sequence_entry(sequence,
		selection->bottomright.order_index);
	if (!bottom_entry) {
		return;
	}		
	self->selection_top_abs = psy_audio_sequenceentry_offset(top_entry) +
		psy_audio_sequencecursor_offset(&selection->topleft);
	self->selection_bottom_abs = psy_audio_sequenceentry_offset(bottom_entry) +
		psy_audio_sequencecursor_offset(&selection->bottomright);
}


void pianogriddraw_draw_step_separators(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	psy_dsp_big_beat_t c;
	psy_ui_Style* style;

	assert(self);

	style = psy_ui_style(STYLE_PATTERNVIEW);
	psy_ui_setcolour(g, style->background.colour);
	for (c = clip.topleft.offset; c <= clip.bottomright.offset;
		c += pianogridstate_step(self->gridstate)) {
		double cpx;

		cpx = pianogridstate_beat_to_px(self->gridstate,
			patternviewstate_draw_offset(self->gridstate->pv, c));
		psy_ui_drawline(g,
			psy_ui_realpoint_make(cpx, 0.0),
			psy_ui_realpoint_make(cpx,
				self->keyboardstate->keyboard_extent_px));
	}
}

/* draws key separators at C and E */
void pianogriddraw_draw_key_separators(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	uint8_t key;

	assert(self);

	// psy_ui_setcolour(g, patternviewskin_separatorcolour(patternviewstate_skin(self->gridstate->pv),
	//	0, 0));	
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {
		if (psy_dsp_iskey_c(key + 1) || psy_dsp_iskey_e(key + 1)) {
			double cpy;

			cpy = keyboardstate_key_to_px(self->keyboardstate, key);
			psy_ui_drawline(g,
				psy_ui_realpoint_make(
					pianogridstate_beat_to_px(self->gridstate,
						patternviewstate_draw_offset(self->gridstate->pv,
							clip.topleft.offset)),
					cpy),
				psy_ui_realpoint_make(
					pianogridstate_beat_to_px(self->gridstate,
						patternviewstate_draw_offset(self->gridstate->pv,
							clip.bottomright.offset)),
					cpy));
		}
	}
}

void pianogriddraw_draw_cursor(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_BlockSelection clip)
{
	assert(self);

	if (self->gridstate->pv->sequence &&
		!(psy_audio_player_playing(workspace_player(self->workspace)) &&
			keyboardmiscconfig_following_song(&self->workspace->config.misc))) {
		psy_ui_Style* style;
		psy_audio_SequenceCursor cursor;
		psy_audio_SequenceEntry* seq_entry;	
		double cursor_offset_abs;
		intptr_t key;
		
		cursor = self->gridstate->pv->cursor;		
		seq_entry = psy_audio_sequence_entry(self->gridstate->pv->sequence,
			psy_audio_sequencecursor_order_index(&cursor));			
		if (!seq_entry) {
			return;
		}
		cursor_offset_abs = psy_audio_sequencecursor_offset(&cursor) +
			psy_audio_sequenceentry_offset(seq_entry);
		if (!(cursor_offset_abs >= psy_audio_sequenceentry_offset(seq_entry) &&
				(cursor_offset_abs < psy_audio_sequenceentry_offset(seq_entry) +
				psy_audio_sequenceentry_length(seq_entry)))) {
			return;			
		}
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = psy_audio_NOTECOMMANDS_MIDDLEC;
		}
		style = psy_ui_style(STYLE_PV_CURSOR);		
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				pianogridstate_beat_to_px(self->gridstate,
					(patternviewstate_single_mode(self->gridstate->pv))
					? psy_audio_sequencecursor_offset(&cursor)
					: cursor_offset_abs),
				keyboardstate_key_to_px(self->keyboardstate, key)),
			psy_ui_realsize_make(
				pianogridstate_steppx(self->gridstate),
				self->keyboardstate->key_extent_px)),
			style->background.colour);		
		// patternviewskin_cursorcolour(patternviewstate_skin(self->gridstate->pv),
		//	0, 0));
	}
}

void pianogriddraw_draw_playbar(PianoGridDraw* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->workspace->player.sequencer.hostseqtime.currplaying) {
		psy_dsp_big_beat_t offset;		
		psy_ui_Style* style;
				
		offset = self->workspace->player.sequencer.hostseqtime.currplaycursor.abs_offset;
		if (patternviewstate_single_mode(self->gridstate->pv)) {			
			psy_audio_SequenceEntry* seq_entry;
			
			seq_entry = psy_audio_sequence_entry(self->gridstate->pv->sequence,
				psy_audio_sequencecursor_order_index(&self->gridstate->pv->cursor));
			if (!seq_entry) {
				return;
			}		
			if (!(offset >= psy_audio_sequenceentry_offset(seq_entry) &&
					offset < psy_audio_sequenceentry_offset(seq_entry) +
					psy_audio_sequenceentry_length(seq_entry))) {
				return;		
			}
			offset = psy_audio_sequencecursor_offset(
				&self->workspace->player.sequencer.hostseqtime.currplaycursor);
		}
		style = psy_ui_style(STYLE_PV_PLAYBAR);
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(											
					pianogridstate_beat_to_px(self->gridstate, offset),
					0.0),
				psy_ui_realsize_make(
					pianogridstate_steppx(self->gridstate),
					self->keyboardstate->keyboard_extent_px)),
			style->background.colour);
	}
}

void pianogriddraw_draw_entries(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	psy_Table lasttrackevent;

	assert(self);

	psy_table_init(&lasttrackevent);
	if (patternviewstate_sequence(self->gridstate->pv)) {
		psy_audio_PatternNode* currnode;
		psy_TableIterator it;
		psy_audio_SequenceTrackIterator ite;
		psy_dsp_big_beat_t seqoffset;
		psy_dsp_big_beat_t bpl;

		/*
		** iterates over the events of the pattern and draws events with notes. To
		** determine the length, the current event is stored for each track. In
		** the next turn the stored last event is drawn to the start of the
		** current. Finally the last events with no ending notereleases are drawn
		** till the end of the pattern.
		*/
		psy_audio_sequencetrackiterator_init(&ite);
		psy_audio_sequence_begin(self->gridstate->pv->sequence,			
				self->gridstate->pv->cursor.order_index.track,
			(patternviewstate_single_mode(self->gridstate->pv)
				? psy_audio_sequencecursor_seqoffset(&self->gridstate->pv->cursor,
					patternviewstate_sequence(self->gridstate->pv))
				: 0.0), &ite);
		currnode = ite.patternnode;
		seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
		bpl = psy_audio_sequencecursor_bpl(&self->gridstate->pv->cursor);
		while (currnode) {
			psy_audio_PatternEntry* curr;
			PianogridTrackEvent* last;

			curr = (psy_audio_PatternEntry*)psy_list_entry(currnode);
			if (curr->offset + seqoffset > clip.bottomright.offset) {
				break;
			}
			if (pianogriddraw_has_track_display(self, curr->track)) {
				psy_audio_PatternEvent* currevent;

				currevent = psy_audio_patternentry_front(curr);
				if (currevent->note <= psy_audio_NOTECOMMANDS_RELEASE) {
					last = pianogriddraw_lasttrackevent_at(self, curr->track,
						&lasttrackevent);
					/* active tells if a last event exists or not */
					if (last->active) {
						/* draw last to start of current */
						if (curr->offset + seqoffset >= clip.topleft.offset) {
							pianogriddraw_draw_event(self, g, last, curr->offset + seqoffset -
								last->offset);
						}
					}
					if (currevent->note == psy_audio_NOTECOMMANDS_RELEASE) {
						/*
						** length is known: one grid step
						** draw with last key
						*/
						if (!last->active) {
							last->note = psy_audio_NOTECOMMANDS_MIDDLEC;
						}
						last->offset = curr->offset + seqoffset;
						last->hover = (self->hoverpatternentry == curr);
						last->noterelease = TRUE;
						pianogriddraw_draw_event(self, g, last,
							pianogridstate_step(self->gridstate));
						/* and skip draw last event */
						last->noterelease = FALSE;
						last->active = FALSE;
					} else {
						last->active = TRUE;
						last->note = currevent->note;
						last->offset = curr->offset + seqoffset;
						last->hover = (self->hoverpatternentry == curr);
					}
				}
			}
			if (patternviewstate_single_mode(self->gridstate->pv)) {			
				psy_audio_patternnode_next(&currnode);
			} else {
				psy_audio_sequencetrackiterator_inc(&ite);
				currnode = ite.patternnode;
				seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
			}
		}
		/*
		** draws remaining events with no noterelease to the end of the
		** pattern
		*/
		for (it = psy_table_begin(&lasttrackevent);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			PianogridTrackEvent* last;
			
			last = (PianogridTrackEvent*)psy_tableiterator_value(&it);
			if (last->active && pianogriddraw_has_track_display(self, last->track)) {
				pianogriddraw_draw_event(self, g, last,
					patternviewstate_length(self->gridstate->pv) +
					(patternviewstate_single_mode(self->gridstate->pv)
						? psy_audio_sequencecursor_seqoffset(&self->gridstate->pv->cursor,
							patternviewstate_sequence(self->gridstate->pv))
						: 0.0) - last->offset);
			}
			last->active = FALSE;
		}
		pianogriddraw_lasttrackevent_dispose(self, &lasttrackevent);
		psy_audio_sequencetrackiterator_dispose(&ite);
	}
}

bool pianogriddraw_has_track_display(PianoGridDraw* self, uintptr_t track)
{
	assert(self);

	return (self->trackdisplay == PIANOROLL_TRACK_DISPLAY_ALL ||
		(self->trackdisplay == PIANOROLL_TRACK_DISPLAY_CURRENT &&
			track == self->gridstate->pv->cursor.track) ||
		(self->trackdisplay == PIANOROLL_TRACK_DISPLAY_ACTIVE &&
			!psy_audio_trackstate_istrackmuted(
				&self->gridstate->pv->sequence->patterns->trackstate, track)));
}

void pianogriddraw_lasttrackevent_dispose(PianoGridDraw* self, psy_Table* lasttrackevent)
{
	assert(self);

	psy_table_dispose_all(lasttrackevent, (psy_fp_disposefunc)NULL);
}

PianogridTrackEvent* pianogriddraw_lasttrackevent_at(PianoGridDraw* self, uintptr_t track, psy_Table* lasttrackevent)
{
	PianogridTrackEvent* rv;

	assert(self);

	rv = (PianogridTrackEvent*)psy_table_at(lasttrackevent, track);
	if (!rv) {
		rv = (PianogridTrackEvent*)malloc(sizeof(PianogridTrackEvent));
		if (rv) {
			rv->offset = -1.0f;
			rv->active = FALSE;
			rv->track = track;
			rv->note = psy_audio_NOTECOMMANDS_EMPTY;
			rv->hover = FALSE;
			rv->noterelease = FALSE;
			psy_table_insert(lasttrackevent, track, rv);
		}
	}
	return rv;
}

void pianogriddraw_draw_event(PianoGridDraw* self, psy_ui_Graphics* g,
	PianogridTrackEvent* ev, psy_dsp_big_beat_t length)
{
	psy_ui_RealRectangle r;
	psy_ui_Colour colour;
	psy_audio_SequenceCursor cursor;
	psy_ui_RealSize corner;
	double left;
	double width;

	assert(self);

	cursor = self->gridstate->pv->cursor;
	left = (patternviewstate_draw_offset(self->gridstate->pv,
		ev->offset)) * self->gridstate->beat_convert.line_px *
		psy_audio_sequencecursor_lpb(&self->gridstate->pv->cursor);
	width = length * self->gridstate->beat_convert.line_px *
		psy_audio_sequencecursor_lpb(&self->gridstate->pv->cursor);
	corner = psy_ui_realsize_make(2, 2);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			left,
			(self->keyboardstate->keymax - ev->note - 1) *
			self->keyboardstate->key_extent_px + 1),
		psy_ui_realsize_make(
			width,
			psy_max(1.0, self->keyboardstate->key_extent_px - 2)));	
	colour = psy_ui_colour_white();
	if (!ev->noterelease) {
		psy_ui_drawsolidroundrectangle(g, r, corner, colour);
	} else {		
		psy_ui_setcolour(g, colour);
		psy_ui_drawroundrectangle(g, r, corner);
	}
}
