/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
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
static void pianogriddraw_updatekeystate(PianoGridDraw*);
static void pianogriddraw_drawgrid(PianoGridDraw*, psy_ui_Graphics*, psy_audio_BlockSelection);
static psy_audio_BlockSelection pianogriddraw_clipselection(PianoGridDraw*, psy_ui_RealRectangle
	clip);
static void pianogriddraw_drawgridcells(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_drawstepseparators(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_drawkeyseparators(PianoGridDraw* self, psy_ui_Graphics*,
	psy_audio_BlockSelection clip);
static void pianogriddraw_drawplaybar(PianoGridDraw*, psy_ui_Graphics*, psy_audio_BlockSelection);
static void pianogriddraw_drawentries(PianoGridDraw*, psy_ui_Graphics*, psy_audio_BlockSelection);
static bool pianogriddraw_hastrackdisplay(PianoGridDraw*, uintptr_t track);
static PianogridTrackEvent* pianogriddraw_lasttrackevent_at(PianoGridDraw* self, uintptr_t track,
	psy_Table* lasttrackevent);
static void pianogriddraw_lasttrackevent_dispose(PianoGridDraw*, psy_Table* lasttrackevent);
static void pianogriddraw_drawevent(PianoGridDraw*, psy_ui_Graphics*,
	PianogridTrackEvent*, psy_dsp_big_beat_t length);
static void pianogriddraw_drawcursor(PianoGridDraw*, psy_ui_Graphics*, psy_audio_BlockSelection);
static psy_ui_Colour pianogriddraw_cellcolour(PianoGridDraw* self, uintptr_t step,
	uint8_t key, bool sel);
static bool pianogriddraw_testselection(PianoGridDraw* self, uint8_t key, double offset);

void pianogriddraw_init(PianoGridDraw* self,
	KeyboardState* keyboardstate, PianoGridState* gridstate,	
	psy_dsp_big_beat_t sequenceentryoffset,
	psy_audio_PatternEntry* hoverpatternentry,	
	PianoTrackDisplay trackdisplay,
	bool cursorchanging, bool cursoronnoterelease,
	psy_audio_BlockSelection selection,
	psy_ui_RealSize size, const psy_ui_TextMetric* tm, Workspace* workspace)
{	
	self->keyboardstate = keyboardstate;
	self->gridstate = gridstate;
	self->size = size;
	self->tm = tm;
	self->sequenceentryoffset = sequenceentryoffset;
	self->hoverpatternentry =hoverpatternentry;	
	self->trackdisplay = trackdisplay;	
	self->cursorchanging = cursorchanging;
	self->cursoronnoterelease = cursoronnoterelease;	
	self->selection = selection;
	self->workspace = workspace;
	self->drawgrid = TRUE;
	self->drawentries = TRUE;
	self->drawcursor = TRUE;
	self->drawplaybar = TRUE;
	self->clip = TRUE;
}

void pianogriddraw_updatekeystate(PianoGridDraw* self)
{	
	assert(self);

	self->keyboardstate->keyheightpx = psy_ui_value_px(
		&self->keyboardstate->keyheight, self->tm, NULL);
	self->keyboardstate->keyboardheightpx = keyboardstate_height(
		self->keyboardstate, self->tm);
}

psy_audio_BlockSelection pianogriddraw_clipselection(PianoGridDraw* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_BlockSelection rv;

	assert(self);

	if (self->clip) {
		pianogridstate_clip(self->gridstate, clip.left, clip.right,
			&rv.topleft.offset, &rv.bottomright.offset);
		pianokeyboardstate_clip(self->keyboardstate,
			clip.top, clip.bottom,
			&rv.bottomright.key, &rv.topleft.key);
	} else {
		rv.topleft.offset = 0;
		rv.bottomright.offset = (patternviewstate_pattern(self->gridstate->pv))
			? psy_audio_pattern_length(patternviewstate_pattern(self->gridstate->pv))
			: 0.0;
		rv.bottomright.key = self->keyboardstate->keymin;
		rv.topleft.key = self->keyboardstate->keymax;
	}
	return rv;
}

void pianogriddraw_ondraw(PianoGridDraw* self, psy_ui_Graphics* g)
{
	psy_audio_BlockSelection clip;

	assert(self);

	if (!patternviewstate_pattern(self->gridstate->pv)) {
		// psy_ui_drawsolidrectangle(g, psy_ui_cliprect(g),
		//	patternviewstate_skin(self->gridstate->pv)->background);
		return;
	}
	pianogriddraw_updatekeystate(self);
	clip = pianogriddraw_clipselection(self, psy_ui_cliprect(g));
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

bool pianogriddraw_testselection(PianoGridDraw* self, uint8_t key, double offset)
{
	return self->selection.valid &&
		key >= self->selection.topleft.key &&
		key < self->selection.bottomright.key&&
		offset >= self->selection.topleft.offset &&
		offset < self->selection.bottomright.offset;
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
bool pianogrid_testselection(Pianogrid* self, uint8_t key, double offset);
static void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmouseup(Pianogrid*, psy_ui_MouseEvent*);
static psy_audio_SequenceCursor pianogrid_makecursor(Pianogrid* self, double x, double y);
/* vtable */
static psy_ui_ComponentVtable pianogrid_vtable;
static bool pianogrid_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianogrid_vtable_init(Pianogrid* self)
{
	assert(self);

	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw = (psy_ui_fp_component_ondraw)pianogrid_ondraw;
		pianogrid_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			pianogrid_onmousedown;
		pianogrid_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			pianogrid_onmouseup;
		pianogrid_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			pianogrid_onmousemove;
		pianogrid_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pianogrid_onpreferredsize;
		pianogrid_vtable_initialized = TRUE;
	}
	return &pianogrid_vtable;
}
/* implementation */
void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, PianoGridState* gridstate,
	Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(pianogrid_base(self), parent, NULL);
	psy_ui_component_setvtable(pianogrid_base(self),
		pianogrid_vtable_init(self));
	psy_ui_component_setwheelscroll(pianogrid_base(self), 4);
	self->workspace = workspace;
	self->gridstate = gridstate;
	self->keyboardstate = keyboardstate;
	self->trackdisplay = PIANOROLL_TRACK_DISPLAY_ALL;
	self->cursorchanging = FALSE;
	self->hoverpatternentry = NULL;
	self->lastplayposition = (psy_dsp_big_beat_t)0.0;
	self->cursoronnoterelease = FALSE;		
	psy_audio_sequencecursor_init(&self->oldcursor);
	psy_ui_component_setoverflow(pianogrid_base(self),
		psy_ui_OVERFLOW_SCROLL);	
}

void pianogrid_onpatternchange(Pianogrid* self, psy_audio_Pattern* pattern)
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

	if (!patternviewstate_pattern(self->gridstate->pv)) {
		// psy_ui_drawsolidrectangle(g, psy_ui_cliprect(g),
		//	patternviewstate_skin(self->gridstate->pv)->background);
		return;
	}
	pianogrid_updatekeystate(self);	
	pianogrid_drawbackground(self, g, 
		pianogrid_clipselection(self, psy_ui_cliprect(g)));
	pianogriddraw_init(&griddraw,
		self->keyboardstate, self->gridstate,
		self->gridstate->pv->cursor.seqoffset,
		self->hoverpatternentry,		
		self->trackdisplay,
		self->cursorchanging, self->cursoronnoterelease,		
		self->gridstate->pv->selection,
		psy_ui_component_scrollsize_px(pianogrid_base(self)),
		psy_ui_component_textmetric(&self->component),
		self->workspace);
	pianogriddraw_ondraw(&griddraw, g);
}

void pianogrid_updatekeystate(Pianogrid* self)
{
	const psy_ui_TextMetric* tm;

	assert(self);

	tm = psy_ui_component_textmetric(&self->component);
	self->keyboardstate->keyheightpx = psy_ui_value_px(
		&self->keyboardstate->keyheight, tm, NULL);
	self->keyboardstate->keyboardheightpx = keyboardstate_height(
		self->keyboardstate, tm);
}

psy_audio_BlockSelection pianogrid_clipselection(Pianogrid* self,
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

void pianogrid_drawbackground(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{	
	psy_ui_RealSize size;
		
	assert(self);
	assert(patternviewstate_pattern(self->gridstate->pv));

	size = psy_ui_component_scrollsize_px(pianogrid_base(self));
	pianogrid_drawuncoveredrightbackground(self, g, size);
	pianogrid_drawuncoveredbottombackground(self, g, size);
}

void pianogrid_drawuncoveredrightbackground(Pianogrid* self,
	psy_ui_Graphics* g, psy_ui_RealSize size)
{
	double blankstart;

	assert(self);
	assert(patternviewstate_pattern(self->gridstate->pv));

	blankstart = pianogridstate_beattopx(self->gridstate,
		psy_audio_pattern_length(patternviewstate_pattern(self->gridstate->pv)));
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
	assert(patternviewstate_pattern(self->gridstate->pv));

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
		c = clip.topleft.offset;
		steps = pianogridstate_beattosteps(self->gridstate, c);
		for (; c <= clip.bottomright.offset;
				c += pianogridstate_step(self->gridstate), ++steps) {			
			psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					pianogridstate_beattopx(self->gridstate, c), cpy),
				psy_ui_realsize_make(
					pianogridstate_steppx(self->gridstate) + 1,
					self->keyboardstate->keyheightpx + 1)),
				pianogriddraw_cellcolour(self, steps, key,
					pianogriddraw_testselection(self, key, c)));
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

bool pianogrid_testselection(Pianogrid* self, uint8_t key, double offset)
{
	return self->gridstate->pv->selection.valid &&
		key >= self->gridstate->pv->selection.topleft.key &&
		key < self->gridstate->pv->selection.bottomright.key&&
		offset >= self->gridstate->pv->selection.topleft.offset &&
		offset < self->gridstate->pv->selection.bottomright.offset;
}

void pianogriddraw_drawstepseparators(PianoGridDraw* self, psy_ui_Graphics* g,
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

		cpx = pianogridstate_beattopx(self->gridstate, c);
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
					pianogridstate_beattopx(self->gridstate, clip.topleft.offset),
					cpy),
				psy_ui_realpoint_make(
					pianogridstate_beattopx(self->gridstate, clip.bottomright.offset),
					cpy));
		}
	}
}

void pianogriddraw_drawcursor(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_BlockSelection clip)
{
	assert(self);

	if (self->workspace->song && !self->cursorchanging && !self->cursoronnoterelease &&
		    !self->cursorchanging &&
			!(psy_audio_player_playing(workspace_player(self->workspace)) &&
			workspace_followingsong(self->workspace))) {
		psy_ui_Style* style;
		psy_audio_SequenceCursor cursor;		
		intptr_t key;
						
		cursor = self->workspace->song->sequence.cursor;
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = psy_audio_NOTECOMMANDS_MIDDLEC;
		}					
		style = psy_ui_style(STYLE_PV_CURSOR);
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				pianogridstate_beattopx(self->gridstate, cursor.offset),
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

	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		psy_dsp_big_beat_t offset;		

		offset = psy_audio_player_position(workspace_player(self->workspace)) -
			self->sequenceentryoffset;
		if (offset >= 0 && offset < psy_audio_pattern_length(
				patternviewstate_pattern(self->gridstate->pv))) {
			psy_ui_Style* style;

			style = psy_ui_style(STYLE_PV_PLAYBAR);
			psy_ui_drawsolidrectangle(g,
				psy_ui_realrectangle_make(
					psy_ui_realpoint_make(
						pianogridstate_beattopx(self->gridstate,
							pianogridstate_quantize(self->gridstate, offset)),
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
	if (patternviewstate_pattern(self->gridstate->pv)) {
		psy_audio_PatternNode* currnode;
		psy_TableIterator it;
				
		/*
		** iterates over the events of the pattern and draws events with notes. To
		** determine the length, the current event is stored for each track. In
		** the next turn the stored last event is drawn to the start of the
		** current. Finally the last events with no ending notereleases are drawn
		** till the end of the pattern.
		*/
		currnode = psy_audio_pattern_begin(patternviewstate_pattern(self->gridstate->pv));
		while (currnode) {
			psy_audio_PatternEntry* curr;
			PianogridTrackEvent* last;

			curr = (psy_audio_PatternEntry*)psy_list_entry(currnode);
			if (pianogriddraw_hastrackdisplay(self, curr->track)) {
				psy_audio_PatternEvent* currevent;

				currevent = psy_audio_patternentry_front(curr);
				if (currevent->note <= psy_audio_NOTECOMMANDS_RELEASE) {					
					last = pianogriddraw_lasttrackevent_at(self, curr->track,
						&lasttrackevent);
					/* active tells if a last event exists or not */
					if (last->active) {
						/* draw last to start of current */
						if (curr->offset >= clip.topleft.offset) {
							pianogriddraw_drawevent(self, g, last, curr->offset -
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
						last->offset = curr->offset;
						last->hover = (self->hoverpatternentry == curr);
						last->noterelease = TRUE;
						pianogriddraw_drawevent(self, g, last, pianogridstate_step(self->gridstate));
						/* and skip draw last event */
						last->noterelease = FALSE;
						last->active = FALSE;
					} else {					
						last->active = TRUE;
						last->note = currevent->note;
						last->offset = curr->offset;
						last->hover = (self->hoverpatternentry == curr);
					}
				}
			}
			psy_audio_patternnode_next(&currnode);
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
				pianogriddraw_drawevent(self, g, last, psy_audio_pattern_length(
					patternviewstate_pattern(self->gridstate->pv)) - last->offset);
			}
			last->active = FALSE;
		}
		pianogriddraw_lasttrackevent_dispose(self, &lasttrackevent);
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
				&workspace_song(self->workspace)->patterns.trackstate, track)));
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

	cursor = self->workspace->song->sequence.cursor;
	left = ev->offset * self->gridstate->pxperbeat;
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
				self->gridstate->pv->cursor.offset ==
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

void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	assert(self);

	pianogrid_updatekeystate(self);
	rv->height = psy_ui_value_make_px((self->keyboardstate->keymax -
		self->keyboardstate->keymin) * self->keyboardstate->keyheightpx);	
	rv->width = (patternviewstate_pattern(self->gridstate->pv))
		? psy_ui_value_make_px(pianogridstate_beattopx(self->gridstate,
			patternviewstate_length(self->gridstate->pv)))
		: psy_ui_value_make_px(0.0);
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	self->dragcursor = pianogrid_makecursor(self, psy_ui_mouseevent_pt(ev).x,
		psy_ui_mouseevent_pt(ev).y);
	self->gridstate->pv->selection.topleft = self->dragcursor;
	self->gridstate->pv->dragselectionbase = self->dragcursor;
	self->lastdragcursor = self->dragcursor;
	self->gridstate->pv->selection.valid = FALSE;
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_SequenceCursor cursor;

	assert(self);

	if (patternviewstate_pattern(self->gridstate->pv)) {
		psy_audio_PatternEntry* oldhover;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;

		oldhover = self->hoverpatternentry;
		node = psy_audio_pattern_findnode(patternviewstate_pattern(self->gridstate->pv),
			self->gridstate->pv->cursor.track,
				pianogridstate_quantize(self->gridstate,
					pianogridstate_pxtobeat(self->gridstate,
						psy_ui_mouseevent_pt(ev).x -
						psy_ui_component_scrollleft_px(&self->component))),
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
		if (psy_ui_mouseevent_button(ev) == 1) {
			cursor = pianogrid_makecursor(self, psy_ui_mouseevent_pt(ev).x,
				psy_ui_mouseevent_pt(ev).y);
			if (cursor.key != self->lastdragcursor.key ||
				cursor.offset != self->lastdragcursor.offset) {
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
	if (cursor.offset >= self->gridstate->pv->dragselectionbase.offset) {
		self->gridstate->pv->selection.topleft.offset = self->gridstate->pv->dragselectionbase.offset;
		self->gridstate->pv->selection.bottomright.offset = cursor.offset + bpl;
	} else {
		self->gridstate->pv->selection.topleft.offset = cursor.offset;
		self->gridstate->pv->selection.bottomright.offset = self->gridstate->pv->dragselectionbase.offset + bpl;
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
	if (cursor.offset >= self->gridstate->pv->dragselectionbase.offset) {
		self->gridstate->pv->selection.topleft.offset = self->gridstate->pv->dragselectionbase.offset;
		self->gridstate->pv->selection.bottomright.offset = cursor.offset + bpl;
	} else {
		self->gridstate->pv->selection.topleft.offset = cursor.offset;
		self->gridstate->pv->selection.bottomright.offset = self->gridstate->pv->dragselectionbase.offset + bpl;
	}
	/* self->midline = 0; */
	if (cursor.offset < self->lastdragcursor.offset) {
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


void pianogrid_onmouseup(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (!self->gridstate->pv->selection.valid && patternviewstate_pattern(self->gridstate->pv)) {
		psy_audio_PatternEvent patternevent;
		psy_audio_SequenceCursor cursor;
		
		psy_audio_patternevent_clear(&patternevent);
		pianogrid_updatekeystate(self);		
		patternevent.note = self->dragcursor.key;
		cursor = self->dragcursor;
		if (psy_ui_mouseevent_button(ev) == 1) {
			/* left button */
			psy_undoredo_execute(&self->workspace->undoredo,
				&insertcommand_allocinit(
					patternviewstate_pattern(self->gridstate->pv),					
					cursor, patternevent, &self->workspace->song->sequence)->command);
			if (self->workspace && workspace_song(self->workspace)) {
				psy_audio_sequence_set_cursor(
					psy_audio_song_sequence(workspace_song(self->workspace)),
					cursor);
			}			
		} else if (psy_ui_mouseevent_button(ev) == 2) {
			/* right button */
			psy_audio_PatternNode* node;
			psy_audio_PatternNode* prev;

			node = psy_audio_pattern_findnode(
				patternviewstate_pattern(self->gridstate->pv),
				self->gridstate->pv->cursor.track, cursor.offset,
					pianogridstate_step(self->gridstate), &prev);
			if (!node) {				
				if (prev) {
					psy_audio_PatternEntry* preventry;

					preventry = psy_audio_patternnode_entry(prev);
					if (preventry->track != cursor.track) {
						prev = psy_audio_patternnode_prev_track(prev, cursor.track);
					}
				}
				node = prev;
			}
			if (node) {
				psy_audio_PatternNode* next;				
				psy_audio_PatternEntry* patternentry;

				patternentry = psy_audio_patternnode_entry(node);				
				if (!pianogrid_keyhittest(self, node, cursor.track, cursor.key)) {					
					pianogrid_invalidatecursor(self);
					return;
				}
				if (patternentry->offset == cursor.offset) {
					next = psy_audio_patternnode_next_track(node, cursor.track);
					if (self->hoverpatternentry == psy_audio_patternnode_entry(node)) {
						self->hoverpatternentry = NULL;
					}
					psy_undoredo_execute(&self->workspace->undoredo,
						&removecommand_allocinit(self->gridstate->pv->pattern,							
							cursor, &self->workspace->song->sequence)->command);
					if (next) {
						psy_audio_PatternEntry* nextentry;

						nextentry = psy_audio_patternnode_entry(next);
						if (psy_audio_patternentry_front(nextentry)->note
								== psy_audio_NOTECOMMANDS_RELEASE) {
							if (self->hoverpatternentry == psy_audio_patternnode_entry(next)) {
								self->hoverpatternentry = NULL;
							}
							cursor.offset = nextentry->offset;
							psy_undoredo_execute(&self->workspace->undoredo,
								&removecommand_allocinit(self->gridstate->pv->pattern,									
									cursor, &self->workspace->song->sequence)->command);
						}
					}
					if (self->workspace && workspace_song(self->workspace)) {
						psy_audio_sequence_set_cursor(
							psy_audio_song_sequence(workspace_song(self->workspace)),
							cursor);
					}					
				} else {
					if (psy_audio_patternentry_front(patternentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
						if (self->workspace && workspace_song(self->workspace)) {
							psy_audio_sequence_set_cursor(
								psy_audio_song_sequence(workspace_song(self->workspace)),
								cursor);
						}
						return;
					} else {
						psy_audio_PatternEvent release;

						next = psy_audio_patternnode_next_track(node, cursor.track);
						if (next) {
							psy_audio_PatternEntry* nextentry;							

							nextentry = psy_audio_patternnode_entry(next);
							if (psy_audio_patternentry_front(nextentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
								psy_audio_SequenceCursor release;

								release = cursor;
								release.offset = nextentry->offset;
								psy_undoredo_execute(&self->workspace->undoredo,
									&removecommand_allocinit(self->gridstate->pv->pattern,										
										release, &self->workspace->song->sequence)->command);
							}
						}						
						psy_audio_patternevent_clear(&release);
						release.note = psy_audio_NOTECOMMANDS_RELEASE;
						psy_undoredo_execute(&self->workspace->undoredo,
							&insertcommand_allocinit(patternviewstate_pattern(self->gridstate->pv),								
								cursor, release, &self->workspace->song->sequence)->command);
						cursor.key = psy_audio_patternentry_front(patternentry)->note;
						if (self->workspace && workspace_song(self->workspace)) {
							psy_audio_sequence_set_cursor(psy_audio_song_sequence(
								workspace_song(self->workspace)), cursor);
						}
					}
				}
			}
		}
	}
	psy_ui_component_invalidate(&self->component);
}

psy_audio_SequenceCursor pianogrid_makecursor(Pianogrid* self, double x, double y)
{
	psy_audio_SequenceCursor rv;

	rv = self->gridstate->pv->cursor;
	psy_audio_sequencecursor_updateseqoffset(&rv,
		&self->workspace->song->sequence);
	rv.absolute = FALSE;
	rv.offset = pianogridstate_quantize(self->gridstate,
		pianogridstate_pxtobeat(self->gridstate, x));
	rv.key = keyboardstate_pxtokey(self->keyboardstate, y);
	rv.track = self->gridstate->pv->cursor.track;
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
		self->oldcursor = self->gridstate->pv->cursor;
		return;
	}
	if (self->gridstate->pv->cursor.track != self->oldcursor.track) {
		psy_ui_component_invalidate(&self->component);
	} else if (self->oldcursor.offset != self->gridstate->pv->cursor.offset ||
			self->oldcursor.key != self->gridstate->pv->cursor.key) {
		pianogrid_invalidateline(self, self->oldcursor.offset);
		pianogrid_invalidateline(self, self->gridstate->pv->cursor.offset);
	}
	self->cursorchanging = FALSE;
	if (psy_audio_player_playing(workspace_player(self->workspace)) && workspace_followingsong(self->workspace)
			&& patternviewstate_pattern(self->gridstate->pv)) {
		bool scrolldown;

		scrolldown = self->lastplayposition <
			psy_audio_player_position(workspace_player(self->workspace));
		pianogrid_invalidateline(self, self->lastplayposition - self->gridstate->pv->cursor.seqoffset);
		self->lastplayposition = psy_audio_player_position(workspace_player(self->workspace));
		pianogrid_invalidateline(self, self->lastplayposition - self->gridstate->pv->cursor.seqoffset);
		if (self->lastplayposition >= self->gridstate->pv->cursor.seqoffset &&
			self->lastplayposition < self->gridstate->pv->cursor.seqoffset +
			patternviewstate_pattern(self->gridstate->pv)->length) {
			if (scrolldown != FALSE) {
				pianogrid_scrollright(self, self->gridstate->pv->cursor);
			} else {
				pianogrid_scrollleft(self, self->gridstate->pv->cursor);
			}
		}		
	}
}

void pianogrid_prevline(Pianogrid* self)
{
	assert(self);

	pianogrid_prevlines(self, workspace_cursorstep(self->workspace),
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
			pianogrid_scrollup(self, cursor);
		} else {
			pianogrid_scrolldown(self, cursor);
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
			pianogrid_scrollup(self, cursor);
		} else {
			pianogrid_scrolldown(self, cursor);
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
		psy_ui_component_scrollleft_px(&self->component)) > cursor.offset) {
		psy_ui_component_setscrollleft(&self->component,
			psy_ui_value_make_px(pianogridstate_quantizebeattopx(self->gridstate,
				cursor.offset)));
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
	line = pianogridstate_beattosteps(self->gridstate, cursor.offset);
	if (visilines < line - psy_ui_component_scrollleft_px(&self->component) /
		pianogridstate_steppx(self->gridstate) + 2) {
		intptr_t dlines;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		dlines = (intptr_t)((line - psy_ui_component_scrollleft_px(&self->component) /
			pianogridstate_steppx(self->gridstate) - visilines + 16));
		self->cursorchanging = TRUE;
		psy_ui_component_setscrollleft(&self->component,
			psy_ui_value_make_px(
				psy_max(0.0, psy_ui_component_scrollleft_px(&self->component) +
					psy_ui_component_scrollstep_width_px(&self->component) * dlines)));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scrollup(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t topline;
	psy_ui_RealRectangle r;
	const psy_ui_TextMetric* tm;

	assert(self);

	tm = psy_ui_component_textmetric(&self->component);
	pianogrid_updatekeystate(self);
	line = self->keyboardstate->keymax - self->gridstate->pv->cursor.key - 1;
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, self->keyboardstate->keyheightpx * line),
		psy_ui_realsize_make(0, self->keyboardstate->keyheightpx));	
	topline = 0;	
	if (psy_ui_component_scrolltop_px(&self->component) > r.top) {
		intptr_t dlines;
		
		dlines = (intptr_t)((psy_ui_component_scrolltop_px(&self->component) - r.top) /
			(self->keyboardstate->keyheightpx));
		self->cursorchanging = TRUE;
		psy_ui_component_setscrolltop(&self->component,
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
	const psy_ui_TextMetric* tm;

	assert(self);

	clientsize = psy_ui_component_clientsize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	visilines = (intptr_t)floor(clientsize.height / (psy_dsp_big_beat_t)self->keyboardstate->keyheightpx);
	topline = (intptr_t)ceil(psy_ui_component_scrolltop_px(&self->component) /
		self->keyboardstate->keyheightpx);
	line = self->keyboardstate->keymax - self->gridstate->pv->cursor.key;
	if (visilines < line - topline) {
		intptr_t dlines;

		dlines = (intptr_t)(line - topline - visilines);
		self->cursorchanging = TRUE;
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_value_make_px(
				psy_ui_component_scrolltop_px(&self->component) +
				psy_ui_component_scrollstep_height_px(&self->component) *
				dlines));
		return FALSE;
	}
	return TRUE;

}

void pianogrid_invalidateline(Pianogrid* self, psy_dsp_big_beat_t position)
{	
	assert(self);
	
	if (patternviewstate_pattern(self->gridstate->pv) && position >= 0.0 &&
			position < psy_audio_pattern_length(patternviewstate_pattern(
				self->gridstate->pv))) {
		psy_ui_RealSize size;		
		double scrolltoppx;
				
		scrolltoppx = psy_ui_component_scrolltop_px(pianogrid_base(self));
		size =  psy_ui_component_scrollsize_px(&self->component);
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					pianogridstate_quantizebeattopx(self->gridstate, position),
					scrolltoppx),
				psy_ui_realsize_make(
					pianogridstate_steppx(self->gridstate), size.height)));
	}
	self->oldcursor = self->gridstate->pv->cursor;
}

void pianogrid_invalidatecursor(Pianogrid* self)
{
	assert(self);

	pianogrid_invalidateline(self, psy_audio_sequencecursor_offset(
		&self->oldcursor));
	pianogrid_invalidateline(self, psy_audio_sequencecursor_offset(
		&self->gridstate->pv->cursor));
	self->oldcursor = self->gridstate->pv->cursor;
}

void pianogrid_set_cursor(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	if (patternviewstate_sequence(self->gridstate->pv)) {
		psy_audio_sequence_set_cursor(patternviewstate_sequence(
			self->gridstate->pv), cursor);
	}
}
