// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackercolumn.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void setcmdall(psy_Property*, int cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext);
static void setcmd(psy_Property*, int cmd, uint32_t keycode,
	const char* key, const char* shorttext);
static void setcolumncolour(PatternViewSkin*, psy_ui_Graphics*,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks);
static const char* notetostr(psy_audio_PatternEvent ev, psy_dsp_NotesTabMode notestabmode,
	bool showemptydate)
{
	static const char* emptynotestr = "- - -";

	if (ev.note != psy_audio_NOTECOMMANDS_EMPTY || !showemptydate) {
		return psy_dsp_notetostr(ev.note, notestabmode);
	}
	return emptynotestr;
}

static int keycodetoint(uint32_t keycode)
{
	if (keycode >= '0' && keycode <= '9') {
		return keycode - '0';
	} else if (keycode >= 'A' && keycode <= 'Z') {
		return keycode - 'A' + 10;
	}
	return -1;
}

static void enterdigit(int digit, int newval, unsigned char* val)
{
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

static void lohi(uint8_t* value, int digit, uint8_t* lo, uint8_t* hi)
{
	*lo = *value & 0x0F;
	*hi = (*value & 0xF0) >> 4;
}

static void digitlohi(int value, int digit, uintptr_t size, uint8_t* lo, uint8_t* hi)
{
	uintptr_t pos;

	pos = (size - 1) - digit / 2;
	lohi((uint8_t*)&value + pos, digit, lo, hi);
}

static void entervaluecolumn(psy_audio_PatternEntry* entry, intptr_t column, intptr_t value)
{
	psy_audio_PatternEvent* ev;

	assert(entry);

	ev = psy_audio_patternentry_front(entry);
	switch (column) {
		case TRACKER_COLUMN_INST:
			ev->inst = (uint16_t)value;
			break;
		case TRACKER_COLUMN_MACH:
			ev->mach = (uint8_t)value;
			break;
		case TRACKER_COLUMN_VOL:
			ev->vol = (uint16_t)value;
			break;
		case TRACKER_COLUMN_CMD:
			ev->cmd = (uint8_t)value;
			break;
		case TRACKER_COLUMN_PARAM:
			ev->parameter = (uint8_t)value;
			break;
		default:
			break;
	}
}

// TrackerColumn
// prototypes
static void trackercolumn_ondraw(TrackerColumn*, psy_ui_Graphics*);
static void trackercolumn_drawtrackevents(TrackerColumn*, psy_ui_Graphics*);
static  TrackerColumnFlags trackercolumn_columnflags(TrackerColumn*,
	uintptr_t line, uintptr_t track, psy_dsp_big_beat_t offset);
static void trackercolumn_drawentry(TrackerColumn*,
	psy_ui_Graphics* g, psy_audio_PatternEntry* entry, double y,
	TrackerColumnFlags columnflags);
static void trackercolumn_drawdigit(TrackerColumn*, psy_ui_Graphics*,
	psy_ui_RealPoint cp, const char* str);
static void trackercolumn_drawresizebar(TrackerColumn*, psy_ui_Graphics*);
static void trackercolumn_onmousedown(TrackerColumn*,
	psy_ui_MouseEvent*);
static void trackercolumn_onmousemove(TrackerColumn*,
	psy_ui_MouseEvent*);
static void trackercolumn_onmouseup(TrackerColumn*,
	psy_ui_MouseEvent*);
static psy_audio_SequenceCursor trackercolumn_makecursor(TrackerColumn*,
	psy_ui_RealPoint);
static void trackercolumn_onpreferredsize(TrackerColumn*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

// vtable
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

void trackercolumn_init(TrackerColumn* self, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerState* gridstate,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	trackercolumn_vtable_init(self);
	psy_ui_component_setalign(trackercolumn_base(self), psy_ui_ALIGN_LEFT);
	self->gridstate = gridstate;	
	self->workspace = workspace;	
	self->index = index;
	self->trackdef = NULL;	
}

TrackerColumn* trackercolumn_alloc(void)
{
	return (TrackerColumn*)malloc(sizeof(TrackerColumn));
}

TrackerColumn* trackercolumn_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerState* gridstate,
	Workspace* workspace)
{
	TrackerColumn* rv;

	rv = trackercolumn_alloc();
	if (rv) {
		trackercolumn_init(rv, parent, view, index, gridstate,
			workspace);
		psy_ui_component_deallocateafterdestroyed(trackercolumn_base(rv));
	}
	return rv;
}

void trackercolumn_ondraw(TrackerColumn* self, psy_ui_Graphics* g)
{	
	trackercolumn_drawtrackevents(self, g);
	if (self->gridstate->trackconfig->colresize &&
			self->gridstate->trackconfig->resizetrack == self->index) {
		trackercolumn_drawresizebar(self, g);		
	}
}

void trackercolumn_drawtrackevents(TrackerColumn* self, psy_ui_Graphics* g)
{	
	psy_List** events;
	psy_List* p;
	double cpy;
	double offset;
	uintptr_t line;	
	
	events = trackereventtable_track(&self->gridstate->trackevents,
		self->index);
	self->trackdef = trackerstate_trackdef(self->gridstate, self->index);
	self->digitsize = psy_ui_realsize_make(
		self->gridstate->trackconfig->textwidth,
		self->gridstate->lineheightpx - 1);
	for (p = *events,
			offset = self->gridstate->trackevents.clip.topleft.offset,
			cpy = trackerstate_beattopx(self->gridstate, offset),
			line = (uintptr_t)(self->gridstate->trackevents.clip.topleft.offset *
				self->gridstate->trackevents.clip.topleft.lpb);
			p != NULL;
			p = p->next, ++line, cpy += self->gridstate->lineheightpx,
			offset += trackerstate_bpl(self->gridstate)) {				
		trackercolumn_drawentry(self, g, (psy_audio_PatternEntry*)p->entry,
			cpy, trackercolumn_columnflags(self,
				line - trackerstate_beattoline(self->gridstate,
					self->gridstate->trackevents.seqoffset),
				self->index, offset));

	}
	self->trackdef = NULL;
}

TrackerColumnFlags trackercolumn_columnflags(TrackerColumn* self,
	uintptr_t line, uintptr_t track, psy_dsp_big_beat_t offset)
{
	TrackerColumnFlags rv;	

	assert(self);
	
	if (self->gridstate->drawbeathighlights) {
		uintptr_t lpb;

		lpb = trackerstate_lpb(self->gridstate);
		rv.beat = (line % lpb) == 0;
		rv.beat4 = (line % (lpb * 4)) == 0;		
		rv.mid = self->gridstate->midline &&
			(line == trackerstate_midline(self->gridstate,
			psy_ui_component_scrolltop_px(psy_ui_component_parent(
				&self->component))));
	} else {
		rv.beat = 0;
		rv.beat4 = 0;
		rv.mid = 0;
	}
	rv.cursor = (self->gridstate->trackevents.currcursorline == line) &&
		(track == self->gridstate->cursor.cursor.track) &&
		self->gridstate->drawcursor;
	rv.selection = psy_audio_patternselection_test(
		&self->gridstate->selection, track, offset);
	rv.playbar = trackerstate_hasplaybar(self->gridstate) &&
		psy_audio_player_playing(workspace_player(self->workspace)) &&
		(self->gridstate->trackevents.currplaybarline == line);
	rv.focus = TRUE;
	return rv;
}

void trackercolumn_drawentry(TrackerColumn* self, psy_ui_Graphics* g,
	psy_audio_PatternEntry* entry, double y, TrackerColumnFlags columnflags)
{
	uintptr_t i;
	psy_ui_RealPoint cp;	

	cp = psy_ui_realpoint_make(0.0, y);
	for (i = 0; i < self->trackdef->numnotes; ++i) {
		psy_audio_PatternEvent* event;
		TrackerColumnFlags currcolumnflags;
		uintptr_t column;
		const char* notestr;		
		psy_ui_RealPoint cp_leftedge;

		if (!entry) {
			entry = &self->gridstate->empty;
		}
		event = psy_audio_patternentry_front(entry);
		currcolumnflags = columnflags;
		currcolumnflags.cursor &= self->gridstate->cursor.cursor.column == 0;
		setcolumncolour(self->gridstate->skin, g, currcolumnflags, entry->track,
			trackerstate_numsongtracks(self->gridstate));
		cp_leftedge = cp;
		cp_leftedge.x += self->gridstate->trackconfig->textleftedge;
		// draw note
		notestr = notetostr(*event, psy_dsp_NOTESTAB_A440, FALSE);
		psy_ui_textoutrectangle(g,
			cp_leftedge, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
			psy_ui_realrectangle_make(cp, psy_ui_realsize_make(
				self->digitsize.width * 3.0, self->digitsize.height)),
			notestr, psy_strlen(notestr));
		cp.x += trackdef_columnwidth(self->trackdef, 0, self->digitsize.width);
		// draw digit columns
		for (column = 1; column < trackdef_numcolumns(self->trackdef); ++column) {
			TrackColumnDef* coldef;
			uintptr_t digit;
			uintptr_t value;
			bool empty;
			uintptr_t num;
			const char* digitstr = NULL;

			coldef = trackdef_columndef(self->trackdef, column);
			if (!coldef) {
				continue;
			}
			value = trackdef_value(self->trackdef, column, entry);
			empty = coldef->emptyvalue == value;
			if (empty) {
				digitstr = (self->gridstate->showemptydata) ? "." : "";
			}
			if (column > TRACKER_COLUMN_VOL) {
				intptr_t cmd;
				psy_List* ev;

				cmd = (column - (int)TRACKER_COLUMN_VOL - 1) / 2;
				ev = psy_list_at(entry->events, cmd);
				if (ev && psy_audio_patternevent_tweakvalue(
					(psy_audio_PatternEvent*)(ev->entry)) != 0) {
					empty = 0;
				}
			}
			currcolumnflags.cursor = columnflags.cursor && (column ==
				self->gridstate->cursor.cursor.column);
			for (num = coldef->numdigits, digit = 0; digit < num; ++digit) {
				if (columnflags.cursor) {
					currcolumnflags.cursor = columnflags.cursor &&
						(self->gridstate->cursor.cursor.column == column) &&
						(self->gridstate->cursor.cursor.digit == digit);
					setcolumncolour(self->gridstate->skin, g, currcolumnflags,
						entry->track, trackerstate_numsongtracks(self->gridstate));
				}
				if (!empty) {
					digitstr = hex_tab[((value >> ((num - digit - 1) * 4)) & 0x0F)];
				}
				trackercolumn_drawdigit(self, g, cp, digitstr);
				cp.x += self->digitsize.width;
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
	cp_leftedge.x += self->gridstate->trackconfig->textleftedge;
	psy_ui_textoutrectangle(g,
		cp_leftedge, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		psy_ui_realrectangle_make(cp, self->digitsize),
		str, psy_strlen(str));
}

void trackercolumn_drawresizebar(TrackerColumn* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	
	size = psy_ui_component_size_px(&self->component);
	if (self->gridstate->trackconfig->noteresize) {
		double notewidth;

		self->trackdef = trackerstate_trackdef(self->gridstate, self->index);
		notewidth = trackdef_columnwidth(self->trackdef, 0,
			self->gridstate->trackconfig->textwidth);
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(notewidth - 3, 0),
				psy_ui_realsize_make(3.0, size.height)),
			psy_ui_colour_white());
	} else {
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(size.width - 3, 0),
				psy_ui_realsize_make(3.0, size.height)),
			psy_ui_colour_white());
	}
}

void trackercolumn_onmousedown(TrackerColumn* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealSize size;
		double notewidth;

		self->trackdef = trackerstate_trackdef(self->gridstate, self->index);
		notewidth = trackdef_columnwidth(self->trackdef, 0,
			self->gridstate->trackconfig->textwidth);
		size = psy_ui_component_size_px(&self->component);
		if (ev->pt.x > notewidth - 5 && ev->pt.x < notewidth) {
			self->gridstate->trackconfig->resizetrack = self->index;
			self->gridstate->trackconfig->colresize = TRUE;
			self->gridstate->trackconfig->noteresize = TRUE;
			self->resizestartsize = size;
			self->gridstate->trackconfig->resizesize = size;
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_capture(&self->component);
		} else if (ev->pt.x > size.width - 5) {
			self->gridstate->trackconfig->resizetrack = self->index;
			self->gridstate->trackconfig->colresize = TRUE;
			self->gridstate->trackconfig->noteresize = FALSE;
			self->resizestartsize = size;
			self->gridstate->trackconfig->resizesize = size;
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_capture(&self->component);
		} else {
			if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
				psy_audio_patternselection_disable(&self->gridstate->selection);
				psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
			}
			self->gridstate->dragselectionbase = trackercolumn_makecursor(
				self, ev->pt);
		}
	}
}

void trackercolumn_onmousemove(TrackerColumn* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	if (self->gridstate->trackconfig->colresize) {
		double basewidth;

		self->trackdef = trackerstate_trackdef(self->gridstate,
			self->index);
		basewidth = trackdef_basewidth(self->trackdef,
			self->gridstate->trackconfig->textwidth);
		if (ev->pt.x > basewidth) {
			self->gridstate->trackconfig->resizesize.width = ev->pt.x;			
		}
	} else {
		self->gridstate->dragcursor =
			trackerstate_checkcursorbounds(self->gridstate,
			trackercolumn_makecursor(self, ev->pt));
	}	
	if (self->gridstate->showresizecursor &&
			(ev->pt.x > size.width - 5)) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}

void trackercolumn_onmouseup(TrackerColumn* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if (self->gridstate->trackconfig->colresize &&
			self->gridstate->trackconfig->resizesize.width > 0.0) {
		double basewidth;
		

		self->trackdef = trackerstate_trackdef(self->gridstate, self->index);
		basewidth = trackdef_basewidth(self->trackdef, self->gridstate->trackconfig->textwidth);
		if (self->gridstate->trackconfig->noteresize) {
			uintptr_t numnotes;

			numnotes = (uintptr_t)psy_max(1.0,
				(psy_max(0.0, self->gridstate->trackconfig->resizesize.width)) /
				trackerstate_defaulttrackwidth(self->gridstate));
			if (self->trackdef != &self->gridstate->trackconfig->trackdef) {
				self->trackdef->numfx = 1;
				self->trackdef->numnotes = numnotes;
			} else if (numnotes > 1) {
				self->trackdef = (TrackDef*)malloc(sizeof(TrackDef));
				if (self->trackdef) {
					trackdef_init(self->trackdef);
					self->trackdef->numfx = 1;
					self->trackdef->numnotes = numnotes;
					psy_table_insert(&self->gridstate->trackconfig->trackconfigs, self->index,
						self->trackdef);
				}
			}

		} else {
			uintptr_t numfx;

			numfx = (uintptr_t)psy_max(1.0,
				(psy_max(0.0, self->gridstate->trackconfig->resizesize.width - basewidth)) /
				(self->gridstate->trackconfig->textwidth * 4.0));
			if (self->trackdef != &self->gridstate->trackconfig->trackdef) {
				self->trackdef->numfx = numfx;
				self->trackdef->numnotes = 1;
			} else if (numfx > 1) {
				self->trackdef = (TrackDef*)malloc(sizeof(TrackDef));
				if (self->trackdef) {
					trackdef_init(self->trackdef);
					self->trackdef->numfx = numfx;
					self->trackdef->numnotes = 1;
					psy_table_insert(&self->gridstate->trackconfig->trackconfigs, self->index,
						self->trackdef);
				}
			}
		}
	}	
	self->gridstate->trackconfig->resizetrack = psy_INDEX_INVALID;	
}

psy_audio_SequenceCursor trackercolumn_makecursor(TrackerColumn* self,
	psy_ui_RealPoint pt)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;	
	double cpx;	
	psy_audio_Sequence* sequence;
	
	rv = self->gridstate->cursor;
	sequence = trackerstate_sequence(self->gridstate);
	if (sequence) {
		psy_audio_sequencecursor_updateseqoffset(&rv,
			sequence);
	}
	rv.cursor.seqoffset = (self->gridstate->singlemode)
		? 0.0
		: rv.seqoffset;	
	rv.cursor.offset = trackerstate_pxtobeat(self->gridstate, pt.y) - rv.cursor.seqoffset;	
	rv.cursor.lpb = trackerstate_lpb(self->gridstate);
	if (trackerstate_pattern(self->gridstate) &&
			rv.cursor.offset >= psy_audio_pattern_length(trackerstate_pattern(self->gridstate))) {
		if (self->gridstate->singlemode) {
			rv.cursor.offset = psy_audio_pattern_length(trackerstate_pattern(self->gridstate)) -
				trackerstate_bpl(self->gridstate);
		}
	}
	rv.cursor.track = self->index;
	rv.cursor.column = 0;
	rv.cursor.digit = 0;
	rv.cursor.key = self->workspace->song->sequence.cursor.cursor.key;
	trackdef = trackerstate_trackdef(self->gridstate, rv.cursor.track);
	cpx = 0;	
	while (rv.cursor.column < trackdef_numcolumns(trackdef) &&
			cpx + trackdef_columnwidth(trackdef, rv.cursor.column,
				self->gridstate->trackconfig->textwidth) < pt.x) {
		cpx += trackdef_columnwidth(trackdef, rv.cursor.column,
			self->gridstate->trackconfig->textwidth);
		++rv.cursor.column;
	}
	rv.cursor.digit = (uintptr_t)((pt.x - cpx) /
		self->gridstate->trackconfig->textwidth);
	if (rv.cursor.digit >= trackdef_numdigits(trackdef, rv.cursor.column)) {
		rv.cursor.digit = trackdef_numdigits(trackdef, rv.cursor.column) - 1;
	}
	self->gridstate->cursor.cursor.patternid =
		self->workspace->song->sequence.cursor.cursor.patternid;		
	return rv;
}

void trackercolumn_onpreferredsize(TrackerColumn* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->gridstate->trackconfig->colresize &&
			self->gridstate->trackconfig->resizetrack == self->index) {
		rv->width = psy_ui_value_make_px(self->gridstate->trackconfig->resizesize.width);
	} else {
		rv->width = psy_ui_value_make_px(trackerstate_trackwidth(
			self->gridstate, self->index));		
	}
	if (self->gridstate->maxlines == psy_INDEX_INVALID) {
		rv->height = psy_ui_value_make_px(
			(trackerstate_numlines(self->gridstate)) *
			self->gridstate->lineheightpx);
	} else {
		rv->height = psy_ui_value_make_px((self->gridstate->maxlines) *
			self->gridstate->lineheightpx);
	}
}

void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks)
{
	assert(skin);

	if (flags.cursor != 0) {
		psy_ui_setbackgroundcolour(g, skin->cursor);
		psy_ui_settextcolour(g,
			patternviewskin_fontcurcolour(skin, track, numtracks));
	} else if (flags.playbar) {
		psy_ui_setbackgroundcolour(g,
			patternviewskin_playbarcolour(skin, track, numtracks));
		psy_ui_settextcolour(g,
			patternviewskin_fontplaycolour(skin, track, numtracks));
	} else if (flags.selection) {
		if (flags.beat4) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selection4beatcolour(skin, track, numtracks));
		} else if (flags.beat) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selectionbeatcolour(skin, track, numtracks));
		} else {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selectioncolour(skin, track, numtracks));
		}
		psy_ui_settextcolour(g,
			patternviewskin_fontselcolour(skin, track, numtracks));
	} else if (flags.mid) {
		psy_ui_setbackgroundcolour(g,
			patternviewskin_midlinecolour(skin, track, numtracks));
		if (flags.cursor != 0) {
			psy_ui_settextcolour(g,
				patternviewskin_fontcurcolour(skin, track, numtracks));
		} else {
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		}
	} else {
		if (flags.beat4) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_row4beatcolour(skin, track, numtracks));
			psy_ui_settextcolour(g, skin->font);
		} else if (flags.beat) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_rowbeatcolour(skin, track, numtracks));
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		} else {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_rowcolour(skin, track, numtracks));
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		}
	}
}
