// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackerview.h"
// local
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

// TrackerLineState
// Prototypes
static double trackerlinestate_offset(TrackerLineState*, int y, unsigned int* lines);
static int trackerlinestate_numlines(TrackerLineState*);
// implementation
void trackerlinestate_init(TrackerLineState* self)
{
	self->lineheight = 13;
	self->lpb = 4;
	self->skin = NULL;
	self->pattern = NULL;	
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	self->drawcursor = TRUE;
	self->visilines = 25;
	self->cursorchanging = FALSE;
}

int trackerlinestate_offsettoscreenline(TrackerLineState* self,
	psy_dsp_big_beat_t offset)
{
	return (int)(offset * self->lpb);
}

psy_dsp_big_beat_t trackerlinestate_offset(TrackerLineState* self, int y, unsigned int* lines)
{
	if (self->pattern) {
		*lines = psy_max(0, y) / self->lineheight;
		return (*lines) * (1.0 / self->lpb);
	}
	*lines = 0;
	return 0;
}

int trackerlinestate_testplaybar(TrackerLineState* self, psy_dsp_big_beat_t offset)
{
	return psy_dsp_testrange(self->lastplayposition - self->sequenceentryoffset,
		offset, 1.0 / self->lpb);
}

int trackerlinestate_numlines(TrackerLineState* self)
{
	return (self->pattern)
		? (int)(psy_audio_pattern_length(self->pattern) * self->lpb)
		: 0;
}

// TrackerConfig
// prototypes
static void trackconfig_initcolumns(TrackConfig*, bool wideinst);
// implementation
void trackconfig_init(TrackConfig* self, bool wideinst)
{
	self->textwidth = 9;
	self->textleftedge = 2;
	self->textleftedge = 2;
	self->patterntrackident = 0;
	self->headertrackident = 0;
	psy_table_init(&self->trackconfigs);
	trackconfig_initcolumns(self, wideinst);
}

void trackconfig_dispose(TrackConfig* self)
{
	psy_table_disposeall(&self->trackconfigs, (psy_fp_disposefunc)NULL);
}

void trackconfig_initcolumns(TrackConfig* self, bool wideinst)
{
	trackdef_init(&self->trackdef);
	if (wideinst) {
		self->trackdef.inst.numdigits = 4;
		self->trackdef.inst.numchars = 4;
	}
}

// TrackerGridState
// implementation
void trackergridstate_init(TrackerGridState* self, TrackConfig* trackconfig)
{
	self->zoom = 1.0;
	self->skin = NULL;
	self->pattern = NULL;	
	self->numtracks = 16;
	self->trackconfig = trackconfig;
	psy_audio_patterncursor_init(&self->cursor);
}

void trackergridstate_dispose(TrackerGridState* self)
{	
}

uintptr_t trackergridstate_paramcol(TrackerGridState* self, uintptr_t track, int x)
{
	uintptr_t rv;
	uintptr_t trackx;
	uintptr_t maincolumnwidth;
	TrackDef* trackdef;

	trackdef = trackergridstate_trackdef(self, track);
	trackx = trackergridstate_track_x(self, track);
	maincolumnwidth = trackergridstate_basewidth(self, track);
	rv = (x - (trackx + maincolumnwidth)) / (4 * self->trackconfig->textwidth);
	return rv;
}

// trackdef
static uintptr_t trackdef_numdigits(TrackDef*, uintptr_t column);
static int trackdef_columnwidth(TrackDef*, int column, int textwidth);
static TrackColumnDef* trackdef_columndef(TrackDef*, int column);
static uintptr_t trackdef_numcolumns(TrackDef*);
static uintptr_t trackdef_value(TrackDef*, uintptr_t column,
	const psy_audio_PatternEntry*);
static uintptr_t trackdef_emptyvalue(TrackDef*, uintptr_t column);
static void trackdef_setvalue(TrackDef*, uintptr_t column,
	psy_audio_PatternEntry*, uintptr_t value);
static int trackdef_width(TrackDef*, int textwidth);
static int trackdef_basewidth(TrackDef* self, int textwidth);

enum {
	CMD_NAVUP = 1024,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_NAVPAGEUP,	///< pgup
	CMD_NAVPAGEDOWN,///< pgdn
	CMD_NAVTOP,		///< home
	CMD_NAVBOTTOM,	///< end

	CMD_COLUMNPREV,	///< tab
	CMD_COLUMNNEXT,	///< s-tab

	CMD_BLOCKSTART,
	CMD_BLOCKEND,
	CMD_BLOCKUNMARK,
	CMD_BLOCKCUT,
	CMD_BLOCKCOPY,
	CMD_BLOCKPASTE,
	CMD_BLOCKMIX,

	CMD_ROWINSERT,
	CMD_ROWDELETE,
	CMD_ROWCLEAR,

	CMD_TRANSPOSEBLOCKINC,
	CMD_TRANSPOSEBLOCKDEC,
	CMD_TRANSPOSEBLOCKINC12,
	CMD_TRANSPOSEBLOCKDEC12,

	CMD_SELECTALL,
	CMD_SELECTCOL,
	CMD_SELECTBAR,

	CMD_SELECTMACHINE,	///< Enter
	CMD_UNDO,
	CMD_REDO,

	CMD_FOLLOWSONG,	
	CMD_DIGIT0,
	CMD_DIGIT1,
	CMD_DIGIT2,
	CMD_DIGIT3,
	CMD_DIGIT4,
	CMD_DIGIT5,
	CMD_DIGIT6,
	CMD_DIGIT7,
	CMD_DIGIT8,
	CMD_DIGIT9,
	CMD_DIGITA,
	CMD_DIGITB,
	CMD_DIGITC,
	CMD_DIGITD,
	CMD_DIGITE,
	CMD_DIGITF
};

// TrackColumnDef
// implementation
void trackcolumndef_init(TrackColumnDef* self, int numdigits, int numchars,
	int marginright, int wrapeditcolumn, int wrapclearcolumn, int emptyvalue)
{
	self->numdigits = numdigits;
	self->numchars = numchars;
	self->marginright = marginright;
	self->wrapeditcolumn = wrapeditcolumn;
	self->wrapclearcolumn = wrapclearcolumn;
	self->emptyvalue = emptyvalue;
}

// TrackDef
void trackdef_init(TrackDef* self)
{
	trackcolumndef_init(&self->note, 1, 3, 1,
		TRACKER_COLUMN_NOTE, TRACKER_COLUMN_NOTE, 0xFF);
	trackcolumndef_init(&self->inst, 2, 2, 1,
		TRACKER_COLUMN_INST, TRACKER_COLUMN_INST, psy_audio_NOTECOMMANDS_INST_EMPTY);
	trackcolumndef_init(&self->mach, 2, 2, 1,
		TRACKER_COLUMN_MACH, TRACKER_COLUMN_MACH, psy_audio_NOTECOMMANDS_MACH_EMPTY);
	trackcolumndef_init(&self->vol, 2, 2, 1,
		TRACKER_COLUMN_VOL, TRACKER_COLUMN_VOL, psy_audio_NOTECOMMANDS_VOL_EMPTY);
	trackcolumndef_init(&self->cmd, 2, 2, 0,
		TRACKER_COLUMN_NONE, TRACKER_COLUMN_CMD, 0x00);
	trackcolumndef_init(&self->param, 2, 2, 1,
		TRACKER_COLUMN_CMD, TRACKER_COLUMN_PARAM, 0x00);
	self->numfx = 1;
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
static void trackergrid_centeroncursor(TrackerGrid*);
static void trackergrid_setcentermode(TrackerGrid*, int mode);
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
static void trackergrid_storecursor(TrackerGrid* self);
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
static int trackergrid_scrollup(TrackerGrid*,
	psy_audio_PatternCursor);
static int trackergrid_scrolldown(TrackerGrid*,
	psy_audio_PatternCursor);
static int trackergrid_scrollleft(TrackerGrid*,
	psy_audio_PatternCursor);
static int trackergrid_scrollright(TrackerGrid*,
	psy_audio_PatternCursor);
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

void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
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

// TrackerHeader
// prototypes
static int trackerheader_preferredtrackwidth(TrackerHeader*);
static void trackerheader_ondraw(TrackerHeader*, psy_ui_Graphics*);
static void trackerheader_drawtrackseparator(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track);
static void trackerheader_drawtrackbackground(TrackerHeader*, psy_ui_Graphics*, int x);
static void trackerheader_drawtrackplayon(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track);
static void trackerheader_drawtracknumber(TrackerHeader*, psy_ui_Graphics*, int x, uintptr_t track);
static void trackerheader_drawtrackleds(TrackerHeader*, psy_ui_Graphics*, int x, uintptr_t track);
static void trackerheader_drawtrackselection(TrackerHeader*, psy_ui_Graphics*, int x, uintptr_t track);
static void trackerheader_onmousedown(TrackerHeader*, psy_ui_MouseEvent*);
static void trackerheader_onpreferredsize(TrackerHeader*, psy_ui_Size* limit, psy_ui_Size* rv);
static void trackerheader_onpatterncursorchanged(TrackerHeader*,
	Workspace*);
// vtable
static psy_ui_ComponentVtable trackerheader_vtable;
static int trackerheader_vtable_initialized = 0;

static void trackerheader_vtable_init(TrackerHeader* self)
{
	if (!trackerheader_vtable_initialized) {
		trackerheader_vtable = *(self->component.vtable);
		trackerheader_vtable.ondraw = (psy_ui_fp_component_ondraw)trackerheader_ondraw;
		trackerheader_vtable.onmousedown = (psy_ui_fp_component_onmousedown)trackerheader_onmousedown;
		trackerheader_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackerheader_onpreferredsize;
		trackerheader_vtable_initialized = 1;
	}
}
// implementation
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerGridState* gridstate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerheader_vtable_init(self);
	self->component.vtable = &trackerheader_vtable;	
	trackerheader_setsharedgridstate(self, gridstate, trackconfig);
	//psy_ui_component_setbackgroundcolour(&self->component, view->skin.background);	
	psy_ui_component_doublebuffer(&self->component);
	self->gridstate->numtracks = 16;
	self->classic = 1;
	self->workspace = workspace;
	self->currtrack = 0;
	self->playon = FALSE;
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerheader_onpatterncursorchanged);
}

void trackerheader_setsharedgridstate(TrackerHeader* self, TrackerGridState*
	gridstate, TrackConfig* trackconfig)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		trackergridstate_init(&self->defaultgridstate, trackconfig);
		self->gridstate = &self->defaultgridstate;
	}
}

void trackerheader_ondraw(TrackerHeader* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int cpx;
	int centerx;
	uintptr_t track;	

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_drawsolidrectangle(g,
		psy_ui_rectangle_make(psy_ui_component_scrollleft(&self->component), 0,
			psy_ui_value_px(&size.width, &tm), psy_ui_value_px(&size.height, &tm)),
		self->gridstate->skin->background);
	for (track = 0, cpx = 0; track < self->gridstate->numtracks; ++track) {
		centerx = psy_max(0, (trackergridstate_trackwidth(self->gridstate, track) -
			self->gridstate->skin->headercoords.background.destwidth) / 2);
		trackerheader_drawtrackseparator(self, g, cpx, track);
		trackerheader_drawtrackbackground(self, g, cpx + centerx);
		trackerheader_drawtrackplayon(self, g, cpx + centerx, track);
		trackerheader_drawtracknumber(self, g, cpx + centerx, track);
		trackerheader_drawtrackleds(self, g, cpx + centerx, track);
		trackerheader_drawtrackselection(self, g, cpx + centerx, track);
		cpx += trackergridstate_trackwidth(self->gridstate, track);
	}
}

void trackerheader_drawtrackseparator(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track)
{
	int trackwidth;
	psy_ui_Size size;
	psy_ui_TextMetric tm;		

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	trackwidth = trackergridstate_trackwidth(self->gridstate, track);
	psy_ui_setcolour(g, self->gridstate->skin->separator);
	psy_ui_drawline(g, x + trackwidth - 1, 0, x + trackwidth - 1,
		psy_ui_value_px(&size.height, &tm));	
}

void trackerheader_drawtrackbackground(TrackerHeader* self, psy_ui_Graphics* g, int x)
{		
	skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
		&self->gridstate->skin->headercoords.background);
}

void trackerheader_drawtrackplayon(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track)
{	
	if (psy_audio_activechannels_playon(&self->workspace->player.playon, track)) {
		skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
			&self->gridstate->skin->headercoords.dplayon);
		self->playon = TRUE;
	}
}

void trackerheader_drawtracknumber(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track)
{
	int trackx0 = track / 10;
	int track0x = track % 10;
	SkinCoord digitx0 = self->gridstate->skin->headercoords.digitx0;
	SkinCoord digit0x = self->gridstate->skin->headercoords.digit0x;
	digitx0.srcx += trackx0 * digitx0.srcwidth;
	digit0x.srcx += track0x * digit0x.srcwidth;
	skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0, &digitx0);
	skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0, &digit0x);
}

void trackerheader_drawtrackleds(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track)
{
	if (self->workspace->song) {
		if (psy_audio_patterns_istrackmuted(&self->workspace->song->patterns,
			track)) {
			skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
				&self->gridstate->skin->headercoords.mute);
		}
		if (psy_audio_patterns_istracksoloed(&self->workspace->song->patterns,
			track)) {
			skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
				&self->gridstate->skin->headercoords.solo);
		}
	}
}

void trackerheader_drawtrackselection(TrackerHeader* self, psy_ui_Graphics* g, int x, uintptr_t track)
{
	psy_audio_PatternCursor editposition;

	editposition = workspace_patterncursor(self->workspace);
	if (track == editposition.track) {
		psy_ui_setcolour(g, self->gridstate->skin->font);
		psy_ui_drawline(g, x, 0,
			psy_min(x + self->gridstate->skin->headercoords.background.destwidth,
				x + trackergridstate_trackwidth(self->gridstate, track)), 0);
	}
}

void trackerheader_onmousedown(TrackerHeader* self, psy_ui_MouseEvent* ev)
{
	if (self->workspace->song) {
		psy_ui_Rectangle r;
		uintptr_t track;
		int track_x;		
		int centerx;

		track = trackergridstate_screentotrack(self->gridstate,
			ev->x + psy_ui_component_scrollleft(&self->component),
			psy_audio_player_numsongtracks(&self->workspace->player));		
		centerx = psy_max(0, (trackergridstate_trackwidth(self->gridstate, track) -
			self->gridstate->skin->headercoords.background.destwidth) / 2);
		track_x = trackergridstate_track_x(self->gridstate, track) + centerx;
		psy_ui_setrectangle(&r,
			self->gridstate->skin->headercoords.mute.destx + track_x,
			self->gridstate->skin->headercoords.mute.desty,
			self->gridstate->skin->headercoords.mute.destwidth,
			self->gridstate->skin->headercoords.mute.destheight);
		if (psy_ui_rectangle_intersect(&r, ev->x +
				psy_ui_component_scrollleft(&self->component), ev->y)) {
			if (psy_audio_patterns_istrackmuted(&self->workspace->song->patterns,
				track)) {
				psy_audio_patterns_unmutetrack(&self->workspace->song->patterns,
					track);
			} else {
				psy_audio_patterns_mutetrack(&self->workspace->song->patterns,
					track);
			}
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_setrectangle(&r,
			self->gridstate->skin->headercoords.solo.destx + track_x,
			self->gridstate->skin->headercoords.solo.desty,
			self->gridstate->skin->headercoords.solo.destwidth,
			self->gridstate->skin->headercoords.solo.destheight);
		if (psy_ui_rectangle_intersect(&r, ev->x +
				psy_ui_component_scrollleft(&self->component), ev->y)) {
			if (psy_audio_patterns_istracksoloed(&self->workspace->song->patterns,
				track)) {
				psy_audio_patterns_deactivatesolotrack(
					&self->workspace->song->patterns);
			} else {
				psy_audio_patterns_activatesolotrack(
					&self->workspace->song->patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		} else {
			psy_audio_PatternCursor cursor;

			cursor = workspace_patterncursor(self->workspace);
			cursor.track = track;
			workspace_setpatterncursor(self->workspace, cursor);
			++self->gridstate->pattern->opcount;
		}
	}
}

void trackerheader_onpreferredsize(TrackerHeader* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(trackergridstate_track_x(self->gridstate,
		self->gridstate->numtracks));
	rv->height = psy_ui_value_makepx(30);		
}

void trackerheader_onpatterncursorchanged(TrackerHeader* self,
	Workspace* sender)
{
	psy_audio_PatternCursor cursor;

	cursor = workspace_patterncursor(sender);
	if (self->currtrack != cursor.track) {
		self->currtrack = cursor.track;
		psy_ui_component_invalidate(&self->component);
	}
}

// TrackerLineNumbers
// prototypes
static void trackerlinenumbers_ondraw(TrackerLineNumbers*, psy_ui_Graphics*);
static void trackerlinenumbers_onpreferredsize(TrackerLineNumbers*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void trackerlinennumbers_onpatterncursorchanged(TrackerLineNumbers*,
	Workspace*);
static void trackerlinenumbers_onscroll(TrackerLineNumbers*, psy_ui_Component* sender);
// vtable
static psy_ui_ComponentVtable trackerlinenumbers_vtable;
static int trackerlinenumbers_vtable_initialized = 0;

static void trackerlinenumbers_vtable_init(TrackerLineNumbers* self)
{
	if (!trackerlinenumbers_vtable_initialized) {
		trackerlinenumbers_vtable = *(self->component.vtable);
		trackerlinenumbers_vtable.ondraw = (psy_ui_fp_component_ondraw)trackerlinenumbers_ondraw;
		trackerlinenumbers_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackerlinenumbers_onpreferredsize;
		trackerlinenumbers_vtable_initialized = 1;
	}
}
// implementation
void trackerlinenumbers_init(TrackerLineNumbers* self, psy_ui_Component* parent,
	TrackerLineState* linestate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumbers_vtable_init(self);
	self->component.vtable = &trackerlinenumbers_vtable;
	self->workspace = workspace;
	trackerlinenumbers_setsharedlinestate(self, linestate);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setbackgroundcolour(&self->component,
		self->linestate->skin->background);
	psy_signal_connect(&self->component.signal_scroll, self,
		trackerlinenumbers_onscroll);
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerlinennumbers_onpatterncursorchanged);
	psy_audio_patterncursor_init(&self->lastcursor);	
}

void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers* self, TrackerLineState*
	linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, psy_ui_Graphics* g)
{
	if (self->linestate->pattern) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		char buffer[20];
		int cpy = 0;
		int line;
		double offset;
		double topoffset;
		double bottomoffset;
		int topline;
		int bottomline;
		psy_audio_PatternCursor cursor;
		char* linecountformat;
				
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		topoffset = trackerlinestate_offset(self->linestate, g->clip.top,
			&topline);
		bottomoffset = trackerlinestate_offset(self->linestate, g->clip.bottom,
			&bottomline);
		cpy = (topline) * self->linestate->lineheight;
		offset = topoffset;
		line = topline;
		cursor = workspace_patterncursor(self->workspace);
		if (workspace_showlinenumbersinhex(self->workspace)) {
			if (workspace_showbeatoffset(self->workspace)) {
				linecountformat = "%.2X %.3f";
			} else {
				linecountformat = "%.2X";
			}
		} else {
			if (workspace_showbeatoffset(self->workspace)) {
				linecountformat = "%3i %.3f";
			} else {
				linecountformat = "%3i";
			}
		}
		while (offset <= bottomoffset &&
			offset < self->linestate->pattern->length) {
			psy_ui_Rectangle r;
			TrackerColumnFlags columnflags;
			int ystart;
			int drawbeat;
			uintptr_t c;
			int flatsize;
			char digit[2];

			drawbeat = workspace_showbeatoffset(self->workspace);
			columnflags.playbar = psy_audio_player_playing(&self->workspace->player) && 
				trackerlinestate_testplaybar(self->linestate, offset);
			columnflags.mid = 0;
			columnflags.cursor = self->linestate->drawcursor && !self->linestate->cursorchanging &&
				testcursor(cursor, cursor.track, offset, self->linestate->lpb);
			columnflags.beat = fmod(offset, 1.0f) == 0.0f;
			columnflags.beat4 = fmod(offset, 4.0f) == 0.0f;
			columnflags.selection = 0;
			setcolumncolour(self->linestate->skin, g, columnflags, 0, 0);			
			psy_snprintf(buffer, 10, linecountformat, line, offset);
			flatsize = tm.tmAveCharWidth + 2;
			digit[1] = '\0';
			for (c = 0; c <= strlen(buffer); ++c) {				
				digit[0] = buffer[c];				
				r = psy_ui_rectangle_make(c * flatsize, cpy, flatsize,
					tm.tmHeight);
				psy_ui_textoutrectangle(g, r.left, r.top,
					psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
					digit, strlen(digit));
			}
			cpy += self->linestate->lineheight;
			ystart = cpy;
			offset += (1.0 / self->linestate->lpb);
			++line;
		}
	}
}

void trackerlinennumbers_onpatterncursorchanged(TrackerLineNumbers* self,
	Workspace* sender)
{
	psy_audio_PatternCursor currcursor;

	currcursor = workspace_patterncursor(sender);
	if (self->lastcursor.offset != currcursor.offset) {
		if (!self->linestate->cursorchanging) {
			trackerlinenumbers_invalidatecursor(self, &self->lastcursor);
		}
		self->linestate->cursorchanging = FALSE;
		trackerlinenumbers_invalidatecursor(self, &currcursor);		
	}
	self->lastcursor = currcursor;
}

void trackerlinenumbers_onscroll(TrackerLineNumbers* self, psy_ui_Component* sender)
{	
	if (self->linestate->cursorchanging) {
		trackerlinenumbers_invalidatecursor(self, &self->lastcursor);
	}
}

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers* self,
	const psy_audio_PatternCursor* cursor)
{	
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	int line;
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	line = trackerlinestate_offsettoscreenline(self->linestate,
		cursor->offset);
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_rectangle_make(
			0, self->linestate->lineheight * line,
			size.width, self->linestate->lineheight));
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, psy_dsp_big_beat_t offset)
{		
	if (self->linestate->pattern) {
		if (offset >= self->linestate->sequenceentryoffset &&
				offset < self->linestate->sequenceentryoffset +
				self->linestate->pattern->length) {
			psy_ui_Size size;
			psy_ui_TextMetric tm;
			int line;

			size = psy_ui_component_size(&self->component);
			tm = psy_ui_component_textmetric(&self->component);
			line = (int)((offset - self->linestate->sequenceentryoffset) *
				self->linestate->lpb);
			psy_ui_component_invalidaterect(&self->component,
				psy_ui_rectangle_make(
					0,
					self->linestate->lineheight * line,
					psy_ui_value_px(&size.width, &tm),
					self->linestate->lineheight));
		}
	}
}

void trackerlinenumbers_onpreferredsize(TrackerLineNumbers* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = (workspace_showbeatoffset(self->workspace))
		? psy_ui_value_makeew(13)
		: psy_ui_value_makeew(6);
	rv->height = psy_ui_value_makepx(0);
}

// LineNumbersLabel
// prototypes
static void trackerlinenumberslabel_ondestroy(TrackerLineNumbersLabel*, psy_ui_Component* sender);
static void trackerlinenumberslabel_updatetext(TrackerLineNumbersLabel*, Translator*);
static void trackerlinenumberslabel_onlanguagechanged(TrackerLineNumbersLabel*, Translator*);
static void trackerlinenumberslabel_setsharedlinestate(TrackerLineNumbersLabel*,
	TrackerLineState*);
static void trackerlinenumberslabel_onmousedown(TrackerLineNumbersLabel*,
	psy_ui_MouseEvent*);
static void trackerlinenumberslabel_ondraw(TrackerLineNumbersLabel*, psy_ui_Graphics*);
static void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel*,
	psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable trackerlinenumberslabel_vtable;
static int trackerlinenumberslabel_vtable_initialized = 0;

static void trackerlinenumberslabel_vtable_init(TrackerLineNumbersLabel* self)
{
	if (!trackerlinenumberslabel_vtable_initialized) {
		trackerlinenumberslabel_vtable = *(self->component.vtable);
		trackerlinenumberslabel_vtable.ondraw = (psy_ui_fp_component_ondraw)
			trackerlinenumberslabel_ondraw;
		trackerlinenumberslabel_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			trackerlinenumberslabel_onmousedown;
		trackerlinenumberslabel_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackerlinenumberslabel_onpreferredsize;
	}
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerLineState* linestate, TrackerView* view,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumberslabel_vtable_init(self);
	self->component.vtable = &trackerlinenumberslabel_vtable;
	trackerlinenumberslabel_setsharedlinestate(self, linestate);
	self->view = view;
	self->linestr = NULL;
	self->defaultstr = NULL;
	trackerlinenumberslabel_updatetext(self, &workspace->translator);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		trackerlinenumberslabel_onlanguagechanged);
	psy_signal_connect(&self->component.signal_destroy, self,
		trackerlinenumberslabel_ondestroy);
}

void trackerlinenumberslabel_ondestroy(TrackerLineNumbersLabel* self, psy_ui_Component* sender)
{
	free(self->linestr);
	free(self->defaultstr);
}

void trackerlinenumberslabel_updatetext(TrackerLineNumbersLabel* self, Translator* translator)
{
	free(self->linestr);
	free(self->defaultstr);
	self->linestr = strdup(translator_translate(translator, "patternview.line"));
	self->defaultstr = strdup(translator_translate(translator, "patternview.defaults"));
}

void trackerlinenumberslabel_onlanguagechanged(TrackerLineNumbersLabel* self, Translator* sender)
{
	trackerlinenumberslabel_updatetext(self, sender);
}

void trackerlinenumberslabel_setsharedlinestate(TrackerLineNumbersLabel* self,
	TrackerLineState* linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
}

void trackerlinenumberslabel_onmousedown(TrackerLineNumbersLabel* self,
	psy_ui_MouseEvent* ev)
{
/*	self->view->header.classic = !self->view->header.classic;
	if (self->view->header.classic) {
		trackerview_setclassicheadercoords(self->view);
	} else {
		trackerview_setheadercoords(self->view);
	}*/
	psy_ui_component_invalidate(&self->view->header.component);
}

void trackerlinenumberslabel_ondraw(TrackerLineNumbersLabel* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;
	psy_ui_IntSize headersize;

	tm = psy_ui_component_textmetric(&self->view->header.component);
	size = psy_ui_component_size(&self->component);
	headersize = psy_ui_intsize_init_size(
		psy_ui_component_preferredsize(&self->view->header.component, &size), &tm);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setrectangle(&r, 0, 0, psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm));
	psy_ui_drawsolidrectangle(g, r, self->linestate->skin->background);
	psy_ui_setbackgroundcolour(g, self->linestate->skin->background);
	psy_ui_settextcolour(g, self->linestate->skin->font);
	psy_ui_textoutrectangle(g, r.left, 0, 0, r, self->linestr, strlen(self->linestr));
	if (self->view->showdefaultline) {
		psy_ui_setfont(g, psy_ui_component_font(
			&self->view->linenumbers.component));
		if ((workspace_showbeatoffset(self->view->workspace))) {
			psy_ui_textoutrectangle(g, r.left, headersize.height, 0,
				r, self->defaultstr, strlen(self->defaultstr));
		} else {
			psy_ui_textoutrectangle(g, r.left, headersize.height, 0,
				r, "Def", strlen("Def"));
		}
	}
}

void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	int height = 0;
	psy_ui_TextMetric tm;
	psy_ui_IntSize headersize;

	tm = psy_ui_component_textmetric(&self->view->header.component);
	headersize = psy_ui_intsize_init_size(
		psy_ui_component_preferredsize(&self->view->header.component, limit), &tm);
	height = headersize.height;
	if (self->view->showdefaultline) {		
		height += self->view->grid.linestate->lineheight;
	}	
	rv->height = psy_ui_value_makepx(height);
	rv->width = (workspace_showbeatoffset(self->view->workspace))
		? psy_ui_value_makeew(10)
		: psy_ui_value_makeew(5);
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

int trackerheader_preferredtrackwidth(TrackerHeader* self)
{
	return self->gridstate->skin->headercoords.background.destwidth;
}

int trackergridstate_track_x(TrackerGridState* self, uintptr_t track)
{
	int rv = 0;
	uintptr_t t;

	for (t = 0; t < track; ++t) {
		rv += trackergridstate_trackwidth(self, t);
	}
	return rv;
}

int trackergridstate_trackwidth(TrackerGridState* self, uintptr_t track)
{
	return trackdef_width(trackergridstate_trackdef(self, track),
		self->trackconfig->textwidth) + 1;
}

uintptr_t trackergridstate_screentotrack(TrackerGridState* self, int x, uintptr_t numsongtracks)
{
	int currx = 0;
	uintptr_t rv = 0;

	while (rv < numsongtracks) {
		currx += trackergridstate_trackwidth(self, rv);
		if (currx > x) {
			break;
		}
		++rv;
	}
	return rv;
}

uintptr_t trackergridstate_basewidth(TrackerGridState* self, uintptr_t track)
{
	return trackdef_basewidth(trackergridstate_trackdef(self, track),
		self->trackconfig->textwidth) + 1;
}

TrackDef* trackergridstate_trackdef(TrackerGridState* self, uintptr_t track)
{
	TrackDef* rv;

	rv = psy_table_at(&self->trackconfig->trackconfigs, track);
	if (!rv) {
		rv = &self->trackconfig->trackdef;
	}
	return rv;
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


uintptr_t trackdef_numdigits(TrackDef* self, uintptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return (coldef)
		? coldef->numdigits
		: 0;
}

uintptr_t trackdef_numcolumns(TrackDef* self)
{
	return 4 + self->numfx * 2;
}

void trackdef_setvalue(TrackDef* self, uintptr_t column,
	psy_audio_PatternEntry* entry, uintptr_t value)
{
	if (column < TRACKER_COLUMN_CMD) {
		switch (column) {
			case TRACKER_COLUMN_NOTE:
				psy_audio_patternentry_front(entry)->note = value;
				break;
			case TRACKER_COLUMN_INST:				
				if ((self->inst.numchars == 2) && value == 0xFF) {
					// this also clears the high byte of the 16bit instrument
					// value, if only two digits are visible
					// (settings wideinstrumentcolum: off)
					psy_audio_patternentry_front(entry)->inst = 0xFFFF;
				} else {
					psy_audio_patternentry_front(entry)->inst = value;
				}
				break;
			case TRACKER_COLUMN_MACH:
				psy_audio_patternentry_front(entry)->mach = value;
				break;
			case TRACKER_COLUMN_VOL:
				psy_audio_patternentry_front(entry)->vol = value;
				break;
			default:
				break;
		}
	} else {
		int c;
		int num;
		psy_List* p;

		column = column - 4;
		num = column / 2;
		c = 0;
		p = entry->events;
		while (c <= num) {
			if (!p) {
				psy_audio_PatternEvent ev;

				psy_audio_patternevent_clear(&ev);
				psy_audio_patternentry_addevent(entry, &ev);
				p = (entry->events)
					? entry->events->tail
					: NULL;
			}
			if (c == num) {
				break;
			}
			if (p) {
				p = p->next;
			}
			++c;
		}
		if (p) {
			psy_audio_PatternEvent* event;

			event = (psy_audio_PatternEvent*)p->entry;
			assert(event);
			if ((column % 2) == 0) {
				event->cmd = value;
			} else {
				event->parameter = value;
			}
		}
	}
}

uintptr_t trackdef_value(TrackDef* self, uintptr_t column,
	const psy_audio_PatternEntry* entry)
{
	uintptr_t rv;

	if (column < TRACKER_COLUMN_CMD) {
		switch (column) {
			case TRACKER_COLUMN_NOTE:
				rv = psy_audio_patternentry_front_const(entry)->note;
				break;
			case TRACKER_COLUMN_INST:
				rv = psy_audio_patternentry_front_const(entry)->inst;
				break;
			case TRACKER_COLUMN_MACH:
				rv = psy_audio_patternentry_front_const(entry)->mach;
				break;
			case TRACKER_COLUMN_VOL:
				rv = psy_audio_patternentry_front_const(entry)->vol;
				break;
			default:
				rv = 0;
				break;
		}
	} else {
		int c;
		int num;
		psy_List* p;

		column = column - 4;
		num = column / 2;
		c = 0;
		p = entry->events;
		while (p && c < num) {
			p = p->next;
			++c;
		}
		if (p) {
			psy_audio_PatternEvent* ev;

			ev = (psy_audio_PatternEvent*)p->entry;
			assert(ev);
			if ((column % 2) == 0) {
				rv = ev->cmd;
			} else {
				rv = ev->parameter;
			}
		} else {
			rv = 0;
		}
	}
	return rv;
}

uintptr_t trackdef_emptyvalue(TrackDef* self, uintptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->emptyvalue : 0;
}

int trackdef_basewidth(TrackDef* self, int textwidth)
{
	int rv = 0;
	uintptr_t column;

	for (column = 0; column < 4; ++column) {
		rv += trackdef_columnwidth(self, column, textwidth);
	}
	return rv;
}

int trackdef_width(TrackDef* self, int textwidth)
{
	int rv = 0;
	uintptr_t column;

	for (column = 0; column < trackdef_numcolumns(self); ++column) {
		rv += trackdef_columnwidth(self, column, textwidth);
	}
	return rv;
}

int trackdef_columnwidth(TrackDef* self, int column, int textwidth)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->numchars * textwidth + coldef->marginright : 0;
}

TrackColumnDef* trackdef_columndef(TrackDef* self, int column)
{
	TrackColumnDef* rv;

	if (column >= TRACKER_COLUMN_CMD) {
		if (column % 2 == 0) {
			column = TRACKER_COLUMN_CMD;
		} else {
			column = TRACKER_COLUMN_PARAM;
		}
	}
	switch (column) {
		case TRACKER_COLUMN_NOTE:
			rv = &self->note;
			break;
		case TRACKER_COLUMN_INST:
			rv = &self->inst;
			break;
		case TRACKER_COLUMN_MACH:
			rv = &self->mach;
			break;
		case TRACKER_COLUMN_VOL:
			rv = &self->vol;
			break;
		case TRACKER_COLUMN_CMD:
			rv = &self->cmd;
			break;
		case TRACKER_COLUMN_PARAM:
			rv = &self->param;
			break;
		default:
			rv = 0;
			break;
	}
	return rv;
}

// TrackerView
// prototypes
static void trackerview_computefontheight(TrackerView*);
static void trackerview_initblockmenu(TrackerView*);
static void trackerview_connectblockmenu(TrackerView*);
static void trackerview_connectplayer(TrackerView*);
static void trackerview_connectworkspace(TrackerView*);
static void trackerview_ondestroy(TrackerView*, psy_ui_Component* sender);
static void trackerview_onmousedown(TrackerView*, psy_ui_MouseEvent*);
static void trackerview_onmouseup(TrackerView*, psy_ui_MouseEvent*);
static void trackerview_onkeydown(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_onkeyup(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_ongridscroll(TrackerView*, psy_ui_Component* sender);
static void trackerview_onzoomboxchanged(TrackerView*, ZoomBox* sender);
static void trackerview_ontimer(TrackerView*, psy_ui_Component* sender,
	uintptr_t timerid);
static void trackerview_numtrackschanged(TrackerView* self, psy_audio_Player*,
	uintptr_t numsongtracks);
static bool trackerview_handlecommand(TrackerView*, psy_ui_KeyEvent*, int cmd);
static int trackerview_pgupdownstep(TrackerView*);
static void trackerview_onalign(TrackerView*);
static void trackerview_togglefollowsong(TrackerView*);
static void trackerview_showlinenumbers(TrackerView*, int showstate);
static void trackerview_showlinenumbercursor(TrackerView*, int showstate);
static void trackerview_showlinenumbersinhex(TrackerView*, int showstate);
static void trackerview_onconfigurationchanged(TrackerView*, Workspace*,
	psy_Property* configurtion);
static void trackerview_readconfiguration(TrackerView*);
static void trackerview_readpgupdowntype(TrackerView*);
static void trackerview_readfont(TrackerView*);
static void trackerview_readtheme(TrackerView*);
static void trackerview_updateksin(TrackerView*);
static void trackerview_onpatternimport(TrackerView*);
static void trackerview_onpatternexport(TrackerView*);
static void trackerview_onlpbchanged(TrackerView*, psy_audio_Player* sender,
	uintptr_t lpb);
static void trackerview_onpatterncursorchanged(TrackerView*,
	Workspace* sender);
static void trackerview_onparametertweak(TrackerView*,
	Workspace* sender, int slot, uintptr_t tweak, float value);
static void trackerview_onskinchanged(TrackerView*, Workspace*);
static void trackerview_oninterpolatecurve(TrackerView*,
	psy_ui_Component* sender);
static void trackerview_oninterpolatecurveviewoncancel(TrackerView*,
	InterpolateCurveView* sender);
static void trackerview_oncolresize(TrackerView*, TrackerGrid* sender);
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
	psy_ui_Component* patternview,
	PatternViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerview_vtable_init(self);
	self->component.vtable = &trackerview_vtable;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		trackerview_ondestroy);
	psy_signal_connect(&self->component.signal_timer, self,
		trackerview_ontimer);
	trackerview_computefontheight(self);
	self->patternview = patternview;
	self->workspace = workspace;
	self->showlinenumbers = 1;
	self->showlinenumbercursor = 1;
	self->showlinenumbersinhex = 1;	
	self->showdefaultline = 1;
	self->pgupdownstep = 4;
	self->pgupdownbeat = TRUE;
	self->pgupdown4beat = FALSE;
	// trackconfig
	trackconfig_init(&self->trackconfig, workspace_showwideinstcolumn(workspace) != FALSE);
	// shared states
	trackerlinestate_init(&self->linestate);
	self->linestate.skin = skin;
	trackergridstate_init(&self->gridstate, &self->trackconfig);
	self->gridstate.skin = skin;
	// Interpolate View
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0, 0, 0, workspace);
	psy_ui_component_setalign(&self->interpolatecurveview.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);
	psy_signal_connect(&self->interpolatecurveview.signal_cancel, self,
		trackerview_oninterpolatecurveviewoncancel);
	// left bar
	psy_ui_component_init(&self->left, patternview);
	psy_ui_component_enablealign(&self->left);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->left, &self->linestate, self,
		workspace);
	psy_ui_component_setalign(&self->linenumberslabel.component, psy_ui_ALIGN_TOP);
	trackerlinenumbers_init(&self->linenumbers, &self->left, &self->linestate,
		workspace);
	psy_ui_component_setalign(&self->linenumbers.component, psy_ui_ALIGN_CLIENT);
	zoombox_init(&self->zoombox, &self->left);
	psy_ui_component_setpreferredsize(&self->zoombox.component,
		psy_ui_size_make(psy_ui_value_makepx(0),
			psy_ui_value_makeeh(1)));
	psy_ui_component_setalign(&self->zoombox.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->zoombox.signal_changed, self,
		trackerview_onzoomboxchanged);
	// Context menu
	trackerview_initblockmenu(self);
	// top bar
	trackerheader_init(&self->header, patternview, &self->trackconfig,
		&self->gridstate, self->workspace);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// pattern default line
	trackergrid_init(&self->griddefaults, patternview, &self->trackconfig,
		NULL, &self->linestate, TRACKERGRID_EDITMODE_LOCAL, workspace);
	self->griddefaults.defaultgridstate.skin = skin;	
	psy_ui_component_setalign(&self->griddefaults.component, psy_ui_ALIGN_TOP);	
	self->griddefaults.columnresize = 1;
	trackergrid_setpattern(&self->griddefaults, &self->workspace->player.patterndefaults);
	// pattern main grid
	trackergrid_init(&self->grid, &self->component, &self->trackconfig,
		&self->gridstate, &self->linestate, TRACKERGRID_EDITMODE_SONG,
		workspace);
	psy_signal_connect(&self->griddefaults.signal_colresize, self,
		trackerview_oncolresize);
	psy_signal_connect(&self->grid.signal_colresize, self,
		trackerview_oncolresize);
	psy_ui_scroller_init(&self->scroller, &self->grid.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->grid.component.signal_scroll, self,
		trackerview_ongridscroll);
	trackerview_connectworkspace(self);
	trackerview_connectplayer(self);
	trackerview_readconfiguration(self);
	psy_signal_connect(&self->workspace->player.signal_numsongtrackschanged, self,
		trackerview_numtrackschanged);
	psy_signal_connect(&self->workspace->player.eventdrivers.signal_input, self,
		trackerview_oneventdriverinput);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackerview_ondestroy(TrackerView* self, psy_ui_Component* sender)
{
	trackergridstate_dispose(&self->gridstate);
	trackconfig_dispose(&self->trackconfig);
}

void trackerview_computefontheight(TrackerView* self)
{
	psy_ui_Font* font;

	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;

		fontinfo = psy_ui_font_fontinfo(font);
		self->zoomheightbase = fontinfo.lfHeight;
	} else {
		self->zoomheightbase = -16;
	}
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

void trackerview_connectplayer(TrackerView* self)
{
	psy_signal_connect(&self->workspace->player.signal_lpbchanged, self,
		trackerview_onlpbchanged);	
}

void trackerview_connectworkspace(TrackerView* self)
{
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		trackerview_onconfigurationchanged);
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerview_onpatterncursorchanged);
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		trackerview_onparametertweak);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		trackerview_onskinchanged);
}

void trackerview_onalign(TrackerView* self)
{
	trackerview_computemetrics(self);
	if (trackergrid_midline(&self->grid)) {
		trackergrid_centeroncursor(&self->grid);
	}
}

void trackerview_numtrackschanged(TrackerView* self, psy_audio_Player* player,
	uintptr_t numsongtracks)
{
	self->gridstate.numtracks = numsongtracks;
	self->griddefaults.gridstate->numtracks = numsongtracks;
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->griddefaults.component);
	psy_ui_component_invalidate(&self->header.component);
}

void trackerview_oninterpolatecurveviewoncancel(TrackerView* self,
	InterpolateCurveView* sender)
{
	trackerview_oninterpolatecurve(self, &self->grid.component);
}

void trackerview_oncolresize(TrackerView* self, TrackerGrid* sender)
{
	if (sender != &self->griddefaults) {
		psy_ui_component_invalidate(&self->griddefaults.component);
	}
	if (sender != &self->grid) {
		psy_ui_component_invalidate(&self->grid.component);
	}
	psy_ui_component_invalidate(&self->header.component);
}

void trackerview_computemetrics(TrackerView* self)
{
	psy_ui_Size gridsize;
	psy_ui_TextMetric gridtm;
	int trackwidth;

	gridsize = psy_ui_component_size(&self->grid.component);
	gridtm = psy_ui_component_textmetric(&self->grid.component);
	self->gridstate.trackconfig->textwidth = (int)(gridtm.tmAveCharWidth * 1.5) + 2;
	self->linestate.lineheight = gridtm.tmHeight + 1;
	trackwidth = psy_max(
		trackergrid_preferredtrackwidth(&self->grid),
		trackerheader_preferredtrackwidth(&self->header));
	self->trackconfig.patterntrackident = (trackwidth -
			trackergrid_preferredtrackwidth(&self->grid)) / 2;
	if (self->trackconfig.patterntrackident < 0) {
		self->trackconfig.patterntrackident = 0;
	}
	self->trackconfig.headertrackident = 0;
	self->linestate.visilines = psy_ui_value_px(&gridsize.height, &gridtm) /
		self->linestate.lineheight;
}

void trackerview_setfont(TrackerView* self, psy_ui_Font* font, bool iszoombase)
{
	psy_ui_component_setfont(&self->component, font);
	psy_ui_component_setfont(&self->griddefaults.component, font);
	psy_ui_component_setfont(&self->grid.component, font);
	psy_ui_component_setfont(&self->linenumbers.component, font);
	if (iszoombase) {
		trackerview_computefontheight(self);
	}
	trackerview_computemetrics(self);
	trackerview_updatescrollstep(self);
	psy_ui_component_align(&self->component);
}

void trackerview_updatescrollstep(TrackerView* self)
{
	int scrollstepx;
	
	scrollstepx = trackergridstate_trackwidth(&self->gridstate,
		trackergridstate_track_x(&self->gridstate,
			trackergridstate_screentotrack(&self->gridstate,
				psy_ui_component_scrollleft(&self->grid.component),
				self->gridstate.numtracks)));
	self->grid.component.scrollstepx = scrollstepx;
	self->griddefaults.component.scrollstepx = scrollstepx;
	self->header.component.scrollstepx = scrollstepx;
	self->grid.component.scrollstepy = self->linestate.lineheight;	
	self->linenumbers.component.scrollstepy = self->linestate.lineheight;
}

void trackerview_onpatterncursorchanged(TrackerView* self, Workspace* sender)
{
	psy_audio_PatternCursor oldcursor;

	oldcursor = self->gridstate.cursor;
	self->gridstate.cursor = workspace_patterncursor(sender);
	if (!psy_audio_patterncursor_equal(&self->gridstate.cursor, &oldcursor)) {
		if (psy_audio_player_playing(&sender->player) && workspace_followingsong(sender)) {
			bool scrolldown;

			scrolldown = self->linestate.lastplayposition <
				psy_audio_player_position(&self->workspace->player);
			trackergrid_invalidateline(&self->grid, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = psy_audio_player_position(&self->workspace->player);
			trackergrid_invalidateline(&self->grid, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
				self->linestate.lastplayposition);
			if (self->linestate.lastplayposition >= self->linestate.sequenceentryoffset &&
				self->linestate.lastplayposition < self->linestate.sequenceentryoffset +
				self->gridstate.pattern->length) {
				if (scrolldown != FALSE) {
					trackergrid_scrolldown(&self->grid, self->gridstate.cursor);
				} else {
					trackergrid_scrollup(&self->grid, self->gridstate.cursor);
				}
			}
			trackergrid_storecursor(&self->grid);
		} else if (self->grid.midline) {
			trackergrid_centeroncursor(&self->grid);			
		}
	}
}

void trackerview_onskinchanged(TrackerView* self, Workspace* sender)
{
	trackerview_updateksin(self);
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
			&InsertCommandAlloc(self->gridstate.pattern, self->grid.bpl,
				self->gridstate.cursor, event, self->workspace)->command);
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
	if (self->gridstate.pattern) {
		psy_ui_OpenDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.psb";

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", filter, "PSB",
			workspace_songs_directory(self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(self->gridstate.pattern,
				psy_ui_opendialog_filename(&dialog),
				1.f / psy_audio_player_lpb(&self->workspace->player));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void trackerview_onpatternexport(TrackerView* self)
{
	if (self->gridstate.pattern) {
		psy_ui_SaveDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.PSB";

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", filter, "PSB",
			workspace_songs_directory(self->workspace));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(self->gridstate.pattern,
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

void trackerview_onzoomboxchanged(TrackerView* self, ZoomBox* sender)
{
	psy_ui_Font* font;

	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font newfont;

		fontinfo = psy_ui_font_fontinfo(font);
		fontinfo.lfHeight = (int)(self->zoomheightbase * zoombox_rate(sender));
		psy_ui_font_init(&newfont, &fontinfo);
		trackerview_setfont(self, &newfont, FALSE);
		psy_ui_font_dispose(&newfont);
		self->gridstate.zoom = zoombox_rate(sender);
		trackerview_computemetrics(self);
		psy_ui_component_align(&self->left);
		psy_ui_component_align(self->patternview);
		psy_ui_component_updateoverflow(&self->grid.component);
		psy_ui_component_invalidate(&self->grid.component);
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_invalidate(&self->linenumbers.component);
		psy_ui_component_invalidate(&self->linenumberslabel.component);		
	}
}

void trackerview_updateksin(TrackerView* self)
{	
	psy_ui_component_setbackgroundcolour(&self->linenumbers.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->blockmenu.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->interpolatecurveview.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setcolour(&self->interpolatecurveview.component,
		patternviewskin_fontcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->scroller.hscroll.sliderpane.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->scroller.vscroll.sliderpane.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_invalidate(&self->component);
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
	if ((ev->keycode == psy_ui_KEY_DOWN) &&
		(ev->target == &self->griddefaults.component)) {
		psy_ui_component_setfocus(&self->grid.component);
		psy_ui_component_invalidate(&self->grid.component);
		psy_ui_keyevent_stoppropagation(ev);
	} else
		if (ev->keycode == psy_ui_KEY_ESCAPE) {
			if (psy_ui_component_visible(&self->interpolatecurveview.component)) {
				trackerview_oninterpolatecurve(self, &self->component);
			} else {
				if (psy_ui_component_visible(&self->blockmenu.component)) {
					trackerview_toggleblockmenu(self);
				}
			}
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
			if (self->gridstate.cursor.column != TRACKER_COLUMN_NOTE) {
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
		self->gridstate.cursor.track = self->grid.chordbegin;
		trackergrid_scrollleft(&self->grid, self->gridstate.cursor);
		trackergrid_advanceline(&self->grid);
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void trackerview_ongridscroll(TrackerView* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleft(&self->grid.component) !=
			psy_ui_component_scrollleft(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->grid.component));
		psy_ui_component_setscrollleft(&self->griddefaults.component,
			psy_ui_component_scrollleft(&self->grid.component));
	}
	if (psy_ui_component_scrolltop(&self->grid.component) !=
			psy_ui_component_scrolltop(&self->linenumbers.component)) {
		psy_ui_component_setscrolltop(&self->linenumbers.component,
			psy_ui_component_scrolltop(&self->grid.component));
	}
}

void trackerview_ontimer(TrackerView* self, psy_ui_Component* sender, uintptr_t timerid)
{
	if (self->gridstate.pattern) {
		if (psy_audio_player_playing(&self->workspace->player)) {
			if (!workspace_followingsong(self->workspace)) {
				trackergrid_invalidateline(&self->grid, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->linestate.lastplayposition);
				self->linestate.lastplayposition =
					psy_audio_player_position(&self->workspace->player);
				trackergrid_invalidateline(&self->grid, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->linestate.lastplayposition);				
			}
			psy_ui_component_invalidate(&self->header.component);
		} else if (self->linestate.lastplayposition != -1) {
			trackergrid_invalidateline(&self->grid,
				self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = -1;
		}
		if (self->gridstate.pattern && self->gridstate.pattern->opcount != self->grid.opcount &&
			self->grid.syncpattern) {
			psy_ui_component_invalidate(&self->grid.component);
			psy_ui_component_invalidate(&self->linenumbers.component);
		}
		if (!psy_audio_player_playing(&self->workspace->player) && self->header.playon) {
			psy_ui_component_invalidate(&self->header.component);
			self->header.playon = FALSE;
		}
		self->grid.opcount = (self->gridstate.pattern)
			? self->gridstate.pattern->opcount
			: 0;
	}
}

void trackerview_onlpbchanged(TrackerView* self, psy_audio_Player* sender, uintptr_t lpb)
{
	self->linestate.lpb = lpb;
	self->grid.bpl = 1 / (psy_dsp_big_beat_t)lpb;	
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->linenumbers.component);
}

void trackerview_onconfigurationchanged(TrackerView* self, Workspace*
	workspace, psy_Property* property)
{
	if (property == &workspace->config) {
		trackerview_readconfiguration(self);
	} else if (psy_property_insection(property, workspace->patternviewtheme)) {
		trackerview_updateksin(self);
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
	} else if (strcmp(psy_property_key(property), "griddefaults") == 0) {
		self->showdefaultline = psy_property_item_int(property);
		if (psy_ui_component_visible(&self->griddefaults.component) != self->showdefaultline) {
			psy_ui_component_togglevisibility(&self->griddefaults.component);
			psy_ui_component_align(&self->left);
		}
	} else if (strcmp(psy_property_key(property), "linenumbers") == 0) {
		trackerview_showlinenumbers(self, psy_property_item_int(property));
	} else if (strcmp(psy_property_key(property), "linenumberscursor") == 0) {
		trackerview_showlinenumbercursor(self, psy_property_item_int(property));
	} else if (strcmp(psy_property_key(property), "linenumbersinhex") == 0) {
		trackerview_showlinenumbersinhex(self, psy_property_item_int(property));
	} else if (strcmp(psy_property_key(property), "wideinstcolumn") == 0) {
		trackconfig_initcolumns(&self->trackconfig, psy_property_item_int(property));
		trackerview_computemetrics(self);
	} else if (strcmp(psy_property_key(property), "drawemptydata") == 0) {
		trackergrid_showemptydata(&self->grid, psy_property_item_int(property));
	} else if (strcmp(psy_property_key(property), "centercursoronscreen") == 0) {
		trackergrid_setcentermode(&self->grid, psy_property_item_int(property));
	} else if (strcmp(psy_property_key(property), "notetab") == 0) {
		self->grid.notestabmode = self->griddefaults.notestabmode =
			workspace_notetabmode(self->workspace);
	} else if (strcmp(psy_property_key(property), "font") == 0) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_item_str(property));
		psy_ui_font_init(&font, &fontinfo);
		trackerview_setfont(self, &font, TRUE);
		psy_ui_font_dispose(&font);
		psy_ui_component_align(&self->component);
	}
}

void trackerview_readconfiguration(TrackerView* self)
{
	psy_Property* p;

	p = psy_property_findsection(&self->workspace->config, "visual.patternview");
	if (p) {
		if (psy_property_at_bool(p, "griddefaults", 1)) {
			self->showdefaultline = 1;
			psy_ui_component_show(&self->griddefaults.component);
		} else {
			self->showdefaultline = 0;
			psy_ui_component_hide(&self->griddefaults.component);
		}
		trackerview_showlinenumbers(self, psy_property_at_bool(p, "linenumbers", 1));
		trackerview_showlinenumbercursor(self, psy_property_at_bool(p, "linenumberscursor", 1));
		trackerview_showlinenumbersinhex(self, psy_property_at_bool(p, "linenumbersinhex", 1));
		self->grid.wraparound = psy_property_at_bool(p, "wraparound", 1);
		trackergrid_showemptydata(&self->grid, psy_property_at_bool(p, "drawemptydata", 1));
		trackergrid_setcentermode(&self->grid, psy_property_at_bool(p, "centercursoronscreen", 1));
		self->grid.notestabmode = self->griddefaults.notestabmode =
			(psy_property_at_bool(p, "notetab", 0))
			? psy_dsp_NOTESTAB_A440
			: psy_dsp_NOTESTAB_A220;
		trackerview_readfont(self);
		trackconfig_initcolumns(&self->trackconfig,
			workspace_showwideinstcolumn(self->workspace));
		trackerview_computemetrics(self);
	}
	self->grid.ft2home = workspace_ft2home(self->workspace);
	self->grid.ft2delete = workspace_ft2delete(self->workspace);
	self->grid.movecursoronestep = workspace_movecursoronestep(self->workspace);
	self->grid.effcursoralwaysdown = workspace_effcursoralwayssdown(self->workspace);
	trackerview_readpgupdowntype(self);
	trackerview_readtheme(self);
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

void trackerview_readfont(TrackerView* self)
{
	psy_Property* pv;

	pv = psy_property_findsection(&self->workspace->config, "visual.patternview");
	if (pv) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));
		psy_ui_font_init(&font, &fontinfo);
		trackerview_setfont(self, &font, TRUE);
		psy_ui_font_dispose(&font);
	}
}

void trackerview_readtheme(TrackerView* self)
{
	trackerview_updateksin(self);
	if (self->grid.midline) {
		trackergrid_centeroncursor(&self->grid);
	}
}

void trackerview_showlinenumbers(TrackerView* self, int showstate)
{
	self->showlinenumbers = showstate;
	if (self->showlinenumbers != 0) {
		if (psy_ui_component_visible(&self->component)) {
			psy_ui_component_show(&self->left);
			psy_ui_component_align(psy_ui_component_parent(&self->left));
		}
	} else {
		psy_ui_component_hide(&self->left);
		psy_ui_component_align(psy_ui_component_parent(&self->left));
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void trackerview_showlinenumbercursor(TrackerView* self, int showstate)
{
	self->showlinenumbercursor = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerview_showlinenumbersinhex(TrackerView* self, int showstate)
{
	self->showlinenumbersinhex = showstate;
	psy_ui_component_invalidate(&self->component);
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
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_invalidate(&self->griddefaults.component);
}

void trackerview_oninterpolatecurve(TrackerView* self, psy_ui_Component* sender)
{
	psy_ui_component_togglevisibility(&self->interpolatecurveview.component);
}
