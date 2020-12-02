// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackerview.h"
// local
#include "patternview.h"
#include "patterncmds.h"
#include "cmdsnotes.h"
#include "skingraphics.h"
// audio
#include <pattern.h>
#include <patternio.h>
#include "../../driver/eventdriver.h"
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>
// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

static char* hex_tab[16] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "A", "B", "C", "D", "E", "F"
};

static int testcursor(psy_audio_PatternCursor cursor, uintptr_t track,
	psy_dsp_big_beat_t offset, uintptr_t lpb)
{
	return cursor.track == track && psy_dsp_testrange(cursor.offset, offset, 1.0 / lpb);
}

static void definecmd(psy_Property*, int cmd, uintptr_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext);

static uintptr_t trackergrid_columnvalue(psy_audio_PatternEvent*, int column);

static int keycodetoint(unsigned int keycode)
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

static void entervaluecolumn(psy_audio_PatternEntry* entry, int column, int value)
{
	psy_audio_PatternEvent* ev;

	ev = psy_audio_patternentry_front(entry);
	switch (column) {
		case TRACKER_COLUMN_INST:
			ev->inst = value;
			break;
		case TRACKER_COLUMN_MACH:
			ev->mach = value;
			break;
		case TRACKER_COLUMN_VOL:
			ev->vol = value;
			break;
		case TRACKER_COLUMN_CMD:
			ev->cmd = value;
			break;
		case TRACKER_COLUMN_PARAM:
			ev->parameter = value;
			break;
		default:
			break;
	}
}

// TrackerGrid
// prototypes
static void trackergrid_ondestroy(TrackerGrid*, psy_ui_Component* sender);
static int trackergrid_preferredtrackwidth(TrackerGrid*);
static void trackergrid_ondraw(TrackerGrid*, psy_ui_Graphics*);
static void trackergrid_drawbackground(TrackerGrid*, psy_ui_Graphics*,
	PatternSelection* clip);
static void trackergrid_drawtrackbackground(TrackerGrid*, psy_ui_Graphics*,
	int track);
static void trackergrid_drawentries(TrackerGrid*, psy_ui_Graphics*,
	PatternSelection* clip);
static void trackergrid_drawentry(TrackerGrid*, psy_ui_Graphics*,
	psy_audio_PatternEntry*, int x, int y, TrackerColumnFlags);
static void trackergrid_drawresizebar(TrackerGrid*, psy_ui_Graphics*,
	PatternSelection*);
static void trackergrid_onkeydown(TrackerGrid*, psy_ui_KeyEvent*);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousewheel(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static psy_audio_PatternCursor trackergrid_makecursor(TrackerGrid*, int x, int y);
static uintptr_t trackergrid_resizecolumn(TrackerGrid*, int x, int y);
static void trackergrid_dragcolumn(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_startdragselection(TrackerGrid*, psy_audio_PatternCursor);
static void trackergrid_dragselection(TrackerGrid*, psy_audio_PatternCursor);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_onfocus(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_onfocuslost(TrackerGrid*, psy_ui_Component* sender);
static int trackergrid_testselection(TrackerGrid*, unsigned int track, double offset);
static void trackergrid_clipblock(TrackerGrid*, const psy_ui_Rectangle*,
	PatternSelection*);
static void trackergrid_drawdigit(TrackerGrid*, psy_ui_Graphics*,
	int x, int y, int value, int empty, int mid);
static void trackergrid_onpreferredsize(TrackerGrid*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void trackergrid_inputevent(TrackerGrid*, const psy_audio_PatternEvent*,
	bool chordmode);
static void trackergrid_enterdigitcolumn(TrackerGrid*, psy_audio_PatternEntry* entry,
	int track, int column, int digit, int digitvalue);

static void trackergrid_invalidatecursor(TrackerGrid*);
static void trackergrid_oninterpolatelinear(TrackerGrid*);
static void trackergrid_onchangegenerator(TrackerGrid*);
static void trackergrid_onchangeinstrument(TrackerGrid*);
static void trackergrid_blockstart(TrackerGrid*);
static void trackergrid_blockend(TrackerGrid*);
static void trackergrid_blockunmark(TrackerGrid*);
static void trackergrid_onblockcut(TrackerGrid*);
static void trackergrid_onblockcopy(TrackerGrid*);
static void trackergrid_onblockpaste(TrackerGrid*);
static void trackergrid_onblockmixpaste(TrackerGrid*);
static void trackergrid_onblockdelete(TrackerGrid*);
static void trackergrid_onblocktransposeup(TrackerGrid*);
static void trackergrid_onblocktransposedown(TrackerGrid*);
static void trackergrid_onblocktransposeup12(TrackerGrid*);
static void trackergrid_onblocktransposedown12(TrackerGrid*);
static void trackergrid_inputvalue(TrackerGrid*, int value, int digit);
static void trackergrid_prevtrack(TrackerGrid*);
static void trackergrid_nexttrack(TrackerGrid*);
static void trackergrid_prevline(TrackerGrid*);
static void trackergrid_advanceline(TrackerGrid*);
static void trackergrid_prevlines(TrackerGrid*, uintptr_t lines, int wrap);
static void trackergrid_advancelines(TrackerGrid*, uintptr_t lines, int wrap);
static void trackergrid_home(TrackerGrid*);
static void trackergrid_end(TrackerGrid*);
static void trackergrid_rowdelete(TrackerGrid*);
static void trackergrid_rowclear(TrackerGrid*);
static void trackergrid_prevcol(TrackerGrid*);
static void trackergrid_nextcol(TrackerGrid*);
static void trackergrid_selectall(TrackerGrid*);
static void trackergrid_selectcol(TrackerGrid*);
static void trackergrid_selectmachine(TrackerGrid*);
static void trackergrid_oninput(TrackerGrid*, psy_audio_Player*,
	psy_audio_PatternEvent*);
static void trackergrid_setdefaultevent(TrackerGrid*,
	psy_audio_Pattern* defaultpattern, psy_audio_PatternEvent*);
// vtable
static psy_ui_ComponentVtable trackergrid_vtable;
static int trackergrid_vtable_initialized = 0;

static void trackergrid_vtable_init(TrackerGrid* self)
{
	if (!trackergrid_vtable_initialized) {
		trackergrid_vtable = *(self->component.vtable);
		trackergrid_vtable.ondraw = (psy_ui_fp_component_ondraw)trackergrid_ondraw;
		trackergrid_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			trackergrid_onkeydown;
		trackergrid_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			trackergrid_onmousedown;
		trackergrid_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			trackergrid_onmousemove;
		trackergrid_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			trackergrid_onmouseup;
		trackergrid_vtable.onmousedoubleclick = (psy_ui_fp_component_onmousedoubleclick)
			trackergrid_onmousedoubleclick;
		trackergrid_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackergrid_onpreferredsize;
	}
}
// implementation
void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerGridState* gridstate, TrackerLineState* linestate,
	TrackerGridEditMode editmode, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackergrid_vtable_init(self);
	self->component.vtable = &trackergrid_vtable;
	trackergrid_setsharedgridstate(self, gridstate, trackconfig);
	trackergrid_setsharedlinestate(self, linestate);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = TRUE;
	self->hasselection = FALSE;	
	self->midline = FALSE;	
	self->columnresize = 0;
	self->dragcolumn = UINTPTR_MAX;
	self->dragcolumnbase = 0;
	self->chordmode = FALSE;
	self->chordbegin = 0;
	self->wraparound = TRUE;
	self->showemptydata = FALSE;
	self->editmode = editmode;
	self->movecursoronestep = FALSE;
	self->ft2home = TRUE;
	self->ft2delete = TRUE;
	self->effcursoralwaysdown = FALSE;
	if (editmode == TRACKERGRID_EDITMODE_SONG) {
		psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	}	
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);
	psy_signal_connect(&self->component.signal_focus, self,
		trackergrid_onfocus);
	psy_signal_connect(&self->component.signal_focuslost, self,
		trackergrid_onfocuslost);
	psy_signal_connect(&self->component.signal_mousewheel, self,
		trackergrid_onmousewheel);
	self->gridstate->numtracks = psy_audio_player_numsongtracks(&self->workspace->player);
	self->lpb = psy_audio_player_lpb(&self->workspace->player);
	self->bpl = 1 / (psy_dsp_big_beat_t)psy_audio_player_lpb(&self->workspace->player);
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;
	psy_audio_patterncursor_init(&self->gridstate->cursor);	
	workspace_setpatterncursor(workspace, self->gridstate->cursor);
	psy_signal_connect(&self->workspace->player.signal_inputevent, self,
		trackergrid_oninput);
	psy_signal_init(&self->signal_colresize);
	psy_signal_connect(&self->component.signal_destroy, self,
		trackergrid_ondestroy);
}

void trackergrid_ondestroy(TrackerGrid* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_colresize);
}

void trackergrid_setsharedgridstate(TrackerGrid* self, TrackerGridState*
	gridstate, TrackConfig* trackconfig)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		trackergridstate_init(&self->defaultgridstate, trackconfig);
		self->gridstate = &self->defaultgridstate;
	}
}

void trackergrid_setsharedlinestate(TrackerGrid* self, TrackerLineState*
	linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
}

void trackergrid_ondraw(TrackerGrid* self, psy_ui_Graphics* g)
{
	PatternSelection clip;

	if (self->gridstate->pattern) {
		trackergrid_clipblock(self, &g->clip, &clip);
		trackergrid_drawbackground(self, g, &clip);
		trackergrid_drawentries(self, g, &clip);
		trackergrid_drawresizebar(self, g, &clip);
	} else {
		psy_ui_drawsolidrectangle(g, g->clip,
			self->gridstate->skin->background);
	}
}

void trackergrid_clipblock(TrackerGrid* self, const psy_ui_Rectangle* clip,
	PatternSelection* block)
{
	int lines;

	block->topleft.track = trackergridstate_screentotrack(self->gridstate, clip->left,
		psy_audio_player_numsongtracks(&self->workspace->player));
	block->topleft.column = 0;
	block->topleft.digit = 0;
	block->topleft.offset = trackerlinestate_offset(self->linestate, clip->top,
		&lines);
	block->topleft.line = lines;
	block->bottomright.track = trackergridstate_screentotrack(self->gridstate,
		clip->right, psy_audio_player_numsongtracks(&self->workspace->player)) + 1;
	if (block->bottomright.track > self->gridstate->numtracks) {
		block->bottomright.track = self->gridstate->numtracks;
	}
	block->bottomright.column = 0;
	block->bottomright.digit = 0;
	block->bottomright.offset = trackerlinestate_offset(self->linestate,
		clip->bottom, &lines);
	block->bottomright.line = lines;
}

void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g, PatternSelection* clip)
{
	uintptr_t track;
	psy_ui_IntSize size;	
	int blankcpx;

	size = psy_ui_component_intsize(&self->component);
	for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
		int trackwidth;
		int cpx;

		cpx = trackergridstate_track_x(self->gridstate, track);
		trackwidth = trackergridstate_trackwidth(self->gridstate, track);
		psy_ui_drawsolidrectangle(g,
			psy_ui_rectangle_make(cpx, psy_ui_component_scrolltop(&self->component),
				trackwidth,
				size.height),
			patternviewskin_separatorcolour(self->gridstate->skin, track, self->gridstate->numtracks));
	}
	if (self->gridstate->numtracks > 0) {
		blankcpx = trackergridstate_track_x(self->gridstate, self->gridstate->numtracks - 1) +
			trackergridstate_trackwidth(self->gridstate, self->gridstate->numtracks - 1);
		if (blankcpx - psy_ui_component_scrollleft(&self->component) < size.width) {
			psy_ui_drawsolidrectangle(g,
				psy_ui_rectangle_make(blankcpx,
					psy_ui_component_scrolltop(&self->component),
					size.width - (blankcpx - psy_ui_component_scrollleft(&self->component)),
					size.height),
				patternviewskin_separatorcolour(self->gridstate->skin, 1, 2));
		}
	}
}

void trackergrid_drawtrackbackground(TrackerGrid* self, psy_ui_Graphics* g, int track)
{	
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
			trackergridstate_track_x(self->gridstate, track),
			0, trackergridstate_trackwidth(self->gridstate, track),
			psy_ui_value_px(&size.height, &tm)),
		self->gridstate->skin->background);
}

int trackergrid_testselection(TrackerGrid* self, unsigned int track, double offset)
{
	return self->hasselection &&
		track >= self->selection.topleft.track &&
		track < self->selection.bottomright.track&&
		offset >= self->selection.topleft.offset &&
		offset < self->selection.bottomright.offset;
}

void trackergrid_drawentries(TrackerGrid* self, psy_ui_Graphics* g, PatternSelection* clip)
{
	unsigned int track;
	int cpx = 0;
	int cpy;
	double offset;
	TrackerColumnFlags columnflags;
	psy_audio_PatternNode* node;
	psy_ui_Size size;
	int halfy;
	int line;
	int lpb;
	psy_audio_PatternEntry empty;

	psy_audio_patternentry_init(&empty);
	size = psy_ui_component_size(&self->component);
	halfy = (self->linestate->visilines / 2) * self->linestate->lineheight +
		psy_ui_component_scrolltop(&self->component);
	cpy = clip->topleft.line * self->linestate->lineheight;
	offset = clip->topleft.offset;
	node = psy_audio_pattern_greaterequal(self->gridstate->pattern, (psy_dsp_big_beat_t)offset);
	line = (int)(offset * 1 / self->bpl + 0.5);
	lpb = (int)(1 / self->bpl + 0.5);
	while (offset <= clip->bottomright.offset && offset < self->gridstate->pattern->length) {
		int fill;

		if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
			columnflags.beat = (line % lpb) == 0;
			columnflags.beat4 = (line % (lpb * 4)) == 0;
			columnflags.mid = self->midline && cpy >= halfy && cpy < halfy + self->linestate->lineheight;
		} else {
			columnflags.beat = 0;
			columnflags.beat4 = 0;
			columnflags.mid = 0;
		}
		fill = 0;
		cpx = trackergridstate_track_x(self->gridstate, clip->topleft.track);
		for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
			int hasevent = 0;

			columnflags.cursor = testcursor(self->gridstate->cursor, track, offset, self->linestate->lpb);
			columnflags.selection = trackergrid_testselection(self, track, offset);
			columnflags.playbar = psy_audio_player_playing(&self->workspace->player) &&
				trackerlinestate_testplaybar(self->linestate, offset);
			while (!fill && node &&
				((psy_audio_PatternEntry*)(node->entry))->track <= track &&
				psy_dsp_testrange_e(((psy_audio_PatternEntry*)(node->entry))->offset,
					offset,
					self->bpl)) {
				psy_audio_PatternEntry* entry;

				entry = (psy_audio_PatternEntry*)(node->entry);
				if (entry->track == track) {
					trackergrid_drawentry(self, g, entry,
						cpx + self->gridstate->trackconfig->patterntrackident,
						cpy,
						columnflags);
					node = node->next;
					hasevent = 1;
					break;
				}
				node = node->next;
			}
			if (!hasevent) {
				empty.track = track;
				trackergrid_drawentry(self, g, &empty,
					cpx + self->gridstate->trackconfig->patterntrackident,
					cpy,
					columnflags);
			} else
				if (node && ((psy_audio_PatternEntry*)(node->entry))->track <= track) {
					fill = 1;
				}
			cpx += trackergridstate_trackwidth(self->gridstate, track);
		}
		// skip remaining events of the line
		while (node && ((psy_audio_PatternEntry*)(node->entry))->offset +
			psy_dsp_epsilon * 2 < offset + self->bpl) {
			node = node->next;
		}
		offset += self->bpl;
		++line;
		cpy += self->linestate->lineheight;
	}
	psy_audio_patternentry_dispose(&empty);
}

void trackergrid_drawresizebar(TrackerGrid* self, psy_ui_Graphics* g, PatternSelection* clip)
{
	if (self->linestate->pattern && self->dragcolumn != UINTPTR_MAX) {
		psy_ui_Rectangle r;

		psy_ui_setrectangle(&r, self->dragcolumnbase, 0, 2,
			(int)(self->linestate->lineheight * psy_audio_pattern_length(self->linestate->pattern) + 0.5));
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00FFFFFF));
	}
}

static void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks)
{
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

uintptr_t trackergrid_columnvalue(psy_audio_PatternEvent* event, int column)
{
	uintptr_t rv;

	switch (column) {
		case TRACKER_COLUMN_NOTE:
			rv = event->note;
			break;
		case TRACKER_COLUMN_INST:
			rv = event->inst;
			break;
		case TRACKER_COLUMN_MACH:
			rv = event->mach;
			break;
		case TRACKER_COLUMN_VOL:
			rv = event->vol;
			break;
		case TRACKER_COLUMN_CMD:
			rv = event->cmd;
			break;
		case TRACKER_COLUMN_PARAM:
			rv = event->parameter;
			break;
		default:
			rv = 0;
			break;
	}
	return rv;
}

void trackergrid_drawdigit(TrackerGrid* self, psy_ui_Graphics* g,
	int x, int y, int value, int empty, int mid)
{
	const char* text;
		
	if (!empty && value < 0x10) {
		text = hex_tab[value];
	} else if (self->showemptydata) {
		text = ".";
	} else {
		text = "";
	}
	psy_ui_textoutrectangle(g,
		x + self->gridstate->trackconfig->textleftedge, y,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		psy_ui_rectangle_make(x, y,
			self->gridstate->trackconfig->textwidth,
			self->linestate->lineheight - 1),
		text, strlen(text));	
}

void trackergrid_onpreferredsize(TrackerGrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	rv->width = psy_ui_value_makepx(trackergridstate_track_x(self->gridstate,
		self->gridstate->numtracks));		
	rv->height = (self->editmode == TRACKERGRID_EDITMODE_SONG)
		? psy_ui_value_makepx(trackerlinestate_numlines(self->linestate) *
			self->component.scrollstepy)
		: psy_ui_value_makepx(self->linestate->lineheight);	
}

void trackergrid_prevtrack(TrackerGrid* self)
{
	int invalidate = 1;

	trackergrid_storecursor(self);
	self->gridstate->cursor.column = 0;
	self->gridstate->cursor.digit = 0;
	if (self->gridstate->cursor.track > 0) {
		--self->gridstate->cursor.track;
		trackergrid_scrollleft(self, self->gridstate->cursor);
	} else if (self->wraparound) {
		self->gridstate->cursor.track =
			psy_audio_player_numsongtracks(&self->workspace->player) - 1;
		invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
	}
	if (invalidate) {
		trackergrid_invalidatecursor(self);		
	}
	workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
}

void trackergrid_storecursor(TrackerGrid* self)
{
	self->oldcursor = self->gridstate->cursor;
}

void trackergrid_nexttrack(TrackerGrid* self)
{
	int invalidate = 1;

	trackergrid_storecursor(self);
	self->gridstate->cursor.column = 0;
	self->gridstate->cursor.digit = 0;
	if (self->gridstate->cursor.track <
		psy_audio_player_numsongtracks(&self->workspace->player) - 1) {
		++self->gridstate->cursor.track;
		invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
	} else if (self->wraparound) {
		self->gridstate->cursor.track = 0;
		invalidate = trackergrid_scrollleft(self, self->gridstate->cursor);
	}
	workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
	if (invalidate) {
		trackergrid_invalidatecursor(self);		
	}
}

int trackergrid_scrollup(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	int line;
	int topline;
	psy_ui_Rectangle r;

	line = trackerlinestate_offsettoscreenline(self->linestate, cursor.offset);
	psy_ui_setrectangle(&r,
		trackergridstate_track_x(self->gridstate, cursor.track),
		self->linestate->lineheight * line,
		trackergridstate_trackwidth(self->gridstate, cursor.track),
		self->linestate->lineheight);
	if (self->midline) {
		psy_ui_Size gridsize;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		gridsize = psy_ui_component_size(&self->component);
		topline = psy_ui_value_px(&gridsize.height, &tm) / self->linestate->lineheight / 2;
	} else {
		topline = 0;
	}
	if (psy_ui_component_scrolltop(&self->component) + topline * self->linestate->lineheight > r.top) {
		int dlines = (psy_ui_component_scrolltop(&self->component) + topline * self->linestate->lineheight - r.top) /
			(self->linestate->lineheight);
		self->linestate->cursorchanging = TRUE;
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_component_scrolltop(&self->component) -
			self->component.scrollstepy * dlines);
		return FALSE;
	}
	return TRUE;
}

int trackergrid_scrolldown(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	int line;
	int visilines;

	visilines = self->linestate->visilines;
	if (self->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerlinestate_offsettoscreenline(self->linestate, cursor.offset);
	if (visilines < line - psy_ui_component_scrolltop(&self->component) /
			self->linestate->lineheight) {
		int dlines;

		dlines = line - psy_ui_component_scrolltop(&self->component) /
			self->linestate->lineheight - visilines;
		self->linestate->cursorchanging = TRUE;
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_component_scrolltop(&self->component) +
			self->component.scrollstepy * dlines);
		return FALSE;
	}
	return TRUE;
}

int trackergrid_scrollleft(TrackerGrid* self, psy_audio_PatternCursor cursor)
{		
	if (trackergridstate_screentotrack(self->gridstate,
				psy_ui_component_scrollleft(&self->component),
				psy_audio_player_numsongtracks(&self->workspace->player))
			> cursor.track) {
		psy_ui_component_setscrollleft(&self->component,
			trackergridstate_track_x(self->gridstate, cursor.track));
		return FALSE;
	}
	return TRUE;
}

int trackergrid_scrollright(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	uintptr_t visitracks;
	uintptr_t tracks;
	psy_ui_Size size;	
	psy_ui_TextMetric tm;	
	int trackright;
	int trackleft;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	trackleft = trackergridstate_screentotrack(self->gridstate,
		psy_ui_component_scrollleft(&self->component),
		psy_audio_player_numsongtracks(&self->workspace->player));
	trackright = trackergridstate_screentotrack(self->gridstate,
		psy_ui_value_px(&size.width, &tm) +
		psy_ui_component_scrollleft(&self->component),
		self->gridstate->numtracks);
	if (trackergridstate_track_x(self->gridstate, trackright) +
				trackergridstate_trackwidth(self->gridstate, trackright)
			> psy_ui_value_px(&size.width, &tm)) {
		--trackright;
	}
	visitracks = trackright - trackleft;
	tracks = cursor.track + 1;
	if (tracks > visitracks + trackergridstate_screentotrack(self->gridstate,
			psy_ui_component_scrollleft(&self->component),
			self->gridstate->numtracks)) {
		psy_ui_component_setscrollleft(&self->component,
			trackergridstate_track_x(self->gridstate, tracks - visitracks));		
		return FALSE;
	}
	return TRUE;
}

void trackergrid_prevline(TrackerGrid* self)
{	
	trackergrid_prevlines(self, workspace_cursorstep(self->workspace),
		self->wraparound);
}

void trackergrid_advanceline(TrackerGrid* self)
{
	trackergrid_advancelines(self, workspace_cursorstep(self->workspace),
		self->wraparound);
}

void trackergrid_advancelines(TrackerGrid* self, uintptr_t lines, int wrap)
{
	if (self->gridstate->pattern) {
		psy_audio_PatternCursorNavigator cursornavigator;

		trackergrid_storecursor(self);
		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			self->gridstate->pattern, self->bpl, wrap);		
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			trackergrid_scrolldown(self, self->gridstate->cursor);
		} else {
			trackergrid_scrollup(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_prevlines(TrackerGrid* self, uintptr_t lines, int wrap)
{
	if (self->gridstate->pattern) {
		psy_audio_PatternCursorNavigator cursornavigator;

		trackergrid_storecursor(self);
		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			self->gridstate->pattern, self->bpl, wrap);		
		if (psy_audio_patterncursornavigator_prevlines(&cursornavigator, lines)) {
			trackergrid_scrolldown(self, self->gridstate->cursor);
		} else {
			trackergrid_scrollup(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_home(TrackerGrid* self)
{
	trackergrid_storecursor(self);
	if (self->ft2home) {
		self->gridstate->cursor.offset = 0.0;
		trackergrid_scrollup(self, self->gridstate->cursor);
	} else {
		if (self->gridstate->cursor.column != 0) {
			self->gridstate->cursor.column = 0;
		} else {
			self->gridstate->cursor.track = 0;
			self->gridstate->cursor.column = 0;
		}
		trackergrid_scrollleft(self, self->gridstate->cursor);
	}
	workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
	trackergrid_invalidatecursor(self);
}

void trackergrid_end(TrackerGrid* self)
{
	trackergrid_storecursor(self);
	if (self->ft2home) {
		self->gridstate->cursor.offset = self->gridstate->pattern->length - self->bpl;
		trackergrid_scrolldown(self, self->gridstate->cursor);
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		if (self->gridstate->cursor.track != self->gridstate->numtracks - 1 ||
				self->gridstate->cursor.digit != columndef->numdigits - 1 ||
				self->gridstate->cursor.column != TRACKER_COLUMN_PARAM) {
			if (self->gridstate->cursor.column == TRACKER_COLUMN_PARAM &&
				self->gridstate->cursor.digit == columndef->numdigits - 1) {
				self->gridstate->cursor.track = self->gridstate->numtracks - 1;
				trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->gridstate->cursor.column = TRACKER_COLUMN_PARAM;
				self->gridstate->cursor.digit = columndef->numdigits - 1;
				trackergrid_scrollright(self, self->gridstate->cursor);
			} else {
				trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->gridstate->cursor.column = TRACKER_COLUMN_PARAM;
				self->gridstate->cursor.digit = columndef->numdigits - 1;
			}			
		}
	}
	workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
	trackergrid_invalidatecursor(self);
}

void trackergrid_onkeydown(TrackerGrid* self, psy_ui_KeyEvent* ev)
{	
	if (self->editmode == TRACKERGRID_EDITMODE_LOCAL) {
		if (ev->keycode == psy_ui_KEY_DOWN) {
			return;
		} else {
			psy_EventDriver* kbd;
			psy_EventDriverInput input;
			psy_EventDriverCmd cmd;

			kbd = workspace_kbddriver(self->workspace);
			input.message = psy_EVENTDRIVER_KEYDOWN;
			input.param1 = psy_audio_encodeinput(ev->keycode,
				self->chordmode ? 0 : ev->shift, ev->ctrl);
			input.param2 = workspace_octave(self->workspace) * 12;
			psy_eventdriver_cmd(kbd, "tracker", input, &cmd);
			if (cmd.id == CMD_NAVLEFT) {
				trackergrid_prevcol(self);
				psy_ui_component_invalidate(&self->component);
				psy_ui_keyevent_stoppropagation(ev);
			} else if (cmd.id == CMD_NAVRIGHT) {
				trackergrid_nextcol(self);
				psy_ui_component_invalidate(&self->component);
				psy_ui_keyevent_stoppropagation(ev);
			} else if (cmd.id == CMD_ROWCLEAR) {
				trackergrid_rowclear(self);
				psy_ui_component_invalidate(&self->component);
				psy_ui_keyevent_stoppropagation(ev);
				return;
			} else {
				if (self->gridstate->cursor.column != TRACKER_COLUMN_NOTE) {
					int digit = keycodetoint(ev->keycode);
					if (digit != -1) {
						trackergrid_inputvalue(self, digit, 1);
						psy_ui_component_invalidate(&self->component);
						psy_ui_keyevent_stoppropagation(ev);
						return;
					}
				}
				{
					psy_EventDriver* kbd;
					psy_EventDriverCmd cmd;
					psy_EventDriverInput input;

					cmd.id = -1;
					kbd = workspace_kbddriver(self->workspace);
					input.message = psy_EVENTDRIVER_KEYDOWN;
					input.param1 = psy_audio_encodeinput(ev->keycode, 0, ev->ctrl);
					psy_eventdriver_cmd(kbd, "notes", input, &cmd);
					trackergrid_inputnote(self,
						(psy_dsp_note_t)(cmd.id + workspace_octave(self->workspace) * 12),
						1);
					psy_ui_component_invalidate(&self->component);
					psy_ui_keyevent_stoppropagation(ev);
					return;
				}
			}
		}
	}
	if (self->editmode != TRACKERGRID_EDITMODE_SONG) {
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void trackergrid_prevcol(TrackerGrid* self)
{
	int invalidate = 1;

	trackergrid_storecursor(self);	
	if (self->gridstate->cursor.column == 0 && self->gridstate->cursor.digit == 0) {
		if (self->gridstate->cursor.track > 0) {
			TrackDef* trackdef;

			--self->gridstate->cursor.track;
			trackdef = trackergridstate_trackdef(self->gridstate,
				self->gridstate->cursor.track);
			self->gridstate->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->gridstate->cursor.digit = trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1;
			trackergrid_scrollleft(self, self->gridstate->cursor);
		} else if (self->wraparound) {
			TrackDef* trackdef;

			self->gridstate->cursor.track = psy_audio_player_numsongtracks(
				&self->workspace->player) - 1;
			trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
			self->gridstate->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->gridstate->cursor.digit = trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1;
			invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
		}
	} else {
		if (self->gridstate->cursor.digit > 0) {
			--self->gridstate->cursor.digit;
		} else {
			TrackDef* trackdef;

			trackdef = trackergridstate_trackdef(self->gridstate,
				self->gridstate->cursor.track);
			--self->gridstate->cursor.column;
			self->gridstate->cursor.digit = trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1;
		}
	}
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
		if (invalidate) {
			trackergrid_invalidatecursor(self);
		}
	}
}

void trackergrid_nextcol(TrackerGrid* self)
{
	TrackDef* trackdef;
	int invalidate = 1;
	
	trackergrid_storecursor(self);
	trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
	if (self->gridstate->cursor.column == trackdef_numcolumns(trackdef) - 1 &&
		self->gridstate->cursor.digit == trackdef_numdigits(trackdef,
			self->gridstate->cursor.column) - 1) {
		if (self->gridstate->cursor.track < psy_audio_player_numsongtracks(
				&self->workspace->player) - 1) {
			self->gridstate->cursor.column = 0;
			self->gridstate->cursor.digit = 0;
			++self->gridstate->cursor.track;
			invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
		} else if (self->wraparound) {
			self->gridstate->cursor.column = 0;
			self->gridstate->cursor.digit = 0;
			self->gridstate->cursor.track = 0;
			invalidate = trackergrid_scrollleft(self, self->gridstate->cursor);
		}
	} else {
		++self->gridstate->cursor.digit;
		if (self->gridstate->cursor.digit >=
			trackdef_numdigits(trackdef, self->gridstate->cursor.column)) {
			++self->gridstate->cursor.column;
			self->gridstate->cursor.digit = 0;
		}
	}
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
		workspace_setpatterncursor(self->workspace,
			self->gridstate->cursor);
		if (invalidate) {
			trackergrid_invalidatecursor(self);
		}
	}
}

void trackergrid_selectall(TrackerGrid* self)
{
	if (self->workspace->song && self->gridstate->pattern) {
		self->selection.topleft.offset = 0;
		self->selection.topleft.track = 0;
		self->selection.bottomright.offset = self->gridstate->pattern->length;
		self->selection.bottomright.track =
			self->workspace->song->patterns.songtracks;
		self->hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_selectcol(TrackerGrid* self)
{
	if (self->workspace->song && self->gridstate->pattern) {
		self->selection.topleft.offset = 0;
		self->selection.topleft.track = self->gridstate->cursor.track;
		self->selection.bottomright.offset = self->gridstate->pattern->length;
		self->selection.bottomright.track = self->gridstate->cursor.track + 1;
		self->hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_selectbar(TrackerGrid* self)
{
	if (self->workspace->song && self->gridstate->pattern) {
		self->selection.topleft.offset = self->gridstate->cursor.offset;
		self->selection.topleft.track = self->gridstate->cursor.track;
		self->selection.bottomright.offset = self->gridstate->cursor.offset + 4.0;
		if (self->gridstate->cursor.offset > self->gridstate->pattern->length) {
			self->gridstate->cursor.offset = self->gridstate->pattern->length;
		}
		self->selection.bottomright.track = self->gridstate->cursor.track + 1;
		self->hasselection = TRUE;
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_selectmachine(TrackerGrid* self)
{
	if (self->workspace->song) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		
		node = psy_audio_pattern_findnode(self->gridstate->pattern,
			self->gridstate->cursor.track,
			(psy_dsp_big_beat_t)self->gridstate->cursor.offset,
			(psy_dsp_big_beat_t)self->bpl,
			&prev);
		if (node) {
			psy_audio_PatternEvent* ev;

			entry = (psy_audio_PatternEntry*)node->entry;
			ev = psy_audio_patternentry_front(entry);
			psy_audio_machines_changeslot(&self->workspace->song->machines,
				ev->mach);
			psy_audio_instruments_select(&self->workspace->song->instruments,
				psy_audio_instrumentindex_make(0, ev->inst));			
		}		
	}
}

void trackergrid_oninput(TrackerGrid* self, psy_audio_Player* sender,
	psy_audio_PatternEvent* ev)
{
	if (self->gridstate->cursor.column == TRACKER_COLUMN_NOTE &&
			ev->note != psy_audio_NOTECOMMANDS_RELEASE) {
		if (workspace_currview(self->workspace) == TABPAGE_PATTERNVIEW &&
				self->editmode == TRACKERGRID_EDITMODE_SONG) {
			trackergrid_setdefaultevent(self, &sender->patterndefaults, ev);
			trackergrid_inputevent(self, ev, self->chordmode);
		}
	}
}

void trackergrid_setdefaultevent(TrackerGrid* self,
	psy_audio_Pattern* defaults,
	psy_audio_PatternEvent* ev)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(defaults, self->gridstate->cursor.track, 0.0,
		self->bpl, &prev);
	if (node) {
		psy_audio_PatternEvent* defaultevent;

		defaultevent = psy_audio_patternentry_front(psy_audio_patternnode_entry(node));
		if (defaultevent->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
			ev->inst = defaultevent->inst;
		}
		if (defaultevent->mach != psy_audio_NOTECOMMANDS_MACH_EMPTY) {
			ev->mach = defaultevent->mach;
		}
		if (defaultevent->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			ev->vol = defaultevent->vol;
		}
	}
}

void trackergrid_rowdelete(TrackerGrid* self)
{
	if (self->gridstate->cursor.offset - self->bpl >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		if (self->ft2delete) {
			trackergrid_prevline(self);
		}
		node = psy_audio_pattern_findnode(self->gridstate->pattern, self->gridstate->cursor.track,
			(psy_dsp_big_beat_t)self->gridstate->cursor.offset, (psy_dsp_big_beat_t)self->bpl, &prev);
		if (node) {
			psy_audio_pattern_remove(self->gridstate->pattern, node);
			psy_audio_sequencer_checkiterators(
				&self->workspace->player.sequencer,
				node);
		}
		p = (prev)
			? prev->next
			: psy_audio_pattern_begin(self->gridstate->pattern);
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->gridstate->cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_big_beat_t offset;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				event = *psy_audio_patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(self->gridstate->pattern, p);
				psy_audio_sequencer_checkiterators(
					&self->workspace->player.sequencer, p);
				offset -= (psy_dsp_big_beat_t)self->bpl;
				node = psy_audio_pattern_findnode(self->gridstate->pattern, track,
					offset,
					(psy_dsp_big_beat_t)self->bpl,
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*psy_audio_patternentry_front(entry) = event;
				} else {
					psy_audio_pattern_insert(self->gridstate->pattern, prev, track,
						(psy_dsp_big_beat_t)offset, &event);
				}
			}
		}
	}
}

void trackergrid_rowclear(TrackerGrid* self)
{
	if (self->gridstate->cursor.column == TRACKER_COLUMN_NOTE) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&RemoveCommandAlloc(self->gridstate->pattern, self->bpl,
			self->gridstate->cursor, self->workspace)->command);
		if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
			trackergrid_advanceline(self);
		}
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		trackergrid_inputvalue(self, columndef->emptyvalue, 0);
	}
}

void trackergrid_inputevent(TrackerGrid* self,
	const psy_audio_PatternEvent* ev, bool chordmode)
{
	trackergrid_preventsync(self);
	psy_undoredo_execute(&self->workspace->undoredo,
		&InsertCommandAlloc(self->gridstate->pattern, self->bpl,
			self->gridstate->cursor, *ev, self->workspace)->command);
	if (chordmode != FALSE) {
		trackergrid_nexttrack(self);
	} else {
		trackergrid_advanceline(self);
	}
	if (ev->note < psy_audio_NOTECOMMANDS_RELEASE) {
		self->gridstate->cursor.key = ev->note;
		workspace_setpatterncursor(self->workspace,
			self->gridstate->cursor);

	}
	trackergrid_enablesync(self);
}

void trackergrid_inputnote(TrackerGrid* self, psy_dsp_note_t note,
	bool chordmode)
{
	psy_audio_Machine* machine;
	psy_audio_PatternEvent ev;

	psy_audio_patternevent_init_all(&ev,
		note,
		psy_audio_NOTECOMMANDS_INST_EMPTY,
		(unsigned char)psy_audio_machines_slot(&self->workspace->song->machines),
		psy_audio_NOTECOMMANDS_VOL_EMPTY,
		0,
		0);
	machine = psy_audio_machines_at(&self->workspace->song->machines, ev.mach);
	if (machine &&
		machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
		ev.inst = psy_audio_instruments_selected(
			&self->workspace->song->instruments).subslot;
	}
	trackergrid_inputevent(self, &ev, chordmode);
}

void trackergrid_inputvalue(TrackerGrid* self, int value, int digit)
{
	if (self->gridstate->pattern && value != -1) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry newentry;
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		psy_audio_patternentry_init(&newentry);
		node = psy_audio_pattern_findnode(self->gridstate->pattern,
			self->gridstate->cursor.track,
			(psy_dsp_big_beat_t)self->gridstate->cursor.offset,
			(psy_dsp_big_beat_t)self->bpl,
			&prev);
		if (node) {
			entry = (psy_audio_PatternEntry*)node->entry;
		} else {
			entry = &newentry;
		}
		if (digit) {
			trackergrid_enterdigitcolumn(self, entry,
				self->gridstate->cursor.track,
				self->gridstate->cursor.column,
				self->gridstate->cursor.digit,
				value);
		} else {
			entervaluecolumn(entry, self->gridstate->cursor.column, value);
		}
		trackergrid_preventsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&InsertCommandAlloc(self->gridstate->pattern, self->bpl,
				self->gridstate->cursor,
				*psy_audio_patternentry_front(entry),
				self->workspace)->command);
		if (self->effcursoralwaysdown) {
			trackergrid_advanceline(self);
		} else {
			if (!digit) {
				if (columndef->wrapclearcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->gridstate->cursor.digit = 0;
					self->gridstate->cursor.column = columndef->wrapclearcolumn;
					trackergrid_advanceline(self);
				}
			} else if (self->gridstate->cursor.digit + 1 >= columndef->numdigits) {
				if (columndef->wrapeditcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->gridstate->cursor.digit = 0;
					self->gridstate->cursor.column = columndef->wrapeditcolumn;
					trackergrid_advanceline(self);
				}
			} else {
				trackergrid_nextcol(self);
			}
		}
		trackergrid_invalidatecursor(self);
		trackergrid_enablesync(self);
		psy_audio_patternentry_dispose(&newentry);
	}
}

void trackergrid_invalidatecursor(TrackerGrid* self)
{		
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_rectangle_make(
			trackergridstate_track_x(self->gridstate, self->oldcursor.track),
			self->linestate->lineheight *
			trackerlinestate_offsettoscreenline(self->linestate,
				self->oldcursor.offset),
			trackergridstate_trackwidth(self->gridstate, self->oldcursor.track),
			self->linestate->lineheight));
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_rectangle_make(
			trackergridstate_track_x(self->gridstate, self->gridstate->cursor.track),
			self->linestate->lineheight *
			trackerlinestate_offsettoscreenline(self->linestate,
				self->gridstate->cursor.offset),
			trackergridstate_trackwidth(self->gridstate, self->gridstate->cursor.track),
			self->linestate->lineheight));	
}

void trackergrid_invalidateline(TrackerGrid* self, psy_dsp_big_beat_t offset)
{
	int line;	

	if (offset >= self->linestate->sequenceentryoffset &&
		offset < self->linestate->sequenceentryoffset + self->gridstate->pattern->length) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size(&self->component);
		line = (int)((offset - self->linestate->sequenceentryoffset)
			/ self->bpl);		
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_rectangle_make(
				psy_ui_component_scrollleft(&self->component),
				self->linestate->lineheight * line,
				psy_ui_value_px(&size.width, &tm),
				self->linestate->lineheight));
	}
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleft(&self->component) < 0) {
		psy_ui_component_setscrollleft(&self->component, 0);
	}
	if (self->midline) {
		int halfvisilines;
		int restoremidline;
		psy_ui_Size size;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size(&self->component);
		halfvisilines = self->linestate->visilines / 2;
		restoremidline = self->midline;
		self->midline = 0;
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_rectangle_make(
				psy_ui_component_scrollleft(&self->component),
				halfvisilines * self->linestate->lineheight +
				psy_ui_component_scrolltop(&self->component),
				psy_ui_value_px(&size.width, &tm),
					self->linestate->lineheight * 2));
		self->midline = restoremidline;
	}
}

void trackergrid_centeroncursor(TrackerGrid* self)
{
	int line;

	line = trackerlinestate_offsettoscreenline(self->linestate,
		self->gridstate->cursor.offset);
	psy_ui_component_setscrolltop(&self->component,
		-(self->linestate->visilines / 2 - line) * self->linestate->lineheight);
}

void trackergrid_setcentermode(TrackerGrid* self, int mode)
{
	self->midline = mode;
	if (mode) {
		psy_ui_component_setoverflow(&self->component,
			psy_ui_OVERFLOW_SCROLL | psy_ui_OVERFLOW_VSCROLLCENTER);
		trackergrid_centeroncursor(self);
	} else {
		psy_ui_component_setoverflow(&self->component,
			psy_ui_OVERFLOW_SCROLL);
		psy_ui_component_setscrolltop(&self->component, 0);
	}
}


void trackergrid_onmousedown(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (self->gridstate->pattern && ev->button == 1) {
		self->dragcolumn = trackergrid_resizecolumn(self, ev->x, ev->y);
		if (self->dragcolumn == UINTPTR_MAX) {
			trackergrid_storecursor(self);
			self->gridstate->cursor = trackergrid_makecursor(self, ev->x, ev->y);
			self->selection.topleft = self->gridstate->cursor;
			self->dragselectionbase = self->gridstate->cursor;
			self->lastdragcursor = self->gridstate->cursor;
			workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
			if (self->hasselection) {
				self->hasselection = 0;
				psy_ui_component_invalidate(&self->component);
			}
			self->hasselection = 0;
			trackergrid_invalidatecursor(self);
			psy_ui_component_setfocus(&self->component);
			psy_ui_component_capture(&self->component);
		} else {
			self->dragcolumnbase = ev->x;
			self->dragtrack = trackergridstate_screentotrack(self->gridstate, ev->x,
				self->gridstate->numtracks);
			self->dragparamcol = trackergridstate_paramcol(self->gridstate, self->dragtrack, ev->x);
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void trackergrid_onmousemove(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_PatternCursor cursor;

	if (ev->button == 1) {
		if (self->dragcolumn != UINTPTR_MAX) {
			uintptr_t paramcol;
			TrackDef* trackdef;

			trackdef = trackergridstate_trackdef(self->gridstate, self->dragtrack);
			paramcol = trackergridstate_paramcol(self->gridstate, self->dragtrack, ev->x);
			self->dragcolumnbase = ev->x;			
			if (self->dragparamcol < paramcol) {
				if (trackdef == &self->gridstate->trackconfig->trackdef) {
					trackdef = malloc(sizeof(TrackDef));
					trackdef_init(trackdef);
					psy_table_insert(&self->gridstate->trackconfig->trackconfigs,
						self->dragtrack, trackdef);
				}
				trackdef->numfx++;
				self->dragparamcol = paramcol;
				psy_signal_emit(&self->signal_colresize, self, 0);
			} else
			if (self->dragparamcol > paramcol) {
				if (trackdef->numfx > 1) {
					trackdef->numfx--;
				}
				if (trackdef->numfx > 1 && trackdef == &self->gridstate->trackconfig->trackdef) {
					free(trackdef);
					psy_table_remove(&self->gridstate->trackconfig->trackconfigs, self->dragtrack);
				}
				self->dragparamcol = paramcol;
				psy_signal_emit(&self->signal_colresize, self, 0);
			}
			//if (self->columnresize) {
				//trackergrid_dragcolumn(self, ev);
			//}
			psy_ui_component_invalidate(&self->component);
		} else {
			cursor = trackergrid_makecursor(self, ev->x, ev->y);
			if (cursor.column != self->lastdragcursor.column ||
				cursor.offset != self->lastdragcursor.offset) {
				if (!self->hasselection) {
					trackergrid_startdragselection(self, cursor);
				} else {
					trackergrid_dragselection(self, cursor);
				}
				psy_ui_component_invalidate(&self->component);
				self->lastdragcursor = cursor;
			}			
		}
	}
}

void trackergrid_startdragselection(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	self->hasselection = 1;
	if (cursor.track >= self->dragselectionbase.track) {
		self->selection.topleft.track = self->dragselectionbase.track;
		self->selection.bottomright.track = cursor.track + 1;
	} else {
		self->selection.topleft.track = cursor.track;
		self->selection.bottomright.track = self->dragselectionbase.track + 1;
	}
	if (cursor.offset >= self->dragselectionbase.offset) {
		self->selection.topleft.offset = self->dragselectionbase.offset;
		self->selection.bottomright.offset = cursor.offset + self->bpl;
	} else {
		self->selection.topleft.offset = cursor.offset;
		self->selection.bottomright.offset = self->dragselectionbase.offset + self->bpl;
	}
	self->selection.bottomright.track += 1;
}

void trackergrid_dragselection(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	int restoremidline = self->midline;
	if (cursor.track >= self->dragselectionbase.track) {
		self->selection.topleft.track = self->dragselectionbase.track;
		self->selection.bottomright.track = cursor.track + 1;
	} else {
		self->selection.topleft.track = cursor.track;
		self->selection.bottomright.track = self->dragselectionbase.track + 1;
	}
	if (cursor.offset >= self->dragselectionbase.offset) {
		self->selection.topleft.offset = self->dragselectionbase.offset;
		self->selection.bottomright.offset = cursor.offset + self->bpl;
	} else {
		self->selection.topleft.offset = cursor.offset;
		self->selection.bottomright.offset = self->dragselectionbase.offset + self->bpl;
	}
	self->midline = 0;	
	if (cursor.offset < self->lastdragcursor.offset) {
		trackergrid_scrollup(self, cursor);
	} else {
		trackergrid_scrolldown(self, cursor);
	}
	if (cursor.track < self->lastdragcursor.track) {
		trackergrid_scrollleft(self, cursor);
	} else {
		trackergrid_scrollright(self, cursor);
	}
	self->midline = restoremidline;
}

void trackergrid_dragcolumn(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (self->dragcolumn != UINTPTR_MAX) {
		uintptr_t track;
		TrackDef* trackdef;

		track = trackergridstate_screentotrack(self->gridstate, ev->x +
			psy_ui_component_scrollleft(&self->component), self->gridstate->numtracks);
		trackdef = trackergridstate_trackdef(self->gridstate, track);
		if (ev->x > self->dragcolumnbase) {
			if (trackdef != &self->gridstate->trackconfig->trackdef) {
				trackdef->numfx++;
			} else {
				trackdef = malloc(sizeof(TrackDef));
				trackdef_init(trackdef);
				trackdef->numfx = 2;
				psy_table_insert(&self->gridstate->trackconfig->trackconfigs, track, trackdef);
			}
		} else if (ev->x < self->dragcolumnbase && trackdef->numfx > 1) {
			if (trackdef != &self->gridstate->trackconfig->trackdef) {
				trackdef->numfx--;
				if (trackdef->numfx == 1) {
					free(trackdef);
					psy_table_remove(&self->gridstate->trackconfig->trackconfigs, track);
				}
			}
		}
		self->dragcolumnbase = ev->x;
		// todo
		//psy_ui_component_invalidate(&self->view->component);
		//trackerview_updatescrollstep(self->view);
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
	} else {
		int resizecolumn;

		resizecolumn = trackergrid_resizecolumn(self, ev->x, ev->y);
		if (resizecolumn != -1) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		}
	}
}

void trackergrid_onmousedoubleclick(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		trackergrid_selectcol(self);
	}
}

void trackergrid_onmousewheel(TrackerGrid* self, psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (ev->ctrl) {
		if (ev->delta > 0) {
			//zoombox_setrate(&self->view->zoombox, zoombox_rate(&self->view->zoombox) + 0.25);
		} else if (ev->delta < 0) {
			//zoombox_setrate(&self->view->zoombox, zoombox_rate(&self->view->zoombox) - 0.25);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

uintptr_t trackergrid_resizecolumn(TrackerGrid* self, int x, int y)
{
	uintptr_t rv;
	TrackDef* trackdef;
	psy_audio_PatternCursor position;
	int lines;
	int coloffset;
	int cpx;

	rv = UINTPTR_MAX;
	position.offset = trackerlinestate_offset(self->linestate, y, &lines);
	position.track = trackergridstate_screentotrack(self->gridstate, x, self->gridstate->numtracks);
	coloffset = (x - self->gridstate->trackconfig->patterntrackident) -
		trackergridstate_track_x(self->gridstate, position.track);
	position.column = 0;
	position.digit = 0;
	trackdef = trackergridstate_trackdef(self->gridstate, position.track);
	cpx = 0;
	while (position.column < trackdef_numcolumns(trackdef) &&
		cpx + trackdef_columnwidth(trackdef, position.column,
			self->gridstate->trackconfig->textwidth) < coloffset) {
		cpx += trackdef_columnwidth(trackdef, position.column,
			self->gridstate->trackconfig->textwidth);
		++position.column;
	}
	position.digit = (coloffset - cpx) / self->gridstate->trackconfig->textwidth;
	if (position.digit >= trackdef_columndef(trackdef, position.column)->numchars) {
		rv = position.column;
	}
	return rv;
}

psy_audio_PatternCursor trackergrid_makecursor(TrackerGrid* self, int x, int y)
{
	psy_audio_PatternCursor rv;
	TrackDef* trackdef;
	int lines;
	int coloffset;
	int cpx;

	rv.offset = trackerlinestate_offset(self->linestate, y, &lines);
	if (self->gridstate->pattern && rv.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
		rv.offset = psy_audio_pattern_length(self->gridstate->pattern) - self->bpl;
	}
	rv.track = trackergridstate_screentotrack(self->gridstate, x, self->gridstate->numtracks);
	if (rv.track >= psy_audio_player_numsongtracks(&self->workspace->player)) {
		rv.track = psy_audio_player_numsongtracks(&self->workspace->player) - 1;
	}
	coloffset = (x - self->gridstate->trackconfig->patterntrackident) -
		trackergridstate_track_x(self->gridstate, rv.track);
	rv.column = 0;
	rv.digit = 0;
	rv.key = self->workspace->patterneditposition.key;
	trackdef = trackergridstate_trackdef(self->gridstate, rv.track);
	cpx = 0;
	while (rv.column < trackdef_numcolumns(trackdef) &&
		cpx + trackdef_columnwidth(trackdef, rv.column,
			self->gridstate->trackconfig->textwidth) < coloffset) {
		cpx += trackdef_columnwidth(trackdef, rv.column,
			self->gridstate->trackconfig->textwidth);
		++rv.column;
	}
	rv.digit = (coloffset - cpx) / self->gridstate->trackconfig->textwidth;
	if (rv.digit >= trackdef_numdigits(trackdef, rv.column)) {
		rv.digit = trackdef_numdigits(trackdef, rv.column) - 1;
	}
	self->gridstate->cursor.patternid =
		workspace_patterncursor(self->workspace).patternid;
	return rv;
}

void trackergrid_onmouseup(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_component_releasecapture(&self->component);
		if (self->dragcolumn != UINTPTR_MAX) {
			self->dragcolumn = UINTPTR_MAX;
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void trackergrid_onfocus(TrackerGrid* self, psy_ui_Component* sender)
{
	trackergrid_invalidatecursor(self);	
}

void trackergrid_onfocuslost(TrackerGrid* self, psy_ui_Component* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_setpattern(TrackerGrid* self, psy_audio_Pattern* pattern)
{
	self->gridstate->pattern = pattern;
	self->linestate->pattern = pattern;
	self->opcount = (pattern)
		? pattern->opcount
		: 0;
	self->gridstate->cursor.offset = 0;
	trackergrid_storecursor(self);
	if (self->midline) {
		trackergrid_centeroncursor(self);
	} else {
		psy_ui_component_setscrolltop(&self->component, 0);
	}
}

void trackergrid_enablesync(TrackerGrid* self)
{
	self->opcount = self->gridstate->pattern->opcount;
	self->syncpattern = TRUE;
}

void trackergrid_preventsync(TrackerGrid* self)
{
	self->opcount = self->gridstate->pattern->opcount;
	self->syncpattern = FALSE;
}

void trackergrid_oninterpolatelinear(TrackerGrid* self)
{
	self->selection.topleft.line = (uintptr_t)(self->selection.topleft.offset / 0.25f);
	self->selection.bottomright.line = (uintptr_t)(self->selection.bottomright.offset / 0.25f);
	psy_audio_pattern_blockinterpolatelinear(self->gridstate->pattern,
		self->selection.topleft, self->selection.bottomright, self->bpl);
}

void trackergrid_onchangegenerator(TrackerGrid* self)
{
	if (self->gridstate->pattern && self->workspace->song) {
		psy_audio_pattern_changemachine(self->gridstate->pattern,
			self->selection.topleft,
			self->selection.bottomright,
			self->workspace->song->machines.slot);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_onchangeinstrument(TrackerGrid* self)
{
	if (self->gridstate->pattern && self->workspace->song) {
		psy_audio_pattern_changeinstrument(self->gridstate->pattern,
			self->selection.topleft,
			self->selection.bottomright,
			psy_audio_instruments_selected(&self->workspace->song->instruments).subslot);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_onblockcut(TrackerGrid* self)
{
	if (self->hasselection) {
		trackergrid_onblockcopy(self);
		trackergrid_onblockdelete(self);
	}
}

void trackergrid_onblockcopy(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_audio_PatternNode* begin;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* prev = 0;
		psy_dsp_big_beat_t offset;
		int trackoffset;

		begin = psy_audio_pattern_greaterequal(self->gridstate->pattern,
			(psy_dsp_big_beat_t)self->selection.topleft.offset);
		offset = (psy_dsp_big_beat_t)self->selection.topleft.offset;
		trackoffset = self->selection.topleft.track;
		psy_audio_pattern_dispose(&self->workspace->patternpaste);
		psy_audio_pattern_init(&self->workspace->patternpaste);
		p = begin;
		while (p != NULL) {
			psy_audio_PatternEntry* entry;
			q = p->next;

			entry = psy_audio_patternnode_entry(p);
			if (entry->offset < self->selection.bottomright.offset) {
				if (entry->track >= self->selection.topleft.track &&
					entry->track < self->selection.bottomright.track) {
					prev = psy_audio_pattern_insert(&self->workspace->patternpaste,
						prev, entry->track - trackoffset,
						entry->offset - offset,
						psy_audio_patternentry_front(entry));
				}
			} else {
				break;
			}
			p = q;
		}
		psy_audio_pattern_setmaxsongtracks(&self->workspace->patternpaste,
			self->selection.bottomright.track -
			self->selection.topleft.track);
		psy_audio_pattern_setlength(&self->workspace->patternpaste,
			(psy_dsp_big_beat_t)(self->selection.bottomright.offset -
				self->selection.topleft.offset));
	}
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_onblockpaste(TrackerGrid* self)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	int trackoffset;
	psy_audio_PatternCursor begin;
	psy_audio_PatternCursor end;

	offset = (psy_dsp_big_beat_t)self->gridstate->cursor.offset;
	trackoffset = self->gridstate->cursor.track;
	p = self->workspace->patternpaste.events;

	begin = end = self->gridstate->cursor;
	end.track += self->workspace->patternpaste.maxsongtracks;
	end.offset += self->workspace->patternpaste.length;
	if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
		end.offset = psy_audio_pattern_length(self->gridstate->pattern);
	}
	psy_audio_pattern_blockremove(self->gridstate->pattern, begin, end);
	// sequencer_checkiterators(&self->workspace->player.sequencer,
	//	node);
	while (p != NULL) {
		psy_audio_PatternEntry* pasteentry;
		psy_audio_PatternNode* node;

		pasteentry = psy_audio_patternnode_entry(p);
		node = psy_audio_pattern_findnode(self->gridstate->pattern,
			pasteentry->track + trackoffset,
			pasteentry->offset + offset,
			(psy_dsp_big_beat_t)self->bpl,
			&prev);
		if (node) {
			psy_audio_PatternEntry* entry;

			entry = (psy_audio_PatternEntry*)node->entry;
			*psy_audio_patternentry_front(entry) = *psy_audio_patternentry_front(pasteentry);
		} else {
			psy_audio_pattern_insert(self->gridstate->pattern,
				prev,
				pasteentry->track + trackoffset,
				pasteentry->offset + offset,
				psy_audio_patternentry_front(pasteentry));
		}
		p = p->next;
	}
	psy_ui_component_invalidate(&self->component);
	if (workspace_ismovecursorwhenpaste(self->workspace)) {
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
			end.offset = psy_audio_pattern_length(self->gridstate->pattern) - self->bpl;
		}
		workspace_setpatterncursor(self->workspace, end);
	}
}

void trackergrid_onblockmixpaste(TrackerGrid* self)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	uintptr_t trackoffset;
	psy_audio_PatternCursor begin;
	psy_audio_PatternCursor end;

	offset = (psy_dsp_big_beat_t)self->gridstate->cursor.offset;
	trackoffset = self->gridstate->cursor.track;
	begin = end = self->gridstate->cursor;
	end.track += self->workspace->patternpaste.maxsongtracks;
	end.offset += self->workspace->patternpaste.length;
	if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
		end.offset = psy_audio_pattern_length(self->gridstate->pattern);
	}
	p = self->workspace->patternpaste.events;
	while (p != NULL) {
		psy_audio_PatternEntry* pasteentry;

		pasteentry = psy_audio_patternnode_entry(p);
		if (!psy_audio_pattern_findnode(self->gridstate->pattern,
				pasteentry->track + trackoffset,
				pasteentry->offset + offset,
				(psy_dsp_big_beat_t)self->bpl,
				&prev)) {
			psy_audio_pattern_insert(self->gridstate->pattern,
				prev,
				pasteentry->track + trackoffset,
				pasteentry->offset + offset,
				psy_audio_patternentry_front(pasteentry));
		}
		psy_audio_patternnode_next(&p);
	}
	psy_ui_component_invalidate(&self->component);
	if (workspace_ismovecursorwhenpaste(self->workspace)) {
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
			end.offset = psy_audio_pattern_length(self->gridstate->pattern) - self->bpl;
		}
		workspace_setpatterncursor(self->workspace, end);
	}
}

void trackergrid_onblockdelete(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_audio_pattern_blockremove(self->gridstate->pattern,
			self->selection.topleft,
			self->selection.bottomright);
		//		sequencer_checkiterators(&self->workspace->player.sequencer,
		//			node);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_blockstart(TrackerGrid* self)
{
	self->selection.topleft = self->gridstate->cursor;
	self->selection.bottomright = self->gridstate->cursor;
	++self->selection.bottomright.track;
	self->selection.bottomright.offset += self->bpl;
	self->hasselection = 1;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockend(TrackerGrid* self)
{
	self->selection.bottomright = self->gridstate->cursor;
	++self->selection.bottomright.track;
	self->selection.bottomright.offset += self->bpl;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockunmark(TrackerGrid* self)
{
	self->hasselection = 0;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_onblocktransposeup(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->gridstate->cursor, +1, self->workspace)->command);
	}
}

void trackergrid_onblocktransposedown(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->gridstate->cursor, -1, self->workspace)->command);
	}
}

void trackergrid_onblocktransposeup12(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->gridstate->cursor, 12, self->workspace)->command);
	}
}

void trackergrid_onblocktransposedown12(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->gridstate->cursor, -12, self->workspace)->command);
	}
}

// PatternBlockMenu
// prototypes
static void patternblockmenu_updatetext(PatternBlockMenu* self, Translator*);
static void patternblockmenu_onlanguagechanged(PatternBlockMenu*, Translator* sender);
// implementation
void patternblockmenu_init(PatternBlockMenu* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_button_init(&self->cut, &self->component);
	psy_ui_button_settext(&self->cut, "Cut");
	psy_ui_button_init(&self->copy, &self->component);
	psy_ui_button_settext(&self->copy, "Copy");
	psy_ui_button_init(&self->paste, &self->component);
	psy_ui_button_settext(&self->paste, "Paste");
	psy_ui_button_init(&self->mixpaste, &self->component);
	psy_ui_button_settext(&self->mixpaste, "MixPaste");
	psy_ui_button_init(&self->del, &self->component);
	psy_ui_button_settext(&self->del, "Delete");

	psy_ui_button_init(&self->interpolatelinear, &self->component);
	psy_ui_button_settext(&self->interpolatelinear, "Interpolate (Linear)");
	psy_ui_button_init(&self->interpolatecurve, &self->component);
	psy_ui_button_settext(&self->interpolatecurve, "Interpolate (Curve)");
	psy_ui_button_init(&self->changegenerator, &self->component);
	psy_ui_button_settext(&self->changegenerator, "Change Generator");
	psy_ui_button_init(&self->changeinstrument, &self->component);
	psy_ui_button_settext(&self->changeinstrument, "Change Instrument");

	psy_ui_button_init(&self->blocktransposeup, &self->component);
	psy_ui_button_settext(&self->blocktransposeup, "Transpose +1");
	psy_ui_button_init(&self->blocktransposedown, &self->component);
	psy_ui_button_settext(&self->blocktransposedown, "Transpose -1");
	psy_ui_button_init(&self->blocktransposeup12, &self->component);
	psy_ui_button_settext(&self->blocktransposeup12, "Transpose +12");
	psy_ui_button_init(&self->blocktransposedown12, &self->component);
	psy_ui_button_settext(&self->blocktransposedown12, "Transpose -12");

	psy_ui_button_init(&self->import, &self->component);
	psy_ui_button_settext(&self->import, "Import (psb)");
	psy_ui_button_init(&self->export, &self->component);
	psy_ui_button_settext(&self->export, "Export (psb)");
	patternblockmenu_updatetext(self, &workspace->translator);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		patternblockmenu_onlanguagechanged);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		NULL));
}

void patternblockmenu_updatetext(PatternBlockMenu* self, Translator* translator)
{	
	psy_ui_button_settext(&self->cut, 
		translator_translate(translator, "edit.cut"));
	psy_ui_button_settext(&self->copy,
		translator_translate(translator, "edit.copy"));
	psy_ui_button_settext(&self->paste,
		translator_translate(translator, "edit.paste"));
	psy_ui_button_settext(&self->mixpaste,
		translator_translate(translator, "edit.mixpaste"));
	psy_ui_button_settext(&self->del,
		translator_translate(translator, "edit.delete"));
	psy_ui_button_settext(&self->interpolatelinear,
		translator_translate(translator, "Interpolate (Linear)"));
	psy_ui_button_settext(&self->interpolatecurve,
		translator_translate(translator, "Interpolate (Curve)"));
	psy_ui_button_settext(&self->changegenerator,
		translator_translate(translator, "Change Generator"));
	psy_ui_button_settext(&self->changeinstrument,
		translator_translate(translator, "Change Instrument"));
	psy_ui_button_settext(&self->blocktransposeup,
		translator_translate(translator, "Transpose +1"));
	psy_ui_button_settext(&self->blocktransposedown,
		translator_translate(translator, "Transpose -1"));
	psy_ui_button_settext(&self->blocktransposeup12,
		translator_translate(translator, "Transpose +12"));
	psy_ui_button_settext(&self->blocktransposedown12,
		translator_translate(translator, "Transpose -12"));
	psy_ui_button_settext(&self->import,
		translator_translate(translator, "Import (psb)"));
	psy_ui_button_settext(&self->export,
		translator_translate(translator, "Export (psb)"));
}

void patternblockmenu_onlanguagechanged(PatternBlockMenu* self, Translator* sender)
{
	patternblockmenu_updatetext(self, sender);
}

int trackergrid_preferredtrackwidth(TrackerGrid* self)
{
	return trackdef_width(&self->gridstate->trackconfig->trackdef,
		self->gridstate->trackconfig->textwidth);
}

void trackergrid_drawentry(TrackerGrid* self, psy_ui_Graphics* g,
	psy_audio_PatternEntry* entry, int x, int y,
	TrackerColumnFlags columnflags)
{
	static const char* emptynotestr = "- - -";
	const char* notestr;
	psy_ui_Rectangle r;
	psy_ui_TextMetric tm;	
	unsigned int column;
	int focus;
	int cpx;
	TrackDef* trackdef;
	psy_audio_PatternEvent* event;
	TrackerColumnFlags currcolumnflags;

	tm = psy_ui_component_textmetric(&self->component);
	currcolumnflags = columnflags;
	event = psy_audio_patternentry_front(entry);
	trackdef = trackergridstate_trackdef(self->gridstate, entry->track);
	focus = psy_ui_component_hasfocus(&self->component);
	currcolumnflags.cursor = self->linestate->drawcursor && focus &&
		columnflags.cursor && self->gridstate->cursor.column == 0;
	setcolumncolour(self->gridstate->skin, g, currcolumnflags, entry->track, self->gridstate->numtracks);
	cpx = 0;
	// draw note	
	psy_ui_setrectangle(&r, x + cpx, y,
		self->gridstate->trackconfig->textwidth * 3,
		tm.tmHeight);	
	notestr = (event->note != psy_audio_NOTECOMMANDS_EMPTY || !self->showemptydata)
		? psy_dsp_notetostr(event->note, self->notestabmode)
		: emptynotestr;	
	psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
		notestr, strlen(notestr));
	cpx += trackdef_columnwidth(trackdef, 0, self->gridstate->trackconfig->textwidth);
	// draw digit columns
	for (column = 1; column < trackdef_numcolumns(trackdef); ++column) {
		uintptr_t digit;
		uintptr_t value;
		uintptr_t empty;
		uintptr_t num;

		value = trackdef_value(trackdef, column, entry);
		empty = trackdef_emptyvalue(trackdef, column) == value;
		if (column > TRACKER_COLUMN_VOL) {
			int cmd;
			psy_List* ev;

			cmd = (column - (int)TRACKER_COLUMN_VOL - 1) / 2;
			ev = psy_list_at(entry->events, cmd);
			if (ev) {
				if (psy_audio_patternevent_tweakvalue(
					(psy_audio_PatternEvent*)(ev->entry)) != 0) {
					empty = 0;
				}
			}
		}
		num = trackdef_numdigits(trackdef, column);
		for (digit = 0; digit < num; ++digit) {
			uint8_t digitvalue;

			digitvalue = ((value >> ((num - digit - 1) * 4)) & 0x0F);
			currcolumnflags.cursor = self->linestate->drawcursor && focus &&
				columnflags.cursor && self->gridstate->cursor.column == column &&
				self->gridstate->cursor.digit == digit;
			setcolumncolour(self->gridstate->skin, g, currcolumnflags, entry->track,
				self->gridstate->numtracks);
			trackergrid_drawdigit(self, g, x + cpx + digit *
				self->gridstate->trackconfig->textwidth, y, digitvalue,
				empty, currcolumnflags.mid);
		}
		cpx += trackdef_columnwidth(trackdef, column,
			self->gridstate->trackconfig->textwidth);
	}
}

void trackergrid_enterdigitcolumn(TrackerGrid* self, psy_audio_PatternEntry* entry,
	int track, int column, int digit, int digitvalue)
{
	TrackDef* trackdef;

	trackdef = trackergridstate_trackdef(self->gridstate, track);
	if (trackdef) {
		uintptr_t value;
		uintptr_t num;
		ptrdiff_t pos;
		uint8_t* data;

		value = trackdef_value(trackdef, column, entry);
		if (digit != 0xF && value == trackdef_emptyvalue(trackdef, column)) {
			value = 0;
		}
		num = trackdef_numdigits(trackdef, column);
		pos = num / 2 - digit / 2 - 1;
		data = (uint8_t*)&value + pos;
		enterdigit(digit % 2, digitvalue, data);
		trackdef_setvalue(trackdef, column, entry, *((uintptr_t*)&value));
	}
}

// TrackerView
// prototypes
static void trackerview_initblockmenu(TrackerView*);
static void trackerview_connectblockmenu(TrackerView*);
static void trackerview_connectworkspace(TrackerView*);
static void trackerview_onmousedown(TrackerView*, psy_ui_MouseEvent*);
static void trackerview_onmouseup(TrackerView*, psy_ui_MouseEvent*);
static void trackerview_onkeydown(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_onkeyup(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_numtrackschanged(TrackerView* self, psy_audio_Player*,
	uintptr_t numsongtracks);
static bool trackerview_handlecommand(TrackerView*, psy_ui_KeyEvent*, int cmd);
static int trackerview_pgupdownstep(TrackerView*);
static void trackerview_onalign(TrackerView*);
static void trackerview_togglefollowsong(TrackerView*);
static void trackerview_onconfigurationchanged(TrackerView*, Workspace*,
	psy_Property* configurtion);
static void trackerview_readconfiguration(TrackerView*);
static void trackerview_readpgupdowntype(TrackerView*);
static void trackerview_onpatternimport(TrackerView*);
static void trackerview_onpatternexport(TrackerView*);
static void trackerview_onparametertweak(TrackerView*,
	Workspace* sender, int slot, uintptr_t tweak, float value);
static void trackerview_oninterpolatecurve(TrackerView*,
	psy_ui_Component* sender);
static void trackerview_oninterpolatecurveviewoncancel(TrackerView*,
	InterpolateCurveView* sender);
static void trackerview_oneventdriverinput(TrackerView*, psy_EventDriver*);

// vtable
static psy_ui_ComponentVtable trackerview_vtable;
static int trackerview_vtable_initialized = 0;

static void trackerview_vtable_init(TrackerView* self)
{
	if (!trackerview_vtable_initialized) {
		trackerview_vtable = *(self->component.vtable);
		trackerview_vtable.onalign = (psy_ui_fp_component_onalign)
			trackerview_onalign;
		trackerview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			trackerview_onmousedown;
		trackerview_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			trackerview_onmouseup;
		trackerview_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			trackerview_onkeydown;
		trackerview_vtable.onkeyup = (psy_ui_fp_component_onkeydown)
			trackerview_onkeyup;
		trackerview_vtable_initialized = 1;
	}
}
// implementation
void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	TrackerGridState* gridstate, TrackerLineState* linestate,
	TrackConfig* trackconfig, struct PatternView* view, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerview_vtable_init(self);
	self->component.vtable = &trackerview_vtable;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);	
	self->view = view;
	self->workspace = workspace;
	self->showdefaultline = 1;
	self->pgupdownstep = 4;
	self->pgupdownbeat = TRUE;
	self->pgupdown4beat = FALSE;		
	// Interpolate View
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0, 0, 0, workspace);
	psy_ui_component_setalign(&self->interpolatecurveview.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);
	psy_signal_connect(&self->interpolatecurveview.signal_cancel, self,
		trackerview_oninterpolatecurveviewoncancel);
	// left bar	
	// Context menu
	trackerview_initblockmenu(self);
	// top bar		
	// pattern main grid
	trackergrid_init(&self->grid, &self->component, trackconfig,
		gridstate, linestate, TRACKERGRID_EDITMODE_SONG,
		workspace);	
	psy_ui_scroller_init(&self->scroller, &self->grid.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	trackerview_connectworkspace(self);
	trackerview_readconfiguration(self);
	psy_signal_connect(&self->workspace->player.signal_numsongtrackschanged, self,
		trackerview_numtrackschanged);
	psy_signal_connect(&self->workspace->player.eventdrivers.signal_input, self,
		trackerview_oneventdriverinput);	
}

void trackerview_initblockmenu(TrackerView* self)
{
	patternblockmenu_init(&self->blockmenu, &self->component, self->workspace);
	psy_ui_component_setalign(&self->blockmenu.component, psy_ui_ALIGN_RIGHT);
	trackerview_connectblockmenu(self);
	psy_ui_component_hide(&self->blockmenu.component);
}

void trackerview_connectblockmenu(TrackerView* self)
{
	psy_signal_connect(&self->blockmenu.changeinstrument.signal_clicked, &self->grid,
		trackergrid_onchangeinstrument);
	psy_signal_connect(&self->blockmenu.cut.signal_clicked, &self->grid,
		trackergrid_onblockcut);
	psy_signal_connect(&self->blockmenu.copy.signal_clicked, &self->grid,
		trackergrid_onblockcopy);
	psy_signal_connect(&self->blockmenu.paste.signal_clicked, &self->grid,
		trackergrid_onblockpaste);
	psy_signal_connect(&self->blockmenu.mixpaste.signal_clicked, &self->grid,
		trackergrid_onblockmixpaste);
	psy_signal_connect(&self->blockmenu.del.signal_clicked, &self->grid,
		trackergrid_onblockdelete);
	psy_signal_connect(&self->blockmenu.blocktransposeup.signal_clicked, &self->grid,
		trackergrid_onblocktransposeup);
	psy_signal_connect(&self->blockmenu.blocktransposedown.signal_clicked, &self->grid,
		trackergrid_onblocktransposedown);
	psy_signal_connect(&self->blockmenu.blocktransposeup12.signal_clicked, &self->grid,
		trackergrid_onblocktransposeup12);
	psy_signal_connect(&self->blockmenu.blocktransposedown12.signal_clicked, &self->grid,
		trackergrid_onblocktransposedown12);
	psy_signal_connect(&self->blockmenu.import.signal_clicked, &self->grid,
		trackerview_onpatternimport);
	psy_signal_connect(&self->blockmenu.export.signal_clicked,
		self, trackerview_onpatternexport);
	psy_signal_connect(&self->blockmenu.interpolatelinear.signal_clicked, &self->grid,
		trackergrid_oninterpolatelinear);
	psy_signal_connect(&self->blockmenu.interpolatecurve.signal_clicked, self,
		trackerview_oninterpolatecurve);
	psy_signal_connect(&self->blockmenu.changegenerator.signal_clicked, &self->grid,
		trackergrid_onchangegenerator);
}

void trackerview_connectworkspace(TrackerView* self)
{
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		trackerview_onconfigurationchanged);	
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		trackerview_onparametertweak);	
}

void trackerview_onalign(TrackerView* self)
{		
	if (trackergrid_midline(&self->grid)) {
		trackergrid_centeroncursor(&self->grid);
	}
}

void trackerview_numtrackschanged(TrackerView* self, psy_audio_Player* player,
	uintptr_t numsongtracks)
{
	self->grid.gridstate->numtracks = numsongtracks;	
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_invalidate(&self->grid.component);	
}

void trackerview_oninterpolatecurveviewoncancel(TrackerView* self,
	InterpolateCurveView* sender)
{
	trackerview_oninterpolatecurve(self, &self->grid.component);
}

void trackerview_updatescrollstep(TrackerView* self)
{
	int scrollstepx;
	
	scrollstepx = trackergridstate_trackwidth(self->grid.gridstate,
		trackergridstate_track_x(self->grid.gridstate,
			trackergridstate_screentotrack(self->grid.gridstate,
				psy_ui_component_scrollleft(&self->grid.component),
				self->grid.gridstate->numtracks)));
	self->grid.component.scrollstepx = scrollstepx;	
	self->grid.component.scrollstepy = self->grid.linestate->lineheight;
	// todo remove patternview reference
	self->view->griddefaults.component.scrollstepx = scrollstepx;
	self->view->header.component.scrollstepx = scrollstepx;
	self->view->left.linenumbers.component.scrollstepy = self->grid.linestate->lineheight;
}

void trackerview_onparametertweak(TrackerView* self, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue)
{
	if (workspace_recordingtweaks(sender)) {
		psy_audio_PatternEvent event;
		psy_audio_Machine* machine;
		int value;

		machine = psy_audio_machines_at(&self->workspace->song->machines, slot);
		assert(machine);
		value = 0; // machine_parametervalue_scaled(machine, tweak, normvalue);
		psy_audio_patternevent_init_all(&event,
			(unsigned char)(
				(workspace_recordtweaksastws(sender))
				? psy_audio_NOTECOMMANDS_TWEAKSLIDE
				: psy_audio_NOTECOMMANDS_TWEAK),
			psy_audio_NOTECOMMANDS_INST_EMPTY,
			(unsigned char)psy_audio_machines_slot(&self->workspace->song->machines),
			psy_audio_NOTECOMMANDS_VOL_EMPTY,
			(unsigned char)((value & 0xFF00) >> 8),
			(unsigned char)(value & 0xFF));
		event.inst = (unsigned char)tweak;
		trackergrid_preventsync(&self->grid);
		psy_undoredo_execute(&self->workspace->undoredo,
			&InsertCommandAlloc(self->grid.gridstate->pattern, self->grid.bpl,
				self->grid.gridstate->cursor, event, self->workspace)->command);
		if (workspace_advancelineonrecordtweak(sender) &&
			!(workspace_followingsong(sender) &&
				psy_audio_player_playing(&sender->player))) {
			trackergrid_advanceline(&self->grid);
		}
		trackergrid_enablesync(&self->grid);
	}
}

void trackerview_onpatternimport(TrackerView* self)
{
	if (self->grid.gridstate->pattern) {
		psy_ui_OpenDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.psb";

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", filter, "PSB",
			workspace_songs_directory(self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(self->grid.gridstate->pattern,
				psy_ui_opendialog_filename(&dialog),
				1.f / psy_audio_player_lpb(&self->workspace->player));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void trackerview_onpatternexport(TrackerView* self)
{
	if (self->grid.gridstate->pattern) {
		psy_ui_SaveDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.PSB";

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", filter, "PSB",
			workspace_songs_directory(self->workspace));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(self->grid.gridstate->pattern,
				psy_ui_savedialog_filename(&dialog),
				1.f / psy_audio_player_lpb(&self->workspace->player),
				psy_audio_player_numsongtracks(&self->workspace->player));
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void trackerview_toggleblockmenu(TrackerView* self)
{
psy_ui_component_togglevisibility(&self->blockmenu.component);
}

void trackerview_togglefollowsong(TrackerView* self)
{
	if (workspace_followingsong(self->workspace)) {
		workspace_stopfollowsong(self->workspace);
	} else {
		workspace_followsong(self->workspace);
	}
}

void trackerview_onmousedown(TrackerView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (psy_ui_component_visible(&self->interpolatecurveview.component)) {
			trackerview_oninterpolatecurve(self, &self->component);
		} else {
			trackerview_toggleblockmenu(self);
		}
	}
}

void trackerview_onmouseup(TrackerView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1 && self->grid.hasselection) {
		interpolatecurveview_setselection(&self->interpolatecurveview,
			self->grid.selection);
	}
}

void trackerview_onkeydown(TrackerView* self, psy_ui_KeyEvent* ev)
{
	/*if ((ev->keycode == psy_ui_KEY_DOWN) &&
		(ev->target == &self->griddefaults.component)) {
		psy_ui_component_setfocus(&self->grid.component);
		psy_ui_component_invalidate(&self->grid.component);
		psy_ui_keyevent_stoppropagation(ev);
	} else*/
		if (ev->keycode == psy_ui_KEY_ESCAPE) {
			if (psy_ui_component_visible(&self->interpolatecurveview.component)) {
				trackerview_oninterpolatecurve(self, &self->component);
			} else {
				if (psy_ui_component_visible(&self->blockmenu.component)) {
					trackerview_toggleblockmenu(self);
				}
			}
			psy_ui_keyevent_stoppropagation(ev);
		}
}

void trackerview_oneventdriverinput(TrackerView* self, psy_EventDriver* sender)
{
	if (workspace_currview(self->workspace) == TABPAGE_PATTERNVIEW) {
		psy_EventDriverCmd cmd;

		cmd = psy_eventdriver_getcmd(sender, "tracker");
		if (cmd.id != -1) {
			if (psy_ui_component_hasfocus(&self->grid.component)) {
				trackerview_handlecommand(self, NULL, cmd.id);
			} else if (cmd.id == CMD_COLUMNNEXT || cmd.id == CMD_COLUMNPREV) {
				trackerview_handlecommand(self, NULL, cmd.id);				
			}
		}		
	}
}

bool trackerview_handlecommand(TrackerView* self, psy_ui_KeyEvent* ev, int cmd)
{
	bool handled = TRUE;
	switch (cmd) {
		case CMD_NAVUP:
			if (self->grid.movecursoronestep) {
				trackergrid_prevlines(&self->grid, 1, 0);
			} else {
				trackergrid_prevline(&self->grid);
			}
			break;
		case CMD_NAVPAGEUP:
			trackergrid_prevlines(&self->grid, trackerview_pgupdownstep(self),
				0);
			break;
		case CMD_NAVDOWN:
			if (self->grid.movecursoronestep) {
				trackergrid_advancelines(&self->grid, 1, 0);
			} else {
				trackergrid_advanceline(&self->grid);
			}
			break;
		case CMD_NAVPAGEDOWN:			
			trackergrid_advancelines(&self->grid, trackerview_pgupdownstep(
				self), 0);
			break;
		case CMD_NAVLEFT:
			trackergrid_prevcol(&self->grid);
			break;
		case CMD_NAVRIGHT:
			trackergrid_nextcol(&self->grid);
			break;
		case CMD_NAVTOP:
			trackergrid_home(&self->grid);
			break;
		case CMD_NAVBOTTOM:
			trackergrid_end(&self->grid);
			break;
		case CMD_COLUMNPREV:
			trackergrid_prevtrack(&self->grid);
			break;
		case CMD_COLUMNNEXT:
			trackergrid_nexttrack(&self->grid);
			break;
		case CMD_BLOCKSTART:
			trackergrid_blockstart(&self->grid);
			break;
		case CMD_BLOCKEND:
			trackergrid_blockend(&self->grid);
			break;
		case CMD_BLOCKUNMARK:
			trackergrid_blockunmark(&self->grid);
			break;
		case CMD_BLOCKCUT:
			trackergrid_onblockcut(&self->grid);
			break;
		case CMD_BLOCKCOPY:
			trackergrid_onblockcopy(&self->grid);
			break;
		case CMD_BLOCKPASTE:
			trackergrid_onblockpaste(&self->grid);
			break;
		case CMD_BLOCKMIX:
			trackergrid_onblockmixpaste(&self->grid);
			break;
		case CMD_TRANSPOSEBLOCKINC:
			trackergrid_onblocktransposeup(&self->grid);
			break;
		case CMD_TRANSPOSEBLOCKDEC:
			trackergrid_onblocktransposedown(&self->grid);
			break;
		case CMD_TRANSPOSEBLOCKINC12:
			trackergrid_onblocktransposeup12(&self->grid);
			break;
		case CMD_TRANSPOSEBLOCKDEC12:
			trackergrid_onblocktransposedown12(&self->grid);
			break;
		case CMD_ROWDELETE:
			trackergrid_rowdelete(&self->grid);
			if (ev) {
				psy_ui_keyevent_stoppropagation(ev);
			}
			break;
		case CMD_ROWCLEAR:
			trackergrid_rowclear(&self->grid);
			if (ev) {
				psy_ui_keyevent_stoppropagation(ev);
			}
			break;
		case CMD_SELECTALL:
			trackergrid_selectall(&self->grid);
			break;
		case CMD_SELECTCOL:
			trackergrid_selectcol(&self->grid);
			break;
		case CMD_SELECTBAR:
			trackergrid_selectbar(&self->grid);
			break;
		case CMD_SELECTMACHINE:
			trackergrid_selectmachine(&self->grid);
			psy_ui_component_setfocus(&self->grid.component);
			break;
		case CMD_UNDO:
			workspace_undo(self->workspace);
			break;
		case CMD_REDO:
			workspace_redo(self->workspace);
			break;
		case CMD_FOLLOWSONG:
			trackerview_togglefollowsong(self);
			break;
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
			if (self->grid.gridstate->cursor.column != TRACKER_COLUMN_NOTE) {
				int digit = cmd - CMD_DIGIT0;
				if (digit != -1) {
					trackergrid_inputvalue(&self->grid, digit, 1);
				}
			}
			break;		
		default:
			handled = FALSE;
		break;
	}
	return handled;
}

int trackerview_pgupdownstep(TrackerView* self)
{
	if (self->pgupdownbeat) {
		return psy_audio_player_lpb(&self->workspace->player);
	} else if (self->pgupdown4beat) {
		return psy_audio_player_lpb(&self->workspace->player) * 4;
	}
	return self->pgupdownstep;
}

void trackerview_onkeyup(TrackerView* self, psy_ui_KeyEvent* ev)
{
	if (self->grid.chordmode && ev->keycode == psy_ui_KEY_SHIFT) {
		self->grid.chordmode = FALSE;
		self->grid.gridstate->cursor.track = self->grid.chordbegin;
		trackergrid_scrollleft(&self->grid, self->grid.gridstate->cursor);
		trackergrid_advanceline(&self->grid);
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void trackerview_onconfigurationchanged(TrackerView* self, Workspace*
	workspace, psy_Property* property)
{
	if (property == &workspace->config) {
		trackerview_readconfiguration(self);
	} else if (psy_property_insection(property, workspace->keyboard)) {		
		trackerview_readpgupdowntype(self);	
		self->grid.effcursoralwaysdown = workspace_effcursoralwayssdown(
				self->workspace);		
		self->grid.ft2home = workspace_ft2home(self->workspace);
		self->grid.ft2delete = workspace_ft2delete(self->workspace);
		if (strcmp(psy_property_key(property), "wraparound") == 0) {
			self->grid.wraparound = psy_property_item_int(property);
		} else if (strcmp(psy_property_key(property), "movecursoronestep") == 0) {
			self->grid.movecursoronestep = psy_property_item_bool(property);
		}
	} else if (strcmp(psy_property_key(property), "beatoffset") == 0) {
		psy_ui_component_align(&self->component);
	} else if (strcmp(psy_property_key(property), "drawemptydata") == 0) {
		trackergrid_showemptydata(&self->grid, psy_property_item_int(property));
	} else if (strcmp(psy_property_key(property), "centercursoronscreen") == 0) {
		trackergrid_setcentermode(&self->grid, psy_property_item_int(property));
	}
}

void trackerview_readconfiguration(TrackerView* self)
{	
	self->grid.ft2home = workspace_ft2home(self->workspace);
	self->grid.ft2delete = workspace_ft2delete(self->workspace);
	self->grid.movecursoronestep = workspace_movecursoronestep(self->workspace);
	self->grid.effcursoralwaysdown = workspace_effcursoralwayssdown(self->workspace);
	trackerview_readpgupdowntype(self);	
}

void trackerview_readpgupdowntype(TrackerView* self)
{
	psy_Property* p;

	p = psy_property_at(self->workspace->keyboard, "pgupdowntype", PSY_PROPERTY_TYPE_CHOICE);
	if (p) {
		int pgupdowntype;

		pgupdowntype = psy_property_item_int(p);
		switch (pgupdowntype) {
		case 0:
			self->pgupdown4beat = FALSE;
			self->pgupdownbeat = TRUE;
			break;
		case 1:
			self->pgupdown4beat = TRUE;
			self->pgupdownbeat = FALSE;
			break;
		case 2:
			self->pgupdown4beat = FALSE;
			self->pgupdownbeat = FALSE;
			self->pgupdownstep = psy_property_at_int(self->workspace->keyboard,
				"pgupdownstep", 4);
			break;
		default:
			break;
		}
	} else {
		self->pgupdown4beat = TRUE;
		self->pgupdownbeat = FALSE;
	}
}

void trackergrid_showemptydata(TrackerGrid* self, int showstate)
{
	self->showemptydata = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerview_makecmds(psy_Property* parent)
{
	psy_Property* cmds;

	cmds = psy_property_settext(
		psy_property_append_section(parent, "tracker"),
		"Tracker");
	definecmd(cmds, CMD_NAVUP, // cmd
		psy_ui_KEY_UP, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navup", "up");
	definecmd(cmds, CMD_NAVDOWN,
		psy_ui_KEY_DOWN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navdown", "down");
	definecmd(cmds, CMD_NAVLEFT,
		psy_ui_KEY_LEFT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navleft", "left");
	definecmd(cmds, CMD_NAVRIGHT,
		psy_ui_KEY_RIGHT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navright", "right");
	definecmd(cmds, CMD_NAVPAGEUP,
		psy_ui_KEY_PRIOR, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpageup", "pageup");
	definecmd(cmds, CMD_NAVPAGEDOWN,
		psy_ui_KEY_NEXT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpagedown", "pagedown");
	definecmd(cmds, CMD_NAVTOP,
		psy_ui_KEY_HOME, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navtop", "track top");
	definecmd(cmds, CMD_NAVBOTTOM,
		psy_ui_KEY_END, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navbottom", "track bottom");
	definecmd(cmds, CMD_COLUMNPREV,
		psy_ui_KEY_TAB, psy_SHIFT_ON, psy_CTRL_OFF,
		"columnprev", "prev col");
	definecmd(cmds, CMD_COLUMNNEXT,
		psy_ui_KEY_TAB, psy_SHIFT_OFF, psy_CTRL_OFF,
		"columnnext", "next col");
	definecmd(cmds, CMD_ROWINSERT,
		psy_ui_KEY_INSERT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowinsert", "ins row");
	definecmd(cmds, CMD_ROWDELETE,
		psy_ui_KEY_BACK, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowdelete", "del row");
	definecmd(cmds, CMD_ROWCLEAR,
		psy_ui_KEY_DELETE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowclear", "clr row");
	definecmd(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	definecmd(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
	definecmd(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	definecmd(cmds, CMD_BLOCKCUT,
		psy_ui_KEY_X, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcut", "cut");
	definecmd(cmds, CMD_BLOCKCOPY,
		psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcopy", "copy");
	definecmd(cmds, CMD_BLOCKPASTE,
		psy_ui_KEY_V, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockpaste", "paste");
	definecmd(cmds, CMD_BLOCKMIX,
		psy_ui_KEY_M, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockmix", "mix");

	definecmd(cmds, CMD_TRANSPOSEBLOCKINC,
		psy_ui_KEY_F12, psy_SHIFT_OFF, psy_CTRL_ON,
		"transposeblockinc", "Trsp+");
	definecmd(cmds, CMD_TRANSPOSEBLOCKDEC,
		psy_ui_KEY_F11, psy_SHIFT_OFF, psy_CTRL_ON,
		"transposeblockdec", "Trsp-");
	definecmd(cmds, CMD_TRANSPOSEBLOCKINC12,
		psy_ui_KEY_F12, psy_SHIFT_ON, psy_CTRL_ON,
		"transposeblockinc12", "Trsp+12");
	definecmd(cmds, CMD_TRANSPOSEBLOCKDEC12,
		psy_ui_KEY_F11, psy_SHIFT_ON, psy_CTRL_ON,
		"transposeblockdec12", "Trsp-12");

	definecmd(cmds, CMD_SELECTALL,
		psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectall", "sel all");
	definecmd(cmds, CMD_SELECTCOL,
		psy_ui_KEY_R, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectcol", "sel col");
	definecmd(cmds, CMD_SELECTBAR,
		psy_ui_KEY_K, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectbar", "sel bar");

	definecmd(cmds, CMD_SELECTMACHINE,
		psy_ui_KEY_RETURN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"selectmachine", "Sel Mac/Ins");
	definecmd(cmds, CMD_UNDO,
		psy_ui_KEY_Z, psy_SHIFT_OFF, psy_CTRL_ON,
		"undo", "undo");
	definecmd(cmds, CMD_REDO,
		psy_ui_KEY_Z, psy_SHIFT_ON, psy_CTRL_ON,
		"redo", "redo");
	definecmd(cmds, CMD_FOLLOWSONG,
		psy_ui_KEY_F, psy_SHIFT_OFF, psy_CTRL_ON,
		"followsong", "follow");

	definecmd(cmds, CMD_DIGIT0,
		psy_ui_KEY_DIGIT0, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit0", "0");
	definecmd(cmds, CMD_DIGIT1,
		psy_ui_KEY_DIGIT1, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit1", "1");
	definecmd(cmds, CMD_DIGIT2,
		psy_ui_KEY_DIGIT2, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit2", "2");
	definecmd(cmds, CMD_DIGIT3,
		psy_ui_KEY_DIGIT3, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit3", "3");
	definecmd(cmds, CMD_DIGIT4,
		psy_ui_KEY_DIGIT4, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit4", "4");
	definecmd(cmds, CMD_DIGIT5,
		psy_ui_KEY_DIGIT5, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit5", "5");
	definecmd(cmds, CMD_DIGIT6,
		psy_ui_KEY_DIGIT6, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit6", "6");
	definecmd(cmds, CMD_DIGIT7,
		psy_ui_KEY_DIGIT7, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit7", "7");
	definecmd(cmds, CMD_DIGIT8,
		psy_ui_KEY_DIGIT8, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit8", "8");
	definecmd(cmds, CMD_DIGIT9,
		psy_ui_KEY_DIGIT9, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digit9", "9");
	definecmd(cmds, CMD_DIGITA,
		psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digitA", "A");
	definecmd(cmds, CMD_DIGITB,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digitB", "B");
	definecmd(cmds, CMD_DIGITC,
		psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digitC", "C");
	definecmd(cmds, CMD_DIGITD,
		psy_ui_KEY_D, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digitD", "D");
	definecmd(cmds, CMD_DIGITE,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digitE", "E");
	definecmd(cmds, CMD_DIGITF,
		psy_ui_KEY_F, psy_SHIFT_OFF, psy_CTRL_OFF,
		"digitF", "F");	
}

// Defines a property with shortcut defaults for the keyboard driver
// key		: cmd id used by the trackerview
// text		: "cmds.key" language dictionary key used by the translator
// shorttext: short description for the keyboard help view
// value	: encoded key shortcut (keycode/shift/ctrl)
void definecmd(psy_Property* cmds, int cmd, uintptr_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint( psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(cmds, key,
		psy_audio_encodeinput(keycode, shift, ctrl), 0, 0),
		cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}

void trackerview_setpattern(TrackerView* self, psy_audio_Pattern* pattern)
{	
	trackergrid_setpattern(&self->grid, pattern);
	interpolatecurveview_setpattern(&self->interpolatecurveview, pattern);	
}

void trackerview_oninterpolatecurve(TrackerView* self, psy_ui_Component* sender)
{
	psy_ui_component_togglevisibility(&self->interpolatecurveview.component);
}
