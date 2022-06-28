/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogriddraw.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"


/* PianogridDraw */

/* prototypes */
static void pianogriddraw_drawgrid(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection);
static psy_audio_BlockSelection pianogriddraw_clipselection(PianoGridDraw*,
	psy_ui_RealRectangle clip);
static void pianogriddraw_drawgridcells(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_drawstepseparators(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_drawkeyseparators(PianoGridDraw* self, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_drawplaybar(PianoGridDraw*, psy_ui_Graphics*, psy_audio_BlockSelection);
static void pianogriddraw_drawentries(PianoGridDraw*, psy_ui_Graphics*, psy_audio_BlockSelection);
static bool pianogriddraw_hastrackdisplay(PianoGridDraw*, uintptr_t track);
static PianogridTrackEvent* pianogriddraw_lasttrackevent_at(PianoGridDraw* self,
	uintptr_t track, psy_Table* lasttrackevent);
static void pianogriddraw_lasttrackevent_dispose(PianoGridDraw*,
	psy_Table* lasttrackevent);
static void pianogriddraw_drawevent(PianoGridDraw*, psy_ui_Graphics*,
	PianogridTrackEvent*, psy_dsp_big_beat_t length);
static void pianogriddraw_drawcursor(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection);
static psy_ui_Colour pianogriddraw_cellcolour(PianoGridDraw* self,
	uintptr_t step, uint8_t key, bool sel);

/* implementation */
void pianogriddraw_init(PianoGridDraw* self,
	KeyboardState* keyboardstate, PianoGridState* gridstate,	
	psy_dsp_big_beat_t sequenceentryoffset,
	psy_audio_PatternEntry* hoverpatternentry,	
	PianoTrackDisplay trackdisplay,
	bool cursoronnoterelease,	
	psy_ui_RealSize size, Workspace* workspace)
{	
	self->workspace = workspace;
	self->keyboardstate = keyboardstate;
	self->gridstate = gridstate;
	self->size = size;	
	self->sequenceentryoffset = sequenceentryoffset;
	self->hoverpatternentry =hoverpatternentry;	
	self->trackdisplay = trackdisplay;	
	self->cursoronnoterelease = cursoronnoterelease;	
	self->drawgrid = TRUE;
	self->drawentries = TRUE;
	self->drawcursor = TRUE;
	self->drawplaybar = TRUE;	
}

psy_audio_BlockSelection pianogriddraw_clipselection(PianoGridDraw* self,
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

void pianogriddraw_ondraw(PianoGridDraw* self, psy_ui_Graphics* g)
{
	psy_audio_BlockSelection clip;

	assert(self);

	if (!patternviewstate_sequence(self->gridstate->pv)) {		
		return;
	}
	clip = pianogriddraw_clipselection(self, psy_ui_graphics_cliprect(g));
	/* self->cursoronnoterelease = FALSE; */
	if (self->drawgrid) {
		pianogriddraw_drawgrid(self, g, clip);
	}
	if (self->drawplaybar) {
		pianogriddraw_drawplaybar(self, g, clip);
	}
	if (self->drawentries) {
		pianogriddraw_drawentries(self, g, clip);
	}
	if (self->drawcursor) {
		pianogriddraw_drawcursor(self, g, clip);
	}
}

void pianogriddraw_drawgrid(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_BlockSelection clip)
{
	assert(self);

	pianogriddraw_drawgridcells(self, g, clip);
	pianogriddraw_drawstepseparators(self, g, clip);
	pianogriddraw_drawkeyseparators(self, g, clip);
}

void pianogriddraw_drawgridcells(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	uint8_t key;

	assert(self);

	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {
		double cpy;
		uintptr_t steps;
		psy_dsp_big_beat_t c;

		cpy = keyboardstate_keytopx(self->keyboardstate, key);
		c = clip.topleft.absoffset;
		steps = pianogridstate_beattosteps(self->gridstate, c);
		for (; c <= clip.bottomright.absoffset;
			c += pianogridstate_step(self->gridstate), ++steps) {
			psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					pianogridstate_beattopx(self->gridstate,
						patternviewstate_draw_offset(self->gridstate->pv, c)),
					cpy),
				psy_ui_realsize_make(
					pianogridstate_steppx(self->gridstate) + 1,
					self->keyboardstate->keyheightpx + 1)),
				pianogriddraw_cellcolour(self, steps, key,
					pianogridstate_testselection(self->gridstate, key, c)));
		}
	}
}

psy_ui_Colour pianogriddraw_cellcolour(PianoGridDraw* self, uintptr_t step, uint8_t key, bool sel)
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


void pianogriddraw_drawstepseparators(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	psy_dsp_big_beat_t c;
	psy_ui_Style* style;

	assert(self);

	style = psy_ui_style(STYLE_PATTERNVIEW);
	psy_ui_setcolour(g, style->background.colour);
	for (c = clip.topleft.absoffset; c <= clip.bottomright.absoffset;
		c += pianogridstate_step(self->gridstate)) {
		double cpx;

		cpx = pianogridstate_beattopx(self->gridstate,
			patternviewstate_draw_offset(self->gridstate->pv, c));
		psy_ui_drawline(g,
			psy_ui_realpoint_make(cpx, 0.0),
			psy_ui_realpoint_make(cpx, self->keyboardstate->keyboardheightpx));
	}
}

/* draws key separators at C and E */
void pianogriddraw_drawkeyseparators(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{
	uint8_t key;

	assert(self);

	// psy_ui_setcolour(g, patternviewskin_separatorcolour(patternviewstate_skin(self->gridstate->pv),
	//	0, 0));	
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {
		if (psy_dsp_iskey_c(key + 1) || psy_dsp_iskey_e(key + 1)) {
			double cpy;

			cpy = keyboardstate_keytopx(self->keyboardstate, key);
			psy_ui_drawline(g,
				psy_ui_realpoint_make(
					pianogridstate_beattopx(self->gridstate,
						patternviewstate_draw_offset(self->gridstate->pv,
							clip.topleft.absoffset)),
					cpy),
				psy_ui_realpoint_make(
					pianogridstate_beattopx(self->gridstate,
						patternviewstate_draw_offset(self->gridstate->pv,
							clip.bottomright.absoffset)),
					cpy));
		}
	}
}

void pianogriddraw_drawcursor(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_BlockSelection clip)
{
	assert(self);

	if (self->gridstate->pv->sequence && !self->cursoronnoterelease &&
		!(psy_audio_player_playing(workspace_player(self->workspace)) &&
			keyboardmiscconfig_following_song(&self->workspace->config.misc))) {
		psy_ui_Style* style;
		psy_audio_SequenceCursor cursor;
		intptr_t key;

		cursor = self->gridstate->pv->cursor;
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = psy_audio_NOTECOMMANDS_MIDDLEC;
		}
		style = psy_ui_style(STYLE_PV_CURSOR);
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				pianogridstate_beattopx(self->gridstate,
					patternviewstate_draw_offset(self->gridstate->pv,
						cursor.absoffset)),
				keyboardstate_keytopx(self->keyboardstate, key)),
			psy_ui_realsize_make(
				pianogridstate_steppx(self->gridstate),
				self->keyboardstate->keyheightpx)),
			style->background.colour);
		// patternviewskin_cursorcolour(patternviewstate_skin(self->gridstate->pv),
		//	0, 0));
	}
}

void pianogriddraw_drawplaybar(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_BlockSelection clip)
{
	assert(self);

	if (self->workspace->host_sequencer_time.currplaying) {
		psy_dsp_big_beat_t offset;

		offset = self->workspace->host_sequencer_time.currplaycursor.absoffset;
		if (offset >= 0 && offset < patternviewstate_length(self->gridstate->pv)) {
			psy_ui_Style* style;

			style = psy_ui_style(STYLE_PV_PLAYBAR);
			psy_ui_drawsolidrectangle(g,
				psy_ui_realrectangle_make(
					psy_ui_realpoint_make(
						pianogridstate_beattopx(self->gridstate,
							pianogridstate_quantize(self->gridstate,
								patternviewstate_draw_offset(self->gridstate->pv,
									offset))),
						0.0),
					psy_ui_realsize_make(
						pianogridstate_steppx(self->gridstate),
						self->keyboardstate->keyboardheightpx)),
				style->background.colour);
			// patternviewskin_playbarcolour(patternviewstate_skin(self->gridstate->pv),
			//	0, psy_audio_song_numsongtracks(workspace_song(self->workspace))));
		}
	}
}

void pianogriddraw_drawentries(PianoGridDraw* self, psy_ui_Graphics* g,
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
				self->gridstate->pv->cursor.orderindex.track,
			(patternviewstate_single_mode(self->gridstate->pv)
				? self->gridstate->pv->cursor.seqoffset
				: 0.0), &ite);
		currnode = ite.patternnode;
		seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
		bpl = psy_audio_sequencecursor_bpl(&self->gridstate->pv->cursor);
		while (currnode) {
			psy_audio_PatternEntry* curr;
			PianogridTrackEvent* last;

			curr = (psy_audio_PatternEntry*)psy_list_entry(currnode);
			if (curr->offset + seqoffset > clip.bottomright.absoffset) {
				break;
			}
			if (pianogriddraw_hastrackdisplay(self, curr->track)) {
				psy_audio_PatternEvent* currevent;

				currevent = psy_audio_patternentry_front(curr);
				if (currevent->note <= psy_audio_NOTECOMMANDS_RELEASE) {
					last = pianogriddraw_lasttrackevent_at(self, curr->track,
						&lasttrackevent);
					/* active tells if a last event exists or not */
					if (last->active) {
						/* draw last to start of current */
						if (curr->offset + seqoffset >= clip.topleft.absoffset) {
							pianogriddraw_drawevent(self, g, last, curr->offset + seqoffset -
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
						pianogriddraw_drawevent(self, g, last,
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
			if (last->active && pianogriddraw_hastrackdisplay(self, last->track)) {
				pianogriddraw_drawevent(self, g, last,
					patternviewstate_length(self->gridstate->pv) +
					(patternviewstate_single_mode(self->gridstate->pv)
						? self->gridstate->pv->cursor.seqoffset
						: 0.0) - last->offset);
			}
			last->active = FALSE;
		}
		pianogriddraw_lasttrackevent_dispose(self, &lasttrackevent);
		psy_audio_sequencetrackiterator_dispose(&ite);
	}
}

bool pianogriddraw_hastrackdisplay(PianoGridDraw* self, uintptr_t track)
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

void pianogriddraw_drawevent(PianoGridDraw* self, psy_ui_Graphics* g,
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
		ev->offset)) * self->gridstate->pxperbeat;
	width = length * self->gridstate->pxperbeat;
	corner = psy_ui_realsize_make(2, 2);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			left,
			(self->keyboardstate->keymax - ev->note - 1) *
			self->keyboardstate->keyheightpx + 1),
		psy_ui_realsize_make(
			width,
			psy_max(1.0, self->keyboardstate->keyheightpx - 2)));
	/* if (ev->hover) {
		colour = patternviewskin_eventhovercolour(patternviewstate_skin(self->gridstate->pv), 0, 0);
	} else if (ev->track == cursor.track) {
		colour = patternviewskin_eventcurrchannelcolour(patternviewstate_skin(self->gridstate->pv),
			0, 0);
	} else {
		colour = patternviewskin_eventcolour(patternviewstate_skin(self->gridstate->pv), ev->track,
			psy_audio_patterns_num_tracks(&self->workspace->song->patterns));
	}*/
	colour = psy_ui_colour_white();
	if (!ev->noterelease) {
		psy_ui_drawsolidroundrectangle(g, r, corner, colour);
	} else {
		if (self->gridstate->pv->cursor.key == ev->note &&
			self->gridstate->pv->cursor.absoffset ==
			pianogridstate_quantize(self->gridstate, ev->offset)) {
			psy_ui_Style* style;

			style = psy_ui_style(STYLE_PV_CURSOR);
			self->cursoronnoterelease = TRUE;
			colour = style->background.colour;
			// patternviewskin_cursorcolour(patternviewstate_skin(self->gridstate->pv),
			//	0, 0);
		}
		psy_ui_setcolour(g, colour);
		psy_ui_drawroundrectangle(g, r, corner);
	}
}


