// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackerview.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <pattern.h>
#include <patternio.h>
#include "cmdsnotes.h"
#include "skinio.h"
#include "skingraphics.h"
#include "resources/resource.h"
#include <uiopendialog.h>
#include <uisavedialog.h>

#include <dir.h>

#include "../../detail/os.h"
#include "../../detail/trace.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

static const psy_dsp_big_beat_t epsilon = 0.0001;

static int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position >= offset && position < offset + width;
}

int testrange_e(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position + 2 * epsilon >= offset &&
		position < offset + width - epsilon;
}

static int testcursor(psy_audio_PatternEditPosition cursor, uintptr_t track,
	psy_dsp_big_beat_t offset, uintptr_t lpb)
{
	return cursor.track == track && testrange(cursor.offset, offset, 1.0 / lpb);
}

// TrackerViewLineState
// Prototypes
static double trackerviewlinestate_offset(TrackerViewLineState*, int y, unsigned int* lines);
static int trackerlinestate_numlines(TrackerViewLineState*);

void trackerviewlinestate_init(TrackerViewLineState* self)
{
	self->lineheight = 13;
	self->lpb = 4;
	self->skin = NULL;
	self->pattern = NULL;	
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	self->drawcursor = TRUE;
	self->visilines = 25;
}

int trackerviewlinestate_offsettoscreenline(TrackerViewLineState* self,
	psy_dsp_big_beat_t offset)
{
	return (int)(offset * self->lpb);
}

psy_dsp_big_beat_t trackerviewlinestate_offset(TrackerViewLineState* self, int y, unsigned int* lines)
{
	if (self->pattern) {
		*lines = max(0, y) / self->lineheight;
		return (*lines) * (1.0 / self->lpb);
	}
	*lines = 0;
	return 0;
}

int trackerviewlinestate_testplaybar(TrackerViewLineState* self, psy_dsp_big_beat_t offset)
{
	return testrange(self->lastplayposition - self->sequenceentryoffset,
		offset, 1.0 / self->lpb);
}

int trackerlinestate_numlines(TrackerViewLineState* self)
{
	return (self->pattern)
		? (int)(psy_audio_pattern_length(self->pattern) * self->lpb)
		: 0;
}


// TrackerGridState
static int trackergridstate_trackwidth(TrackerGridState*, uintptr_t track);
static TrackDef* trackergridstate_trackdef(TrackerGridState*, uintptr_t track);

void trackergridstate_init(TrackerGridState* self)
{
	self->skin = NULL;
	self->pattern = NULL;
	self->textwidth = 9;
	self->textleftedge = 2;
	self->textleftedge = 2;
	self->patterntrackident = 0;
	self->headertrackident = 0;
	self->numtracks = 16;
	psy_table_init(&self->trackconfigs);
}

void trackergridstate_dispose(TrackerGridState* self)
{
	psy_table_disposeall(&self->trackconfigs,
		(psy_fp_disposefunc)NULL);
}


static uintptr_t trackergrid_columnvalue(psy_audio_PatternEvent*, int column);
static void trackerview_initcolumns(TrackerView*);

static void trackerview_computefontheight(TrackerView*);
static void trackerview_initblockmenu(TrackerView*);
static void trackerview_connectblockmenu(TrackerView*);
static void trackerview_connectplayer(TrackerView*);
static void trackerview_connectworkspace(TrackerView*);
static void trackerview_setfont(TrackerView*, psy_ui_Font*, bool iszoombase);
static void trackerview_initmetrics(TrackerView*);
static void trackerview_updatescrollstep(TrackerView*);
static void trackerview_computemetrics(TrackerView*);
static void trackerview_ondestroy(TrackerView*, psy_ui_Component* sender);
static void trackerview_onkeydown(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_onkeyup(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_ongridscroll(TrackerView*, psy_ui_Component* sender,
	int stepx, int stepy);
static void trackerview_onzoomboxchanged(TrackerView*, ZoomBox* sender);
static void trackerview_ontimer(TrackerView*, psy_ui_Component* sender,
	uintptr_t timerid);
static void trackerview_onseqlinetick(TrackerView*, psy_audio_Sequencer*);
static void trackergrid_inputevent(TrackerGrid*, const psy_audio_PatternEvent*,
	int chordmode);
static void enterdigitcolumn(TrackerView*, psy_audio_PatternEntry*, int track,
	int column, int digit, int value);
static void enterdigit(int digit, int newval, unsigned char* val);
static void digitlohi(int value, int digit, uintptr_t size, uint8_t* lo, uint8_t* hi);
static void lohi(uint8_t* value, int digit, uint8_t* lo, uint8_t* hi);

static void entervaluecolumn(psy_audio_PatternEntry*, int column, int value);
static int keycodetoint(unsigned int keycode);
static void trackerview_setcentermode(TrackerView*, int mode);
static void trackerview_handlecommand(TrackerView*, psy_ui_KeyEvent*, int cmd);
static void trackerview_onalign(TrackerView*);
static void trackergrid_enterevent(TrackerGrid*, psy_ui_KeyEvent*);
static void trackerview_togglefollowsong(TrackerView*);
static void trackergrid_rowdelete(TrackerGrid*);
static void trackergrid_rowclear(TrackerGrid*);
static void trackergrid_prevcol(TrackerGrid*);
static void trackergrid_nextcol(TrackerGrid*);
static void trackerview_prevline(TrackerView*);
static void trackerview_prevlines(TrackerView*, uintptr_t lines, int wrap);
static void trackerview_advanceline(TrackerView*);
static void trackerview_advancelines(TrackerView*, uintptr_t lines, int wrap);
static void trackerview_home(TrackerView*);
static void trackerview_end(TrackerView*);
static void trackerview_selectall(TrackerView*);
static void trackerview_selectcol(TrackerView*);
static void trackerview_prevtrack(TrackerView*);
static void trackerview_nexttrack(TrackerView*);
static void trackerview_enablesync(TrackerView*);
static void trackerview_preventsync(TrackerView*);
static int trackerview_scrollleft(TrackerView*, psy_audio_PatternEditPosition);
static int trackerview_scrollright(TrackerView*, psy_audio_PatternEditPosition);
static int trackerview_scrollup(TrackerView*, psy_audio_PatternEditPosition);
static int trackerview_scrolldown(TrackerView*, psy_audio_PatternEditPosition);
static void trackerview_showlinenumbers(TrackerView*, int showstate);
static void trackerview_showlinenumbercursor(TrackerView*, int showstate);
static void trackerview_showlinenumbersinhex(TrackerView*, int showstate);
static void trackerview_showemptydata(TrackerView*, int showstate);
static void trackerview_setclassicheadercoords(TrackerView*);
static void trackerview_setheadercoords(TrackerView*);
static void trackerview_setcoords(TrackerView* self, psy_Properties*);
static void trackerview_setheadertextcoords(TrackerView*);
static void trackerview_onconfigchanged(TrackerView*, Workspace*,
	psy_Properties*);
static void trackerview_readconfig(TrackerView*);
static void trackerview_readtheme(TrackerView*);
static void trackerview_oninput(TrackerView*, psy_audio_Player*,
	psy_audio_PatternEvent*);
static void trackerview_setdefaultevent(TrackerView*,
	psy_audio_Pattern* defaultpattern, psy_audio_PatternEvent*);
static void trackerview_initinputs(TrackerView*);
static void trackerview_invalidatecursor(TrackerView*,
	const psy_audio_PatternEditPosition*);
static void trackerview_invalidateline(TrackerView*, psy_dsp_big_beat_t offset);
static void trackerview_initdefaultskin(TrackerView*);
static void trackerview_onpatternimport(TrackerView*);
static void trackerview_onpatternexport(TrackerView*);
static void trackerview_onlpbchanged(TrackerView*, psy_audio_Player* sender,
	uintptr_t lpb);
static void trackerview_onpatterneditpositionchanged(TrackerView*,
	Workspace* sender);
static void trackerview_onparametertweak(TrackerView*,
	Workspace* sender, int slot, uintptr_t tweak, float value);
static void trackerview_onskinchanged(TrackerView*, Workspace*);
static int trackergridstate_track_x(TrackerGridState*, uintptr_t track);
static uintptr_t trackergridstate_screentotrack(TrackerGridState*, int x,
	uintptr_t numsongtracks);
static void trackerview_oninterpolatecurveviewoncancel(TrackerView*,
	InterpolateCurveView* sender);
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

static void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerView* view);
static void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const psy_audio_PatternEditPosition*);
static void trackerlinenumbers_invalidateline(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset);

enum {
	CMD_NAVUP,
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

	CMD_UNDO,
	CMD_REDO,

	CMD_FOLLOWSONG
};

// TrackColumnDef
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
		TRACKER_COLUMN_INST, TRACKER_COLUMN_INST, NOTECOMMANDS_INST_EMPTY);
	trackcolumndef_init(&self->mach, 2, 2, 1,
		TRACKER_COLUMN_MACH, TRACKER_COLUMN_MACH, NOTECOMMANDS_MACH_EMPTY);
	trackcolumndef_init(&self->vol, 2, 2, 1,
		TRACKER_COLUMN_VOL, TRACKER_COLUMN_VOL, NOTECOMMANDS_VOL_EMPTY);
	trackcolumndef_init(&self->cmd, 2, 2, 0,
		TRACKER_COLUMN_NONE, TRACKER_COLUMN_CMD, 0x00);
	trackcolumndef_init(&self->param, 2, 2, 1,
		TRACKER_COLUMN_CMD, TRACKER_COLUMN_PARAM, 0x00);
	self->numfx = 1;
}

// Commands
typedef struct {
	psy_Command command;
	psy_audio_PatternEditPosition cursor;
	psy_audio_Pattern* pattern;
	double bpl;
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	int insert;
	Workspace* workspace;
} InsertCommand;

static void InsertCommandDispose(InsertCommand*);
static void InsertCommandExecute(InsertCommand*);
static void InsertCommandRevert(InsertCommand*);

// vtable
static psy_CommandVtable insertcommandcommand_vtable;
static int insertcommandcommand_vtable_initialized = 0;

static void insertcommandcommand_vtable_init(InsertCommand* self)
{
	if (!insertcommandcommand_vtable_initialized) {
		insertcommandcommand_vtable = *(self->command.vtable);
		insertcommandcommand_vtable.dispose = (psy_fp_command)InsertCommandDispose;
		insertcommandcommand_vtable.execute = (psy_fp_command)InsertCommandExecute;
		insertcommandcommand_vtable.revert = (psy_fp_command)InsertCommandRevert;
		insertcommandcommand_vtable_initialized = 1;
	}
}

InsertCommand* InsertCommandAlloc(psy_audio_Pattern* pattern, double bpl,
	psy_audio_PatternEditPosition cursor, psy_audio_PatternEvent event,
	Workspace* workspace)
{
	InsertCommand* rv;

	rv = malloc(sizeof(InsertCommand));
	psy_command_init(&rv->command);
	insertcommandcommand_vtable_init(rv);
	rv->command.vtable = &insertcommandcommand_vtable;
	rv->cursor = cursor;
	rv->bpl = bpl;
	rv->event = event;
	rv->insert = 0;
	rv->pattern = pattern;
	rv->workspace = workspace;
	return rv;
}

void InsertCommandDispose(InsertCommand* self) { }

void InsertCommandExecute(InsertCommand* self)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(self->pattern,
		self->cursor.track,
		(psy_dsp_big_beat_t)self->cursor.offset,
		(psy_dsp_big_beat_t)self->bpl, &prev);
	if (node) {
		self->oldevent = psy_audio_pattern_event(self->pattern, node);
		psy_audio_pattern_setevent(self->pattern, node, &self->event);
		self->insert = 0;
	} else {
		node = psy_audio_pattern_insert(self->pattern,
			prev,
			self->cursor.track,
			(psy_dsp_big_beat_t)self->cursor.offset,
			&self->event);
		self->insert = 1;
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
}

void InsertCommandRevert(InsertCommand* self)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(self->pattern,
		self->cursor.track,
		(psy_dsp_big_beat_t)self->cursor.offset,
		(psy_dsp_big_beat_t)self->bpl, &prev);
	if (node) {
		if (self->insert) {
			psy_audio_pattern_remove(self->pattern, node);
			psy_audio_sequencer_checkiterators(
				&self->workspace->player.sequencer,
				node);
		} else {
			psy_audio_pattern_setevent(self->pattern, node, &self->oldevent);
		}
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
}

// BlockTranspose
typedef struct {
	psy_Command command;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	psy_audio_PatternEditPosition cursor;
	PatternSelection block;
	int transposeoffset;
	Workspace* workspace;
} BlockTransposeCommand;

static void BlockTransposeCommandDispose(BlockTransposeCommand*);
static void BlockTransposeCommandExecute(BlockTransposeCommand*);
static void BlockTransposeCommandRevert(BlockTransposeCommand*);

// vtable
static psy_CommandVtable blocktransposecommand_vtable;
static int blocktransposecommand_vtable_initialized = 0;

static void blocktransposecommandcommand_vtable_init(BlockTransposeCommand* self)
{
	if (!blocktransposecommand_vtable_initialized) {
		blocktransposecommand_vtable = *(self->command.vtable);
		blocktransposecommand_vtable.dispose = (psy_fp_command)BlockTransposeCommandDispose;
		blocktransposecommand_vtable.execute = (psy_fp_command)BlockTransposeCommandExecute;
		blocktransposecommand_vtable.revert = (psy_fp_command)BlockTransposeCommandRevert;
		blocktransposecommand_vtable_initialized = 1;
	}
}

BlockTransposeCommand* BlockTransposeCommandAlloc(psy_audio_Pattern* pattern,
	PatternSelection block, psy_audio_PatternEditPosition cursor, int transposeoffset,
	Workspace* workspace)
{
	BlockTransposeCommand* rv;

	rv = malloc(sizeof(BlockTransposeCommand));
	psy_command_init(&rv->command);
	blocktransposecommandcommand_vtable_init(rv);
	rv->command.vtable = &blocktransposecommand_vtable;
	rv->pattern = pattern;
	psy_audio_pattern_init(&rv->oldpattern);
	rv->block = block;
	rv->cursor = cursor;
	rv->transposeoffset = transposeoffset;
	rv->workspace = workspace;
	return rv;
}

void BlockTransposeCommandDispose(BlockTransposeCommand* self)
{
	psy_audio_pattern_dispose(&self->oldpattern);
}

void BlockTransposeCommandExecute(BlockTransposeCommand* self)
{
	workspace_setpatterneditposition(self->workspace, self->cursor);
	psy_audio_pattern_copy(&self->oldpattern, self->pattern);
	psy_audio_pattern_blocktranspose(self->pattern,
		self->block.topleft,
		self->block.bottomright, self->transposeoffset);
}

void BlockTransposeCommandRevert(BlockTransposeCommand* self)
{
	assert(self->pattern);
	workspace_setpatterneditposition(self->workspace, self->cursor);
	psy_audio_pattern_copy(self->pattern, &self->oldpattern);
}

/// TrackerGrid
static void trackergrid_computecolumns(TrackerGrid*, int textwidth);
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
static void trackergrid_onkeydown(TrackerGrid*, psy_ui_KeyEvent*);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousewheel(TrackerGrid*, psy_ui_Component* sender, psy_ui_MouseEvent*);
static psy_audio_PatternEditPosition trackergrid_makecursor(TrackerGrid* self, int x, int y);
static int trackergrid_resizecolumn(TrackerGrid*, int x, int y);
static void trackergrid_dragcolumn(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_startdragselection(TrackerGrid*, psy_audio_PatternEditPosition);
static void trackergrid_dragselection(TrackerGrid*, psy_audio_PatternEditPosition);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender,
	int stepx, int stepy);
static void trackergrid_onfocuslost(TrackerGrid*, psy_ui_Component* sender);
static int trackergrid_testselection(TrackerGrid*, unsigned int track, double offset);
static void trackergrid_clipblock(TrackerGrid*, const psy_ui_Rectangle*,
	PatternSelection*);
static void trackergrid_drawdigit(TrackerGrid* self, psy_ui_Graphics*,
	int x, int y, int value, int empty, int mid);
static void trackergrid_onpreferredsize(TrackerGrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void trackergrid_invalidatecursor(TrackerGrid*,
	const psy_audio_PatternEditPosition* cursor);
static void trackergrid_oninterpolatelinear(TrackerGrid*);
static void trackergrid_oninterpolatecurve(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_onchangegenerator(TrackerGrid*);
static void trackergrid_onchangeinstrument(TrackerGrid*);
static void trackergrid_blockstart(TrackerGrid*);
static void trackergrid_blockend(TrackerGrid*);
static void trackergrid_blockunmark(TrackerGrid*);
static void trackerview_centeroncursor(TrackerView*);
static void trackerview_toggleblockmenu(TrackerView*);
static void trackergrid_onblockcut(TrackerGrid*);
static void trackergrid_onblockcopy(TrackerGrid*);
static void trackergrid_onblockpaste(TrackerGrid*);
static void trackergrid_onblockmixpaste(TrackerGrid*);
static void trackergrid_onblockdelete(TrackerGrid*);
static void trackergrid_onblocktransposeup(TrackerGrid*);
static void trackergrid_onblocktransposedown(TrackerGrid*);
static void trackergrid_onblocktransposeup12(TrackerGrid*);
static void trackergrid_onblocktransposedown12(TrackerGrid*);
static void trackergrid_inputnote(TrackerGrid*, psy_dsp_note_t, int chordmode);
static void trackergrid_inputvalue(TrackerGrid*, int value, int digit);
static void trackergrid_prevtrack(TrackerGrid*);
static void trackergrid_nexttrack(TrackerGrid*);
static int trackergrid_scrollup(TrackerGrid*,
	psy_audio_PatternEditPosition);
static int trackergrid_scrolldown(TrackerGrid*,
	psy_audio_PatternEditPosition);
static int trackergrid_scrollleft(TrackerGrid*,
	psy_audio_PatternEditPosition);
static int trackergrid_scrollright(TrackerGrid*,
	psy_audio_PatternEditPosition);
static void trackergrid_prevline(TrackerGrid*);
static void trackergrid_prevlines(TrackerGrid*, uintptr_t lines, int wrap);
static void trackergrid_advanceline(TrackerGrid*);
static void trackergrid_advancelines(TrackerGrid*, uintptr_t lines, int wrap);
static void trackergrid_home(TrackerGrid*);
static void trackergrid_end(TrackerGrid*);

static psy_ui_ComponentVtable trackergrid_vtable;
static int trackergrid_vtable_initialized = 0;

static void trackergrid_vtable_init(TrackerGrid* self)
{
	if (!trackergrid_vtable_initialized) {
		trackergrid_vtable = *(self->component.vtable);
		trackergrid_vtable.ondraw = (psy_ui_fp_ondraw)trackergrid_ondraw;
		trackergrid_vtable.onkeydown = (psy_ui_fp_onkeydown)
			trackergrid_onkeydown;
		trackergrid_vtable.onmousedown = (psy_ui_fp_onmousedown)
			trackergrid_onmousedown;
		trackergrid_vtable.onmousemove = (psy_ui_fp_onmousemove)
			trackergrid_onmousemove;
		trackergrid_vtable.onmouseup = (psy_ui_fp_onmouseup)
			trackergrid_onmouseup;
		trackergrid_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			trackergrid_onmousedoubleclick;
		trackergrid_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			trackergrid_onpreferredsize;
	}
}

void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent,
	TrackerView* view, TrackerGridState* gridstate, TrackerViewLineState* linestate,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackergrid_vtable_init(self);
	self->component.vtable = &trackergrid_vtable;
	trackergrid_setsharedgridstate(self, gridstate);
	trackergrid_setsharedlinestate(self, linestate);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	self->view = view;
	self->editmode = TRACKERGRID_EDITMODE_SONG;
	self->hasselection = 0;
	self->midline = 1;
	self->columnresize = 0;
	self->dragcolumn = -1;
	self->dragcolumnbase = 0;	
	self->view->workspace->chordmode = 0;
	self->chordbegin = 0;
	self->workspace = workspace;
	self->component.overflow = psy_ui_OVERFLOW_SCROLL |
		psy_ui_OVERFLOW_VSCROLLCENTER;
	self->tm = psy_ui_component_textmetric(&self->component);	
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);
	psy_signal_connect(&self->component.signal_focuslost, self,
		trackergrid_onfocuslost);
	psy_signal_connect(&self->component.signal_mousewheel, self,
		trackergrid_onmousewheel);
	self->gridstate->numtracks = psy_audio_player_numsongtracks(&self->workspace->player);
	self->lpb = psy_audio_player_lpb(&self->workspace->player);
	self->bpl = 1 / (psy_dsp_big_beat_t)psy_audio_player_lpb(&self->workspace->player);
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;
	self->cursor.track = 0;
	self->cursor.offset = 0;
	self->cursor.column = 0;
	self->cursor.digit = 0;
	workspace_setpatterneditposition(workspace, self->cursor);
	self->cursorstep = 0.25;
	trackergrid_computecolumns(self, 9);
}

void trackergrid_setsharedgridstate(TrackerGrid* self, TrackerGridState*
	gridstate)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		trackergridstate_init(&self->defaultgridstate);
		self->gridstate = &self->defaultgridstate;
	}
}

void trackergrid_setsharedlinestate(TrackerGrid* self, TrackerViewLineState*
	linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerviewlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
}

void trackerview_initcolumns(TrackerView* self)
{
	trackdef_init(&self->gridstate.defaulttrackdef);
	if (workspace_showwideinstcolumn(self->workspace)) {
		self->gridstate.defaulttrackdef.inst.numdigits = 4;
		self->gridstate.defaulttrackdef.inst.numchars = 4;
	}
}

void trackergrid_computecolumns(TrackerGrid* self, int textwidth)
{
}

void TrackerViewApplyProperties(TrackerView* self, psy_Properties* p)
{
	const char* pattern_header_skin_name;

	self->skin.separator = psy_properties_int(p, "pvc_separator", 0x00292929);
	self->skin.separator2 = psy_properties_int(p, "pvc_separator2", 0x00292929);
	self->skin.background = psy_properties_int(p, "pvc_background", 0x00292929);
	self->skin.background2 = psy_properties_int(p, "pvc_background2", 0x00292929);
	self->skin.row4beat = psy_properties_int(p, "pvc_row4beat", 0x00595959);
	self->skin.row4beat2 = psy_properties_int(p, "pvc_row4beat2", 0x00595959);
	self->skin.rowbeat = psy_properties_int(p, "pvc_rowbeat", 0x00363636);
	self->skin.rowbeat2 = psy_properties_int(p, "pvc_rowbeat2", 0x00363636);
	self->skin.row = psy_properties_int(p, "pvc_row", 0x003E3E3E);
	self->skin.row2 = psy_properties_int(p, "pvc_row2", 0x003E3E3E);
	self->skin.font = psy_properties_int(p, "pvc_font", 0x00CACACA);
	self->skin.font2 = psy_properties_int(p, "pvc_font2", 0x00CACACA);
	self->skin.fontPlay = psy_properties_int(p, "pvc_fontplay", 0x00FFFFFF);
	self->skin.fontCur2 = psy_properties_int(p, "pvc_fontcur2", 0x00FFFFFF);
	self->skin.fontSel = psy_properties_int(p, "pvc_fontsel", 0x00FFFFFF);
	self->skin.fontSel2 = psy_properties_int(p, "pvc_fontsel2", 0x00FFFFFF);
	self->skin.selection = psy_properties_int(p, "pvc_selection", 0x009B7800);
	self->skin.selection2 = psy_properties_int(p, "pvc_selection2", 0x009B7800);
	self->skin.playbar = psy_properties_int(p, "pvc_playbar", 0x009F7B00);
	self->skin.playbar2 = psy_properties_int(p, "pvc_playbar2", 0x009F7B00);
	self->skin.cursor = psy_properties_int(p, "pvc_cursor", 0x009F7B00);
	self->skin.cursor2 = psy_properties_int(p, "pvc_cursor2", 0x009F7B00);
	self->skin.midline = psy_properties_int(p, "pvc_midline", 0x007D6100);
	self->skin.midline2 = psy_properties_int(p, "pvc_midline2", 0x007D6100);
	psy_ui_component_setbackgroundcolor(
		&self->linenumbers.component, self->skin.background);
	pattern_header_skin_name = psy_properties_readstring(p, "pattern_header_skin",
		0);
	if (pattern_header_skin_name) {
		char path[_MAX_PATH];
		char filename[_MAX_PATH];

		strcpy(filename, pattern_header_skin_name);
		strcat(filename, ".bmp");
		psy_dir_findfile(workspace_skins_directory(self->workspace),
			filename, path);
		if (path[0] != '\0') {
			psy_ui_Bitmap bmp;

			psy_ui_bitmap_init(&bmp);
			if (psy_ui_bitmap_load(&bmp, path) == 0) {
				psy_ui_bitmap_dispose(&self->skin.bitmap);
				self->skin.bitmap = bmp;
			}
		}
		strcpy(filename, pattern_header_skin_name);
		strcat(filename, ".psh");
		psy_dir_findfile(workspace_skins_directory(self->workspace),
			filename, path);
		if (path[0] != '\0') {
			psy_Properties* coords;

			coords = psy_properties_create();
			skin_loadpsh(coords, path);
			trackerview_setcoords(self, coords);
			properties_free(coords);
		}
	}
	psy_ui_component_invalidate(&self->component);
}

void trackerview_setcoords(TrackerView* self, psy_Properties* p)
{
	const char* s;
	int vals[4];

	if (s = psy_properties_readstring(p, "background_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->skin.headercoords.background.srcx = vals[0];
		self->skin.headercoords.background.srcy = vals[1];
		self->skin.headercoords.background.destwidth = vals[2];
		self->skin.headercoords.background.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "number_0_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->skin.headercoords.digitx0.srcx = vals[0];
		self->skin.headercoords.digitx0.srcy = vals[1];
		self->skin.headercoords.digit0x.srcx = vals[0];
		self->skin.headercoords.digit0x.srcy = vals[1];
		self->skin.headercoords.digitx0.srcwidth = vals[2];
		self->skin.headercoords.digitx0.srcheight = vals[3];
		self->skin.headercoords.digit0x.srcwidth = vals[2];
		self->skin.headercoords.digit0x.srcheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "record_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->skin.headercoords.record.srcx = vals[0];
		self->skin.headercoords.record.srcy = vals[1];
		self->skin.headercoords.record.destwidth = vals[2];
		self->skin.headercoords.record.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "mute_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->skin.headercoords.mute.srcx = vals[0];
		self->skin.headercoords.mute.srcy = vals[1];
		self->skin.headercoords.mute.destwidth = vals[2];
		self->skin.headercoords.mute.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "solo_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->skin.headercoords.solo.srcx = vals[0];
		self->skin.headercoords.solo.srcy = vals[1];
		self->skin.headercoords.solo.destwidth = vals[2];
		self->skin.headercoords.solo.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "digit_x0_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->skin.headercoords.digitx0.destx = vals[0];
		self->skin.headercoords.digitx0.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "digit_0x_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->skin.headercoords.digit0x.destx = vals[0];
		self->skin.headercoords.digit0x.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "record_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->skin.headercoords.record.destx = vals[0];
		self->skin.headercoords.record.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "mute_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->skin.headercoords.mute.destx = vals[0];
		self->skin.headercoords.mute.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "solo_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->skin.headercoords.solo.destx = vals[0];
		self->skin.headercoords.solo.desty = vals[1];
	}
}

void trackergrid_ondraw(TrackerGrid* self, psy_ui_Graphics* g)
{
	PatternSelection clip;

	if (self->gridstate->pattern) {
		trackergrid_clipblock(self, &g->clip, &clip);
		//trackergrid_drawbackground(self, g, &clip);
		trackergrid_drawentries(self, g, &clip);
	} else {
		psy_ui_drawsolidrectangle(g, g->clip, self->gridstate->skin->background);
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
	block->topleft.offset = trackerviewlinestate_offset(self->linestate, clip->top,
		&lines);
	block->topleft.line = lines;
	block->bottomright.track = trackergridstate_screentotrack(self->gridstate,
		clip->right, psy_audio_player_numsongtracks(&self->workspace->player)) + 1;
	if (block->bottomright.track > self->gridstate->numtracks) {
		block->bottomright.track = self->gridstate->numtracks;
	}
	block->bottomright.column = 0;
	block->bottomright.digit = 0;
	block->bottomright.offset = trackerviewlinestate_offset(self->linestate,
		clip->bottom, &lines);
	block->bottomright.line = lines;
}

void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g, PatternSelection* clip)
{
	psy_ui_Rectangle r;
	unsigned int track;
	psy_ui_Size size;
	int trackswidth;

	size = psy_ui_component_size(&self->component);
	for (track = clip->topleft.track; track < clip->bottomright.track;
		++track) {
		trackergrid_drawtrackbackground(self, g, track);
	}
	trackswidth = trackergridstate_track_x(self->gridstate, self->gridstate->numtracks - 1) +
		trackergridstate_trackwidth(self->gridstate, self->gridstate->numtracks - 1);
	psy_ui_setrectangle(&r, trackswidth, -psy_ui_component_scrolltop(&self->component),
		psy_ui_value_px(&size.width, &self->tm) - trackswidth,
		psy_ui_value_px(&size.height, &self->tm));
	//psy_ui_drawsolidrectangle(g, r, self->view->skin.background);
}

void trackergrid_drawtrackbackground(TrackerGrid* self, psy_ui_Graphics* g, int track)
{	
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
			trackergridstate_track_x(self->gridstate, track),
			0, trackergridstate_trackwidth(self->gridstate, track),
			psy_ui_value_px(&size.height, &self->tm)),
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

	patternentry_init(&empty);
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

			columnflags.cursor = testcursor(self->cursor, track, offset, self->linestate->lpb);
			columnflags.selection = trackergrid_testselection(self, track, offset);
			columnflags.playbar = psy_audio_player_playing(&self->workspace->player) &&
				trackerviewlinestate_testplaybar(self->linestate, offset);
			while (!fill && node &&
				((psy_audio_PatternEntry*)(node->entry))->track <= track &&
				testrange_e(((psy_audio_PatternEntry*)(node->entry))->offset,
					offset,
					self->bpl)) {
				psy_audio_PatternEntry* entry;

				entry = (psy_audio_PatternEntry*)(node->entry);
				if (entry->track == track) {
					trackergrid_drawentry(self, g, entry,
						cpx + self->gridstate->patterntrackident,
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
					cpx + self->gridstate->patterntrackident,
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
			epsilon * 2 < offset + self->bpl) {
			node = node->next;
		}
		offset += self->bpl;
		++line;
		cpy += self->linestate->lineheight;		
	}
	patternentry_dispose(&empty);
}

void setcolumncolor(TrackerSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags)
{
	if (flags.cursor != 0) {
		psy_ui_setbackgroundcolor(g, skin->cursor);
		psy_ui_settextcolor(g, skin->fontCur);
	} else
		if (flags.playbar) {
			psy_ui_setbackgroundcolor(g, skin->playbar);
			psy_ui_settextcolor(g, skin->fontPlay);
		} else
			if (flags.selection != 0) {
				psy_ui_setbackgroundcolor(g, skin->cursor);
				psy_ui_settextcolor(g, skin->fontCur);
			} else
				if (flags.mid) {
					psy_ui_setbackgroundcolor(g, skin->midline);
					if (flags.cursor != 0) {
						psy_ui_settextcolor(g, skin->fontCur);
					} else {
						psy_ui_settextcolor(g, skin->font);
					}
				} else {
					if (flags.beat4) {
						psy_ui_setbackgroundcolor(g, skin->row4beat);
						psy_ui_settextcolor(g, skin->font);
					} else
						if (flags.beat) {
							psy_ui_setbackgroundcolor(g, skin->rowbeat);
							psy_ui_settextcolor(g, skin->font);
						} else {
							psy_ui_setbackgroundcolor(g, skin->row);
							psy_ui_settextcolor(g, skin->font);
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
	char buffer[20];
	psy_ui_Rectangle r;
	psy_ui_setrectangle(&r, x, y, self->gridstate->textwidth,
		self->tm.tmHeight);	
	if (!empty) {
		psy_snprintf(buffer, 2, "%X", value);
	} else {
		if (self->view->showemptydata) {
			psy_snprintf(buffer, 2, "%s", ".");
		} else {
			psy_snprintf(buffer, 2, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g, r.left + self->gridstate->textleftedge, r.top,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, buffer, strlen(buffer));
}

void trackergrid_onpreferredsize(TrackerGrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	rv->width = psy_ui_value_makepx(trackergridstate_track_x(self->gridstate,
		self->gridstate->numtracks));
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {		
		rv->height = psy_ui_value_makepx(trackerlinestate_numlines(self->linestate) *
			self->component.scrollstepy);
	} else {
		rv->height = psy_ui_value_makepx(self->linestate->lineheight);
	}
}

void trackergrid_prevtrack(TrackerGrid* self)
{
	psy_audio_PatternEditPosition oldcursor;
	int invalidate = 1;

	oldcursor = self->cursor;
	self->cursor.column = 0;
	self->cursor.digit = 0;
	if (self->cursor.track > 0) {
		--self->cursor.track;
		trackerview_scrollleft(self->view, self->cursor);
	} else
		if (self->view->wraparound) {
			self->cursor.track =
				psy_audio_player_numsongtracks(&self->view->workspace->player) - 1;
			invalidate = trackerview_scrollright(self->view, self->cursor);
		}
	if (invalidate) {
		trackergrid_invalidatecursor(self, &oldcursor);
		trackergrid_invalidatecursor(self, &self->cursor);
		trackerview_invalidatecursor(self->view, &oldcursor);
		trackerview_invalidatecursor(self->view, &self->cursor);
	}
	workspace_setpatterneditposition(self->view->workspace, self->cursor);
}

void trackergrid_nexttrack(TrackerGrid* self)
{
	psy_audio_PatternEditPosition oldcursor;
	int invalidate = 1;

	oldcursor = self->cursor;
	self->cursor.column = 0;
	self->cursor.digit = 0;
	if (self->cursor.track <
		psy_audio_player_numsongtracks(&self->view->workspace->player) - 1) {
		++self->cursor.track;
		invalidate = trackerview_scrollright(self->view, self->cursor);
	} else
		if (self->view->wraparound) {
			self->cursor.track = 0;
			invalidate = trackerview_scrollleft(self->view, self->cursor);
		}
	workspace_setpatterneditposition(self->view->workspace, self->cursor);
	if (invalidate) {
		trackergrid_invalidatecursor(self, &oldcursor);
		trackergrid_invalidatecursor(self, &self->cursor);
		trackerview_invalidatecursor(self->view, &oldcursor);
		trackerview_invalidatecursor(self->view, &self->cursor);
	}
}

int trackergrid_scrollup(TrackerGrid* self, psy_audio_PatternEditPosition cursor)
{
	int line;
	int topline;
	int rv = 1;
	psy_ui_Rectangle r;

	line = trackerviewlinestate_offsettoscreenline(self->linestate, cursor.offset);
	psy_ui_setrectangle(&r,
		trackergridstate_track_x(self->gridstate, cursor.track),
		self->linestate->lineheight * line,
		trackergridstate_trackwidth(self->gridstate, cursor.track),
		self->linestate->lineheight);
	if (self->midline) {
		psy_ui_Size gridsize;

		gridsize = psy_ui_component_size(&self->component);
		topline = psy_ui_value_px(&gridsize.height, &self->tm) / self->linestate->lineheight / 2;
	} else {
		topline = 0;
	}
	if (psy_ui_component_scrolltop(&self->component) + topline * self->linestate->lineheight > r.top) {
		int dlines = (psy_ui_component_scrolltop(&self->component) + topline * self->linestate->lineheight - r.top) /
			(self->linestate->lineheight);
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_component_scrolltop(&self->component) -
			self->component.scrollstepy * dlines);
		psy_signal_emit(&self->component.signal_scroll,
			&self->component, 2, 0, dlines);
		psy_ui_component_scrollstep(&self->component, 0, dlines);
		psy_ui_component_setverticalscrollposition(&self->component,
			psy_ui_component_verticalscrollposition(&self->component) - dlines);
		rv = 0;
	}
	return rv;
}

int trackergrid_scrolldown(TrackerGrid* self, psy_audio_PatternEditPosition cursor)
{
	int line;
	int visilines;
	int rv = 1;

	visilines = self->linestate->visilines;
	if (self->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerviewlinestate_offsettoscreenline(self->linestate, cursor.offset);
	if (visilines < line - psy_ui_component_scrolltop(&self->component) / self->linestate->lineheight) {
		int dlines;

		dlines = line - psy_ui_component_scrolltop(&self->component) / self->linestate->lineheight - visilines;
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_component_scrolltop(&self->component) +
			self->component.scrollstepy * dlines);
		psy_signal_emit(&self->component.signal_scroll,
			&self->component, 2, 0, -dlines);
		psy_ui_component_scrollstep(&self->component, 0, -dlines);
		psy_ui_component_setverticalscrollposition(&self->component,
			psy_ui_component_verticalscrollposition(
				&self->component) + dlines);
		rv = 0;
	}
	return rv;
}

int trackergrid_scrollleft(TrackerGrid* self, psy_audio_PatternEditPosition cursor)
{
	uintptr_t tracks;
	int invalidate = 1;

	tracks = cursor.track;
	if (trackergridstate_screentotrack(self->gridstate, psy_ui_component_scrollleft(&self->component),
		psy_audio_player_numsongtracks(&self->workspace->player)) > tracks) {
		psy_ui_component_setscrollleft(&self->component,
			trackergridstate_track_x(self->gridstate, tracks));
		psy_ui_component_setscrollleft(&self->view->griddefaults.component,
			psy_ui_component_scrollleft(&self->component));
		psy_ui_component_invalidate(&self->view->griddefaults.component);
		psy_ui_component_update(&self->view->griddefaults.component);
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_update(&self->component);
		trackerview_updatescrollstep(self->view);
		invalidate = 0;
	}
	return invalidate;
}

int trackergrid_scrollright(TrackerGrid* self, psy_audio_PatternEditPosition cursor)
{
	int invalidate = 1;
	uintptr_t visitracks;
	uintptr_t tracks;
	psy_ui_Size size;
	psy_ui_Size gridsize;
	psy_ui_TextMetric tm;
	psy_ui_TextMetric gridtm;
	int trackright;
	int trackleft;

	gridsize = psy_ui_component_size(&self->component);
	size = psy_ui_component_size(&self->view->component);
	tm = psy_ui_component_textmetric(&self->view->component);
	gridtm = psy_ui_component_textmetric(&self->component);
	trackleft = trackergridstate_screentotrack(self->gridstate,
		psy_ui_component_scrollleft(&self->component),
		psy_audio_player_numsongtracks(&self->workspace->player));
	trackright = trackergridstate_screentotrack(self->gridstate,
		psy_ui_value_px(&size.width, &tm) +
		psy_ui_component_scrollleft(&self->component),
		self->gridstate->numtracks);
	if (trackergridstate_track_x(self->gridstate, trackright) +
		trackergridstate_trackwidth(self->gridstate, trackright) > psy_ui_value_px(&gridsize.width,
		&gridtm)) {
		--trackright;
	}
	visitracks = trackright - trackleft;
	tracks = cursor.track + 1;
	if (tracks > visitracks + trackergridstate_screentotrack(self->gridstate,
			psy_ui_component_scrollleft(&self->component), self->gridstate->numtracks)) {
		psy_ui_component_setscrollleft(&self->component,
			trackergridstate_track_x(self->gridstate, tracks - visitracks));		
		psy_ui_component_setscrollleft(&self->view->griddefaults.component,
			psy_ui_component_scrollleft(&self->component));
		psy_ui_component_invalidate(&self->view->griddefaults.component);
		psy_ui_component_update(&self->view->griddefaults.component);		
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_update(&self->component);		
		invalidate = 0;
		trackerview_updatescrollstep(self->view);
	}
	return invalidate;
}

void trackergrid_prevline(TrackerGrid* self)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->cursor;
	trackerview_prevlines(self->view, workspace_cursorstep(self->workspace),
		self->view->wraparound);
}

void trackergrid_prevlines(TrackerGrid* self, uintptr_t lines, int wrap)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->cursor;
	self->cursor.offset -= lines * self->bpl;
	if (self->cursor.offset < 0) {
		if (wrap) {
			self->cursor.offset += self->gridstate->pattern->length;
			if (self->cursor.offset < 0) {
				self->cursor.offset = 0;
			}
			trackerview_scrolldown(self->view, self->cursor);
		} else {
			self->cursor.offset = 0;
			trackerview_scrollup(self->view, self->cursor);
		}
	} else {
		trackerview_scrollup(self->view, self->cursor);
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
	trackergrid_invalidatecursor(self, &oldcursor);
	trackergrid_invalidatecursor(self, &self->cursor);
	trackerview_invalidatecursor(self->view, &oldcursor);
	trackerview_invalidatecursor(self->view, &self->cursor);
}

void trackergrid_advanceline(TrackerGrid* self)
{
	trackergrid_advancelines(self, workspace_cursorstep(
		self->workspace), self->view->wraparound);
}

void trackergrid_advancelines(TrackerGrid* self, uintptr_t lines, int wrap)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->cursor;
	self->cursor.offset += lines * self->bpl;
	if (self->cursor.offset >= self->gridstate->pattern->length) {
		if (wrap) {
			self->cursor.offset = self->cursor.offset -
				self->gridstate->pattern->length;
			if (self->cursor.offset > self->gridstate->pattern->length - self->bpl) {
				self->cursor.offset = self->gridstate->pattern->length - self->bpl;
			}
			trackerview_scrollup(self->view, self->cursor);
		} else {
			self->cursor.offset = self->gridstate->pattern->length - self->bpl;
			trackerview_scrolldown(self->view, self->cursor);
		}
	} else {
		trackerview_scrolldown(self->view, self->cursor);
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
	trackergrid_invalidatecursor(self, &oldcursor);
	trackergrid_invalidatecursor(self, &self->cursor);
	trackerview_invalidatecursor(self->view, &oldcursor);
	trackerview_invalidatecursor(self->view, &self->cursor);
}

void trackergrid_home(TrackerGrid* self)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->cursor;
	self->cursor.offset = 0;
	trackerview_scrollup(self->view, self->cursor);
	workspace_setpatterneditposition(self->workspace, self->cursor);
	trackergrid_invalidatecursor(self, &oldcursor);
	trackergrid_invalidatecursor(self, &self->cursor);
	trackerview_invalidatecursor(self->view, &oldcursor);
	trackerview_invalidatecursor(self->view, &self->cursor);
}

void trackergrid_end(TrackerGrid* self)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->cursor;
	self->cursor.offset = self->gridstate->pattern->length - self->bpl;
	trackerview_scrolldown(self->view, self->cursor);
	workspace_setpatterneditposition(self->workspace, self->cursor);
	trackergrid_invalidatecursor(self, &oldcursor);
	trackergrid_invalidatecursor(self, &self->cursor);
	trackerview_invalidatecursor(self->view, &oldcursor);
	trackerview_invalidatecursor(self->view, &self->cursor);
}

void trackerview_centeroncursor(TrackerView* self)
{
	int line;
	line = trackerviewlinestate_offsettoscreenline(&self->linestate,
		self->grid.cursor.offset);
	psy_ui_component_setscrolltop(&self->grid.component,
		-(self->linestate.visilines / 2 - line) * self->linestate.lineheight);
	psy_ui_component_setscrolltop(&self->linenumbers.component,
		psy_ui_component_scrolltop(&self->grid.component));	
}

void trackergrid_onkeydown(TrackerGrid* self, psy_ui_KeyEvent* ev)
{
	if (self->editmode == TRACKERGRID_EDITMODE_LOCAL) {
		if (ev->keycode == psy_ui_KEY_ESCAPE) {
			psy_ui_component_setfocus(&self->view->grid.component);
			psy_ui_keyevent_stoppropagation(ev);
		} else {
			int cmd;

			cmd = psy_audio_inputs_cmd(&self->view->inputs, psy_audio_encodeinput(ev->keycode, ev->shift,
				ev->ctrl));
			if (cmd == CMD_NAVLEFT) {
				trackergrid_prevcol(self);
				psy_ui_component_invalidate(&self->component);
				psy_ui_keyevent_stoppropagation(ev);
			} else
				if (cmd == CMD_NAVRIGHT) {
					trackergrid_nextcol(self);
					psy_ui_component_invalidate(&self->component);
					psy_ui_keyevent_stoppropagation(ev);
				} else
					if (cmd == CMD_ROWCLEAR) {
						trackergrid_rowclear(self);
						psy_ui_component_invalidate(&self->component);
						psy_ui_keyevent_stoppropagation(ev);
						return;
					} else {
						if (self->cursor.column != TRACKER_COLUMN_NOTE) {
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
							EventDriverCmd cmd;
							EventDriverData input;

							cmd.id = -1;
							kbd = workspace_kbddriver(self->view->workspace);
							input.message = EVENTDRIVER_KEYDOWN;
							input.param1 = psy_audio_encodeinput(ev->keycode, 0, ev->ctrl);
							kbd->cmd(kbd, input, &cmd);
							trackergrid_inputnote(self,
								(psy_dsp_note_t)(cmd.id + workspace_octave(self->view->workspace) * 12),
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
	psy_audio_PatternEditPosition oldcursor;
	oldcursor = self->cursor;

	if (self->cursor.column == 0 && self->cursor.digit == 0) {
		if (self->cursor.track > 0) {
			TrackDef* trackdef;

			--self->cursor.track;
			trackdef = trackergridstate_trackdef(self->gridstate,
				self->cursor.track);
			self->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->cursor.digit = trackdef_numdigits(trackdef,
				self->cursor.column) - 1;
			trackerview_scrollleft(self->view, self->cursor);
		} else
			if (self->view->wraparound) {
				TrackDef* trackdef;

				self->cursor.track = psy_audio_player_numsongtracks(
					&self->view->workspace->player) - 1;
				trackdef = trackergridstate_trackdef(self->gridstate, self->cursor.track);
				self->cursor.column = trackdef_numcolumns(trackdef) - 1;
				self->cursor.digit = trackdef_numdigits(trackdef,
					self->cursor.column) - 1;
				invalidate = trackerview_scrollright(self->view, self->cursor);
			}
	} else {
		if (self->cursor.digit > 0) {
			--self->cursor.digit;
		} else {
			TrackDef* trackdef;

			trackdef = trackergridstate_trackdef(self->gridstate,
				self->cursor.track);
			--self->cursor.column;
			self->cursor.digit = trackdef_numdigits(trackdef,
				self->cursor.column) - 1;
		}
	}
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
		workspace_setpatterneditposition(self->view->workspace, self->cursor);
		if (invalidate) {
			trackergrid_invalidatecursor(self, &oldcursor);
			trackergrid_invalidatecursor(self, &self->cursor);
			trackerview_invalidatecursor(self->view, &oldcursor);
			trackerview_invalidatecursor(self->view, &self->cursor);
		}
	}
}

void trackergrid_nextcol(TrackerGrid* self)
{
	TrackDef* trackdef;
	int invalidate = 1;
	psy_audio_PatternEditPosition oldcursor;
	oldcursor = self->cursor;

	trackdef = trackergridstate_trackdef(self->gridstate, self->cursor.track);
	if (self->cursor.column == trackdef_numcolumns(trackdef) - 1 &&
		self->cursor.digit == trackdef_numdigits(trackdef,
			self->cursor.column) - 1) {
		if (self->cursor.track < psy_audio_player_numsongtracks(
			&self->view->workspace->player) - 1) {
			self->cursor.column = 0;
			self->cursor.digit = 0;
			++self->cursor.track;
			invalidate = trackerview_scrollright(self->view, self->cursor);
		} else
			if (self->view->wraparound) {
				self->cursor.column = 0;
				self->cursor.digit = 0;
				self->cursor.track = 0;
				invalidate = trackerview_scrollleft(self->view, self->cursor);
			}
	} else {
		++self->cursor.digit;
		if (self->cursor.digit >=
			trackdef_numdigits(trackdef, self->cursor.column)) {
			++self->cursor.column;
			self->cursor.digit = 0;
		}
	}
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
		workspace_setpatterneditposition(self->view->workspace,
			self->cursor);
		if (invalidate) {
			trackergrid_invalidatecursor(self, &oldcursor);
			trackergrid_invalidatecursor(self, &self->cursor);
			trackerview_invalidatecursor(self->view, &oldcursor);
			trackerview_invalidatecursor(self->view, &self->cursor);
		}
	}
}

void trackerview_prevline(TrackerView* self)
{
	trackergrid_prevline(&self->grid);
}

void trackerview_prevlines(TrackerView* self, uintptr_t lines, int wrap)
{
	trackergrid_prevlines(&self->grid, lines, wrap);
}

void trackerview_advanceline(TrackerView* self)
{
	trackergrid_advanceline(&self->grid);
}

void trackerview_advancelines(TrackerView* self, uintptr_t lines, int wrap)
{
	trackergrid_advancelines(&self->grid, lines, wrap);
}

void trackerview_home(TrackerView* self)
{
	trackergrid_home(&self->grid);
}

void trackerview_end(TrackerView* self)
{
	trackergrid_end(&self->grid);
}

void trackerview_selectall(TrackerView* self)
{
	if (self->workspace->song && self->gridstate.pattern) {
		self->grid.selection.topleft.offset = 0;
		self->grid.selection.topleft.track = 0;
		self->grid.selection.bottomright.offset = self->gridstate.pattern->length;
		self->grid.selection.bottomright.track =
			self->workspace->song->patterns.songtracks;
		self->grid.hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}
}

void trackerview_selectcol(TrackerView* self)
{
	if (self->workspace->song && self->gridstate.pattern) {
		self->grid.selection.topleft.offset = 0;
		self->grid.selection.topleft.track = self->grid.cursor.track;
		self->grid.selection.bottomright.offset = self->gridstate.pattern->length;
		self->grid.selection.bottomright.track = self->grid.cursor.track + 1;
		self->grid.hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}
}

void trackerview_selectbar(TrackerView* self)
{
	if (self->workspace->song && self->gridstate.pattern) {
		self->grid.selection.topleft.offset = self->grid.cursor.offset;
		self->grid.selection.topleft.track = self->grid.cursor.track;
		self->grid.selection.bottomright.offset = self->grid.cursor.offset + 4.0;
		if (self->grid.cursor.offset > self->gridstate.pattern->length) {
			self->grid.cursor.offset = self->gridstate.pattern->length;
		}
		self->grid.selection.bottomright.track = self->grid.cursor.track + 1;
		self->grid.hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}
}

int trackerview_scrollup(TrackerView* self, psy_audio_PatternEditPosition cursor)
{
	return trackergrid_scrollup(&self->grid, cursor);
}

int trackerview_scrolldown(TrackerView* self, psy_audio_PatternEditPosition cursor)
{
	return trackergrid_scrolldown(&self->grid, cursor);	
}

void trackerview_prevtrack(TrackerView* self)
{
	trackergrid_prevtrack(&self->grid);
}

void trackerview_nexttrack(TrackerView* self)
{
	trackergrid_nexttrack(&self->grid);	
}

int trackerview_scrollleft(TrackerView* self, psy_audio_PatternEditPosition cursor)
{	
	bool invalidate;

	invalidate = trackergrid_scrollleft(&self->grid, cursor);
	psy_ui_component_setscrollleft(&self->header.component,
		psy_ui_component_scrollleft(&self->grid.component));
	return invalidate;
}

int trackerview_scrollright(TrackerView* self, psy_audio_PatternEditPosition cursor)
{
	bool invalidate;
	
	invalidate = trackergrid_scrollright(&self->grid, cursor);
	psy_ui_component_setscrollleft(&self->header.component,
		psy_ui_component_scrollleft(&self->grid.component));
	return invalidate;	
}

void trackerview_onkeydown(TrackerView* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (psy_ui_component_visible(&self->interpolatecurveview.component)) {
			trackergrid_oninterpolatecurve(&self->grid, &self->grid.component);
		} else {
			trackerview_hideblockmenu(self);
		}
	} else {
		int cmd;
		cmd = psy_audio_inputs_cmd(&self->inputs,
			psy_audio_encodeinput(ev->keycode, ev->shift, ev->ctrl));
		trackerview_handlecommand(self, ev, cmd);
	}
}

void trackerview_handlecommand(TrackerView* self, psy_ui_KeyEvent* ev, int cmd)
{
	switch (cmd) {
	case CMD_NAVUP:
		trackerview_prevline(self);
		break;
	case CMD_NAVPAGEUP:
		trackerview_prevlines(self,
			psy_audio_player_lpb(&self->workspace->player), 0);
		break;
	case CMD_NAVDOWN:
		trackerview_advanceline(self);
		break;
	case CMD_NAVPAGEDOWN:
		trackerview_advancelines(self,
			psy_audio_player_lpb(&self->workspace->player), 0);
		break;
	case CMD_NAVLEFT:
		trackergrid_prevcol(&self->grid);
		break;
	case CMD_NAVRIGHT:
		trackergrid_nextcol(&self->grid);
		break;
	case CMD_NAVTOP:
		trackerview_home(self);
		break;
	case CMD_NAVBOTTOM:
		trackerview_end(self);
		break;
	case CMD_COLUMNPREV:
		trackerview_prevtrack(self);
		break;
	case CMD_COLUMNNEXT:
		trackerview_nexttrack(self);
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
		psy_ui_keyevent_stoppropagation(ev);
		break;
	case CMD_ROWCLEAR:
		trackergrid_rowclear(&self->grid);
		psy_ui_keyevent_stoppropagation(ev);
		return;
		break;
	case CMD_SELECTALL:
		trackerview_selectall(self);
		break;
	case CMD_SELECTCOL:
		trackerview_selectcol(self);
		break;
	case CMD_SELECTBAR:
		trackerview_selectbar(self);
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
	default:
		trackergrid_enterevent(&self->grid, ev);
		break;
	}
}

void trackergrid_enterevent(TrackerGrid* self, psy_ui_KeyEvent* ev)
{
	if (self->cursor.column != TRACKER_COLUMN_NOTE) {
		int digit = keycodetoint(ev->keycode);
		if (digit != -1) {
			trackergrid_inputvalue(self, digit, 1);
			psy_ui_keyevent_stoppropagation(ev);
			return;
		}
	}
	{
		psy_EventDriver* kbd;
		EventDriverCmd cmd;
		EventDriverData input;

		cmd.id = -1;
		kbd = workspace_kbddriver(self->view->workspace);
		input.message = EVENTDRIVER_KEYDOWN;
		input.param1 = psy_audio_encodeinput(ev->keycode, 0, ev->ctrl);
		kbd->cmd(kbd, input, &cmd);
		if (cmd.id == NOTECOMMANDS_RELEASE) {
			trackergrid_inputnote(self, NOTECOMMANDS_RELEASE,
				self->view->workspace->chordmode);
			psy_ui_keyevent_stoppropagation(ev);
			return;
		}
		if (cmd.id != -1 && cmd.id <= NOTECOMMANDS_RELEASE &&
			ev->shift && !self->view->workspace->chordmode
			&& ev->keycode != psy_ui_KEY_SHIFT) {
			self->view->workspace->chordmode = 1;
			self->chordbegin = self->cursor.track;
		}
	}
}

void trackerview_togglefollowsong(TrackerView* self)
{
	if (workspace_followingsong(self->workspace)) {
		workspace_stopfollowsong(self->workspace);
	} else {
		workspace_followsong(self->workspace);
	}
}

void trackergrid_rowdelete(TrackerGrid* self)
{
	if (self->cursor.offset - self->bpl >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		trackerview_prevline(self->view);
		node = psy_audio_pattern_findnode(self->gridstate->pattern, self->cursor.track,
			(psy_dsp_big_beat_t)self->cursor.offset, (psy_dsp_big_beat_t)self->bpl, &prev);
		if (node) {
			psy_audio_pattern_remove(self->gridstate->pattern, node);
			psy_audio_sequencer_checkiterators(
				&self->view->workspace->player.sequencer,
				node);
			psy_ui_component_invalidate(&self->view->linenumbers.component);
		}
		if (prev) {
			p = prev->next;
		} else {
			p = psy_audio_pattern_begin(self->gridstate->pattern);
		}
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_big_beat_t offset;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				event = *patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(self->gridstate->pattern, p);
				psy_audio_sequencer_checkiterators(
					&self->view->workspace->player.sequencer, p);
				offset -= (psy_dsp_big_beat_t)self->bpl;
				node = psy_audio_pattern_findnode(self->gridstate->pattern, track,
					offset,
					(psy_dsp_big_beat_t)self->bpl,
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*patternentry_front(entry) = event;
				} else {
					psy_audio_pattern_insert(self->gridstate->pattern, prev, track,
						(psy_dsp_big_beat_t)offset, &event);
				}
			}
		}
		psy_ui_component_invalidate(&self->view->component);
	}
}

void trackergrid_rowclear(TrackerGrid* self)
{
	if (self->cursor.column == TRACKER_COLUMN_NOTE) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* node;

		node = psy_audio_pattern_findnode(self->gridstate->pattern, self->cursor.track,
			(psy_dsp_big_beat_t)self->cursor.offset,
			(psy_dsp_big_beat_t)self->bpl, &prev);
		if (node) {
			psy_audio_pattern_remove(self->gridstate->pattern, node);
			if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
				psy_audio_sequencer_checkiterators(
					&self->view->workspace->player.sequencer,
					node);
				psy_ui_component_invalidate(&self->view->linenumbers.component);
			}
		}
		if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
			trackerview_advanceline(self->view);
		}
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->cursor.track);
		columndef = trackdef_columndef(trackdef, self->cursor.column);
		trackergrid_inputvalue(self, columndef->emptyvalue, 0);
	}
}

void trackerview_onkeyup(TrackerView* self, psy_ui_KeyEvent* ev)
{
	if (self->workspace->chordmode && ev->keycode == psy_ui_KEY_SHIFT) {
		self->grid.cursor.track = self->grid.chordbegin;
		trackerview_scrollleft(self, self->grid.cursor);
		trackerview_advanceline(self);
		self->workspace->chordmode = 0;
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void trackerview_oninput(TrackerView* self, psy_audio_Player* sender,
	psy_audio_PatternEvent* event)
{
	if (psy_ui_component_hasfocus(&self->grid.component) &&
		self->grid.cursor.column == TRACKER_COLUMN_NOTE &&
		event->note != NOTECOMMANDS_RELEASE) {
		trackerview_setdefaultevent(self,
			&sender->patterndefaults,
			event);
		trackergrid_inputevent(&self->grid, event,
			self->workspace->chordmode);
	}
}

void trackerview_setdefaultevent(TrackerView* self,
	psy_audio_Pattern* patterndefaults,
	psy_audio_PatternEvent* ev)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(patterndefaults, self->grid.cursor.track, 0,
		(psy_dsp_big_beat_t)self->grid.bpl, &prev);
	if (node) {
		psy_audio_PatternEvent* defaultevent;

		defaultevent = patternentry_front(psy_audio_patternnode_entry(node));
		if (defaultevent->inst != NOTECOMMANDS_INST_EMPTY) {
			ev->inst = defaultevent->inst;
		}
		if (defaultevent->mach != NOTECOMMANDS_MACH_EMPTY) {
			ev->mach = defaultevent->mach;
		}
		if (defaultevent->vol != NOTECOMMANDS_VOL_EMPTY) {
			ev->vol = defaultevent->vol;
		}
	}
}

void trackerview_enablesync(TrackerView* self)
{
	self->opcount = self->gridstate.pattern->opcount;
	self->syncpattern = 1;
}

void trackerview_preventsync(TrackerView* self)
{
	self->opcount = self->gridstate.pattern->opcount;
	self->syncpattern = 0;
}

void trackergrid_inputevent(TrackerGrid* self,
	const psy_audio_PatternEvent* event, int chordmode)
{
	trackerview_preventsync(self->view);
	psy_undoredo_execute(&self->view->workspace->undoredo,
		&InsertCommandAlloc(self->gridstate->pattern, self->bpl,
			self->cursor, *event, self->view->workspace)->command);
	if (chordmode) {
		trackerview_nexttrack(self->view);
	} else {
		trackerview_advanceline(self->view);
	}
	trackerview_enablesync(self->view);
}


void trackergrid_inputnote(TrackerGrid* self, psy_dsp_note_t note,
	int chordmode)
{
	psy_audio_Machine* machine;
	psy_audio_PatternEvent event;

	patternevent_init_all(&event,
		note,
		NOTECOMMANDS_INST_EMPTY,
		(unsigned char)psy_audio_machines_slot(&self->view->workspace->song->machines),
		NOTECOMMANDS_VOL_EMPTY,
		0,
		0);
	machine = psy_audio_machines_at(&self->view->workspace->song->machines, event.mach);
	if (machine &&
		machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
		event.inst = self->view->workspace->song->instruments.slot.subslot;
	}
	trackergrid_inputevent(self, &event, chordmode);
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

		trackdef = trackergridstate_trackdef(self->gridstate, self->cursor.track);
		columndef = trackdef_columndef(trackdef, self->cursor.column);
		patternentry_init(&newentry);
		node = psy_audio_pattern_findnode(self->gridstate->pattern,
			self->cursor.track,
			(psy_dsp_big_beat_t)self->cursor.offset,
			(psy_dsp_big_beat_t)self->bpl,
			&prev);
		if (node) {
			entry = (psy_audio_PatternEntry*)node->entry;
		} else {
			entry = &newentry;
		}
		if (digit) {
			enterdigitcolumn(self->view, entry,
				self->cursor.track,
				self->cursor.column,
				self->cursor.digit,
				value);
		} else {
			entervaluecolumn(entry, self->cursor.column, value);
		}
		trackerview_preventsync(self->view);
		psy_undoredo_execute(&self->view->workspace->undoredo,
			&InsertCommandAlloc(self->gridstate->pattern, self->bpl,
				self->cursor,
				*patternentry_front(entry),
				self->view->workspace)->command);
		if (!digit) {
			if (columndef->wrapclearcolumn == TRACKER_COLUMN_NONE) {
				trackergrid_nextcol(self);
			} else {
				self->cursor.digit = 0;
				self->cursor.column = columndef->wrapclearcolumn;
				trackerview_advanceline(self->view);
			}
		} else
			if (self->cursor.digit + 1 >= columndef->numdigits) {
				if (columndef->wrapeditcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->cursor.digit = 0;
					self->cursor.column = columndef->wrapeditcolumn;
					trackerview_advanceline(self->view);
				}
			} else {
				trackergrid_nextcol(self);
			}
		trackergrid_invalidatecursor(self, &self->cursor);
		trackerview_enablesync(self->view);
		patternentry_dispose(&newentry);
	}
}

int keycodetoint(unsigned int keycode) {
	int rv = -1;

	if (keycode >= '0' && keycode <= '9') {
		rv = keycode - '0';
	} else
		if (keycode >= 'A' && keycode <= 'Z') {
			rv = keycode - 'A' + 10;
		}
	return rv;
}

void enterdigitcolumn(TrackerView* self, psy_audio_PatternEntry* entry,
	int track, int column, int digit, int digitvalue)
{
	TrackDef* trackdef;

	trackdef = trackergridstate_trackdef(&self->gridstate, track);
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

void enterdigit(int digit, int newval, unsigned char* val)
{
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else
		if (digit == 1) {
			*val = (*val & 0xF0) | (newval & 0x0F);
		}
}

void digitlohi(int value, int digit, uintptr_t size, uint8_t* lo, uint8_t* hi)
{
	uintptr_t pos;

	pos = (size - 1) - digit / 2;
	lohi((uint8_t*)&value + pos, digit, lo, hi);
}

void lohi(uint8_t* value, int digit, uint8_t* lo, uint8_t* hi)
{
	*lo = *value & 0x0F;
	*hi = (*value & 0xF0) >> 4;
}

void entervaluecolumn(psy_audio_PatternEntry* entry, int column, int value)
{
	psy_audio_PatternEvent* event;

	event = patternentry_front(entry);
	switch (column) {
	case TRACKER_COLUMN_INST:
		event->inst = value;
		break;
	case TRACKER_COLUMN_MACH:
		event->mach = value;
		break;
	case TRACKER_COLUMN_VOL:
		event->vol = value;
		break;
	case TRACKER_COLUMN_CMD:
		event->cmd = value;
		break;
	case TRACKER_COLUMN_PARAM:
		event->parameter = value;
		break;
	default:
		break;
	}
}

void trackergrid_invalidatecursor(TrackerGrid* self,
	const psy_audio_PatternEditPosition* cursor)
{	
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_rectangle_make(
			trackergridstate_track_x(self->gridstate, cursor->track),
			self->linestate->lineheight *
			trackerviewlinestate_offsettoscreenline(self->linestate,
				cursor->offset),
			trackergridstate_trackwidth(self->gridstate, cursor->track),
			self->linestate->lineheight));
}

void trackerview_invalidatecursor(TrackerView* self, const psy_audio_PatternEditPosition* cursor)
{
	trackerlinenumbers_invalidatecursor(&self->linenumbers, cursor);
}

void trackerview_invalidateline(TrackerView* self, psy_dsp_big_beat_t offset)
{
	int line;	

	if (offset >= self->linestate.sequenceentryoffset &&
		offset < self->linestate.sequenceentryoffset + self->gridstate.pattern->length) {
		psy_ui_Size size;

		size = psy_ui_component_size(&self->component);
		line = (int)((offset - self->linestate.sequenceentryoffset)
			/ self->grid.bpl);		
		psy_ui_component_invalidaterect(&self->grid.component,
			psy_ui_rectangle_make(
				-psy_ui_component_scrollleft(&self->grid.component),
				self->linestate.lineheight * line,
				psy_ui_value_px(&size.width, &self->grid.tm),
				self->linestate.lineheight));
	}
}

void trackerview_ongridscroll(TrackerView* self, psy_ui_Component* sender,
	int stepx, int stepy)
{
	if (stepx != 0) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->grid.component));
		psy_ui_component_setscrollleft(&self->griddefaults.component,
			psy_ui_component_scrollleft(&self->grid.component));
		psy_ui_component_invalidate(&self->griddefaults.component);
		psy_ui_component_invalidate(&self->header.component);
	}
	if (stepy != 0) {
		psy_ui_component_setscrolltop(&self->linenumbers.component,
			psy_ui_component_scrolltop(&self->grid.component));
		psy_ui_component_invalidate(&self->linenumbers.component);
		psy_ui_component_update(&self->linenumbers.component);
	}
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender, int stepx,
	int stepy)
{
	if (psy_ui_component_scrollleft(&self->component) < 0) {
		psy_ui_component_setscrollleft(&self->component, 0);
	}
	if (self->midline) {
		int halfvisilines;
		int restoremidline;
		psy_ui_Size size;

		size = psy_ui_component_size(&self->component);
		halfvisilines = self->linestate->visilines / 2;
		restoremidline = self->midline;
		self->midline = 0;
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_rectangle_make(
				psy_ui_component_scrollleft(&self->component),
				halfvisilines * self->linestate->lineheight +
				psy_ui_component_scrolltop(&self->component),
				psy_ui_value_px(&size.width, &self->tm),
				self->linestate->lineheight * 2
		));
		self->midline = restoremidline;
	}
}

void trackergrid_onmousedown(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (psy_ui_component_visible(&self->view->interpolatecurveview.component)) {
			trackergrid_oninterpolatecurve(self, &self->component);
		} else {
			trackerview_toggleblockmenu(self->view);
		}
	} else
		if (self->gridstate->pattern) {
			if (ev->button == 1) {
				psy_audio_PatternEditPosition oldcursor;

				self->dragcolumn = trackergrid_resizecolumn(self, ev->x, ev->y);
				if (self->dragcolumn == -1) {
					oldcursor = self->cursor;
					self->cursor = trackergrid_makecursor(self, ev->x, ev->y);
					self->selection.topleft = self->cursor;
					self->dragselectionbase = self->cursor;
					self->lastdragcursor = self->cursor;
					workspace_setpatterneditposition(self->view->workspace, self->cursor);
					if (self->hasselection) {
						self->hasselection = 0;
						psy_ui_component_invalidate(&self->component);
					}
					self->hasselection = 0;
					trackergrid_invalidatecursor(self, &oldcursor);
					trackergrid_invalidatecursor(self, &self->cursor);
					trackerview_invalidatecursor(self->view, &oldcursor);
					trackerview_invalidatecursor(self->view, &self->cursor);
					psy_ui_component_setfocus(&self->component);
					psy_ui_component_capture(&self->component);
				} else {
					self->dragcolumnbase = ev->x;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
				}
			}
		}
}

void trackergrid_onmousemove(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_PatternEditPosition cursor;

	if (ev->button == 1) {
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
		if (self->columnresize) {
			trackergrid_dragcolumn(self, ev);
		}
	}
}

void trackergrid_startdragselection(TrackerGrid* self, psy_audio_PatternEditPosition cursor)
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

void trackergrid_dragselection(TrackerGrid* self, psy_audio_PatternEditPosition cursor)
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
		trackerview_scrollup(self->view, cursor);
	} else {
		trackerview_scrolldown(self->view, cursor);
	}
	if (cursor.track < self->lastdragcursor.track) {
		trackerview_scrollleft(self->view, cursor);
	} else {
		trackerview_scrollright(self->view, cursor);
	}
	self->midline = restoremidline;
}

void trackergrid_dragcolumn(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (self->dragcolumn != -1) {
		uintptr_t track;
		TrackDef* trackdef;

		track = trackergridstate_screentotrack(self->gridstate, ev->x +
			psy_ui_component_scrollleft(&self->component), self->gridstate->numtracks);
		trackdef = trackergridstate_trackdef(self->gridstate, track);
		if (ev->x > self->dragcolumnbase) {
			if (trackdef != &self->gridstate->defaulttrackdef) {
				trackdef->numfx++;
			} else {
				trackdef = malloc(sizeof(TrackDef));
				trackdef_init(trackdef);
				trackdef->numfx = 2;
				psy_table_insert(&self->gridstate->trackconfigs, track, trackdef);
			}
		} else
			if (ev->x < self->dragcolumnbase && trackdef->numfx > 1) {
				if (trackdef != &self->gridstate->defaulttrackdef) {
					trackdef->numfx--;
					if (trackdef->numfx == 1) {
						free(trackdef);
						psy_table_remove(&self->gridstate->trackconfigs, track);
					}
				}
			}
		self->dragcolumnbase = ev->x;
		psy_ui_component_invalidate(&self->view->component);
		trackerview_updatescrollstep(self->view);
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
		trackerview_selectcol(self->view);
	}
}

void trackergrid_onmousewheel(TrackerGrid* self, psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (ev->ctrl) {
		if (ev->delta > 0) {
			zoombox_setrate(&self->view->zoombox, zoombox_rate(&self->view->zoombox) + 0.25);
		} else
			if (ev->delta < 0) {
				zoombox_setrate(&self->view->zoombox, zoombox_rate(&self->view->zoombox) - 0.25);
			}
		ev->preventdefault = 1;
	}
}

int trackergrid_resizecolumn(TrackerGrid* self, int x, int y)
{
	int rv;
	TrackDef* trackdef;
	psy_audio_PatternEditPosition position;
	int lines;
	int coloffset;
	int cpx;

	rv = -1;
	position.offset = trackerviewlinestate_offset(self->linestate, y, &lines);
	position.track = trackergridstate_screentotrack(self->gridstate, x, self->gridstate->numtracks);
	coloffset = (x - self->gridstate->patterntrackident) -
		trackergridstate_track_x(self->gridstate, position.track);
	position.column = 0;
	position.digit = 0;
	trackdef = trackergridstate_trackdef(self->gridstate, position.track);
	cpx = 0;
	while (position.column < trackdef_numcolumns(trackdef) &&
		cpx + trackdef_columnwidth(trackdef, position.column,
			self->gridstate->textwidth) < coloffset) {
		cpx += trackdef_columnwidth(trackdef, position.column,
			self->gridstate->textwidth);
		++position.column;
	}
	position.digit = (coloffset - cpx) / self->gridstate->textwidth;
	if (position.digit >= trackdef_columndef(trackdef, position.column)->numchars) {
		rv = position.column;
	}
	return rv;
}

psy_audio_PatternEditPosition trackergrid_makecursor(TrackerGrid* self, int x, int y)
{
	psy_audio_PatternEditPosition rv;
	TrackDef* trackdef;
	int lines;
	int coloffset;
	int cpx;

	rv.offset = trackerviewlinestate_offset(self->linestate, y, &lines);
	if (self->gridstate->pattern && rv.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
		rv.offset = psy_audio_pattern_length(self->gridstate->pattern) - self->bpl;
	}
	rv.track = trackergridstate_screentotrack(self->gridstate, x, self->gridstate->numtracks);
	if (rv.track >= psy_audio_player_numsongtracks(&self->view->workspace->player)) {
		rv.track = psy_audio_player_numsongtracks(&self->view->workspace->player) - 1;
	}
	coloffset = (x - self->gridstate->patterntrackident) -
		trackergridstate_track_x(self->gridstate, rv.track);
	rv.column = 0;
	rv.digit = 0;
	trackdef = trackergridstate_trackdef(self->gridstate, rv.track);
	cpx = 0;
	while (rv.column < trackdef_numcolumns(trackdef) &&
		cpx + trackdef_columnwidth(trackdef, rv.column,
			self->gridstate->textwidth) < coloffset) {
		cpx += trackdef_columnwidth(trackdef, rv.column,
			self->gridstate->textwidth);
		++rv.column;
	}
	rv.digit = (coloffset - cpx) / self->gridstate->textwidth;
	if (rv.digit >= trackdef_numdigits(trackdef, rv.column)) {
		rv.digit = trackdef_numdigits(trackdef, rv.column) - 1;
	}
	self->cursor.pattern =
		workspace_patterneditposition(self->view->workspace).pattern;
	return rv;
}

void trackergrid_onmouseup(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_component_releasecapture(&self->component);
		self->dragcolumn = -1;
		if (self->hasselection) {
			interpolatecurveview_setselection(&self->view->interpolatecurveview,
				self->selection);
		}
	}
}

void trackergrid_onfocuslost(TrackerGrid* self, psy_ui_Component* sender)
{
	psy_ui_component_invalidate(&self->component);
}

// trackerview
static void trackerview_numtrackschanged(TrackerView* self, psy_audio_Player* player,
	unsigned int numsongtracks);

static psy_ui_ComponentVtable trackerview_vtable;
static int trackerview_vtable_initialized = 0;

static void trackerview_vtable_init(TrackerView* self)
{
	if (!trackerview_vtable_initialized) {
		trackerview_vtable = *(self->component.vtable);
		trackerview_vtable.onalign = (psy_ui_fp_onalign)
			trackerview_onalign;
		trackerview_vtable.onkeydown = (psy_ui_fp_onkeydown)
			trackerview_onkeydown;
		trackerview_vtable.onkeyup = (psy_ui_fp_onkeydown)
			trackerview_onkeyup;
		trackerview_vtable_initialized = 1;
	}
}

void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	psy_ui_Component* patternview,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerview_vtable_init(self);
	self->component.vtable = &trackerview_vtable;
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		trackerview_ondestroy);
	psy_signal_connect(&self->component.signal_timer, self,
		trackerview_ontimer);
	trackerview_computefontheight(self);
	self->patternview = patternview;
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;
	self->doseqtick = 0;
	self->cursorstep = 1;
	self->wraparound = 1;
	self->showlinenumbers = 1;
	self->showlinenumbercursor = 1;
	self->showlinenumbersinhex = 1;
	self->showemptydata = 0;
	self->showdefaultline = 1;	
	trackerview_initdefaultskin(self);
	// shared states
	trackerviewlinestate_init(&self->linestate);
	self->linestate.skin = &self->skin;
	trackergridstate_init(&self->gridstate);
	self->gridstate.skin = &self->skin;
	trackerview_initcolumns(self);
	trackerview_initmetrics(self);
	trackerview_initinputs(self);	
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
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->left, self);
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
	trackerheader_init(&self->header, patternview, &self->gridstate,
		self->workspace);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// pattern default line
	trackergrid_init(&self->griddefaults, &self->component, self,
		&self->gridstate, &self->linestate, workspace);
	self->griddefaults.component.overflow = psy_ui_OVERFLOW_HIDDEN;
	psy_ui_component_setalign(&self->griddefaults.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->griddefaults.component,
		psy_ui_size_make(psy_ui_value_makepx(0),
			psy_ui_value_makeeh(1.1)));
	self->griddefaults.editmode = TRACKERGRID_EDITMODE_LOCAL;
	self->griddefaults.columnresize = 1;
	trackergrid_setpattern(&self->griddefaults, &self->workspace->player.patterndefaults);
	// pattern main grid
	trackergrid_init(&self->grid, &self->component, self, &self->gridstate,
		&self->linestate, workspace);
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->grid.component.signal_scroll, self,
		trackerview_ongridscroll);
	trackerview_connectworkspace(self);
	trackerview_connectplayer(self);
	trackerview_readconfig(self);
	psy_signal_connect(&self->workspace->player.signal_numsongtrackschanged, self,
		trackerview_numtrackschanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
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
	patternblockmenu_init(&self->blockmenu, &self->component);
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
	psy_signal_connect(&self->blockmenu.interpolatecurve.signal_clicked, &self->grid,
		trackergrid_oninterpolatecurve);
	psy_signal_connect(&self->blockmenu.changegenerator.signal_clicked, &self->grid,
		trackergrid_onchangegenerator);
}

void trackerview_connectplayer(TrackerView* self)
{
	psy_signal_connect(&self->workspace->player.signal_lpbchanged, self,
		trackerview_onlpbchanged);
	psy_signal_connect(&self->workspace->player.signal_inputevent, self,
		trackerview_oninput);
	psy_signal_connect(&self->workspace->player.sequencer.signal_linetick,
		self, trackerview_onseqlinetick);
}

void trackerview_connectworkspace(TrackerView* self)
{
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		trackerview_onconfigchanged);
	psy_signal_connect(&self->workspace->signal_patterneditpositionchanged, self,
		trackerview_onpatterneditpositionchanged);
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		trackerview_onparametertweak);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		trackerview_onskinchanged);
}

void trackerview_ondestroy(TrackerView* self, psy_ui_Component* sender)
{
	psy_audio_inputs_dispose(&self->inputs);
	trackergridstate_dispose(&self->gridstate);	
}

void trackerview_initdefaultskin(TrackerView* self)
{
	psy_ui_bitmap_init(&self->skin.bitmap);
	psy_ui_bitmap_loadresource(&self->skin.bitmap, IDB_HEADERSKIN);
	trackerview_setclassicheadercoords(self);
}

void trackerview_setheadercoords(TrackerView* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 79, 40, 17, 17, 75, 66, 3, 17, 17 };
	static SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11, 0 };
	static SkinCoord digitx0 = { 0, 23, 9, 17, 15, 3, 9, 17, 0 };
	static SkinCoord digit0x = { 0, 23, 9, 17, 22, 3, 9, 17, 0 };

	self->skin.headercoords.background = background;
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;
}

void trackerview_setclassicheadercoords(TrackerView* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 79, 40, 17, 17, 66, 3, 17, 17, 0 };
	static SkinCoord solo = { 62, 40, 17, 17, 47, 3, 17, 17, 0 };
	static SkinCoord digitx0 = { 0, 23, 9, 17, 15, 3, 9, 17, 0 };
	static SkinCoord digit0x = { 0, 23, 9, 17, 22, 3, 9, 17, 0 };

	self->skin.headercoords.background = background;
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;
}

void trackerview_setheadertextcoords(TrackerView* self)
{
	SkinCoord background = { 2, 57, 103, 23, 0, 0, 103, 23, 0 };
	SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	SkinCoord mute = { 81, 18, 11, 11, 75, 3, 11, 11, 0 };
	SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11, 0 };
	SkinCoord digitx0 = { 0, 80, 6, 12, 5, 8, 6, 12, 0 };
	SkinCoord digit0x = { 0, 80, 6, 12, 11, 8, 6, 12, 0 };

	self->skin.headercoords.background = background;
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;
}

// trackerheader

// trackerheader
static int trackerheader_preferredtrackwidth(TrackerHeader*);
static void trackerheader_ondraw(TrackerHeader*, psy_ui_Graphics* g);
static void trackerheader_onmousedown(TrackerHeader*, psy_ui_MouseEvent*);
static void trackerheader_onpreferredsize(TrackerHeader*, psy_ui_Size* limit, psy_ui_Size* rv);
static void trackerheader_onpatterneditpositionchanged(TrackerHeader*,
	Workspace*);

static psy_ui_ComponentVtable trackerheader_vtable;
static int trackerheader_vtable_initialized = 0;

static void trackerheader_vtable_init(TrackerHeader* self)
{
	if (!trackerheader_vtable_initialized) {
		trackerheader_vtable = *(self->component.vtable);
		trackerheader_vtable.ondraw = (psy_ui_fp_ondraw)trackerheader_ondraw;
		trackerheader_vtable.onmousedown = (psy_ui_fp_onmousedown)trackerheader_onmousedown;
		trackerheader_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			trackerheader_onpreferredsize;
		trackerheader_vtable_initialized = 1;
	}
}

void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackerGridState* gridstate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerheader_vtable_init(self);
	self->component.vtable = &trackerheader_vtable;	
	trackerheader_setsharedgridstate(self, gridstate);
	//psy_ui_component_setbackgroundcolor(&self->component, view->skin.background);	
	psy_ui_component_doublebuffer(&self->component);
	self->gridstate->numtracks = 16;
	self->classic = 1;
	self->workspace = workspace;
	self->currtrack = 0;
	psy_signal_connect(&self->workspace->signal_patterneditpositionchanged, self,
		trackerheader_onpatterneditpositionchanged);
}

void trackerheader_setsharedgridstate(TrackerHeader* self, TrackerGridState*
	gridstate)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		trackergridstate_init(&self->defaultgridstate);
		self->gridstate = &self->defaultgridstate;
	}
}

void trackerheader_ondraw(TrackerHeader* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int cpx = 0;
	uintptr_t track;
	psy_audio_PatternEditPosition editposition;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	cpx = 0;
	editposition = workspace_patterneditposition(self->workspace);
	psy_ui_drawsolidrectangle(g,
		psy_ui_rectangle_make(psy_ui_component_scrollleft(&self->component), 0,
			psy_ui_value_px(&size.width, &tm), psy_ui_value_px(&size.height, &tm)),
		self->gridstate->skin->background);
	for (track = 0; track < self->gridstate->numtracks; ++track) {
		int trackx0 = track / 10;
		int track0x = track % 10;
		SkinCoord digitx0 = self->gridstate->skin->headercoords.digitx0;
		SkinCoord digit0x = self->gridstate->skin->headercoords.digit0x;
		digitx0.srcx += trackx0 * digitx0.srcwidth;
		digit0x.srcx += track0x * digit0x.srcwidth;
		skin_blitpart(g, &self->gridstate->skin->bitmap, cpx, 0,
			&self->gridstate->skin->headercoords.background);
		skin_blitpart(g, &self->gridstate->skin->bitmap, cpx, 0, &digitx0);
		skin_blitpart(g, &self->gridstate->skin->bitmap, cpx, 0, &digit0x);
		if (self->workspace->song) {
			if (patterns_istrackmuted(&self->workspace->song->patterns,
				track)) {
				skin_blitpart(g, &self->gridstate->skin->bitmap, cpx, 0,
					&self->gridstate->skin->headercoords.mute);
			}
			if (patterns_istracksoloed(&self->workspace->song->patterns,
				track)) {
				skin_blitpart(g, &self->gridstate->skin->bitmap, cpx, 0,
					&self->gridstate->skin->headercoords.solo);
			}
		}
		if (track == editposition.track) {
			
			psy_ui_setcolor(g, self->gridstate->skin->font);
			psy_ui_drawline(g, cpx, 0,
				min(cpx + self->gridstate->skin->headercoords.background.destwidth,
					cpx + trackergridstate_trackwidth(self->gridstate, track)), 0);
		}
		cpx += trackergridstate_trackwidth(self->gridstate, track);
	}	
}

void trackerheader_onmousedown(TrackerHeader* self, psy_ui_MouseEvent* ev)
{
	if (self->workspace->song) {
		psy_ui_Rectangle r;
		uintptr_t track;
		int track_x;		

		track = trackergridstate_screentotrack(self->gridstate,
			ev->x + psy_ui_component_scrollleft(&self->component),
			psy_audio_player_numsongtracks(&self->workspace->player));
		track_x = trackergridstate_track_x(self->gridstate, track);
		psy_ui_setrectangle(&r,
			self->gridstate->skin->headercoords.mute.destx + track_x,
			self->gridstate->skin->headercoords.mute.desty,
			self->gridstate->skin->headercoords.mute.destwidth,
			self->gridstate->skin->headercoords.mute.destheight);
		if (psy_ui_rectangle_intersect(&r, ev->x +
				psy_ui_component_scrollleft(&self->component), ev->y)) {
			if (patterns_istrackmuted(&self->workspace->song->patterns,
				track)) {
				patterns_unmutetrack(&self->workspace->song->patterns,
					track);
			} else {
				patterns_mutetrack(&self->workspace->song->patterns,
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
			if (patterns_istracksoloed(&self->workspace->song->patterns,
				track)) {
				patterns_deactivatesolotrack(
					&self->workspace->song->patterns);
			} else {
				patterns_activatesolotrack(
					&self->workspace->song->patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		} else {
			psy_audio_PatternEditPosition cursor;

			cursor = workspace_patterneditposition(self->workspace);
			cursor.track = track;
			workspace_setpatterneditposition(self->workspace, cursor);
			++self->gridstate->pattern->opcount;
		}
	}
}

void trackerheader_onpreferredsize(TrackerHeader* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(0);
	rv->height = psy_ui_value_makepx(30);
}

void trackerheader_onpatterneditpositionchanged(TrackerHeader* self,
	Workspace* sender)
{
	psy_audio_PatternEditPosition cursor;

	cursor = workspace_patterneditposition(sender);
	if (self->currtrack != cursor.track) {
		self->currtrack = cursor.track;
		psy_ui_component_invalidate(&self->component);
	}
}

// TrackerLineNumbers

static void trackerlinenumbers_ondraw(TrackerLineNumbers*, psy_ui_Graphics*);
static void trackerlinenumbers_onpreferredsize(TrackerLineNumbers*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable trackerlinenumbers_vtable;
static int trackerlinenumbers_vtable_initialized = 0;

static void trackerlinenumbers_vtable_init(TrackerLineNumbers* self)
{
	if (!trackerlinenumbers_vtable_initialized) {
		trackerlinenumbers_vtable = *(self->component.vtable);
		trackerlinenumbers_vtable.ondraw = (psy_ui_fp_ondraw)trackerlinenumbers_ondraw;
		trackerlinenumbers_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			trackerlinenumbers_onpreferredsize;
		trackerlinenumbers_vtable_initialized = 1;
	}
}

void trackerlinenumbers_init(TrackerLineNumbers* self, psy_ui_Component* parent,
	TrackerViewLineState* linestate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumbers_vtable_init(self);
	self->component.vtable = &trackerlinenumbers_vtable;
	self->workspace = workspace;
	trackerlinenumbers_setsharedlinestate(self, linestate);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setbackgroundcolor(&self->component,
		self->linestate->skin->background);
}

void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers* self, TrackerViewLineState*
	linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerviewlinestate_init(&self->defaultlinestate);
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
		psy_audio_PatternEditPosition cursor;
				
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		topoffset = trackerviewlinestate_offset(self->linestate, g->clip.top,
			&topline);
		bottomoffset = trackerviewlinestate_offset(self->linestate, g->clip.bottom,
			&bottomline);
		cpy = (topline) * self->linestate->lineheight;
		offset = topoffset;
		line = topline;
		cursor = workspace_patterneditposition(self->workspace);
		while (offset <= bottomoffset &&
			offset < self->linestate->pattern->length) {
			psy_ui_Rectangle r;
			TrackerColumnFlags columnflags;
			int ystart;
			int drawbeat;

			drawbeat = workspace_showbeatoffset(self->workspace);
			columnflags.playbar = psy_audio_player_playing(&self->workspace->player) && 
				trackerviewlinestate_testplaybar(self->linestate, offset);
			columnflags.mid = 0;
			columnflags.cursor = self->linestate->drawcursor &&
				testcursor(cursor, cursor.track, offset, self->linestate->lpb);
			columnflags.beat = fmod(offset, 1.0f) == 0.0f;
			columnflags.beat4 = fmod(offset, 4.0f) == 0.0f;
			columnflags.selection = 0;
			setcolumncolor(self->linestate->skin, g, columnflags);
			
			if (workspace_showlinenumbersinhex(self->workspace)) {
				if (drawbeat) {
					psy_snprintf(buffer, 10, "%.2X %.3f", line, offset);
				} else {
					psy_snprintf(buffer, 10, "%.2X", line);
				}
			} else {
				if (drawbeat) {
					psy_snprintf(buffer, 10, "%.2d %.3f", line, offset);
				} else {
					psy_snprintf(buffer, 10, "%.2d", line);
				}
			}
			r = psy_ui_rectangle_make(0, cpy,
				psy_ui_value_px(&size.width - 2, &tm), tm.tmHeight);
			psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_OPAQUE, r, buffer,
				strlen(buffer));
			cpy += self->linestate->lineheight;
			ystart = cpy;
			offset += (1.0 / self->linestate->lpb);
			++line;
		}
	}
}

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers* self,
	const psy_audio_PatternEditPosition* cursor)
{	
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	int line;
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	line = trackerviewlinestate_offsettoscreenline(self->linestate,
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
					self->linestate->lineheight
				));
		}
	}
}

void trackerlinenumbers_onpreferredsize(TrackerLineNumbers* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makeew(12);
	rv->height = psy_ui_value_makepx(0);
}

// LineNumbersLabel
static void trackerlinenumberslabel_onmousedown(TrackerLineNumbersLabel*,
	psy_ui_MouseEvent* ev);
static void trackerlinenumberslabel_ondraw(TrackerLineNumbersLabel*, psy_ui_Graphics*);
static void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel*,
	psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable trackerlinenumberslabel_vtable;
static int trackerlinenumberslabel_vtable_initialized = 0;

static void trackerlinenumberslabel_vtable_init(TrackerLineNumbersLabel* self)
{
	if (!trackerlinenumberslabel_vtable_initialized) {
		trackerlinenumberslabel_vtable = *(self->component.vtable);
		trackerlinenumberslabel_vtable.ondraw = (psy_ui_fp_ondraw)
			trackerlinenumberslabel_ondraw;
		trackerlinenumberslabel_vtable.onmousedown = (psy_ui_fp_onmousedown)
			trackerlinenumberslabel_onmousedown;
		trackerlinenumberslabel_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			trackerlinenumberslabel_onpreferredsize;
	}
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerView* view)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumberslabel_vtable_init(self);
	self->component.vtable = &trackerlinenumberslabel_vtable;
	self->view = view;
}

void trackerlinenumberslabel_onmousedown(TrackerLineNumbersLabel* self,
	psy_ui_MouseEvent* ev)
{
	self->view->header.classic = !self->view->header.classic;
	if (self->view->header.classic) {
		trackerview_setclassicheadercoords(self->view);
	} else {
		trackerview_setheadercoords(self->view);
	}
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
	psy_ui_setbackgroundcolor(g, self->view->skin.background);
	psy_ui_settextcolor(g, self->view->skin.font);
	psy_ui_textoutrectangle(g, r.left, 0, 0, r, "Line", strlen("Line"));
	if (self->view->showdefaultline) {
		psy_ui_textoutrectangle(g, r.left, headersize.height, 0,
			r, "Defaults", strlen("Defaults"));
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
		height += tm.tmHeight + 1;
	}	
	rv->height = psy_ui_value_makepx(height);
}

void trackerview_ontimer(TrackerView* self, psy_ui_Component* sender, uintptr_t timerid)
{
	if (self->gridstate.pattern) {
		if (self->doseqtick && psy_audio_player_playing(&self->workspace->player)) {
			if (!workspace_followingsong(self->workspace)) {
				trackerview_invalidateline(self, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->linestate.lastplayposition);
				self->linestate.lastplayposition =
					psy_audio_player_position(&self->workspace->player);
				trackerview_invalidateline(self, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->linestate.lastplayposition);
			}
		} else {
			if (self->linestate.lastplayposition != -1) {
				trackerview_invalidateline(self,
					self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->linestate.lastplayposition);
				self->linestate.lastplayposition = -1;
			}
		}
		if (self->gridstate.pattern && self->gridstate.pattern->opcount != self->opcount &&
			self->syncpattern) {
			psy_ui_component_invalidate(&self->grid.component);
			psy_ui_component_invalidate(&self->linenumbers.component);
		}
		self->opcount = self->gridstate.pattern ? self->gridstate.pattern->opcount : 0;
	}
}

void trackerview_onseqlinetick(TrackerView* self, psy_audio_Sequencer* sender)
{
	if (psy_audio_sequencer_playing(sender)) {
		self->doseqtick = 1;
	}
}

void trackerview_onlpbchanged(TrackerView* self, psy_audio_Player* sender, uintptr_t lpb)
{
	self->linestate.lpb = lpb;
	self->grid.bpl = 1 / (psy_dsp_big_beat_t)lpb;
}

void trackerview_onconfigchanged(TrackerView* self, Workspace* workspace,
	psy_Properties* property)
{
	if (property == workspace->config) {
		trackerview_readconfig(self);
	} else
		if (psy_properties_insection(property, workspace->patternviewtheme)) {
			TrackerViewApplyProperties(self, workspace->patternviewtheme);
		} else
			if (strcmp(psy_properties_key(property), "wraparound") == 0) {
				self->wraparound = psy_properties_value(property);
				psy_ui_component_invalidate(&self->component);
			} else
				if (strcmp(psy_properties_key(property), "beatoffset") == 0) {
					psy_ui_component_align(&self->component);
				} else
					if (strcmp(psy_properties_key(property), "griddefaults") == 0) {
						if (psy_properties_value(property)) {
							psy_ui_component_show(&self->griddefaults.component);
							self->showdefaultline = 1;
						} else {
							psy_ui_component_hide(&self->griddefaults.component);
							self->showdefaultline = 0;
						}
						psy_ui_component_align(&self->left);
						psy_ui_component_align(&self->component);
					} else
						if (strcmp(psy_properties_key(property), "linenumbers") == 0) {
							trackerview_showlinenumbers(self, psy_properties_value(property));
						} else
							if (strcmp(psy_properties_key(property), "linenumberscursor") == 0) {
								trackerview_showlinenumbercursor(self, psy_properties_value(property));
							} else
								if (strcmp(psy_properties_key(property), "linenumbersinhex") == 0) {
									trackerview_showlinenumbersinhex(self, psy_properties_value(property));
								} else
									if (strcmp(psy_properties_key(property), "wideinstcolumn") == 0) {
										trackerview_initcolumns(self);
										trackerview_computemetrics(self);
									} else
										if (strcmp(psy_properties_key(property), "drawemptydata") == 0) {
											trackerview_showemptydata(self, psy_properties_value(property));
										} else
											if (strcmp(psy_properties_key(property), "centercursoronscreen") == 0) {
												trackerview_setcentermode(self, psy_properties_value(property));
											} else
												if (strcmp(psy_properties_key(property), "notetab") == 0) {
													self->grid.notestabmode = self->griddefaults.notestabmode =
														workspace_notetabmode(self->workspace);
												} else
													if (strcmp(psy_properties_key(property), "font") == 0) {
														psy_ui_FontInfo fontinfo;
														psy_ui_Font font;

														psy_ui_fontinfo_init_string(&fontinfo,
															psy_properties_valuestring(property));
														psy_ui_font_init(&font, &fontinfo);
														trackerview_setfont(self, &font, TRUE);
														psy_ui_font_dispose(&font);
														psy_ui_component_align(&self->component);
													}
}

void trackerview_readconfig(TrackerView* self)
{
	psy_Properties* pv;

	pv = psy_properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {
		if (psy_properties_bool(pv, "griddefaults", 1)) {
			self->showdefaultline = 1;
			psy_ui_component_show(&self->griddefaults.component);
		} else {
			self->showdefaultline = 0;
			psy_ui_component_hide(&self->griddefaults.component);
		}
		trackerview_showlinenumbers(self, psy_properties_bool(pv, "linenumbers", 1));
		trackerview_showlinenumbercursor(self, psy_properties_bool(pv, "linenumberscursor", 1));
		trackerview_showlinenumbersinhex(self, psy_properties_bool(pv, "linenumbersinhex", 1));
		self->wraparound = psy_properties_bool(pv, "wraparound", 1);
		trackerview_showemptydata(self, psy_properties_bool(pv, "drawemptydata", 1));
		trackerview_setcentermode(self, psy_properties_bool(pv, "centercursoronscreen", 1));
		self->grid.notestabmode = self->griddefaults.notestabmode =
			(psy_properties_bool(pv, "notetab", 0))
			? psy_dsp_NOTESTAB_A440
			: psy_dsp_NOTESTAB_A220;
		{
			psy_ui_FontInfo fontinfo;
			psy_ui_Font font;

			psy_ui_fontinfo_init_string(&fontinfo,
				psy_properties_readstring(pv, "font", "tahoma;-16"));
			psy_ui_font_init(&font, &fontinfo);
			trackerview_setfont(self, &font, TRUE);
			psy_ui_font_dispose(&font);
		}
		trackerview_initcolumns(self);
		trackerview_computemetrics(self);
	}
	trackerview_readtheme(self);
}

void trackerview_readtheme(TrackerView* self)
{
	TrackerViewApplyProperties(self, self->workspace->patternviewtheme);
	if (self->grid.midline) {
		trackerview_centeroncursor(self);
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

void trackerview_showemptydata(TrackerView* self, int showstate)
{
	self->showemptydata = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerview_setcentermode(TrackerView* self, int mode)
{
	self->grid.midline = mode;	
	if (mode) {
		self->grid.component.overflow = psy_ui_OVERFLOW_SCROLL | psy_ui_OVERFLOW_VSCROLLCENTER;
		trackerview_centeroncursor(self);
	} else {
		self->grid.component.overflow = psy_ui_OVERFLOW_SCROLL;
		psy_ui_component_setscrolltop(&self->grid.component, 0);
		psy_ui_component_setscrolltop(&self->linenumbers.component, 0);
	}
}

void trackerview_initinputs(TrackerView* self)
{
	psy_audio_inputs_init(&self->inputs);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_UP, 0, 0), CMD_NAVUP);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_DOWN, 0, 0), CMD_NAVDOWN);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_LEFT, 0, 0), CMD_NAVLEFT);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_RIGHT, 0, 0), CMD_NAVRIGHT);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_PRIOR, 0, 0), CMD_NAVPAGEUP);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_NEXT, 0, 0), CMD_NAVPAGEDOWN);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_HOME, 0, 0), CMD_NAVTOP);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_END, 0, 0), CMD_NAVBOTTOM);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_TAB, 1, 0), CMD_COLUMNPREV);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_TAB, 0, 0), CMD_COLUMNNEXT);

	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_INSERT, 0, 0), CMD_ROWINSERT);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_BACK, 0, 0), CMD_ROWDELETE);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_DELETE, 0, 0), CMD_ROWCLEAR);

	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('B', 0, 1), CMD_BLOCKSTART);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('E', 0, 1), CMD_BLOCKEND);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('U', 0, 1), CMD_BLOCKUNMARK);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('X', 0, 1), CMD_BLOCKCUT);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('C', 0, 1), CMD_BLOCKCOPY);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('V', 0, 1), CMD_BLOCKPASTE);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('M', 0, 1), CMD_BLOCKMIX);

	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_F12, 0, 1), CMD_TRANSPOSEBLOCKINC);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_F11, 0, 1), CMD_TRANSPOSEBLOCKDEC);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_F12, 1, 1), CMD_TRANSPOSEBLOCKINC12);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput(psy_ui_KEY_F11, 1, 1), CMD_TRANSPOSEBLOCKDEC12);

	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('A', 0, 1), CMD_SELECTALL);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('R', 0, 1), CMD_SELECTCOL);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('K', 0, 1), CMD_SELECTBAR);

	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('Z', 0, 1), CMD_UNDO);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('Z', 1, 1), CMD_REDO);
	psy_audio_inputs_define(&self->inputs, psy_audio_encodeinput('F', 0, 1), CMD_FOLLOWSONG);
}

void trackerview_setpattern(TrackerView* self, psy_audio_Pattern* pattern)
{
	if (pattern) {
		self->opcount = pattern->opcount;
	}
	self->linestate.pattern = pattern;
	self->gridstate.pattern = pattern;
	interpolatecurveview_setpattern(&self->interpolatecurveview, pattern);
	psy_ui_component_setscrolltop(&self->linenumbers.component, 0);
	trackergrid_setpattern(&self->grid, pattern);
	if (self->grid.midline) {
		trackerview_centeroncursor(self);
	}
	psy_ui_component_invalidate(&self->linenumbers.component);
	psy_ui_component_invalidate(&self->header.component);

}

void trackergrid_setpattern(TrackerGrid* self, psy_audio_Pattern* pattern)
{
	self->gridstate->pattern = pattern;
	self->cursor.offset = 0;
	psy_ui_component_setscrolltop(&self->component, 0);	
}

void trackergrid_oninterpolatelinear(TrackerGrid* self)
{
	self->selection.topleft.line = (uintptr_t)(self->selection.topleft.offset / 0.25f);
	self->selection.bottomright.line = (uintptr_t)(self->selection.bottomright.offset / 0.25f);
	psy_audio_pattern_blockinterpolatelinear(self->gridstate->pattern,
		self->selection.topleft,
		self->selection.bottomright,
		(psy_dsp_big_beat_t)self->bpl);
}

void trackergrid_oninterpolatecurve(TrackerGrid* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(
		&self->view->interpolatecurveview.component)) {
		psy_ui_component_hide(
			&self->view->interpolatecurveview.component);
		psy_ui_component_align(&self->view->component);
	} else {
		psy_ui_component_show(
			&self->view->interpolatecurveview.component);
		psy_ui_component_align(&self->view->component);
	}
}

void trackergrid_onchangegenerator(TrackerGrid* self)
{
	if (self->gridstate->pattern && self->view->workspace->song) {
		psy_audio_pattern_changemachine(self->gridstate->pattern,
			self->selection.topleft,
			self->selection.bottomright,
			self->view->workspace->song->machines.slot);
		psy_ui_component_invalidate(&self->view->component);
	}
}

void trackergrid_onchangeinstrument(TrackerGrid* self)
{
	if (self->gridstate->pattern && self->view->workspace->song) {
		psy_audio_pattern_changeinstrument(self->gridstate->pattern,
			self->selection.topleft,
			self->selection.bottomright,
			self->view->workspace->song->instruments.slot.subslot);
		psy_ui_component_invalidate(&self->view->component);
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
		psy_audio_pattern_dispose(&self->view->workspace->patternpaste);
		psy_audio_pattern_init(&self->view->workspace->patternpaste);
		p = begin;
		while (p != NULL) {
			psy_audio_PatternEntry* entry;
			q = p->next;

			entry = psy_audio_patternnode_entry(p);
			if (entry->offset < self->selection.bottomright.offset) {
				if (entry->track >= self->selection.topleft.track &&
					entry->track < self->selection.bottomright.track) {
					prev = psy_audio_pattern_insert(&self->view->workspace->patternpaste,
						prev, entry->track - trackoffset,
						entry->offset - offset,
						patternentry_front(entry));
				}
			} else {
				break;
			}
			p = q;
		}
		psy_audio_pattern_setmaxsongtracks(&self->view->workspace->patternpaste,
			self->selection.bottomright.track -
			self->selection.topleft.track);
		psy_audio_pattern_setlength(&self->view->workspace->patternpaste,
			(psy_dsp_big_beat_t)(self->selection.bottomright.offset -
				self->selection.topleft.offset));

	}
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_onblockpaste(TrackerGrid* self)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	int trackoffset;
	psy_audio_PatternEditPosition begin;
	psy_audio_PatternEditPosition end;

	offset = (psy_dsp_big_beat_t)self->cursor.offset;
	trackoffset = self->cursor.track;
	p = self->view->workspace->patternpaste.events;

	begin = end = self->cursor;
	end.track += self->view->workspace->patternpaste.maxsongtracks;
	end.offset += self->view->workspace->patternpaste.length;
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
			*patternentry_front(entry) = *patternentry_front(pasteentry);
		} else {
			psy_audio_pattern_insert(self->gridstate->pattern,
				prev,
				pasteentry->track + trackoffset,
				pasteentry->offset + offset,
				patternentry_front(pasteentry));
		}
		p = p->next;
	}
	psy_ui_component_invalidate(&self->view->component);
	if (workspace_ismovecursorwhenpaste(self->view->workspace)) {
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
			end.offset = psy_audio_pattern_length(self->gridstate->pattern) - self->bpl;
		}
		workspace_setpatterneditposition(self->view->workspace, end);
	}
}

void trackergrid_onblockmixpaste(TrackerGrid* self)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	uintptr_t trackoffset;
	psy_audio_PatternEditPosition begin;
	psy_audio_PatternEditPosition end;

	offset = (psy_dsp_big_beat_t)self->cursor.offset;
	trackoffset = self->cursor.track;
	begin = end = self->cursor;
	end.track += self->view->workspace->patternpaste.maxsongtracks;
	end.offset += self->view->workspace->patternpaste.length;
	if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
		end.offset = psy_audio_pattern_length(self->gridstate->pattern);
	}
	p = self->view->workspace->patternpaste.events;
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
				patternentry_front(pasteentry));
		}
		psy_audio_patternnode_next(&p);
	}
	psy_ui_component_invalidate(&self->view->component);
	if (workspace_ismovecursorwhenpaste(self->view->workspace)) {
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(self->gridstate->pattern)) {
			end.offset = psy_audio_pattern_length(self->gridstate->pattern) - self->bpl;
		}
		workspace_setpatterneditposition(self->view->workspace, end);
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
	self->selection.topleft = self->cursor;
	self->selection.bottomright = self->cursor;
	++self->selection.bottomright.track;
	self->selection.bottomright.offset += self->bpl;
	self->hasselection = 1;
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_blockend(TrackerGrid* self)
{
	self->selection.bottomright = self->cursor;
	++self->selection.bottomright.track;
	self->selection.bottomright.offset += self->bpl;
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_blockunmark(TrackerGrid* self)
{
	self->hasselection = 0;
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_onblocktransposeup(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->cursor, +1, self->view->workspace)->command);
	}
}

void trackergrid_onblocktransposedown(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->cursor, -1, self->view->workspace)->command);
	}
}

void trackergrid_onblocktransposeup12(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->cursor, 12, self->view->workspace)->command);
	}
}

void trackergrid_onblocktransposedown12(TrackerGrid* self)
{
	if (self->hasselection) {
		psy_undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->gridstate->pattern,
				self->selection,
				self->cursor, -12, self->view->workspace)->command);
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

void trackerview_showblockmenu(TrackerView* self)
{
	psy_ui_component_show(&self->blockmenu.component);
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->linenumbers.component);
}

void trackerview_hideblockmenu(TrackerView* self)
{
	if (psy_ui_component_visible(&self->blockmenu.component)) {
		psy_ui_component_hide(&self->blockmenu.component);
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->linenumbers.component);
	}
}

void trackerview_toggleblockmenu(TrackerView* self)
{
	if (self->blockmenu.component.visible) {
		trackerview_hideblockmenu(self);
	} else {
		trackerview_showblockmenu(self);
	}
}

void trackerview_onzoomboxchanged(TrackerView* self, ZoomBox* sender)
{
	psy_Properties* pv;

	pv = psy_properties_findsection(self->workspace->config,
		"visual.patternview");
	if (pv) {
		psy_ui_Font* font;

		font = psy_ui_component_font(&self->component);
		if (font) {
			psy_ui_FontInfo fontinfo;
			psy_ui_Font newfont;

			fontinfo = psy_ui_font_fontinfo(font);
			fontinfo.lfHeight = (int)(self->zoomheightbase *
				zoombox_rate(sender));
			psy_ui_font_init(&newfont, &fontinfo);
			trackerview_setfont(self, &newfont, FALSE);
			psy_ui_font_dispose(&newfont);
			psy_ui_component_align(self->patternview);
			psy_ui_component_updateoverflow(&self->grid.component);			
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_invalidate(&self->header.component);
			psy_ui_component_invalidate(&self->linenumbers.component);
			psy_ui_component_invalidate(&self->linenumberslabel.component);
		}
	}
}

void patternblockmenu_init(PatternBlockMenu* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
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

	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		NULL));
}

void trackerview_onpatterneditpositionchanged(TrackerView* self, Workspace* sender)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;
	self->grid.cursor = workspace_patterneditposition(sender);
	if (!psy_audio_patterneditposition_equal(&self->grid.cursor, &oldcursor)) {
		trackerview_invalidatecursor(self, &oldcursor);
		if (psy_audio_player_playing(&sender->player) && workspace_followingsong(sender)) {
			int scrolldown;

			scrolldown = self->linestate.lastplayposition <
				psy_audio_player_position(&self->workspace->player);
			trackerview_invalidateline(self, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = psy_audio_player_position(&self->workspace->player);
			trackerview_invalidateline(self, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
				self->linestate.lastplayposition);
			if (self->linestate.lastplayposition >= self->linestate.sequenceentryoffset &&
				self->linestate.lastplayposition < self->linestate.sequenceentryoffset +
				self->gridstate.pattern->length) {
				if (scrolldown) {
					trackerview_scrolldown(self, self->grid.cursor);
				} else {
					trackerview_scrollup(self, self->grid.cursor);
				}
			}
		} else {
			if (self->grid.midline) {
				trackerview_centeroncursor(self);
			}
		}
	}
}

void trackerview_onskinchanged(TrackerView* self, Workspace* sender)
{
	TrackerViewApplyProperties(self, sender->patternviewtheme);
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
		patternevent_init_all(&event,
			(unsigned char)(
				workspace_recordtweaksastws(sender)
				? NOTECOMMANDS_TWEAKSLIDE
				: NOTECOMMANDS_TWEAK),
			NOTECOMMANDS_INST_EMPTY,
			(unsigned char)psy_audio_machines_slot(&self->workspace->song->machines),
			NOTECOMMANDS_VOL_EMPTY,
			(unsigned char)((value & 0xFF00) >> 8),
			(unsigned char)(value & 0xFF));
		event.inst = (unsigned char)tweak;
		trackerview_preventsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&InsertCommandAlloc(self->gridstate.pattern, self->grid.bpl,
				self->grid.cursor, event, self->workspace)->command);
		if (workspace_advancelineonrecordtweak(sender) &&
			!(workspace_followingsong(sender) &&
				psy_audio_player_playing(&sender->player))) {
			trackerview_advanceline(self);
		} else {
			trackerview_invalidatecursor(self, &self->grid.cursor);
		}
		trackerview_enablesync(self);
	}
}

int trackergrid_preferredtrackwidth(TrackerGrid* self)
{
	return trackdef_width(&self->gridstate->defaulttrackdef,
		self->gridstate->textwidth);
}

int trackerheader_preferredtrackwidth(TrackerHeader* self)
{
	return self->gridstate->skin->headercoords.background.destwidth;
}

void trackerview_initmetrics(TrackerView* self)
{	
	self->metrics.tm.tmHeight = 12;	
}

void trackerview_computemetrics(TrackerView* self)
{
	psy_ui_Size gridsize;
	psy_ui_TextMetric gridtm;
	int trackwidth;

	gridsize = psy_ui_component_size(&self->grid.component);
	gridtm = psy_ui_component_textmetric(&self->grid.component);
	self->grid.tm = gridtm;
	self->griddefaults.tm = gridtm;
	self->metrics.tm = psy_ui_component_textmetric(&self->component);
	self->gridstate.textwidth = (int)(self->metrics.tm.tmAveCharWidth * 1.5) + 2;	
	trackergrid_computecolumns(&self->grid, self->gridstate.textwidth);
	self->linestate.lineheight = self->metrics.tm.tmHeight + 1;
	trackwidth = max(
		trackergrid_preferredtrackwidth(&self->grid),
		trackerheader_preferredtrackwidth(&self->header));
	self->gridstate.patterntrackident =
		(trackwidth -
			trackergrid_preferredtrackwidth(&self->grid)) / 2;
	if (self->gridstate.patterntrackident < 0) {
		self->gridstate.patterntrackident = 0;
	}
	self->gridstate.headertrackident = 0;
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
	self->grid.component.scrollstepx = trackergridstate_trackwidth(&self->gridstate,
		trackergridstate_track_x(&self->gridstate,
			trackergridstate_screentotrack(&self->gridstate,
				psy_ui_component_scrollleft(&self->grid.component),
				self->gridstate.numtracks)));
	self->grid.component.scrollstepy = self->linestate.lineheight;
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
		self->textwidth) + 1;
}

uintptr_t trackergridstate_screentotrack(TrackerGridState* self, int x, uintptr_t numsongtracks)
{
	int curr = 0;
	uintptr_t rv = 0;

	while (rv < numsongtracks) {
		curr += trackergridstate_trackwidth(self, rv);
		if (curr > x) {
			break;
		}
		++rv;
	}
	return rv;
}

TrackDef* trackergridstate_trackdef(TrackerGridState* self, uintptr_t track)
{
	TrackDef* rv;

	rv = psy_table_at(&self->trackconfigs, track);
	if (!rv) {
		rv = &self->defaulttrackdef;
	}
	return rv;
}

void trackerview_oninterpolatecurveviewoncancel(TrackerView* self,
	InterpolateCurveView* sender)
{
	trackergrid_oninterpolatecurve(&self->grid, &self->grid.component);
}

void trackergrid_drawentry(TrackerGrid* self, psy_ui_Graphics* g,
	psy_audio_PatternEntry* entry, int x, int y,
	TrackerColumnFlags columnflags)
{
	static const char* emptynotestr = "- - -";
	const char* notestr;
	psy_ui_Rectangle r;

	unsigned int column;
	int focus;
	int cpx;
	TrackDef* trackdef;
	psy_audio_PatternEvent* event;
	TrackerColumnFlags currcolumnflags;
	currcolumnflags = columnflags;
	event = patternentry_front(entry);
	trackdef = trackergridstate_trackdef(self->gridstate, entry->track);
	focus = psy_ui_component_hasfocus(&self->component);
	currcolumnflags.cursor = self->linestate->drawcursor && focus && columnflags.cursor && self->cursor.column == 0;
	setcolumncolor(&self->view->skin, g, currcolumnflags);
	cpx = 0;
	// draw note	
	psy_ui_setrectangle(&r, x + cpx, y,
		self->gridstate->textwidth * 3,
		self->view->metrics.tm.tmHeight);	
	notestr = (event->note != NOTECOMMANDS_EMPTY || !self->view->showemptydata)
		? psy_dsp_notetostr(event->note, self->notestabmode)
		: emptynotestr;
	psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
		notestr, strlen(notestr));
	cpx += trackdef_columnwidth(trackdef, 0, self->gridstate->textwidth);
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
				columnflags.cursor && self->cursor.column == column &&
				self->cursor.digit == digit;
			setcolumncolor(&self->view->skin, g, currcolumnflags);
			trackergrid_drawdigit(self, g, x + cpx + digit *
				self->gridstate->textwidth, y, digitvalue,
				empty, currcolumnflags.mid);
		}
		cpx += trackdef_columnwidth(trackdef, column,
			self->gridstate->textwidth);
	}
}

uintptr_t trackdef_numdigits(TrackDef* self, uintptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->numdigits : 0;
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
			patternentry_front(entry)->note = value;
			break;
		case TRACKER_COLUMN_INST:
			patternentry_front(entry)->inst = value;
			break;
		case TRACKER_COLUMN_MACH:
			patternentry_front(entry)->mach = value;
			break;
		case TRACKER_COLUMN_VOL:
			patternentry_front(entry)->vol = value;
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

				patternevent_clear(&ev);
				patternentry_addevent(entry, &ev);
				p = entry->events->tail;
			}
			if (c == num) {
				break;
			}
			p = p->next;
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
			rv = patternentry_front_const(entry)->note;
			break;
		case TRACKER_COLUMN_INST:
			rv = patternentry_front_const(entry)->inst;
			break;
		case TRACKER_COLUMN_MACH:
			rv = patternentry_front_const(entry)->mach;
			break;
		case TRACKER_COLUMN_VOL:
			rv = patternentry_front_const(entry)->vol;
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
			psy_audio_PatternEvent* event;

			event = (psy_audio_PatternEvent*)p->entry;
			assert(event);
			if ((column % 2) == 0) {
				rv = event->cmd;
			} else {
				rv = event->parameter;
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

void trackerview_onalign(TrackerView* self)
{
	trackerview_computemetrics(self);
	if (self->grid.midline) {
		trackerview_centeroncursor(self);
	}
}

void trackerview_numtrackschanged(TrackerView* self, psy_audio_Player* player,
	unsigned int numsongtracks)
{
	self->gridstate.numtracks = numsongtracks;
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->header.component);
}
