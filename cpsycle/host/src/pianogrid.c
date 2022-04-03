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
			workspace_followingsong(self->workspace))) {
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

		offset = self->workspace->host_sequencer_time.currplayposition;
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
			psy_audio_sequence_track_at(self->gridstate->pv->sequence,
				self->gridstate->pv->cursor.orderindex.track),
			((self->gridstate->pv->singlemode)
				? self->gridstate->pv->cursor.seqoffset
				: 0.0), &ite);
		currnode = ite.patternnode;
		seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
		bpl = (1.0 / self->gridstate->pv->cursor.lpb);
		while (currnode) {
			psy_audio_PatternEntry* curr;
			PianogridTrackEvent* last;

			curr = (psy_audio_PatternEntry*)psy_list_entry(currnode);
			if (curr->offset + seqoffset + bpl > clip.bottomright.absoffset) {
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
			if (self->gridstate->pv->singlemode) {
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
					((self->gridstate->pv->singlemode)
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

	psy_table_disposeall(lasttrackevent, (psy_fp_disposefunc)NULL);
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
			psy_audio_patterns_numtracks(&self->workspace->song->patterns));
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


/* Pianogrid */

/* prototypes */
static void pianogrid_ondraw(Pianogrid*, psy_ui_Graphics*);
static psy_audio_BlockSelection pianogrid_clipselection(Pianogrid*,
	psy_ui_RealRectangle clip);
static void pianogrid_drawbackground(Pianogrid*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogrid_drawuncoveredrightbackground(Pianogrid*,
	psy_ui_Graphics* g, psy_ui_RealSize);
static void pianogrid_drawuncoveredbottombackground(Pianogrid*,
	psy_ui_Graphics*, psy_ui_RealSize);
static void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_on_mouse_down(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_on_mouse_up(Pianogrid*, psy_ui_MouseEvent*);
static psy_audio_SequenceCursor pianogrid_makecursor(Pianogrid* self, double x, double y);

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
			pianogrid_ondraw;
		pianogrid_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_down;
		pianogrid_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_up;
		pianogrid_vtable.onmousemove =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_onmousemove;
		pianogrid_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			pianogrid_onpreferredsize;
		pianogrid_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pianogrid_base(self), &pianogrid_vtable);
}

/* implementation */
void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, PianoGridState* gridstate,
	Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(pianogrid_base(self), parent, NULL);	
	pianogrid_vtable_init(self);
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
	psy_audio_sequencecursor_init(&self->old_cursor);
	psy_ui_component_setoverflow(pianogrid_base(self),
		psy_ui_OVERFLOW_SCROLL);	
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

void pianogrid_ondraw(Pianogrid* self, psy_ui_Graphics* g)
{
	PianoGridDraw griddraw;	

	assert(self);

	if (!patternviewstate_sequence(self->gridstate->pv)) {		
		return;
	}	
	pianogrid_drawbackground(self, g, 
		pianogrid_clipselection(self, psy_ui_graphics_cliprect(g)));
	pianogriddraw_init(&griddraw,
		self->keyboardstate, self->gridstate,
		self->gridstate->pv->cursor.seqoffset,
		self->hoverpatternentry,		
		self->trackdisplay,
		self->cursoronnoterelease,
		psy_ui_component_scrollsize_px(pianogrid_base(self)),		
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

void pianogrid_drawbackground(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{	
	psy_ui_RealSize size;
		
	assert(self);	

	size = psy_ui_component_scrollsize_px(pianogrid_base(self));
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
	if (blankstart - psy_ui_component_scrollleft_px(&self->component) <
			size.width) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				blankstart,
				psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width -
				(blankstart - psy_ui_component_scrollleft_px(
					pianogrid_base(self))),
				size.height)),
			psy_ui_component_backgroundcolour(&self->component));
			// patternviewskin_separatorcolour(patternviewstate_skin(
			//	self->gridstate->pv), 1, 2));
	}
}

void pianogrid_drawuncoveredbottombackground(Pianogrid* self,
	psy_ui_Graphics* g, psy_ui_RealSize size)
{
	double blankstart;

	assert(self);	

	blankstart = self->keyboardstate->keyboardheightpx;
	if (blankstart - psy_ui_component_scrolltop_px(&self->component) <
			size.height) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleft_px(pianogrid_base(self)),
				blankstart),
			psy_ui_realsize_make(size.width,
				size.height - (blankstart - psy_ui_component_scrolltop_px(
					pianogrid_base(self))))),
			psy_ui_component_backgroundcolour(&self->component));
			// patternviewskin_separatorcolour(patternviewstate_skin(
			//     self->gridstate->pv), 1, 2));
	}
}

void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	assert(self);
	
	rv->height = psy_ui_value_make_px((self->keyboardstate->keymax -
		self->keyboardstate->keymin) * self->keyboardstate->keyheightpx);	
	rv->width = psy_ui_value_make_px(pianogridstate_beattopx(self->gridstate,
		patternviewstate_length(self->gridstate->pv)));		
}

void pianogrid_on_mouse_down(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	self->dragcursor = pianogrid_makecursor(self, psy_ui_mouseevent_pt(ev).x,
		psy_ui_mouseevent_pt(ev).y);	
	self->gridstate->pv->dragselectionbase = self->dragcursor;
	self->lastdragcursor = self->dragcursor;
	if (psy_ui_mouseevent_button(ev) != 2) {
		self->gridstate->pv->selection.topleft = self->dragcursor;	
		self->gridstate->pv->selection.valid = FALSE;
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
		node = psy_audio_pattern_findnode(
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
	} else if (!psy_ui_mouseevent_ctrlkey(ev) && self->edit_mode &&	
			(psy_ui_mouseevent_button(ev) == 1)) {
		psy_audio_PatternEvent patternevent;
		psy_audio_SequenceCursor cursor;

		self->gridstate->pv->selection.valid = FALSE;
		psy_audio_patternevent_clear(&patternevent);
		patternevent.note = self->dragcursor.key;
		cursor = self->dragcursor;
		self->preventscrollleft = TRUE;
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(
				patternviewstate_pattern(self->gridstate->pv),
				cursor, patternevent, &self->workspace->song->sequence)->command);
		self->preventscrollleft = FALSE;
		pianogrid_set_cursor(self, cursor);
	}	
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
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
		node = psy_audio_pattern_findnode(pattern,
			self->gridstate->pv->cursor.track,
				pianogridstate_quantize(self->gridstate,
					pianogridstate_pxtobeat(self->gridstate,
						psy_ui_mouseevent_pt(ev).x -
						psy_ui_component_scrollleft_px(&self->component)) +
					(self->gridstate->pv->singlemode)
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
			keyboardstate_pxtokey(self->keyboardstate, psy_ui_mouseevent_pt(ev).y))) {
			self->hoverpatternentry = psy_audio_patternnode_entry(node);
		} else {
			self->hoverpatternentry = NULL;
		}
		if (self->hoverpatternentry != oldhover) {
			psy_ui_component_invalidate(&self->component);
		}
		if (((psy_ui_mouseevent_button(ev) == 1) && (!self->edit_mode || psy_ui_mouseevent_ctrlkey(ev)))) {
			cursor = pianogrid_makecursor(self, psy_ui_mouseevent_pt(ev).x,
				psy_ui_mouseevent_pt(ev).y);
			if (cursor.key != self->lastdragcursor.key ||
				cursor.absoffset != self->lastdragcursor.absoffset) {
				if (!self->gridstate->pv->selection.valid) {
					pianogrid_startdragselection(self, cursor);
				} else {
					pianogrid_dragselection(self, cursor);
				}
				psy_ui_component_invalidate(&self->component);
				self->lastdragcursor = cursor;
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
	if (cursor.absoffset < self->lastdragcursor.absoffset) {
		/* pianogrid_scrollleft(self, cursor); */
	} else {	
		/* pianogrid_scrollright(self, cursor); */
	}
	if (cursor.key < self->lastdragcursor.key) {
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
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (!self->gridstate->pv->selection.valid) {		
		if (!self->edit_mode || ((psy_ui_mouseevent_button(ev) == 1 && psy_ui_mouseevent_ctrlkey(ev)))) {
			pianogrid_set_cursor(self, self->dragcursor);			
		}		
	}	
}

psy_audio_SequenceCursor pianogrid_makecursor(Pianogrid* self, double x, double y)
{
	psy_audio_SequenceCursor rv;

	rv = self->gridstate->pv->cursor;
	psy_audio_sequencecursor_updateseqoffset(&rv,
		&self->workspace->song->sequence);	
	rv.absoffset = pianogridstate_quantize(self->gridstate,
		pianogridstate_pxtobeat(self->gridstate, x)) +
		((self->gridstate->pv->singlemode)
		? rv.seqoffset
		: 0);
	rv.key = keyboardstate_pxtokey(self->keyboardstate, y);	
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
	if (!psy_ui_component_drawvisible(&self->component)) {
		self->old_cursor = self->gridstate->pv->cursor;
		return;
	}
	pianogrid_invalidate_cursor(self);	
}

void pianogrid_prevline(Pianogrid* self)
{
	assert(self);

	pianogrid_prevlines(self, workspace_cursorstep(self->workspace), TRUE);
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

	pianogrid_advancelines(self, workspace_cursorstep(
		self->workspace), TRUE);
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
		psy_ui_component_scrollleft_px(&self->component)) > patternviewstate_draw_offset(
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
	if (visilines < line - psy_ui_component_scrollleft_px(&self->component) /
		pianogridstate_steppx(self->gridstate) + 2) {
		intptr_t dlines;
		
		dlines = (intptr_t)((line - psy_ui_component_scrollleft_px(&self->component) /
			pianogridstate_steppx(self->gridstate) - visilines + 16));
		self->component.blitscroll = TRUE;
		psy_ui_component_set_scroll_left(&self->component,		
			psy_ui_value_make_px(
				psy_max(0.0, psy_ui_component_scrollleft_px(&self->component) +
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
	linepx = self->keyboardstate->keyheightpx * line;
	topline = 0;
	dlines = (intptr_t)((psy_ui_component_scrolltop_px(&self->component) - linepx) /
		(self->keyboardstate->keyheightpx));
	if (dlines > 0) {		
		psy_ui_component_set_scroll_top(&self->component,
			psy_ui_value_make_px(
			psy_max(0, psy_ui_component_scrolltop_px(&self->component) -
				psy_ui_component_scrollstep_height_px(&self->component) * dlines)));
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
	visilines = (intptr_t)floor(clientsize.height / self->keyboardstate->keyheightpx);
	topline = (intptr_t)ceil(psy_ui_component_scrolltop_px(&self->component) /
		self->keyboardstate->keyheightpx);
	line = self->keyboardstate->keymax - cursor.key + 1;
	dlines = (intptr_t)(line - topline - visilines);
	if (dlines > 0) {		
		psy_ui_component_set_scroll_top(&self->component,
			psy_ui_value_make_px(
				psy_ui_component_scrolltop_px(&self->component) +
				psy_ui_component_scrollstep_height_px(&self->component) *
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

	scrolltoppx = psy_ui_component_scrolltop_px(pianogrid_base(self));
	size = psy_ui_component_clientsize_px(&self->component);
	stepsize = psy_ui_realsize_make(pianogridstate_steppx(self->gridstate),
		size.height);
	seqline = 0;
	if (self->gridstate->pv->singlemode) {
		seqline = (intptr_t)(self->gridstate->pv->cursor.seqoffset *
			self->gridstate->pv->cursor.lpb);
	}
	r1 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line - seqline) * stepsize.width, scrolltoppx), stepsize);
	psy_ui_component_invalidaterect(pianogrid_base(self), r1);	
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

	scrolltoppx = psy_ui_component_scrolltop_px(pianogrid_base(self));	
	size = psy_ui_component_clientsize_px(&self->component);
	stepsize = psy_ui_realsize_make(pianogridstate_steppx(self->gridstate),
		size.height);
	seqline = 0;
	if (self->gridstate->pv->singlemode) {
		seqline = (intptr_t)(self->gridstate->pv->cursor.seqoffset *
			self->gridstate->pv->cursor.lpb);
	}
	r1 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line1 - seqline) * stepsize.width, scrolltoppx), stepsize);
	r2 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line2 - seqline) * stepsize.width, scrolltoppx), stepsize);
	psy_ui_realrectangle_union(&r1, &r2);
	psy_ui_component_invalidaterect(pianogrid_base(self), r1);
}

void pianogrid_invalidate_cursor(Pianogrid* self)
{
	assert(self);

	pianogrid_invalidate_line(self, self->old_cursor.linecache);
	pianogrid_invalidate_line(self, self->gridstate->pv->cursor.linecache);
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
