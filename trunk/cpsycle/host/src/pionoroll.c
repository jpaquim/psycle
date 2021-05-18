/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianoroll.h"
/* local */
#include "trackergridstate.h"
#include "patterncmds.h"
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
static void pianogriddraw_drawgrid(PianoGridDraw*, psy_ui_Graphics*, psy_audio_PatternSelection);
static psy_audio_PatternSelection pianogriddraw_clipselection(PianoGridDraw*, psy_ui_RealRectangle
	clip);
static void pianogriddraw_drawgridcells(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogriddraw_drawstepseparators(PianoGridDraw*, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogriddraw_drawkeyseparators(PianoGridDraw* self, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogriddraw_drawplaybar(PianoGridDraw*, psy_ui_Graphics*, psy_audio_PatternSelection);
static void pianogriddraw_drawentries(PianoGridDraw*, psy_ui_Graphics*, psy_audio_PatternSelection);
static bool pianogriddraw_hastrackdisplay(PianoGridDraw*, uintptr_t track);
static PianogridTrackEvent* pianogriddraw_lasttrackevent_at(PianoGridDraw* self, uintptr_t track,
	psy_Table* lasttrackevent);
static void pianogriddraw_lasttrackevent_dispose(PianoGridDraw*, psy_Table* lasttrackevent);
static void pianogriddraw_drawevent(PianoGridDraw*, psy_ui_Graphics*,
	PianogridTrackEvent*, psy_dsp_big_beat_t length);
static void pianogriddraw_drawcursor(PianoGridDraw*, psy_ui_Graphics*, psy_audio_PatternSelection);
static psy_ui_Colour pianogriddraw_cellcolour(PianoGridDraw* self, uintptr_t step,
	uint8_t key, bool sel);
bool pianogriddraw_testselection(PianoGridDraw* self, uint8_t key, double offset);

void pianogriddraw_init(PianoGridDraw* self,
	KeyboardState* keyboardstate, PianoGridState* gridstate,	
	psy_dsp_big_beat_t sequenceentryoffset,
	psy_audio_PatternEntry* hoverpatternentry,	
	PianoTrackDisplay trackdisplay,
	bool cursorchanging, bool cursoronnoterelease,
	psy_audio_PatternSelection selection,
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

psy_audio_PatternSelection pianogriddraw_clipselection(PianoGridDraw* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_PatternSelection rv;

	assert(self);

	if (self->clip) {
		pianogridstate_clip(self->gridstate, clip.left, clip.right,
			&rv.topleft.offset, &rv.bottomright.offset);
		pianokeyboardstate_clip(self->keyboardstate,
			clip.top, clip.bottom,
			&rv.bottomright.key, &rv.topleft.key);
	} else {
		rv.topleft.offset = 0;
		rv.bottomright.offset = (pianogridstate_pattern(self->gridstate))
			? psy_audio_pattern_length(pianogridstate_pattern(self->gridstate))
			: 0.0;
		rv.bottomright.key = self->keyboardstate->keymin;
		rv.topleft.key = self->keyboardstate->keymax;
	}
	return rv;
}

void pianogriddraw_ondraw(PianoGridDraw* self, psy_ui_Graphics* g)
{
	psy_audio_PatternSelection clip;

	assert(self);

	if (!pianogridstate_pattern(self->gridstate)) {
		psy_ui_drawsolidrectangle(g, g->clip,
			self->gridstate->skin->background);
		return;
	}
	pianogriddraw_updatekeystate(self);
	clip = pianogriddraw_clipselection(self, g->clip);
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
static void pianogrid_updatekeystate(Pianogrid*);
static psy_audio_PatternSelection pianogrid_clipselection(Pianogrid*,
	psy_ui_RealRectangle clip);
static void pianogrid_drawbackground(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
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
static psy_audio_PatternCursor pianogrid_makecursor(Pianogrid* self, double x, double y);
static void pianogrid_startdragselection(Pianogrid*, psy_audio_PatternCursor);
static void pianogrid_dragselection(Pianogrid*, psy_audio_PatternCursor);
static bool pianogrid_keyhittest(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track, uint8_t cursorkey);
static bool pianogrid_scrollright(Pianogrid*, psy_audio_PatternCursor);
static bool pianogrid_scrollleft(Pianogrid*, psy_audio_PatternCursor);
static bool pianogrid_scrollup(Pianogrid*, psy_audio_PatternCursor);
static bool pianogrid_scrolldown(Pianogrid*, psy_audio_PatternCursor);
static void pianogrid_prevline(Pianogrid*);
static void pianogrid_prevlines(Pianogrid*, uintptr_t lines, bool wrap);
static void pianogrid_prevkeys(Pianogrid*, uint8_t lines, bool wrap);
static void pianogrid_advanceline(Pianogrid*);
static void pianogrid_advancelines(Pianogrid*, uintptr_t lines, bool wrap);
static void pianogrid_advancekeys(Pianogrid*, uint8_t lines, bool wrap);
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
	psy_ui_component_setbackgroundmode(pianogrid_base(self),
		psy_ui_NOBACKGROUND);
	psy_ui_component_doublebuffer(pianogrid_base(self));
	psy_ui_component_setwheelscroll(pianogrid_base(self), 4);
	pianogrid_setsharedgridstate(self, gridstate);
	pianogrid_setsharedkeyboardstate(self, keyboardstate);
	self->trackdisplay = PIANOROLL_TRACK_DISPLAY_ALL;
	self->cursorchanging = FALSE;
	self->workspace = workspace;
	self->hoverpatternentry = NULL;
	self->lastplayposition = (psy_dsp_big_beat_t)0.0;
	self->sequenceentryoffset = (psy_dsp_big_beat_t)0.0;
	self->cursoronnoterelease = FALSE;
	self->pgupdownstep = 4;
	psy_audio_patternselection_init(&self->selection);
	psy_audio_patterncursor_init(&self->oldcursor);	
	psy_ui_component_setoverflow(pianogrid_base(self), psy_ui_OVERFLOW_SCROLL);	
}

void pianogrid_setsharedgridstate(Pianogrid* self, PianoGridState* gridstate)
{
	assert(self);

	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		pianogridstate_init(&self->defaultgridstate, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianogrid_setsharedkeyboardstate(Pianogrid* self, KeyboardState*
	keyboardstate)
{
	assert(self);

	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate, NULL);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
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

	if (!pianogridstate_pattern(self->gridstate)) {
		psy_ui_drawsolidrectangle(g, g->clip,
			self->gridstate->skin->background);
		return;
	}
	pianogrid_updatekeystate(self);	
	pianogrid_drawbackground(self, g, 
		pianogrid_clipselection(self, g->clip));
	pianogriddraw_init(&griddraw,
		self->keyboardstate, self->gridstate,
		self->sequenceentryoffset,
		self->hoverpatternentry,		
		self->trackdisplay,
		self->cursorchanging, self->cursoronnoterelease,		
		self->selection,
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

psy_audio_PatternSelection pianogrid_clipselection(Pianogrid* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_PatternSelection rv;

	assert(self);

	pianogridstate_clip(self->gridstate, clip.left, clip.right,
		&rv.topleft.offset, &rv.bottomright.offset);
	pianokeyboardstate_clip(self->keyboardstate,
		clip.top, clip.bottom,
		&rv.bottomright.key, &rv.topleft.key);	
	return rv;
}

void pianogrid_drawbackground(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{	
	psy_ui_RealSize size;
		
	assert(self);
	assert(pianogridstate_pattern(self->gridstate));

	size = psy_ui_component_scrollsize_px(pianogrid_base(self));
	pianogrid_drawuncoveredrightbackground(self, g, size);
	pianogrid_drawuncoveredbottombackground(self, g, size);
}

void pianogrid_drawuncoveredrightbackground(Pianogrid* self,
	psy_ui_Graphics* g, psy_ui_RealSize size)
{
	double blankstart;

	assert(self);
	assert(pianogridstate_pattern(self->gridstate));

	blankstart = pianogridstate_beattopx(self->gridstate,
		psy_audio_pattern_length(pianogridstate_pattern(self->gridstate)));
	if (blankstart - psy_ui_component_scrollleftpx(&self->component) <
			size.width) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					blankstart,
					psy_ui_component_scrolltop_px(&self->component)),
				psy_ui_realsize_make(size.width - (blankstart - psy_ui_component_scrollleftpx(
					pianogrid_base(self))),
					size.height)),
			patternviewskin_separatorcolour(self->gridstate->skin, 1, 2));
	}
}

void pianogrid_drawuncoveredbottombackground(Pianogrid* self,
	psy_ui_Graphics* g, psy_ui_RealSize size)
{
	double blankstart;

	assert(self);
	assert(pianogridstate_pattern(self->gridstate));

	blankstart = self->keyboardstate->keyboardheightpx;
	if (blankstart - psy_ui_component_scrolltop_px(&self->component) <
			size.height) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					psy_ui_component_scrollleftpx(pianogrid_base(self)),
					blankstart),
				psy_ui_realsize_make(size.width,
					size.height - (blankstart - psy_ui_component_scrolltop_px(
					pianogrid_base(self))))),
			patternviewskin_separatorcolour(self->gridstate->skin, 1, 2));
	}
}

void pianogriddraw_drawgrid(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_PatternSelection clip)
{
	assert(self);

	pianogriddraw_drawgridcells(self, g, clip);	
	pianogriddraw_drawstepseparators(self, g, clip);
	pianogriddraw_drawkeyseparators(self, g, clip);
}

void pianogriddraw_drawgridcells(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
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

	assert(self);

	if ((step % (self->gridstate->lpb * 4)) == 0) {
		if (sel) {
			rv = patternviewskin_selection4beatcolour(self->gridstate->skin, 0, 0);
		} else {
			rv = patternviewskin_row4beatcolour(self->gridstate->skin, 0, 0);
		}
	} else if ((step % self->gridstate->lpb) == 0) {
		if (sel) {
			rv = patternviewskin_selectionbeatcolour(self->gridstate->skin, 0, 0);
		} else {
			rv = patternviewskin_rowbeatcolour(self->gridstate->skin, 0, 0);
		}
	} else {
		if (sel) {
			rv = patternviewskin_selectioncolour(self->gridstate->skin, 0, 0);
		} else {
			rv = patternviewskin_rowcolour(self->gridstate->skin, 0, 0);
		}
	}
	if (psy_dsp_isblack(key)) {
		psy_ui_colour_add_rgb(&rv, -4, -4, -4);
	}
	return rv;
}

bool pianogrid_testselection(Pianogrid* self, uint8_t key, double offset)
{
	return self->selection.valid &&
		key >= self->selection.topleft.key &&
		key < self->selection.bottomright.key&&
		offset >= self->selection.topleft.offset &&
		offset < self->selection.bottomright.offset;
}

void pianogriddraw_drawstepseparators(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{
	psy_dsp_big_beat_t c;	

	assert(self);
		
	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, 0));
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
	psy_audio_PatternSelection clip)
{	
	uint8_t key;	

	assert(self);

	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, 0));	
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

void pianogriddraw_drawcursor(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_PatternSelection clip)
{
	assert(self);

	if (!self->cursorchanging && !self->cursoronnoterelease &&
			!(psy_audio_player_playing(workspace_player(self->workspace)) &&
			workspace_followingsong(self->workspace))) {
		psy_audio_PatternCursor cursor;		
		intptr_t key;
						
		cursor = workspace_patterncursor(self->workspace);
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = psy_audio_NOTECOMMANDS_MIDDLEC;
		}						
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				pianogridstate_beattopx(self->gridstate, cursor.offset),
				keyboardstate_keytopx(self->keyboardstate, key)),
			psy_ui_realsize_make(
				pianogridstate_steppx(self->gridstate),
				self->keyboardstate->keyheightpx)),
			patternviewskin_cursorcolour(self->gridstate->skin,
				0, 0));
	}
}

void pianogriddraw_drawplaybar(PianoGridDraw* self, psy_ui_Graphics* g, psy_audio_PatternSelection clip)
{
	assert(self);

	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		psy_dsp_big_beat_t offset;

		offset = psy_audio_player_position(workspace_player(self->workspace)) -
			self->sequenceentryoffset;
		if (offset >= 0 && offset < psy_audio_pattern_length(
				pianogridstate_pattern(self->gridstate))) {			
			psy_ui_drawsolidrectangle(g,
				psy_ui_realrectangle_make(
					psy_ui_realpoint_make(
						pianogridstate_beattopx(self->gridstate,
							pianogridstate_quantize(self->gridstate, offset)),
						0.0),
				psy_ui_realsize_make(
					pianogridstate_steppx(self->gridstate),
					self->keyboardstate->keyboardheightpx)),
				patternviewskin_playbarcolour(self->gridstate->skin,
					0, psy_audio_song_numsongtracks(workspace_song(self->workspace))));
		}
	}
}

void pianogriddraw_drawentries(PianoGridDraw* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{	
	psy_Table lasttrackevent;

	assert(self);

	psy_table_init(&lasttrackevent);
	if (pianogridstate_pattern(self->gridstate)) {		
		psy_audio_PatternNode* currnode;
		psy_TableIterator it;
				
		/*
		** iterates over the events of the pattern and draws events with notes. To
		** determine the length, the current event is stored for each track. In
		** the next turn the stored last event is drawn to the start of the
		** current. Finally the last events with no ending notereleases are drawn
		** till the end of the pattern.
		*/
		currnode = psy_audio_pattern_begin(pianogridstate_pattern(self->gridstate));
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
					pianogridstate_pattern(self->gridstate)) - last->offset);
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
			track == self->gridstate->cursor.track) ||
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

	rv = psy_table_at(lasttrackevent, track);
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
	psy_audio_PatternCursor cursor;
	psy_ui_RealSize corner;
	double left;
	double width;

	assert(self);

	cursor = workspace_patterncursor(self->workspace);
	left = ev->offset * self->gridstate->pxperbeat;
	width = length * self->gridstate->pxperbeat;
	corner = psy_ui_realsize_make(2, 2);
	psy_ui_setrectangle(&r, left,
		(self->keyboardstate->keymax - ev->note - 1) *
		self->keyboardstate->keyheightpx + 1, width,
		psy_max(1.0, self->keyboardstate->keyheightpx - 2));
	if (ev->hover) {
		colour = patternviewskin_eventhovercolour(self->gridstate->skin, 0, 0);
	} else if (ev->track == cursor.track) {
		colour = patternviewskin_eventcurrchannelcolour(self->gridstate->skin,
			0, 0);
	} else {		
		colour = patternviewskin_eventcolour(self->gridstate->skin, ev->track,
			psy_audio_patterns_numtracks(&self->workspace->song->patterns));
	}
	if (!ev->noterelease) {
		psy_ui_drawsolidroundrectangle(g, r, corner, colour);
	} else {		
		if (self->gridstate->cursor.key == ev->note &&
				self->gridstate->cursor.offset ==
			pianogridstate_quantize(self->gridstate, ev->offset)) {
			self->cursoronnoterelease = TRUE;
			colour = patternviewskin_cursorcolour(self->gridstate->skin,
				0, 0);
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
	if (pianogridstate_pattern(self->gridstate)) {
		rv->width = psy_ui_value_make_px(self->gridstate->pxperbeat *
			psy_audio_pattern_length(pianogridstate_pattern(self->gridstate)));
	} else {
		rv->width = psy_ui_value_make_px(0);
	}
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	self->dragcursor = pianogrid_makecursor(self, ev->pt.x, ev->pt.y);	
	self->selection.topleft = self->dragcursor;
	self->dragselectionbase = self->dragcursor;
	self->lastdragcursor = self->dragcursor;
	self->selection.valid = FALSE;
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_PatternCursor cursor;

	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternEntry* oldhover;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;

		oldhover = self->hoverpatternentry;
		node = psy_audio_pattern_findnode(pianogridstate_pattern(self->gridstate),
			self->gridstate->cursor.track,
				pianogridstate_quantize(self->gridstate,
					pianogridstate_pxtobeat(self->gridstate,
						ev->pt.x - psy_ui_component_scrollleftpx(&self->component))),
				pianogridstate_step(self->gridstate), &prev);
		if (!node) {
			if (prev) {
				psy_audio_PatternEntry* preventry;

				preventry = psy_audio_patternnode_entry(prev);
				if (preventry->track != self->gridstate->cursor.track) {
					prev = psy_audio_pattern_prev_track(pianogridstate_pattern(self->gridstate),
						prev, self->gridstate->cursor.track);
				}
				if (psy_audio_patternentry_front(preventry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
					prev = NULL;
				}
			}
			node = prev;
		}
		if (node && pianogrid_keyhittest(self, node, self->gridstate->cursor.track,
			keyboardstate_pxtokey(self->keyboardstate, ev->pt.y))) {
			self->hoverpatternentry = psy_audio_patternnode_entry(node);
		} else {
			self->hoverpatternentry = NULL;
		}
		if (self->hoverpatternentry != oldhover) {
			psy_ui_component_invalidate(&self->component);
		}
		if (ev->button == 1) {
			cursor = pianogrid_makecursor(self, ev->pt.x, ev->pt.y);
			if (cursor.key != self->lastdragcursor.key ||
				cursor.offset != self->lastdragcursor.offset) {
				if (!self->selection.valid) {
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

void pianogrid_startdragselection(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	psy_dsp_big_beat_t bpl;

	self->selection.valid = TRUE;
	bpl = 1.0 / psy_audio_player_lpb(workspace_player(self->workspace));	
	self->selection.topleft.track = cursor.track;
	self->selection.bottomright.track = cursor.track + 1;
	if (cursor.key >= self->dragselectionbase.key) {
		self->selection.topleft.key = self->dragselectionbase.key;
		self->selection.bottomright.key = cursor.key;
	} else {
		self->selection.topleft.key = cursor.key;
		self->selection.bottomright.key = self->dragselectionbase.key;
	}
	if (cursor.offset >= self->dragselectionbase.offset) {
		self->selection.topleft.offset = self->dragselectionbase.offset;
		self->selection.bottomright.offset = cursor.offset + bpl;
	} else {
		self->selection.topleft.offset = cursor.offset;
		self->selection.bottomright.offset = self->dragselectionbase.offset + bpl;
	}
	self->selection.bottomright.key += 1;
}

void pianogrid_dragselection(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	psy_dsp_big_beat_t bpl;

	bpl = 1.0 / psy_audio_player_lpb(workspace_player(self->workspace));
	/* intptr_t restoremidline = self->midline; */
	if (cursor.key >= self->dragselectionbase.key) {
		self->selection.topleft.key = self->dragselectionbase.key;
		self->selection.bottomright.key = cursor.key + 1;
	} else {
		self->selection.topleft.key = cursor.key;
		self->selection.bottomright.key = self->dragselectionbase.key + 1;
	}
	if (cursor.offset >= self->dragselectionbase.offset) {
		self->selection.topleft.offset = self->dragselectionbase.offset;
		self->selection.bottomright.offset = cursor.offset + bpl;
	} else {
		self->selection.topleft.offset = cursor.offset;
		self->selection.bottomright.offset = self->dragselectionbase.offset + bpl;
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

	if (!self->selection.valid && pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternEvent patternevent;
		psy_audio_PatternCursor cursor;

		psy_ui_component_setfocus(&self->component);
		psy_audio_patternevent_clear(&patternevent);
		pianogrid_updatekeystate(self);		
		patternevent.note = self->dragcursor.key;
		cursor = self->dragcursor;
		if (ev->button == 1) {
			/* left button */
			psy_undoredo_execute(&self->workspace->undoredo,
				&insertcommand_alloc(
					pianogridstate_pattern(self->gridstate),
					pianogridstate_step(self->gridstate),
					cursor, patternevent,self->workspace)->command);
			workspace_setpatterncursor(self->workspace, cursor);			
		} else if (ev->button == 2) {
			/* right button */
			psy_audio_PatternNode* node;
			psy_audio_PatternNode* prev;

			node = psy_audio_pattern_findnode(
				pianogridstate_pattern(self->gridstate),
				self->gridstate->cursor.track, cursor.offset,
					pianogridstate_step(self->gridstate), &prev);
			if (!node) {				
				if (prev) {
					psy_audio_PatternEntry* preventry;

					preventry = psy_audio_patternnode_entry(prev);
					if (preventry->track != cursor.track) {
						prev = psy_audio_pattern_prev_track(pianogridstate_pattern(self->gridstate),
							prev, cursor.track);
					}
				}
				node = prev;
			}
			if (node) {
				psy_audio_PatternNode* next;				
				psy_audio_PatternEntry* patternentry;

				patternentry = psy_audio_patternnode_entry(node);				
				if (!pianogrid_keyhittest(self, node, cursor.track, cursor.key)) {
					pianogrid_storecursor(self);
					workspace_setpatterncursor(self->workspace, cursor);
					pianogrid_invalidatecursor(self);
					return;
				}
				if (patternentry->offset == cursor.offset) {
					next = psy_audio_pattern_next_track(pianogridstate_pattern(self->gridstate),
						node, cursor.track);
					if (self->hoverpatternentry == psy_audio_patternnode_entry(node)) {
						self->hoverpatternentry = NULL;
					}
					psy_undoredo_execute(&self->workspace->undoredo,
						&removecommand_alloc(self->gridstate->pattern,
							pianogridstate_step(self->gridstate),
							cursor, self->workspace)->command);
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
								&removecommand_alloc(self->gridstate->pattern,
									pianogridstate_step(self->gridstate),
									cursor, self->workspace)->command);
						}
					}
					workspace_setpatterncursor(self->workspace, cursor);
				} else {
					if (psy_audio_patternentry_front(patternentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
						workspace_setpatterncursor(self->workspace, cursor);
						return;
					} else {
						psy_audio_PatternEvent release;

						next = psy_audio_pattern_next_track(pianogridstate_pattern(self->gridstate), node,
							cursor.track);
						if (next) {
							psy_audio_PatternEntry* nextentry;							

							nextentry = psy_audio_patternnode_entry(next);
							if (psy_audio_patternentry_front(nextentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
								psy_audio_PatternCursor release;

								release = cursor;
								release.offset = nextentry->offset;
								psy_undoredo_execute(&self->workspace->undoredo,
									&removecommand_alloc(self->gridstate->pattern,
										pianogridstate_step(self->gridstate),
										release, self->workspace)->command);
							}
						}						
						psy_audio_patternevent_clear(&release);
						release.note = psy_audio_NOTECOMMANDS_RELEASE;
						psy_undoredo_execute(&self->workspace->undoredo,
							&insertcommand_alloc(pianogridstate_pattern(self->gridstate),
								pianogridstate_step(self->gridstate),
								cursor, release,
								self->workspace)->command);
						cursor.key = psy_audio_patternentry_front(patternentry)->note;
						workspace_setpatterncursor(self->workspace, cursor);
					}
				}
			}
		}
	}
	psy_ui_component_invalidate(&self->component);
}

psy_audio_PatternCursor pianogrid_makecursor(Pianogrid* self, double x, double y)
{
	psy_audio_PatternCursor rv;

	psy_audio_patterncursor_init(&rv);
	rv.offset = pianogridstate_quantize(self->gridstate,
		pianogridstate_pxtobeat(self->gridstate, x));
	rv.key = keyboardstate_pxtokey(self->keyboardstate, y);
	rv.track = self->gridstate->cursor.track;
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
		prevtrack = psy_audio_pattern_prev_track(pianogridstate_pattern(
			self->gridstate), node, track);
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

void pianogrid_setcursor(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	psy_audio_PatternCursor oldcursor;

	assert(self);

	oldcursor = self->gridstate->cursor;
	self->gridstate->cursor = cursor;
	if (self->gridstate->cursor.track != oldcursor.track) {
		psy_ui_component_invalidate(&self->component);
	} else if (oldcursor.offset != self->gridstate->cursor.offset &&
			!self->cursorchanging) {
		pianogrid_invalidateline(self, oldcursor.offset);
		pianogrid_invalidateline(self, self->gridstate->cursor.offset);
	}
	self->cursorchanging = FALSE;
	if (psy_audio_player_playing(workspace_player(self->workspace)) && workspace_followingsong(self->workspace)
			&& pianogridstate_pattern(self->gridstate)) {
		bool scrolldown;

		scrolldown = self->lastplayposition <
			psy_audio_player_position(workspace_player(self->workspace));
		pianogrid_invalidateline(self, self->lastplayposition - self->sequenceentryoffset);
		self->lastplayposition = psy_audio_player_position(workspace_player(self->workspace));
		pianogrid_invalidateline(self, self->lastplayposition - self->sequenceentryoffset);		
		if (self->lastplayposition >= self->sequenceentryoffset &&
			self->lastplayposition < self->sequenceentryoffset +
			pianogridstate_pattern(self->gridstate)->length) {
			if (scrolldown != FALSE) {
				pianogrid_scrollright(self, self->gridstate->cursor);
			} else {
				pianogrid_scrollleft(self, self->gridstate->cursor);
			}
		}
		pianogrid_storecursor(self);
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

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->cursor,
			pianogridstate_pattern(self->gridstate),
			pianogridstate_step(self->gridstate), wrap, 0);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_prevlines(&cursornavigator,
				lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace,
			pianogridstate_cursor(self->gridstate));
		pianogrid_invalidatecursor(self);
	}
}

void pianogrid_prevkeys(Pianogrid* self, uint8_t lines, bool wrap)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->cursor,
			pianogridstate_pattern(self->gridstate),
			pianogridstate_step(self->gridstate), wrap,
			self->keyboardstate->keymax);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_prevkeys(&cursornavigator,
			lines)) {
			pianogrid_scrollup(self, self->gridstate->cursor);
		} else {
			pianogrid_scrolldown(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace,
			pianogridstate_cursor(self->gridstate));
		pianogrid_invalidatecursor(self);
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

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			pianogridstate_pattern(self->gridstate),
			pianogridstate_step(self->gridstate), wrap, 0);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
		pianogrid_invalidatecursor(self);
	}
}

void pianogrid_advancekeys(Pianogrid* self, uint8_t lines, bool wrap)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			pianogridstate_pattern(self->gridstate),
			pianogridstate_step(self->gridstate), wrap,
			self->keyboardstate->keymax);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_advancekeys(&cursornavigator, lines)) {
			pianogrid_scrollup(self, self->gridstate->cursor);
		} else {
			pianogrid_scrolldown(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
		pianogrid_invalidatecursor(self);
	}
}

bool pianogrid_scrollleft(Pianogrid* self, psy_audio_PatternCursor cursor)
{	
	assert(self);
	
	if (pianogridstate_pxtobeat(self->gridstate,
			psy_ui_component_scrollleftpx(&self->component)) > cursor.offset) {
		psy_ui_component_setscrollleft(&self->component,
			psy_ui_value_make_px(pianogridstate_quantizebeattopx(self->gridstate,
				cursor.offset)));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scrollright(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	intptr_t line;
	intptr_t visilines;
	psy_ui_RealSize clientsize;

	assert(self);

	clientsize = psy_ui_component_clientsize_px(&self->component);
	visilines = (intptr_t)((clientsize.width /
		(psy_dsp_big_beat_t)self->gridstate->pxperbeat) * self->gridstate->lpb);	
	--visilines;	
	line = pianogridstate_beattosteps(self->gridstate, cursor.offset);
	if (visilines < line - psy_ui_component_scrollleftpx(&self->component) /
		pianogridstate_steppx(self->gridstate) + 2) {
		intptr_t dlines;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		dlines = (intptr_t)((line - psy_ui_component_scrollleftpx(&self->component) /
			pianogridstate_steppx(self->gridstate) - visilines + 16));
		self->cursorchanging = TRUE;
		psy_ui_component_setscrollleft(&self->component,
			psy_ui_value_make_px(
				psy_max(0.0, psy_ui_component_scrollleftpx(&self->component) +
					psy_ui_component_scrollstep_width_px(&self->component) * dlines)));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scrollup(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	intptr_t line;
	intptr_t topline;
	psy_ui_RealRectangle r;
	const psy_ui_TextMetric* tm;

	assert(self);

	tm = psy_ui_component_textmetric(&self->component);
	pianogrid_updatekeystate(self);
	line = self->keyboardstate->keymax - self->gridstate->cursor.key - 1;
	psy_ui_setrectangle(&r,
		0,
		self->keyboardstate->keyheightpx * line,
		0,
		self->keyboardstate->keyheightpx);	
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

bool pianogrid_scrolldown(Pianogrid* self, psy_audio_PatternCursor cursor)
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
	line = self->keyboardstate->keymax - self->gridstate->cursor.key;
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
	
	if (pianogridstate_pattern(self->gridstate) && position >= 0.0 &&
			position < psy_audio_pattern_length(pianogridstate_pattern(
			self->gridstate))) {
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
}

void pianogrid_invalidatecursor(Pianogrid* self)
{
	assert(self);

	pianogrid_invalidateline(self, psy_audio_patterncursor_offset(
		&self->oldcursor));
	pianogrid_invalidateline(self, psy_audio_patterncursor_offset(
		&self->gridstate->cursor));
}

void pianogrid_storecursor(Pianogrid* self)
{
	assert(self);

	self->oldcursor = self->gridstate->cursor;
}

/* PianoBar */
/* implenentation */
void pianobar_init(PianoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
		
	psy_ui_component_init(&self->component, parent, NULL);
	margin = psy_ui_defaults_hmargin(psy_ui_defaults());
	psy_ui_margin_setleft(&margin, psy_ui_value_make_ew(1.0));
	psy_ui_margin_setbottom(&margin, psy_ui_value_make_ew(0.5));
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		margin);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->keys, pianobar_base(self), NULL,
		"Keyboard");
	zoombox_init(&self->zoombox_keyheight, &self->component);
	psy_ui_combobox_init(&self->keytype, pianobar_base(self), NULL);
	psy_ui_combobox_setcharnumber(&self->keytype, 8);	
	psy_ui_combobox_addtext(&self->keytype, "Keys");	
	psy_ui_combobox_addtext(&self->keytype, "Notes");
	psy_ui_combobox_addtext(&self->keytype, "Drums");
	psy_ui_combobox_setcursel(&self->keytype, 0);
	psy_ui_label_init_text(&self->tracks, pianobar_base(self), NULL,
		"Show Tracks");
	psy_ui_button_init_text(&self->tracks_all, pianobar_base(self), NULL, "All");
	psy_ui_button_highlight(&self->tracks_all);
	psy_ui_button_init_text(&self->track_curr, pianobar_base(self), NULL, "Current");
	psy_ui_button_init_text(&self->tracks_active, pianobar_base(self), NULL, "Active");
	psy_ui_button_init_text(&self->blockmenu, pianobar_base(self), NULL, "Block Menu");
	psy_ui_component_setalign(psy_ui_button_base(&self->blockmenu), psy_ui_ALIGN_RIGHT);
}

/* Pianoroll */
/* protoypes */
static void pianoroll_ontimer(Pianoroll*, uintptr_t timerid);
static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player*,
	uintptr_t lpb);
static void pianoroll_onalign(Pianoroll*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_ongridscroll(Pianoroll*, psy_ui_Component* sender);
static void pianoroll_onbeatwidthchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onkeyheightchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onkeytypeselchange(Pianoroll*, psy_ui_Component* sender, int sel);
static void pianoroll_ondisplayalltracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_ondisplaycurrenttrack(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_ondisplayactivetracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_updatetrackdisplaybuttons(Pianoroll*);
static void pianoroll_onthemechanged(Pianoroll*, PatternViewConfig*, psy_Property* theme);
static void pianoroll_updatetheme(Pianoroll*);
static void pianoroll_onpatterncursorchanged(Pianoroll*, Workspace* sender);
/* vtable */
static psy_ui_ComponentVtable pianoroll_vtable;
static bool pianoroll_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoroll_vtable_init(Pianoroll* self)
{
	assert(self);

	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign =
			(psy_ui_fp_component_onalign)
			pianoroll_onalign;
		pianoroll_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			pianoroll_onmousedown;		
		pianoroll_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = TRUE;
	}
	return &pianoroll_vtable;
}
/* implenentation */
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	PatternViewSkin* skin, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component, pianoroll_vtable_init(self));
	self->workspace = workspace;
	patterncmds_init(&self->cmds, workspace);
	self->opcount = 0;
	self->syncpattern = 1;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);		
	/* shared states */
	keyboardstate_init(&self->keyboardstate, skin);
	pianogridstate_init(&self->gridstate, skin);	
	/* left area (keyboardheader, keyboard) */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_beatwidth, &self->left);
	psy_signal_connect(&self->zoombox_beatwidth.signal_changed, self,
		pianoroll_onbeatwidthchanged);
	psy_ui_component_setalign(zoombox_base(&self->zoombox_beatwidth),
		psy_ui_ALIGN_TOP);
	/* Keyboard */
	psy_ui_component_init(&self->keyboardpane, &self->left, NULL);
	psy_ui_component_setalign(&self->keyboardpane, psy_ui_ALIGN_CLIENT);
	pianokeyboard_init(&self->keyboard, &self->keyboardpane, &self->keyboardstate);
	psy_ui_component_setalign(&self->keyboard.component,
		psy_ui_ALIGN_FIXED_RESIZE);	
	/* top area (beatruler) */
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	pianoruler_init(&self->header, &self->top, &self->gridstate);
	psy_ui_component_setalign(pianoruler_base(&self->header),
		psy_ui_ALIGN_FIXED_RESIZE);
	/* client area (event grid) */
	pianogrid_init(&self->grid, &self->component, &self->keyboardstate,
		&self->gridstate, self->workspace);
	psy_ui_scroller_init(&self->scroller, pianogrid_base(&self->grid),
		&self->component, NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_FIXED_RESIZE);
	/* bar */
	pianobar_init(&self->bar, &self->scroller.component, self->workspace);	
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->bar.zoombox_keyheight.signal_changed, self,
		pianoroll_onkeyheightchanged);
	psy_ui_component_setalign(&self->bar.zoombox_keyheight.component,
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&pianogrid_base(&self->grid)->signal_scroll, self,
		pianoroll_ongridscroll);
	psy_signal_connect(&self->bar.keytype.signal_selchanged, self,
		pianoroll_onkeytypeselchange);
	psy_signal_connect(&self->bar.tracks_all.signal_clicked, self,
		pianoroll_ondisplayalltracks);
	psy_signal_connect(&self->bar.track_curr.signal_clicked, self,
		pianoroll_ondisplaycurrenttrack);
	psy_signal_connect(&self->bar.tracks_active.signal_clicked, self,
		pianoroll_ondisplayactivetracks);	
	psy_signal_connect(
		&psycleconfig_patview(workspace_conf(workspace))->signal_themechanged,
		self, pianoroll_onthemechanged);
	psy_signal_connect(&self->workspace->signal_patterncursorchanged,
		self, pianoroll_onpatterncursorchanged);
	pianoroll_updatetheme(self);
	psy_ui_component_starttimer(&self->component, 0, PIANOROLL_REFRESHRATE);
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	assert(self);

	patterncmds_setpattern(&self->cmds, pattern);
	pianogridstate_setpattern(&self->gridstate, pattern);
	pianogrid_onpatternchange(&self->grid, pattern);
	psy_ui_component_setscroll(pianogrid_base(&self->grid),
		psy_ui_point_make(psy_ui_value_zero(),
			psy_ui_component_scrolltop(pianogrid_base(&self->grid))));
	psy_ui_component_setscroll(pianoruler_base(&self->header),
		psy_ui_point_zero());
	pianoroll_updatescroll(self);
}

void pianoroll_ontimer(Pianoroll* self, uintptr_t timerid)
{
	assert(self);

	if (pianogridstate_pattern(&self->gridstate) &&
			psy_ui_component_visible(&self->component)) {
		if (psy_audio_player_playing(workspace_player(self->workspace))) {
			intptr_t line;
			line = (intptr_t)(psy_audio_player_position(workspace_player(self->workspace)) /
				self->gridstate.lpb);
			if (self->grid.lastplayposition / self->gridstate.lpb != line) {
				pianogrid_invalidateline(&self->grid,
					self->grid.lastplayposition - self->grid.sequenceentryoffset);
				self->grid.lastplayposition =
					psy_audio_player_position(workspace_player(self->workspace));
				pianogrid_invalidateline(&self->grid,
					self->grid.lastplayposition - self->grid.sequenceentryoffset);
			}
		} else if (self->grid.lastplayposition != -1) {
			pianogrid_invalidateline(&self->grid,
				self->grid.lastplayposition - self->grid.sequenceentryoffset);
			self->grid.lastplayposition = -1;
		}		
		if (pianogridstate_pattern(&self->gridstate) &&
				pianogridstate_pattern(&self->gridstate)->opcount != self->opcount &&
				self->syncpattern) {
			psy_ui_component_invalidate(pianogrid_base(&self->grid));
		}
		self->opcount = pianogridstate_pattern(&self->gridstate)
			? pianogridstate_pattern(&self->gridstate)->opcount
			: 0;
	}
}

void pianoroll_onalign(Pianoroll* self)
{
	assert(self);

	pianoroll_updatescroll(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_setfocus(pianogrid_base(&self->grid));
}

void pianoroll_updatescroll(Pianoroll* self)
{	
	assert(self);	
	
	pianogrid_updatekeystate(&self->grid);
	psy_ui_component_setscrollstep(pianogrid_base(&self->grid),
		psy_ui_size_make_px(
			pianogridstate_steppx(&self->gridstate),
			self->keyboardstate.keyheightpx));
	psy_ui_component_setscrollstep_height(pianokeyboard_base(&self->keyboard),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_setscrollstep_width(pianoruler_base(&self->header),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianokeyboard_base(&self->keyboard));
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	assert(self);

	pianogridstate_setlpb(&self->gridstate, lpb);
	psy_ui_component_setscrollstep_width(pianogrid_base(&self->grid),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoruler_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_onpatterncursorchanged(Pianoroll* self,
	Workspace* sender)
{	
	assert(self);

	pianogrid_setcursor(&self->grid, workspace_patterncursor(self->workspace));
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scrollleftpx(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrollleftpx(pianoruler_base(&self->header))) {
		psy_ui_component_setscrollleft(pianoruler_base(&self->header),
			psy_ui_component_scrollleft(pianogrid_base(&self->grid)));		
	}
	if (psy_ui_component_scrolltop_px(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrolltop_px(&self->keyboard.component)) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(pianogrid_base(&self->grid)));		
	}
}

void pianoroll_onbeatwidthchanged(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	pianogridstate_setzoom(&self->gridstate, zoombox_rate(sender));
	psy_ui_component_setscrollstep_width(pianogrid_base(&self->grid),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoruler_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_align(&self->top);
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void pianoroll_onkeyheightchanged(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	self->keyboardstate.keyheight = psy_ui_mul_value_real(
		self->keyboardstate.defaultkeyheight, zoombox_rate(sender));
	pianogrid_updatekeystate(&self->grid);
	psy_ui_component_setscrollstep_height(pianogrid_base(&self->grid),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_setscrollstep_height(pianokeyboard_base(&self->keyboard),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrolltop(&self->keyboard.component,
		psy_ui_component_scrolltop(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);	
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void pianoroll_onkeytypeselchange(Pianoroll* self, psy_ui_Component* sender, int sel)
{
	assert(self);

	if (sel >= 0 && sel < KEYBOARDTYPE_NUM) {
		pianokeyboard_setkeyboardtype(&self->keyboard, (KeyboardType)sel);
		psy_ui_component_align(pianoroll_base(self));
	}	
}

void pianoroll_ondisplayalltracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_ALL);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_ondisplaycurrenttrack(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_CURRENT);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_ondisplayactivetracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_ACTIVE);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_updatetrackdisplaybuttons(Pianoroll* self)
{
	assert(self);

	psy_ui_button_disablehighlight(&self->bar.tracks_all);
	psy_ui_button_disablehighlight(&self->bar.track_curr);
	psy_ui_button_disablehighlight(&self->bar.tracks_active);
	switch (pianogrid_trackdisplay(&self->grid)) {
		case PIANOROLL_TRACK_DISPLAY_ALL:
			psy_ui_button_highlight(&self->bar.tracks_all);			
			break;
		case PIANOROLL_TRACK_DISPLAY_CURRENT:
			psy_ui_button_highlight(&self->bar.track_curr);
			break;
		case PIANOROLL_TRACK_DISPLAY_ACTIVE:			
			psy_ui_button_highlight(&self->bar.tracks_active);
			break;
		default:			
			break;
	}	
}

void pianoroll_makecmds(psy_Property* parent)
{
	psy_Property* cmds;

	assert(parent);

	cmds = psy_property_settext(psy_property_append_section(parent,
		"pianoroll"), "Pianoroll");
	setcmdall(cmds, CMD_NAVUP,
		psy_ui_KEY_UP, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navup", "up");	
	setcmdall(cmds, CMD_NAVDOWN,
		psy_ui_KEY_DOWN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navdown", "down");
	setcmdall(cmds, CMD_NAVLEFT,
		psy_ui_KEY_LEFT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navleft", "left");
	setcmdall(cmds, CMD_NAVRIGHT,
		psy_ui_KEY_RIGHT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navright", "right");
	setcmdall(cmds, CMD_NAVPAGEUP,
		psy_ui_KEY_PRIOR, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpageup", "pageup");
	setcmdall(cmds, CMD_NAVPAGEDOWN,
		psy_ui_KEY_NEXT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpagedown", "pagedown");
	setcmdall(cmds, CMD_NAVPAGEUPKEYBOARD,
		psy_ui_KEY_PRIOR, psy_SHIFT_ON, psy_CTRL_OFF,
		"navpageupkbd", "pageup kbd");
	setcmdall(cmds, CMD_NAVPAGEDOWNKEYBOARD,
		psy_ui_KEY_NEXT, psy_SHIFT_ON, psy_CTRL_OFF,
		"navpagedownkbd", "pagedown kbd");
	setcmdall(cmds, CMD_ENTER,
		psy_ui_KEY_SPACE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"enter", "enter");
	setcmdall(cmds, CMD_ROWCLEAR,
		psy_ui_KEY_DELETE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowclear", "clr row");
	setcmdall(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	setcmdall(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
	setcmdall(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	setcmdall(cmds, CMD_BLOCKCUT,
		psy_ui_KEY_X, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcut", "cut");
	setcmdall(cmds, CMD_BLOCKCOPY,
		psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcopy", "copy");
	setcmdall(cmds, CMD_BLOCKPASTE,
		psy_ui_KEY_V, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockpaste", "paste");
	setcmdall(cmds, CMD_BLOCKMIX,
		psy_ui_KEY_M, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockmix", "mix");
	setcmdall(cmds, CMD_BLOCKDELETE,
		0, 0, 0,
		"blockdelete", "blkdel");

	setcmdall(cmds, CMD_SELECTALL,
		psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectall", "sel all");
	setcmdall(cmds, CMD_SELECTBAR,
		psy_ui_KEY_K, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectbar", "sel bar");

	setcmdall(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	setcmdall(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	setcmdall(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
}

bool pianoroll_handlecommand(Pianoroll* self, uintptr_t cmd)
{
	bool handled;

	assert(self);
	
	handled = TRUE;
	switch (cmd) {
		case CMD_NAVUP:
			pianoroll_navup(self);
			break;
		case CMD_NAVPAGEUP:
			pianogrid_prevlines(&self->grid, self->grid.pgupdownstep, 0);
			break;
		case CMD_NAVPAGEUPKEYBOARD:
			pianogrid_advancekeys(&self->grid, 12, 0);
			break;
		case CMD_NAVDOWN:
			pianoroll_navdown(self);
			break;
		case CMD_NAVPAGEDOWN:
			pianogrid_advancelines(&self->grid, self->grid.pgupdownstep, 0);
			break;
		case CMD_NAVPAGEDOWNKEYBOARD:
			pianogrid_prevkeys(&self->grid, 12, 0);			
			break;
		case CMD_NAVLEFT:
			pianogrid_prevline(&self->grid);		
			break;
		case CMD_NAVRIGHT:
			pianogrid_advanceline(&self->grid);
			break;				
		case CMD_BLOCKSTART:
			pianoroll_blockstart(self);
			break;
		case CMD_BLOCKEND:
			pianoroll_blockend(self);
			break;
		case CMD_ENTER: {
			pianoroll_enter(self);
			break; }
		case CMD_ROWCLEAR:		
			pianoroll_rowclear(self);
			break;
		case CMD_BLOCKPASTE:
			pianoroll_blockpaste(self);
			break;
		case CMD_BLOCKCOPY:
			pianoroll_blockcopy(self);
			break;
		case CMD_BLOCKCUT:
			pianoroll_blockcut(self);
			break;
		case CMD_BLOCKDELETE:
			pianoroll_blockdelete(self);
			break;
		case CMD_SELECTALL:
			pianoroll_selectall(self);
			break;
		case CMD_SELECTBAR:
			pianoroll_selectbar(self);
			break;
		case CMD_BLOCKUNMARK:
			pianoroll_blockunmark(self);
			break;			
		default:
			handled = FALSE;
			break;
	}
	return handled;
}

/*
** Defines a property with shortcut defaults for the keyboard driver
** key		: cmd id used by the trackerview
** text		: "cmds.key" language dictionary key used by the translator
** shorttext: short description for the keyboard help view
** value	: encoded key shortcut (keycode/shift/ctrl)
*/
void setcmdall(psy_Property* cmds, uintptr_t cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	assert(cmds);

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint(psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(cmds, key,
			psy_audio_encodeinput(keycode, shift, ctrl, 0), 0, 0),
			cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}

void pianoroll_navup(Pianoroll* self)
{
	if (self->gridstate.cursor.key < self->keyboardstate.keymax - 1) {
		pianogrid_storecursor(&self->grid);
		++self->gridstate.cursor.key;
		pianogrid_scrollup(&self->grid, self->gridstate.cursor);
		workspace_setpatterncursor(self->workspace,
			self->gridstate.cursor);
		pianogrid_invalidatecursor(&self->grid);
	} 
}

void pianoroll_navdown(Pianoroll* self)
{
	if (self->gridstate.cursor.key > self->keyboardstate.keymin) {
		pianogrid_storecursor(&self->grid);
		--self->gridstate.cursor.key;
		pianogrid_scrolldown(&self->grid, self->gridstate.cursor);
		workspace_setpatterncursor(self->workspace,
			self->gridstate.cursor);
		pianogrid_invalidatecursor(&self->grid);
	}
}

void pianoroll_enter(Pianoroll* self)
{
	psy_audio_PatternEvent patternevent;

	psy_audio_patternevent_clear(&patternevent);
	patternevent.note = self->gridstate.cursor.key;
	psy_undoredo_execute(&self->workspace->undoredo,
		&insertcommand_alloc(pianogridstate_pattern(&self->gridstate),
			pianogridstate_step(&self->gridstate),
			self->gridstate.cursor, patternevent,
			self->workspace)->command);
	pianogrid_advanceline(&self->grid);
}

void pianoroll_rowclear(Pianoroll* self)
{
	psy_undoredo_execute(&self->workspace->undoredo,
		&removecommand_alloc(self->gridstate.pattern,
			pianogridstate_step(&self->gridstate),
			self->gridstate.cursor, self->workspace)->command);
	pianogrid_advanceline(&self->grid);
}

void pianoroll_blockcut(Pianoroll* self)
{
	assert(self);
	
	pianoroll_blockcopy(self);
	pianoroll_blockdelete(self);	
}

void pianoroll_blockcopy(Pianoroll* self)
{
	assert(self);

	patterncmds_blockcopy(&self->cmds, self->grid.selection);	
}

void pianoroll_blockpaste(Pianoroll* self)
{
	assert(self);

	patterncmds_blockpaste(&self->cmds, self->gridstate.cursor, FALSE);
	psy_ui_component_invalidate(&self->component);	
}

void pianoroll_blockdelete(Pianoroll* self)
{
	assert(self);

	patterncmds_blockdelete(&self->cmds, self->grid.selection);	
	psy_ui_component_invalidate(&self->grid.component);	
}

void pianoroll_selectall(Pianoroll* self)
{
	if (workspace_song(self->workspace) && pianogridstate_pattern(
			&self->gridstate)) {
		self->grid.selection = psy_audio_patternselection_make(
			psy_audio_patterncursor_make_all(0, 0.0, 0),
			psy_audio_patterncursor_make_all(				
				psy_audio_song_numsongtracks(workspace_song(self->workspace)),				
				pianogridstate_length(&self->gridstate),
				psy_audio_NOTECOMMANDS_B9));		
		psy_ui_component_invalidate(&self->component);
	}
}

void pianoroll_selectbar(Pianoroll* self)
{
	if (workspace_song(self->workspace) && pianogridstate_pattern(
			&self->gridstate)) {
		if (self->gridstate.cursor.offset > pianogridstate_length(&self->gridstate)) {
			self->gridstate.cursor.offset = pianogridstate_length(&self->gridstate);
		}
		self->grid.selection = psy_audio_patternselection_make(
			psy_audio_patterncursor_make_all(
				self->gridstate.cursor.track,
				self->gridstate.cursor.offset,
				0),
			psy_audio_patterncursor_make_all(
				self->gridstate.cursor.track + 1,
				pianogridstate_length(&self->gridstate),
				0));
		psy_audio_patternselection_enable(&self->grid.selection);
		psy_ui_component_invalidate(&self->component);
	}
}

void pianoroll_blockunmark(Pianoroll* self)
{
	psy_audio_patternselection_disable(&self->grid.selection);
	psy_ui_component_invalidate(&self->component);
}

void pianoroll_blockstart(Pianoroll* self)
{
	self->grid.dragselectionbase = self->gridstate.cursor;
	pianogrid_startdragselection(&self->grid, self->gridstate.cursor);
	psy_ui_component_invalidate(&self->component);
}

void pianoroll_blockend(Pianoroll* self)
{
	pianogrid_dragselection(&self->grid, self->gridstate.cursor);	
	psy_ui_component_invalidate(&self->component);
}

void pianoroll_onthemechanged(Pianoroll* self, PatternViewConfig* config, psy_Property* theme)
{
	pianoroll_updatetheme(self);
}

void pianoroll_updatetheme(Pianoroll* self)
{
	assert(self);

	if (self->gridstate.skin) {
		psy_ui_component_setbackgroundcolour(pianoruler_base(&self->header),
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		psy_ui_component_setbackgroundcolour(zoombox_base(&self->zoombox_beatwidth),
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		/* hbar */
		psy_ui_component_setbackgroundcolour(&self->scroller.hscroll.pane.component,
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		psy_ui_component_setcolour(&self->scroller.hscroll.pane.component,
			patternviewskin_rowcolour(self->gridstate.skin, 0, 0));
		/* vbar */
		psy_ui_component_setbackgroundcolour(&self->scroller.vscroll.pane.component,
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		psy_ui_component_setcolour(&self->scroller.vscroll.pane.component,
			patternviewskin_rowcolour(self->gridstate.skin, 0, 0));
		/* psy_ui_component_setbackgroundcolour(pianobar_base(&self->bar), 
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0)); */
	}
}
