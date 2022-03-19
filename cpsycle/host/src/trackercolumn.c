/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackercolumn.h"
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"

static const char* notetostr(psy_audio_PatternEvent ev,
	psy_dsp_NotesTabMode notestabmode, bool showemptydate)
{
	static const char* emptynotestr = "- - -";

	if (ev.note != psy_audio_NOTECOMMANDS_EMPTY || !showemptydate) {
		return psy_dsp_notetostr(ev.note, notestabmode);
	}
	return emptynotestr;
}

/* TrackerColumn */
/* prototypes */
static void trackercolumn_ondraw(TrackerColumn*, psy_ui_Graphics*);
static void trackercolumn_drawtrackevents(TrackerColumn*, psy_ui_Graphics*);
static  TrackerColumnFlags trackercolumn_columnflags(TrackerColumn*,
	uintptr_t line);
static void trackercolumn_drawentry(TrackerColumn*, psy_ui_Graphics*,
	psy_audio_PatternEntry*, double y, TrackerColumnFlags, TrackDef*);
static void trackercolumn_drawdigit(TrackerColumn*, psy_ui_Graphics*,
	psy_ui_RealPoint cp, const char* str);
static void trackercolumn_drawresizebar(TrackerColumn*, psy_ui_Graphics*);
static void trackercolumn_onmousedown(TrackerColumn*, psy_ui_MouseEvent*);
static void trackercolumn_onmousemove(TrackerColumn*, psy_ui_MouseEvent*);
static void trackercolumn_onmouseup(TrackerColumn*, psy_ui_MouseEvent*);
static void trackercolumn_onpreferredsize(TrackerColumn*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void trackercolumn_updatecursor(TrackerColumn*, double position);
static bool trackercolumn_isovernote(const TrackerColumn*, double position);
static bool trackercolumn_isovercolumn(const TrackerColumn*, double position);

/* vtable */
static psy_ui_ComponentVtable trackercolumn_vtable;
static bool trackercolumn_vtable_initialized = FALSE;

static void trackercolumn_vtable_init(TrackerColumn* self)
{
	if (!trackercolumn_vtable_initialized) {
		trackercolumn_vtable = *(self->component.vtable);
		trackercolumn_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackercolumn_ondraw;
		trackercolumn_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			trackercolumn_onpreferredsize;
		trackercolumn_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackercolumn_onmousedown;
		trackercolumn_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			trackercolumn_onmousemove;
		trackercolumn_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			trackercolumn_onmouseup;
		trackercolumn_vtable_initialized = TRUE;
	}
	self->component.vtable = &trackercolumn_vtable;
}

/* implementation */
void trackercolumn_init(TrackerColumn* self, psy_ui_Component* parent,
	uintptr_t index, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackercolumn_vtable_init(self);		
	psy_ui_component_setalign(trackercolumn_base(self), psy_ui_ALIGN_LEFT);
	self->state = state;
	self->workspace = workspace;	
	self->track = index;
}

TrackerColumn* trackercolumn_alloc(void)
{
	return (TrackerColumn*)malloc(sizeof(TrackerColumn));
}

TrackerColumn* trackercolumn_allocinit(psy_ui_Component* parent,
	uintptr_t index, TrackerState* state, Workspace* workspace)
{
	TrackerColumn* rv;

	rv = trackercolumn_alloc();
	if (rv) {
		trackercolumn_init(rv, parent, index, state, workspace);
		psy_ui_component_deallocateafterdestroyed(trackercolumn_base(rv));
	}
	return rv;
}

void trackercolumn_ondraw(TrackerColumn* self, psy_ui_Graphics* g)
{		
	trackercolumn_drawtrackevents(self, g);	
	if (trackdrag_trackactive(&self->state->trackconfig->resize,
			self->track)) {
		trackercolumn_drawresizebar(self, g);
	}
}

void trackercolumn_drawtrackevents(TrackerColumn* self, psy_ui_Graphics* g)
{	
	psy_List** events;
	psy_List* p;
	double cpy;	
	uintptr_t line;
	TrackDef* trackdef;
	
	events = trackereventtable_track(&self->state->trackevents,
		self->track);
	trackdef = trackerconfig_trackdef(self->state->trackconfig, self->track);
	self->digitsize = psy_ui_realsize_make(
		self->state->trackconfig->textwidth,
		self->state->lineheightpx - 1);	
	for (p = *events,			
			cpy = trackerstate_beattopx(self->state,
				patternviewstate_draw_offset(self->state->pv,
					self->state->trackevents.clip.topleft.absoffset)),
			line = (uintptr_t)(self->state->trackevents.clip.topleft.absoffset *
				self->state->trackevents.clip.topleft.lpb);
			p != NULL;
			p = p->next, ++line, cpy += self->state->lineheightpx) {
		trackercolumn_drawentry(self, g, (psy_audio_PatternEntry*)p->entry,
			cpy, trackercolumn_columnflags(self, line), trackdef);
	}	
}

TrackerColumnFlags trackercolumn_columnflags(TrackerColumn* self,
	uintptr_t line)
{
	TrackerColumnFlags rv;	

	assert(self);
	
	if (self->state->drawbeathighlights) {
		uintptr_t lpb;

		lpb = patternviewstate_lpb(self->state->pv);
		rv.beat = (line % lpb) == 0;
		rv.beat4 = (line % (lpb * 4)) == 0;		
		rv.mid = self->state->midline &&
			(line == trackerstate_midline(self->state,
			psy_ui_component_scrolltop_px(psy_ui_component_parent(
				&self->component))));
	} else {
		rv.beat = 0;
		rv.beat4 = 0;
		rv.mid = 0;
	}		 
	rv.cursor = !self->state->prevent_cursor &&
		(psy_audio_sequencecursor_line(&self->state->pv->cursor) == line) &&
		(self->track == self->state->pv->cursor.track);
	rv.selection = psy_audio_blockselection_test_line(&self->state->pv->selection,
		self->track, line);	
	rv.playbar = self->state->draw_playbar && self->workspace->host_sequencer_time.currplaying &&
		(self->workspace->host_sequencer_time.currplayline == (line +
			((self->state->pv->singlemode)
				? patternviewstate_beattoline(self->state->pv, self->state->pv->cursor.seqoffset)
				: 0)));	
	rv.focus = TRUE;
	return rv;
}

void trackercolumn_drawentry(TrackerColumn* self, psy_ui_Graphics* g,
	psy_audio_PatternEntry* entry, double y, TrackerColumnFlags columnflags,
	TrackDef* trackdef)
{
	uintptr_t noteindex;	
	psy_ui_RealPoint cp;		
	psy_List* curr;
	char* emptystr;
	psy_audio_PatternEvent emptyevent;	

	if (!entry) {
		entry = &self->state->empty;
	}		
	cp = psy_ui_realpoint_make(0.0, y);
	emptystr = (self->state->showemptydata) ? "." : "";
	curr = entry->events;	
	psy_audio_patternevent_init(&emptyevent);
	for (noteindex = 0; noteindex < trackdef_visinotes(trackdef); ++noteindex) {
		psy_audio_PatternEvent* ev;
		TrackerColumnFlags currcolumnflags;
		uintptr_t column;		
		const char* notestr;		
		psy_ui_RealPoint cp_leftedge;
	
		if (curr) {
			ev = (psy_audio_PatternEvent*)curr->entry;
			curr = curr->next;
		} else {
			ev = &emptyevent;
		}		
		cp_leftedge = cp;
		cp_leftedge.x += self->state->trackconfig->textleftedge;
		/* draw note */
		if (!self->state->trackconfig->multicolumn || noteindex == 0) {
			psy_ui_Colour bg;
			psy_ui_Colour fore;

			column = 0;
			currcolumnflags = columnflags;
			currcolumnflags.cursor =
				columnflags.cursor &&
				(self->state->pv->cursor.column == 0) &&
				(self->state->pv->cursor.noteindex == noteindex);
			trackerstate_columncolours(self->state, currcolumnflags,
				entry->track, &bg, &fore);
			psy_ui_setbackgroundcolour(g, bg);
			psy_ui_settextcolour(g, fore);			
			notestr = notetostr(*ev, psy_dsp_NOTESTAB_A440, FALSE);
			psy_ui_textoutrectangle(g,
				cp_leftedge,
				psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
				psy_ui_realrectangle_make(cp, psy_ui_realsize_make(
					self->digitsize.width * 3.0, self->digitsize.height)),
				notestr, psy_strlen(notestr));
			cp.x += trackdef_columnwidth(trackdef, 0, self->digitsize.width);
		}
		/* draw digit columns */
		if (self->state->trackconfig->multicolumn && noteindex > 0) {
			column = PATTERNEVENT_COLUMN_CMD;
		} else {
			column = 1;
		}
		for (; column < trackdef_numcolumns(trackdef); ++column) {
			TrackColumnDef* coldef;
			uintptr_t digit;
			uintptr_t value;
			bool empty;
			uintptr_t num;
			const char* digitstr = NULL;

			coldef = trackdef_columndef(trackdef, column);
			if (!coldef) {
				continue;
			}			
			value = trackdef_event_value(trackdef, column, ev);			
			empty = coldef->emptyvalue == value;
			if (empty) {
				digitstr = emptystr;
			}
			if (column > PATTERNEVENT_COLUMN_VOL &&
					psy_audio_patternevent_tweakvalue(ev) != 0) {
				empty = FALSE;				
			}
			currcolumnflags.cursor =
				columnflags.cursor &&
				(column == self->state->pv->cursor.column) &&
				(self->state->pv->cursor.noteindex == noteindex);
			for (num = coldef->numdigits, digit = 0; digit < num;
					++digit, cp.x += self->digitsize.width) {
				psy_ui_Colour bg;
				psy_ui_Colour fore;

				currcolumnflags = columnflags;
				currcolumnflags.cursor = columnflags.cursor &&
					(self->state->pv->cursor.column == column) &&
					(self->state->pv->cursor.digit == digit) &&
					(self->state->pv->cursor.noteindex == noteindex);
				trackerstate_columncolours(self->state, currcolumnflags,
					entry->track, &bg, &fore);
				psy_ui_setbackgroundcolour(g, bg);
				psy_ui_settextcolour(g, fore);				
				if (!empty) {
					digitstr = hex_tab[((value >> ((num - digit - 1) * 4)) & 0x0F)];
				}
				trackercolumn_drawdigit(self, g, cp, digitstr);
			}
			cp.x += coldef->marginright;
		}
	}
}

void trackercolumn_drawdigit(TrackerColumn* self, psy_ui_Graphics* g,
	psy_ui_RealPoint cp, const char* str)
{
	psy_ui_RealPoint cp_leftedge;

	cp_leftedge = cp;
	cp_leftedge.x += self->state->trackconfig->textleftedge;
	psy_ui_textoutrectangle(g, cp_leftedge,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, psy_ui_realrectangle_make(
			cp, self->digitsize), str, psy_strlen(str));
}

void trackercolumn_drawresizebar(TrackerColumn* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	
	size = psy_ui_component_size_px(&self->component);
	if (self->state->trackconfig->multicolumn) {
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(size.width - 3, 0),
				psy_ui_realsize_make(3.0, size.height)),
			psy_ui_colour_white());
	} else {
		double notewidth;
		TrackDef* trackdef;

		trackdef = trackerconfig_trackdef(self->state->trackconfig, self->track);
		notewidth = trackdef_columnwidth(trackdef, 0,
			self->state->trackconfig->textwidth);
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(notewidth - 3, 0),
				psy_ui_realsize_make(3.0, size.height)),
			psy_ui_colour_white());
	}
}

void trackercolumn_onmousedown(TrackerColumn* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;
	double notewidth;
	TrackDef* trackdef;

	if (psy_ui_mouseevent_button(ev) != 1) {
		return;
	}	
	trackdef = trackerconfig_trackdef(self->state->trackconfig, self->track);
	notewidth = trackdef_columnwidth(trackdef, 0,
		self->state->trackconfig->textwidth);
	size = psy_ui_component_size_px(&self->component);
	if (psy_ui_mouseevent_pt(ev).x > notewidth - 5 && psy_ui_mouseevent_pt(ev).x < notewidth) {
		self->state->trackconfig->multicolumn = FALSE;
		trackdrag_start(&self->state->trackconfig->resize, self->track,
			size.width);
	} else if (psy_ui_mouseevent_pt(ev).x > size.width - 5) {
		self->state->trackconfig->multicolumn = TRUE;
		trackdrag_start(&self->state->trackconfig->resize, self->track,
			size.width);		
	} else {
		if (psy_audio_blockselection_valid(&self->state->pv->selection)) {
			psy_audio_blockselection_disable(&self->state->pv->selection);
			psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
		}
		self->state->pv->dragselectionbase = trackerstate_make_cursor(
			self->state, psy_ui_mouseevent_pt(ev), self->track);
	}
	if (trackdrag_active(&self->state->trackconfig->resize)) {		
		psy_ui_component_capture(&self->component);
		trackconfig_resize(self->state->trackconfig, self->track, size.width);
	}	
}

void trackercolumn_onmousemove(TrackerColumn* self, psy_ui_MouseEvent* ev)
{			
	trackconfig_resize(self->state->trackconfig, self->track, psy_ui_mouseevent_pt(ev).x);
	trackercolumn_updatecursor(self, psy_ui_mouseevent_pt(ev).x);
}

void trackercolumn_updatecursor(TrackerColumn* self, double position)
{	
	if (trackercolumn_isovernote(self, position) || trackercolumn_isovercolumn(
			self, position)) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}

bool trackercolumn_isovernote(const TrackerColumn* self, double position)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return (position > size.width - 5);
}

bool trackercolumn_isovercolumn(const TrackerColumn* self, double position)
{
	return (position >= self->state->trackconfig->textwidth * 3 - 5 &&
		position < self->state->trackconfig->textwidth * 3);
}

void trackercolumn_onmouseup(TrackerColumn* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);	
	trackconfig_resize(self->state->trackconfig, self->track,
		psy_ui_mouseevent_pt(ev).x);
}

void trackercolumn_onpreferredsize(TrackerColumn* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	psy_ui_Component* parent;

	parent = psy_ui_component_parent(&self->component);
	psy_ui_size_setpx(rv, trackerstate_trackwidth(self->state, self->track),
		patternviewstate_numlines(self->state->pv) *
		self->state->lineheightpx);
}
