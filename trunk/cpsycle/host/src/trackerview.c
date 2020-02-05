// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackerview.h"
#include <pattern.h>
#include <patternio.h>
#include "cmdsnotes.h"
#include "skinio.h"
#include "skingraphics.h"
#include "resources/resource.h"
#include <uiopendialog.h>
#include <uisavedialog.h>

#include <dir.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "../../detail/portable.h"

#define TIMERID_TRACKERVIEW 600
static const psy_dsp_big_beat_t epsilon = 0.0001;

static void trackergrid_setpattern(TrackerGrid*, psy_audio_Pattern* pattern);
static uintptr_t trackergrid_columnvalue(psy_audio_PatternEvent*, int column);
static void trackerview_initcolumns(TrackerView*);
static void trackergrid_computecolumns(TrackerGrid*, int textwidth);
static int trackergrid_preferredtrackwidth(TrackerGrid*);
static void trackergrid_ondraw(TrackerGrid*, psy_ui_Graphics*);
static void trackergrid_drawbackground(TrackerGrid*, psy_ui_Graphics*,
	TrackerGridBlock* clip);
static void trackergrid_drawtrackbackground(TrackerGrid*, psy_ui_Graphics*,
	int track);
static void trackergrid_drawentries(TrackerGrid*, psy_ui_Graphics*,
	TrackerGridBlock* clip);
static void trackergrid_drawentry(TrackerGrid*, psy_ui_Graphics*,
	psy_audio_PatternEntry*, int x, int y, TrackerColumnFlags);
static void trackergrid_onkeydown(TrackerGrid*,psy_ui_KeyEvent*);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousewheel(TrackerGrid*, psy_ui_Component* sender, psy_ui_MouseEvent*);
static PatternEditPosition trackergrid_makecursor(TrackerGrid* self, int x, int y);
static int trackergrid_resizecolumn(TrackerGrid*, int x, int y);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender,
	int stepx, int stepy);
static void trackergrid_onfocuslost(TrackerGrid*, psy_ui_Component* sender);
static int trackergrid_testcursor(TrackerGrid*, uintptr_t track,
	psy_dsp_big_beat_t offset);
static int trackergrid_testselection(TrackerGrid*, unsigned int track, double offset);
static int trackergrid_testplaybar(TrackerGrid* self, psy_dsp_big_beat_t offset);
static void trackergrid_clipblock(TrackerGrid*, const psy_ui_Rectangle*,
	TrackerGridBlock*);
static void trackergrid_drawdigit(TrackerGrid* self, psy_ui_Graphics*,
	int x, int y, int value, int empty);
static void trackergrid_adjustscroll(TrackerGrid*);
static double trackergrid_offset(TrackerGrid*, int y, unsigned int* lines);
static void trackergrid_numtrackschanged(TrackerGrid*, psy_audio_Player*,
	unsigned int numtracks);
void trackergrid_invalidatecursor(TrackerGrid*,
	const PatternEditPosition* cursor);

static void trackerview_initblockmenu(TrackerView*);
static void trackerview_connectblockmenu(TrackerView*);
static void trackerview_connectplayer(TrackerView*);
static void trackerview_connectworkspace(TrackerView*);
static void trackerview_setfont(TrackerView*, psy_ui_Font*);
static void trackerview_initmetrics(TrackerView*);
static void trackerview_updatescrollstep(TrackerView*);
static void trackerview_computemetrics(TrackerView*);
static void trackerview_ondestroy(TrackerView*, psy_ui_Component* sender);
static void trackerview_onkeydown(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_onkeyup(TrackerView*, psy_ui_KeyEvent*);
static void trackerview_onscroll(TrackerView*, psy_ui_Component* sender,
	int stepx, int stepy);
static void trackerview_onzoomboxchanged(TrackerView*, ZoomBox* sender);
static void trackerview_ontimer(TrackerView*, psy_ui_Component* sender, 
	int timerid);
static void trackerview_onseqlinetick(TrackerView*, psy_audio_Sequencer*);
static void trackerview_onalign(TrackerView*);
static void trackergrid_inputevent(TrackerGrid*, const psy_audio_PatternEvent*,
	int chordmode);
static void trackergrid_inputnote(TrackerGrid*, psy_dsp_note_t, int chordmode);
static void trackergrid_inputvalue(TrackerGrid*, int value, int digit);
static void enterdigitcolumn(TrackerView*, psy_audio_PatternEntry*, int track,
	int column, int digit, int value);
static void enterdigit(int digit, int newval, unsigned char* val);
static void digitlohi(int value, int digit, uintptr_t size, uint8_t* lo, uint8_t* hi);
static void lohi(uint8_t* value, int digit, uint8_t* lo, uint8_t* hi);

static void entervaluecolumn(psy_audio_PatternEntry*, int column, int value);
static int keycodetoint(unsigned int keycode);
static void trackerview_setcentermode(TrackerView*, int mode);
static void trackerview_handlecommand(TrackerView*, psy_ui_KeyEvent*, int cmd);
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
static int trackerview_scrollleft(TrackerView*);
static int trackerview_scrollright(TrackerView*);
static int trackerview_scrollup(TrackerView*);
static int trackerview_scrolldown(TrackerView*);
static void trackerview_showlinenumbers(TrackerView*, int showstate);
static void trackerview_showlinenumbercursor(TrackerView*, int showstate);
static void trackerview_showlinenumbersinhex(TrackerView*, int showstate);
static void trackerview_showemptydata(TrackerView*, int showstate);
static int trackerview_numlines(TrackerView*);
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
	const PatternEditPosition*);
static void trackerview_invalidateline(TrackerView*, psy_dsp_beat_t offset);
static void trackerview_initdefaultskin(TrackerView*);
static int trackerview_offsettoscreenline(TrackerView*, psy_dsp_big_beat_t);
static void trackergrid_oninterpolatelinear(TrackerGrid*);
static void trackergrid_oninterpolatecurve(TrackerGrid*);
static void trackergrid_onchangegenerator(TrackerGrid*);
static void trackergrid_onchangeinstrument(TrackerGrid*);
static void trackergrid_blockstart(TrackerGrid*);
static void trackergrid_blockend(TrackerGrid*);
static void trackergrid_blockunmark(TrackerGrid*);
static void trackerview_showblockmenu(TrackerView*);
static void trackerview_hideblockmenu(TrackerView*);
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
static void trackerview_onpatternimport(TrackerView*);
static void trackerview_onpatternexport(TrackerView*);
static void trackerview_onlpbchanged(TrackerView*, psy_audio_Player* sender,
	uintptr_t lpb);
static void trackerview_onpatterneditpositionchanged(TrackerView*,
	Workspace* sender);
static void trackerview_onparametertweak(TrackerView*,
	Workspace* sender, int slot, uintptr_t tweak, float value);
static void trackerview_onskinchanged(TrackerView*, Workspace*);
static int trackerview_trackwidth(TrackerView*, uintptr_t track);
static int trackerview_track_x(TrackerView*, uintptr_t track);
static uintptr_t trackerview_screentotrack(TrackerView*, int x);
static TrackDef* trackerview_trackdef(TrackerView* self, uintptr_t track);
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
// trackerheader
static int trackerheader_preferredtrackwidth(TrackerHeader*);
static void trackerheader_ondraw(TrackerHeader*, psy_ui_Component* sender,
	psy_ui_Graphics* g);
static void trackerheader_onmousedown(TrackerHeader*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);

static void OnLineNumbersLabelDraw(TrackerLineNumbersLabel*,
	psy_ui_Component* sender, psy_ui_Graphics*);
static void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel*,
	psy_ui_Component* sender);

static void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerView* view);
static void trackerlinenumbers_ondraw(TrackerLineNumbers*,
	psy_ui_Component* sender, psy_ui_Graphics*);
static void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const PatternEditPosition*);
static void trackerlinenumbers_invalidateline(TrackerLineNumbers*,
	psy_dsp_beat_t offset);
static int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width);
static int testrange_e(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width);

static psy_ui_ComponentVtable trackergrid_vtable;
static int trackergrid_vtable_initialized = 0;


static void trackergrid_vtable_init(TrackerGrid* self)
{
	if (!trackergrid_vtable_initialized) {
		trackergrid_vtable = *(self->component.vtable);		
		trackergrid_vtable.ondraw = (psy_ui_fp_ondraw) trackergrid_ondraw;
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
	}
}

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
	Command command;
	PatternEditPosition cursor;
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

InsertCommand* InsertCommandAlloc(psy_audio_Pattern* pattern, double bpl,
	PatternEditPosition cursor, psy_audio_PatternEvent event,
	Workspace* workspace)
{
	InsertCommand* rv;
	
	rv = malloc(sizeof(InsertCommand));
	rv->command.dispose = InsertCommandDispose;
	rv->command.execute = InsertCommandExecute;
	rv->command.revert = InsertCommandRevert;
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
	PatternNode* node;
	PatternNode* prev;

	node = pattern_findnode(self->pattern,
		self->cursor.track,
		(psy_dsp_beat_t)self->cursor.offset,		
		(psy_dsp_beat_t)self->bpl, &prev);	
	if (node) {
		self->oldevent = pattern_event(self->pattern, node);
		pattern_setevent(self->pattern, node, &self->event);
		self->insert = 0;
	} else {
		node = pattern_insert(self->pattern,
			prev,
			self->cursor.track, 
			(psy_dsp_beat_t)self->cursor.offset,
			&self->event);
		self->insert = 1;
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
}

void InsertCommandRevert(InsertCommand* self)
{
	PatternNode* node;
	PatternNode* prev;

	node = pattern_findnode(self->pattern,
			self->cursor.track,
			(psy_dsp_beat_t)self->cursor.offset,
			(psy_dsp_beat_t)self->bpl, &prev);
	if (node) {
		if (self->insert) {		
			pattern_remove(self->pattern, node);
			psy_audio_sequencer_checkiterators(
				&self->workspace->player.sequencer,
				node);
		} else {				
			pattern_setevent(self->pattern, node, &self->oldevent);		
		}
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
}

// BlockTranspose
typedef struct {
	Command command;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	PatternEditPosition cursor;	
	TrackerGridBlock block;	
	int transposeoffset;
	Workspace* workspace;
} BlockTransposeCommand;

static void BlockTransposeCommandDispose(BlockTransposeCommand*);
static void BlockTransposeCommandExecute(BlockTransposeCommand*);
static void BlockTransposeCommandRevert(BlockTransposeCommand*);

BlockTransposeCommand* BlockTransposeCommandAlloc(psy_audio_Pattern* pattern,
	TrackerGridBlock block, PatternEditPosition cursor, int transposeoffset,
	Workspace* workspace)
{
	BlockTransposeCommand* rv;
	
	rv = malloc(sizeof(BlockTransposeCommand));
	rv->command.dispose = BlockTransposeCommandDispose;
	rv->command.execute = BlockTransposeCommandExecute;
	rv->command.revert = BlockTransposeCommandRevert;	
	rv->pattern = pattern;
	pattern_init(&rv->oldpattern);
	rv->block = block;
	rv->cursor = cursor;
	rv->transposeoffset = transposeoffset;
	rv->workspace = workspace;	
	return rv;
}

void BlockTransposeCommandDispose(BlockTransposeCommand* self)
{
	pattern_dispose(&self->oldpattern);
}

void BlockTransposeCommandExecute(BlockTransposeCommand* self)
{			
	workspace_setpatterneditposition(self->workspace, self->cursor);
	pattern_copy(&self->oldpattern, self->pattern);	
	pattern_blocktranspose(self->pattern, 
		self->block.topleft,
		self->block.bottomright, self->transposeoffset);	
}

void BlockTransposeCommandRevert(BlockTransposeCommand* self)
{		
	assert(self->pattern);
	workspace_setpatterneditposition(self->workspace, self->cursor);
	pattern_copy(self->pattern, &self->oldpattern);	
}

/// TrackerGrid
void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent,
	TrackerView* view, psy_audio_Player* player)
{			
	psy_ui_component_init(&self->component, parent);
	trackergrid_vtable_init(self);
	self->component.vtable = &trackergrid_vtable;
	psy_ui_component_doublebuffer(&self->component);	
	self->view = view;
	self->editmode = TRACKERGRID_EDITMODE_SONG;
	self->hasselection = 0;	
	self->midline = 1;
	self->columnresize = 0;
	self->dragcolumn = -1;
	self->dragcolumnbase = 0;
	self->drawcursor = 1;
	self->view->workspace->chordmode = 0;
	self->chordbegin = 0;
	self->pattern = 0;	
	psy_signal_connect(&player->signal_numsongtrackschanged, self,
		trackergrid_numtrackschanged);	
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);
	psy_signal_connect(&self->component.signal_focuslost, self,
		trackergrid_onfocuslost);
	psy_signal_connect(&self->component.signal_mousewheel, self,
		trackergrid_onmousewheel);
	self->player = player;	
	self->numtracks = player_numsongtracks(player);
	self->lpb = player_lpb(self->player);
	self->bpl = 1 / (psy_dsp_big_beat_t) player_lpb(self->player);
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;	
	self->cursor.track = 0;
	self->cursor.offset = 0;
	self->cursor.column = 0;
	self->cursor.digit = 0;
	workspace_setpatterneditposition(self->view->workspace, self->cursor);
	self->cursorstep = 0.25;
	self->dx = 0;
	self->dy = 0;	
	trackergrid_computecolumns(self, 9);		
	self->component.wheelscroll = 4;	
}

void trackerview_initcolumns(TrackerView* self)
{
	trackdef_init(&self->defaulttrackdef);	
	if (workspace_showwideinstcolumn(self->workspace)) {
		self->defaulttrackdef.inst.numdigits = 4;
		self->defaulttrackdef.inst.numchars = 4;			
	}
}

void trackergrid_computecolumns(TrackerGrid* self, int textwidth)
{
	
}

void trackergrid_numtrackschanged(TrackerGrid* self, psy_audio_Player* player,
	unsigned int numsongtracks)
{	
	self->numtracks = numsongtracks;	
	self->view->header.numtracks = numsongtracks;	
	trackergrid_adjustscroll(self);
	psy_ui_component_invalidate(&self->view->component);
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
	self->skin.font2 = psy_properties_int(p, "pvc_font2", 0x00CACACA );
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
	if (s =psy_properties_readstring(p, "record_on_source", 0)) {
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
  	TrackerGridBlock clip;
	if (self->pattern) {
		trackergrid_clipblock(self, &g->clip, &clip);
		trackergrid_drawbackground(self, g, &clip);
		trackergrid_drawentries(self, g, &clip);
	} else {
		psy_ui_drawsolidrectangle(g, g->clip, self->view->skin.background);	
	}
}

psy_dsp_big_beat_t trackergrid_offset(TrackerGrid* self, int y, unsigned int* lines)
{
	double offset = 0;	
	int cpy = 0;		
	int first = 1;
	unsigned int count;
	unsigned int remaininglines = 0;
	
	count = (y >= 0) ? y / self->view->metrics.lineheight : 0;
	if (self->pattern) {
		PatternNode* curr = self->pattern->events;
		*lines = 0;		
		while (curr) {		
			psy_audio_PatternEntry* entry;		
			first = 1;
			do {
				entry = (psy_audio_PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->bpl))
				{
					if ((*lines) >= count) {
						break;
					}
					if (entry->track == 0 && !first) {				
					}							
					first = 0;
					curr = curr->next;
				} else {					
					break;
				}
				if ((int)(*lines) >= count) {
					break;
				}
			} while (curr);
			if ((*lines) >= count) {
				break;
			}
			++(*lines);
			offset += self->bpl;
		}	
		remaininglines =  (count - (*lines));
		*lines += remaininglines;
	}
	return offset + remaininglines * self->bpl;
}

void trackergrid_clipblock(TrackerGrid* self, const psy_ui_Rectangle* clip,
	TrackerGridBlock* block)
{	
	int lines;
	
	block->topleft.track = trackerview_screentotrack(self->view, clip->left
		- self->dx);
	block->topleft.column = 0;
	block->topleft.digit = 0;
	block->topleft.offset =  trackergrid_offset(self, clip->top - self->dy,
		&lines);
	block->topleft.line = lines;	
	block->bottomright.track = trackerview_screentotrack(self->view,		
		clip->right - self->dx) + 1;		
	if (block->bottomright.track > self->numtracks) {
		block->bottomright.track = self->numtracks;
	}
	block->bottomright.column = 0;
	block->bottomright.digit = 0;
	block->bottomright.offset = trackergrid_offset(self,
		clip->bottom - self->dy, &lines);
	block->bottomright.line = lines;	
}

void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g, TrackerGridBlock* clip)
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
	trackswidth = trackerview_track_x(self->view, self->numtracks - 1) +
		trackerview_trackwidth(self->view, self->numtracks - 1);
	psy_ui_setrectangle(&r,
		trackswidth + self->dx, 0, 
		size.width - trackswidth + self->dx,
		size.height);
	psy_ui_drawsolidrectangle(g, r, self->view->skin.background);
}

void trackergrid_drawtrackbackground(TrackerGrid* self, psy_ui_Graphics* g, int track)
{
	psy_ui_Rectangle r;
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	psy_ui_setrectangle(&r, trackerview_track_x(self->view, track) + self->dx,
		0, trackerview_trackwidth(self->view, track), size.height);
	psy_ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

int trackergrid_testselection(TrackerGrid* self, unsigned int track, double offset)
{
	return self->hasselection &&
		track >= self->selection.topleft.track &&
		track < self->selection.bottomright.track &&
		offset >= self->selection.topleft.offset &&
		offset < self->selection.bottomright.offset;
}

int trackergrid_testcursor(TrackerGrid* self, uintptr_t track, psy_dsp_big_beat_t offset)
{
	return self->cursor.track == track && 
		testrange(self->cursor.offset, offset, self->bpl);
}

int trackergrid_testplaybar(TrackerGrid* self, psy_dsp_big_beat_t offset)
{
	return player_playing(self->player) && 
		testrange(self->view->lastplayposition -
				      self->view->sequenceentryoffset,
				  offset, self->bpl); 	
}

void trackergrid_drawentries(TrackerGrid* self, psy_ui_Graphics* g, TrackerGridBlock* clip)
{	
	unsigned int track;
	int cpx = 0;	
	int cpy;
	double offset;	
	TrackerColumnFlags columnflags;
	PatternNode* node;
	psy_ui_Size size;
	int halfy;	
	psy_audio_PatternEntry empty;

	patternentry_init(&empty);
	size = psy_ui_component_size(&self->component);
	halfy = (self->view->metrics.visilines / 2) * self->view->metrics.lineheight;	
	cpy = clip->topleft.line * self->view->metrics.lineheight + self->dy;
	offset = clip->topleft.offset;	
	node = pattern_greaterequal(self->pattern, (psy_dsp_beat_t)offset);
	while (offset <= clip->bottomright.offset && offset < self->pattern->length) {
		int fill;

		if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
			columnflags.beat = fabs(fmod(offset, 1.0f)) < 0.01f ;
			columnflags.beat4 = fabs(fmod(offset, 4.0f)) < 0.01f;
			columnflags.mid = self->midline && cpy >= halfy && cpy < halfy + self->view->metrics.lineheight;
		} else {
			columnflags.beat = 0;
			columnflags.beat4 = 0;
			columnflags.mid = 0;
		}		
		fill = 0;
		cpx = trackerview_track_x(self->view, clip->topleft.track) + self->dx;
		for (track =  clip->topleft.track; track < clip->bottomright.track;
				++track) {
			int hasevent = 0;			

			columnflags.cursor = trackergrid_testcursor(self, track, offset);
			columnflags.selection = trackergrid_testselection(self, track, offset);
			columnflags.playbar = trackergrid_testplaybar(self, offset);
			while (!fill && node &&
				   ((psy_audio_PatternEntry*)(node->entry))->track <= track &&
				   testrange_e(((psy_audio_PatternEntry*)(node->entry))->offset,
					  offset,
					  self->bpl)) {
				psy_audio_PatternEntry* entry;
									
				entry = (psy_audio_PatternEntry*)(node->entry);					
				if (entry->track == track) {
					trackergrid_drawentry(self, g, entry, 
						cpx + self->view->metrics.patterntrackident,
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
					cpx + self->view->metrics.patterntrackident,
					cpy,
					columnflags);				
			} else
			if (node && ((psy_audio_PatternEntry*)(node->entry))->track <= track) {
				fill = 1;
			}
			cpx += trackerview_trackwidth(self->view, track);
		}
		// skip remaining events of the line
		while (node && ((psy_audio_PatternEntry*)(node->entry))->offset +
				2*epsilon < offset + self->bpl) {
			node = node->next;
		}								
		offset += self->bpl;
		cpy += self->view->metrics.lineheight;		
	}
	patternentry_dispose(&empty);
}

int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position >= offset && position < offset + width; 
}

int testrange_e(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position + 2*epsilon >= offset &&
		position < offset + width - epsilon;
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
	int x, int y, int value, int empty)
{
	char buffer[20];	
	psy_ui_Rectangle r;
	psy_ui_setrectangle(&r, x, y, self->view->metrics.textwidth,
			self->view->metrics.tm.tmHeight);
	if (!empty) {
		psy_snprintf(buffer, 2, "%X", value);
	} else {
		if (self->view->showemptydata) {
			psy_snprintf(buffer, 2, "%s", ".");
		} else {
			psy_snprintf(buffer, 2, "%s", "");	
		}
	}
	psy_ui_textoutrectangle(g, r.left + self->view->metrics.textleftedge, r.top,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, buffer, strlen(buffer));
}

void trackergrid_adjustscroll(TrackerGrid* self)
{
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
		psy_ui_Size size;	
		int vscrollmax;
		uintptr_t visitracks;

		size = psy_ui_component_size(&self->component);
		visitracks = trackerview_screentotrack(self->view,
			psy_ui_component_size(&self->component).width - self->dx) -
			trackerview_screentotrack(self->view, -self->dx);		
		psy_ui_component_sethorizontalscrollrange(&self->component, 0,
			self->numtracks - visitracks);
		vscrollmax = trackerview_numlines(self->view);
		if (!self->midline) {
			vscrollmax -= self->view->metrics.visilines;
		} else {
			vscrollmax -= 1;
		}
		psy_ui_component_setverticalscrollrange(&self->component, 0, vscrollmax);
		if (self->midline) {		
			trackerview_centeroncursor(self->view);
		}		
		psy_ui_component_sethorizontalscrollposition(&self->component,
			trackerview_screentotrack(self->view, -self->dx));
	}
}

void trackerview_centeroncursor(TrackerView* self)
{
	int line;		
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset);
	self->grid.dy = (self->metrics.visilines / 2 - line) * self->metrics.lineheight;
	self->linenumbers.dy = self->grid.dy;
	psy_ui_component_setverticalscrollposition(&self->grid.component,
		line);
}

unsigned int NumSublines(psy_audio_Pattern* pattern, double offset, double bpl)
{
	PatternNode* node = pattern_greaterequal(pattern, (psy_dsp_beat_t)offset);	
	unsigned int currsubline = 0;
	int first = 1;

	while (node) {
		psy_audio_PatternEntry* entry;
		
		entry = (psy_audio_PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			break;
		}				
		if (entry->track == 0 && !first) {
			++currsubline;			
		}
		node = node->next;
		first = 0;
	}
	return currsubline;
}

void trackergrid_onkeydown(TrackerGrid* self, psy_ui_KeyEvent* ev)
{	
	if (self->editmode == TRACKERGRID_EDITMODE_LOCAL) {
		if (ev->keycode == psy_ui_KEY_ESCAPE) {
			psy_ui_component_setfocus(&self->view->grid.component);
			psy_ui_keyevent_stoppropagation(ev);
		} else {
			int cmd;

			cmd = inputs_cmd(&self->view->inputs, encodeinput(ev->keycode, ev->shift,
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
					input.param1 = encodeinput(ev->keycode, 0, ev->ctrl);
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
	PatternEditPosition oldcursor;
	oldcursor = self->cursor;
	
	if (self->cursor.column == 0 && self->cursor.digit == 0) {		
		if (self->cursor.track > 0) {
			TrackDef* trackdef;

			--self->cursor.track;
			trackdef = trackerview_trackdef(self->view,
				self->cursor.track);
			self->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->cursor.digit = trackdef_numdigits(trackdef,
				self->cursor.column) - 1;
			trackerview_scrollleft(self->view);
		} else 
		if (self->view->wraparound) {
			TrackDef* trackdef;

			self->cursor.track = player_numsongtracks(
				&self->view->workspace->player) - 1;
			trackdef = trackerview_trackdef(self->view, self->cursor.track);
			self->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->cursor.digit = trackdef_numdigits(trackdef,
				self->cursor.column) - 1;
			invalidate = trackerview_scrollright(self->view);
		}
	} else {
		if (self->cursor.digit > 0) {
			--self->cursor.digit;
		} else {
			TrackDef* trackdef;

			trackdef = trackerview_trackdef(self->view,
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
	PatternEditPosition oldcursor;
	oldcursor = self->cursor;
	
	trackdef = trackerview_trackdef(self->view, self->cursor.track);
	if (self->cursor.column == trackdef_numcolumns(trackdef) - 1 &&
		self->cursor.digit == trackdef_numdigits(trackdef,
			self->cursor.column) - 1) {		
		if (self->cursor.track < player_numsongtracks(
			&self->view->workspace->player) - 1) {
			self->cursor.column = 0;
			self->cursor.digit = 0;
			++self->cursor.track;		
			invalidate = trackerview_scrollright(self->view);
		} else
		if (self->view->wraparound) {
			self->cursor.column = 0;
			self->cursor.digit = 0;
			self->cursor.track = 0;
			invalidate = trackerview_scrollleft(self->view);
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
	PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;	
	trackerview_prevlines(self, workspace_cursorstep(self->workspace),
		self->wraparound);		
}

void trackerview_prevlines(TrackerView* self, uintptr_t lines, int wrap)
{
	PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;
	self->grid.cursor.offset -= lines * self->grid.bpl;		
	if (self->grid.cursor.offset < 0) {
		if (wrap) {
			self->grid.cursor.offset += self->grid.pattern->length;
			if (self->grid.cursor.offset < 0) {
				self->grid.cursor.offset = 0;
			}
			trackerview_scrolldown(self);
		} else {
			self->grid.cursor.offset = 0;
			trackerview_scrollup(self);
		}			
	} else {
		trackerview_scrollup(self);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);		
	trackergrid_invalidatecursor(&self->grid, &oldcursor);
	trackergrid_invalidatecursor(&self->grid, &self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);
}

void trackerview_advanceline(TrackerView* self)
{	
	trackerview_advancelines(self, workspace_cursorstep(self->workspace),
		self->wraparound);	
}

void trackerview_advancelines(TrackerView* self, uintptr_t lines, int wrap)
{
	PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;		
	self->grid.cursor.offset += lines * self->grid.bpl;
	if (self->grid.cursor.offset >= self->grid.pattern->length) {
		if (wrap) {
			self->grid.cursor.offset = self->grid.cursor.offset -
				self->grid.pattern->length;
			if (self->grid.cursor.offset > self->grid.pattern->length - self->grid.bpl) {
				self->grid.cursor.offset = self->grid.pattern->length - self->grid.bpl;
			}
			trackerview_scrollup(self);
		} else {
			self->grid.cursor.offset = self->grid.pattern->length -
				self->grid.bpl;
			trackerview_scrolldown(self);
		}
	} else {
		trackerview_scrolldown(self);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	trackergrid_invalidatecursor(&self->grid, &oldcursor);
	trackergrid_invalidatecursor(&self->grid, &self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);	
}

void trackerview_home(TrackerView* self)
{
	PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;		
	self->grid.cursor.offset = 0;
	trackerview_scrollup(self);	
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	trackergrid_invalidatecursor(&self->grid, &oldcursor);
	trackergrid_invalidatecursor(&self->grid, &self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);	
}

void trackerview_end(TrackerView* self)
{
	PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;		
	self->grid.cursor.offset = self->grid.pattern->length - self->grid.bpl;
	trackerview_scrolldown(self);	
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	trackergrid_invalidatecursor(&self->grid, &oldcursor);
	trackergrid_invalidatecursor(&self->grid, &self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);	
}

void trackerview_selectall(TrackerView* self)
{
	if (self->workspace->song && self->grid.pattern) {
		self->grid.selection.topleft.offset = 0;
		self->grid.selection.topleft.track = 0;
		self->grid.selection.bottomright.offset = self->grid.pattern->length;
		self->grid.selection.bottomright.track = 
			self->workspace->song->patterns.songtracks;
		self->grid.hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}			
}

void trackerview_selectcol(TrackerView* self)
{
	if (self->workspace->song && self->grid.pattern) {
		self->grid.selection.topleft.offset = 0;
		self->grid.selection.topleft.track = self->grid.cursor.track;
		self->grid.selection.bottomright.offset = self->grid.pattern->length;
		self->grid.selection.bottomright.track = self->grid.cursor.track + 1;
		self->grid.hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}		
}

void trackerview_selectbar(TrackerView* self)
{
	if (self->workspace->song && self->grid.pattern) {
		self->grid.selection.topleft.offset = self->grid.cursor.offset;
		self->grid.selection.topleft.track = self->grid.cursor.track;
		self->grid.selection.bottomright.offset = self->grid.cursor.offset + 4.0;
		if (self->grid.cursor.offset > self->grid.pattern->length) {
			self->grid.cursor.offset = self->grid.pattern->length;
		}
		self->grid.selection.bottomright.track = self->grid.cursor.track + 1;
		self->grid.hasselection = 1;
		psy_ui_component_invalidate(&self->component);
	}		
}

int trackerview_scrollup(TrackerView* self)
{
	int line;
	int topline;
	int rv = 1;
	psy_ui_Rectangle r;		
	
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset);		
	psy_ui_setrectangle(&r,
		trackerview_track_x(self, self->grid.cursor.track),
		self->metrics.lineheight * line,		
		trackerview_trackwidth(self, self->grid.cursor.track),
		self->metrics.lineheight);
	if (self->grid.midline) {
		topline = psy_ui_component_size(&self->grid.component).height  
			/ self->metrics.lineheight / 2;		
	} else {
		topline = 0;
	}
	if (-self->grid.dy + topline * self->metrics.lineheight > r.top) {
		int dlines = (-self->grid.dy + topline * self->metrics.lineheight - r.top) /
			(self->metrics.lineheight);
		psy_signal_emit(&self->grid.component.signal_scroll,
			&self->grid.component, 2, 0, dlines);
		psy_ui_component_scrollstep(&self->grid.component, 0, dlines);
		psy_ui_component_setverticalscrollposition( &self->grid.component, 
			psy_ui_component_verticalscrollposition(&self->grid.component) - dlines);
		rv = 0;
	}	
	return rv;
}

int trackerview_scrolldown(TrackerView* self)
{
	int line;
	int visilines;
	int rv = 1;	
	
	visilines = self->metrics.visilines;
	if (self->grid.midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset);		
	if (visilines < line + self->grid.dy / self->metrics.lineheight) {
		int dlines;

		dlines = line + self->grid.dy / self->metrics.lineheight - visilines;
		psy_signal_emit(&self->grid.component.signal_scroll,
			&self->grid.component, 2, 0, -dlines);
		psy_ui_component_scrollstep(&self->grid.component, 0, -dlines);
		psy_ui_component_setverticalscrollposition(
			&self->grid.component, 
			psy_ui_component_verticalscrollposition(&self->grid.component) + dlines);
		rv = 0;		
	}
	return rv;
}

void trackerview_prevtrack(TrackerView* self)
{
	PatternEditPosition oldcursor;
	int invalidate = 1;

	oldcursor = self->grid.cursor;
	self->grid.cursor.column = 0;
	self->grid.cursor.digit = 0;
	if (self->grid.cursor.track > 0) {
		--self->grid.cursor.track;
		trackerview_scrollleft(self);
	} else 
	if (self->wraparound) {
		self->grid.cursor.track = 
			player_numsongtracks(&self->workspace->player) - 1;
		invalidate = trackerview_scrollright(self);
	}
	if (invalidate) {
		trackergrid_invalidatecursor(&self->grid, &oldcursor);
		trackergrid_invalidatecursor(&self->grid, &self->grid.cursor);
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);	
}

void trackerview_nexttrack(TrackerView* self)
{
	PatternEditPosition oldcursor;
	int invalidate = 1;

	oldcursor = self->grid.cursor;
	self->grid.cursor.column = 0;
	self->grid.cursor.digit = 0;
	if (self->grid.cursor.track < 
		player_numsongtracks(&self->workspace->player) - 1) {
		++self->grid.cursor.track;
		invalidate = trackerview_scrollright(self);
	} else
	if (self->wraparound) {
		self->grid.cursor.track = 0;
		invalidate = trackerview_scrollleft(self);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);		
	if (invalidate) {
		trackergrid_invalidatecursor(&self->grid, &oldcursor);
		trackergrid_invalidatecursor(&self->grid, &self->grid.cursor);
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

int trackerview_scrollleft(TrackerView* self)
{	
	uintptr_t tracks;
	int invalidate = 1;
	
	tracks = self->grid.cursor.track;
	if (trackerview_screentotrack(self, -self->grid.dx) > tracks) {
		self->grid.dx = -trackerview_track_x(self, tracks);		
		self->header.dx = self->grid.dx;		
		self->griddefaults.dx = self->grid.dx;
		psy_ui_component_invalidate(&self->griddefaults.component);
		psy_ui_component_update(&self->griddefaults.component);
		psy_ui_component_invalidate(&self->grid.component);
		psy_ui_component_update(&self->grid.component);				
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_update(&self->header.component);
		psy_ui_component_sethorizontalscrollposition(&self->grid.component, 0);
		trackerview_updatescrollstep(self);
		invalidate = 0;
	}
	return invalidate;
}

int trackerview_scrollright(TrackerView* self)
{
	int invalidate = 1;
	uintptr_t visitracks;
	uintptr_t tracks;
		
	visitracks = trackerview_screentotrack(self,
			psy_ui_component_size(&self->component).width - self->grid.dx) -
		trackerview_screentotrack(self, -self->grid.dx);
	tracks = self->grid.cursor.track + 1;
	if (tracks > visitracks + trackerview_screentotrack(self, -self->grid.dx)) {
		self->grid.dx = -trackerview_track_x(self, tracks - visitracks);
		self->header.dx = self->grid.dx;		
		self->griddefaults.dx = self->grid.dx;
		psy_ui_component_invalidate(&self->griddefaults.component);
		psy_ui_component_update(&self->griddefaults.component);
		psy_ui_component_invalidate(&self->header.component);		
		psy_ui_component_update(&self->header.component);
		psy_ui_component_invalidate(&self->grid.component);		
		psy_ui_component_update(&self->grid.component);
		psy_ui_component_sethorizontalscrollposition(&self->grid.component, 
			tracks - visitracks);
		invalidate = 0;
		trackerview_updatescrollstep(self);
	}			
	return invalidate;
}

void trackerview_onkeydown(TrackerView* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		trackerview_hideblockmenu(self);
	} else {
		int cmd;
		cmd = inputs_cmd(&self->inputs, encodeinput(ev->keycode, ev->shift,
			ev->ctrl));
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
				player_lpb(&self->workspace->player), 0);
		break;
		case CMD_NAVDOWN:
			trackerview_advanceline(self);
		break;
		case CMD_NAVPAGEDOWN:
			trackerview_advancelines(self,
				player_lpb(&self->workspace->player), 0);
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
		input.param1 = encodeinput(ev->keycode, 0, ev->ctrl);
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
	}
	else {
		workspace_followsong(self->workspace);
	}
}

void trackergrid_rowdelete(TrackerGrid* self)
{
	if (self->cursor.offset - self->bpl >= 0) {
		PatternNode* prev;
		PatternNode* p;
		PatternNode* q;
		PatternNode* node;

		trackerview_prevline(self->view);
		node = pattern_findnode(self->pattern, self->cursor.track,
			(psy_dsp_beat_t)self->cursor.offset, (psy_dsp_beat_t)self->bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
			psy_audio_sequencer_checkiterators(
				&self->view->workspace->player.sequencer,
				node);
			psy_ui_component_invalidate(&self->view->linenumbers.component);
		}
		if (prev) {
			p = prev->next;
		} else {
			p = self->pattern->events;
		}
		for (; p != 0; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = (psy_audio_PatternEntry*)p->entry;
			if (entry->track == self->cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_beat_t offset;
				uintptr_t track;
				PatternNode* node;
				PatternNode* prev;

				event = *patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				pattern_remove(self->pattern, p);
				psy_audio_sequencer_checkiterators(
					&self->view->workspace->player.sequencer, p);
				offset -= (psy_dsp_beat_t)self->bpl;
				node = pattern_findnode(self->pattern, track,
					offset,
					(psy_dsp_beat_t)self->bpl,
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*patternentry_front(entry) = event;
				}
				else {
					pattern_insert(self->pattern, prev, track,
						(psy_dsp_beat_t) offset, &event);
				}
			}
		}
		psy_ui_component_invalidate(&self->view->component);
	}
}

void trackergrid_rowclear(TrackerGrid* self)
{
	if (self->cursor.column == TRACKER_COLUMN_NOTE) {
		PatternNode* prev;
		PatternNode* node;

		node = pattern_findnode(self->pattern, self->cursor.track,
			(psy_dsp_beat_t)self->cursor.offset,
			(psy_dsp_beat_t)self->bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
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

		trackdef = trackerview_trackdef(self->view, self->cursor.track);
		columndef = trackdef_columndef(trackdef, self->cursor.column);
		trackergrid_inputvalue(self, columndef->emptyvalue, 0);		
	}
}

void trackerview_onkeyup(TrackerView* self, psy_ui_KeyEvent* ev)
{
	if (self->workspace->chordmode && ev->keycode == psy_ui_KEY_SHIFT) {
		self->grid.cursor.track = self->grid.chordbegin;
		trackerview_scrollleft(self);
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
	PatternNode* node;
	PatternNode* prev;
				
	node = pattern_findnode(patterndefaults, self->grid.cursor.track, 0,
		(psy_dsp_beat_t)self->grid.bpl, &prev);
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
	self->opcount = self->grid.pattern->opcount;
	self->syncpattern = 1;
}

void trackerview_preventsync(TrackerView* self)
{
	self->opcount = self->grid.pattern->opcount;
	self->syncpattern = 0;
}

void trackergrid_inputevent(TrackerGrid* self,
	const psy_audio_PatternEvent* event, int chordmode)
{	
	trackerview_preventsync(self->view);
	undoredo_execute(&self->view->workspace->undoredo,
		&InsertCommandAlloc(self->pattern, self->bpl,
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
		(unsigned char) machines_slot(&self->view->workspace->song->machines),
		NOTECOMMANDS_VOL_EMPTY,
		0,
		0);
	machine = machines_at(&self->view->workspace->song->machines, event.mach);
	if (machine && 
			machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
		event.inst = self->view->workspace->song->instruments.slot;
	}
	trackergrid_inputevent(self, &event, chordmode);
}

void trackergrid_inputvalue(TrackerGrid* self, int value, int digit)
{
	if (self->pattern && value != -1) {
		PatternNode* prev;	
		psy_audio_PatternEntry* entry;
		PatternNode* node;
		psy_audio_PatternEntry newentry;
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackerview_trackdef(self->view, self->cursor.track);
		columndef = trackdef_columndef(trackdef, self->cursor.column);		
		patternentry_init(&newentry);
		node = pattern_findnode(self->pattern,
					self->cursor.track,
					(psy_dsp_beat_t)self->cursor.offset,
					(psy_dsp_beat_t)self->bpl,
					&prev);						
		if (node) {
			entry = (psy_audio_PatternEntry*) node->entry;
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
		undoredo_execute(&self->view->workspace->undoredo,
				&InsertCommandAlloc(self->pattern, self->bpl,
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

	if (keycode >= '0' && keycode <='9') {
		rv = keycode - '0';
	} else
	if (keycode >= 'A' && keycode <='Z') {
		rv = keycode - 'A' + 10;
	}
	return rv;
}

void enterdigitcolumn(TrackerView* self, psy_audio_PatternEntry* entry,
	int track, int column, int digit, int digitvalue)
{			
	TrackDef* trackdef;	

	trackdef = trackerview_trackdef(self, track);
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
		data = (uint8_t*) &value + pos;
		enterdigit(digit % 2, digitvalue, data);
		trackdef_setvalue(trackdef, column, entry, *((uintptr_t*) &value));
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
	lohi((uint8_t*) &value + pos, digit, lo, hi);
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
	const PatternEditPosition* cursor)
{
	int line;		
	psy_ui_Rectangle r;		

	line = trackerview_offsettoscreenline(self->view, cursor->offset);
	psy_ui_setrectangle(&r,
		trackerview_track_x(self->view, cursor->track) + self->dx,
		self->view->metrics.lineheight * line + self->dy,
		trackerview_trackwidth(self->view, cursor->track),
		self->view->metrics.lineheight);
	psy_ui_component_invalidaterect(&self->component, &r);
}

void trackerview_invalidatecursor(TrackerView* self, const PatternEditPosition* cursor)
{	
	trackerlinenumbers_invalidatecursor(&self->linenumbers, cursor);	
}

void trackerview_invalidateline(TrackerView* self, psy_dsp_beat_t offset)
{
	int line;	
	psy_ui_Rectangle r;	
	
	if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset + self->grid.pattern->length) {
		line = (int) ((offset - self->sequenceentryoffset) 
			/ self->grid.bpl);
		psy_ui_setrectangle(&r,
			self->grid.dx,
			self->metrics.lineheight * line + self->grid.dy,
			psy_ui_component_size(&self->component).width - self->grid.dx,
			self->metrics.lineheight);
		psy_ui_component_invalidaterect(&self->grid.component, &r);
	}
}

void trackerview_onscroll(TrackerView* self, psy_ui_Component* sender, int stepx,
	int stepy)
{
	if (stepx != 0) {
		self->header.dx = self->grid.dx;		
		self->griddefaults.dx = self->grid.dx;
		psy_ui_component_invalidate(&self->griddefaults.component);		
		psy_ui_component_invalidate(&self->header.component);
	}
	if (stepy != 0) {
		self->linenumbers.dy = self->grid.dy;
		psy_ui_component_invalidate(&self->linenumbers.component);		
		psy_ui_component_update(&self->linenumbers.component);
	} 
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender, int stepx,
	int stepy)
{		
	self->dx += (stepx * sender->scrollstepx);
	self->dy += (stepy * sender->scrollstepy);

	if (self->dx > 0) {
		self->dx = 0;
	}	
	if (self->midline) {
		psy_ui_Size size;
		int halfvisilines;	
		int restoremidline;		
		psy_ui_Rectangle r;		
			
		size = psy_ui_component_size(&self->component);		
		halfvisilines = self->view->metrics.visilines / 2;
		psy_ui_setrectangle(&r,
			0, halfvisilines * self->view->metrics.lineheight,
			size.width, self->view->metrics.lineheight * 2);
		restoremidline = self->midline;
		self->midline = 0;
		psy_ui_component_invalidaterect(&self->component, &r);
		self->midline = restoremidline;
	}
}

void trackergrid_onmousedown(TrackerGrid* self, psy_ui_MouseEvent* ev)
{	
	if (ev->button == 2) {
		trackerview_toggleblockmenu(self->view);
	} else
	if (self->pattern) {
		if (ev->button == 1) {			
			PatternEditPosition oldcursor;

			self->dragcolumn = trackergrid_resizecolumn(self, ev->x, ev->y);
			if (self->dragcolumn == -1) {
				oldcursor = self->cursor;
				self->cursor = trackergrid_makecursor(self, ev->x, ev->y);			
				self->selection.topleft = self->cursor;
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
	PatternEditPosition cursor;

	if (ev->button == 1) {
		cursor = trackergrid_makecursor(self, ev->x, ev->y);
		/*if (cursor.col != self->cursor.col ||
			cursor.offset != self->cursor.offset) {
			self->hasselection = 1;
			self->selection.bottomright = cursor;
			self->selection.bottomright.offset += self->bpl;
			self->selection.bottomright.track += 1;

			psy_ui_component_invalidate(&self->component);		
		}*/
	}
	if (self->columnresize) {
		if (self->dragcolumn != -1) {
			uintptr_t track;
			TrackDef* trackdef;			

			track = trackerview_screentotrack(self->view, ev->x - self->dx);
			trackdef = trackerview_trackdef(self->view, track);
			if (ev->x > self->dragcolumnbase) {
				if (trackdef != &self->view->defaulttrackdef) {
					trackdef->numfx++;
				} else {
					trackdef = malloc(sizeof(TrackDef));
					trackdef_init(trackdef);
					trackdef->numfx = 2;
					psy_table_insert(&self->view->trackconfigs, track, trackdef);				
				}
			} else
			if (ev->x < self->dragcolumnbase && trackdef->numfx > 1) {
				if (trackdef != &self->view->defaulttrackdef) {
					trackdef->numfx--;
					if (trackdef->numfx == 1) {
						free(trackdef);
						psy_table_remove(&self->view->trackconfigs, track);
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
}

void trackergrid_onmousedoubleclick(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	trackerview_selectcol(self->view);
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
	PatternEditPosition position;
	int lines;
	int coloffset;	
	int cpx;
		
	rv = -1;
	position.offset = trackergrid_offset(self, y - self->dy, &lines);	
	position.track = trackerview_screentotrack(self->view, x - self->dx);
	coloffset = (x - self->dx - self->view->metrics.patterntrackident) -
		trackerview_track_x(self->view, position.track);
	position.column = 0;
	position.digit = 0;
	trackdef = trackerview_trackdef(self->view, position.track);
	cpx = 0;
	while (position.column < trackdef_numcolumns(trackdef) &&
			cpx + trackdef_columnwidth(trackdef, position.column,
			self->view->metrics.textwidth) < coloffset) {		
		cpx += trackdef_columnwidth(trackdef, position.column, 
			self->view->metrics.textwidth);
		++position.column;
	}	
	position.digit = (coloffset - cpx) / self->view->metrics.textwidth;
	if (position.digit >= trackdef_columndef(trackdef, position.column)->numchars) {
		rv = position.column;
	}	
	return rv;
}

PatternEditPosition trackergrid_makecursor(TrackerGrid* self, int x, int y)
{
	PatternEditPosition rv;
	TrackDef* trackdef;	
	int lines;
	int coloffset;	
	int cpx;
	
	rv.offset = trackergrid_offset(self, y - self->dy, &lines);	
	rv.track = trackerview_screentotrack(self->view, x - self->dx);
	coloffset = (x - self->dx - self->view->metrics.patterntrackident) -
		trackerview_track_x(self->view, rv.track);
	rv.column = 0;
	rv.digit = 0;
	trackdef = trackerview_trackdef(self->view, rv.track);
	cpx = 0;
	while (rv.column < trackdef_numcolumns(trackdef) &&
			cpx + trackdef_columnwidth(trackdef, rv.column,
			self->view->metrics.textwidth) < coloffset) {		
		cpx += trackdef_columnwidth(trackdef, rv.column, 
			self->view->metrics.textwidth);
		++rv.column;
	}	
	rv.digit = (coloffset - cpx) / self->view->metrics.textwidth;
	if (rv.digit >= trackdef_numdigits(trackdef, rv.column)) {
		rv.digit = trackdef_numdigits(trackdef, rv.column) - 1;
	}
	self->cursor.pattern = 
		workspace_patterneditposition(self->view->workspace).pattern;
	return rv;
}
	
void trackergrid_onmouseup(TrackerGrid* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	self->dragcolumn = -1;
}

void trackergrid_onfocuslost(TrackerGrid* self, psy_ui_Component* sender)
{
	psy_ui_component_invalidate(&self->component);
}

// trackerview
static psy_ui_ComponentVtable trackerview_vtable;
static int trackerview_vtable_initialized = 0;

static void trackerview_vtable_init(TrackerView* self)
{
	if (!trackerview_vtable_initialized) {
		trackerview_vtable = *(self->component.vtable);
		trackerview_vtable.onalign = (psy_ui_fp_onalign) trackerview_onalign;
		trackerview_vtable.onkeydown = (psy_ui_fp_onkeydown)
			trackerview_onkeydown;
		trackerview_vtable.onkeyup = (psy_ui_fp_onkeydown)
			trackerview_onkeyup;
		trackerview_vtable_initialized = 1;
	}
}

void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	trackerview_vtable_init(self);
	self->component.vtable = &trackerview_vtable;
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		trackerview_ondestroy);
	psy_signal_connect(&self->component.signal_timer, self,
		trackerview_ontimer);	
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;
	self->doseqtick = 0;
	self->cursorstep = 1;
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	self->wraparound = 1;
	self->showlinenumbers = 1;
	self->showlinenumbercursor = 1;
	self->showlinenumbersinhex = 1;
	self->showemptydata = 0;
	psy_table_init(&self->trackconfigs);
	trackerview_initcolumns(self);	
	trackerview_initmetrics(self);	
	trackerview_initinputs(self);			
	trackerview_initdefaultskin(self);		
	trackerheader_init(&self->header, &self->component, self);	
	self->header.numtracks = player_numsongtracks(&workspace->player);
	// pattern default line
	trackergrid_init(&self->griddefaults, &self->component, self, &workspace->player);	
	self->griddefaults.editmode = TRACKERGRID_EDITMODE_LOCAL;
	self->griddefaults.columnresize = 1;
	trackergrid_setpattern(&self->griddefaults, &self->workspace->player.patterndefaults);	
	// pattern main grid
	trackergrid_init(&self->grid, &self->component, self, &workspace->player);
	psy_signal_connect(&self->grid.component.signal_scroll, self,
		trackerview_onscroll);	
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component, self);	
	trackerlinenumbers_init(&self->linenumbers, &self->component, self);	
	zoombox_init(&self->zoombox, &self->component);
	psy_signal_connect(&self->zoombox.signal_changed, self,
		trackerview_onzoomboxchanged);
	trackerview_initblockmenu(self);	
	trackerview_connectworkspace(self);	
	trackerview_connectplayer(self);
	trackerview_readconfig(self);
	psy_ui_component_starttimer(&self->component, TIMERID_TRACKERVIEW, 50);
}

void trackerview_initblockmenu(TrackerView* self)
{
	patternblockmenu_init(&self->blockmenu, &self->component);
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
	inputs_dispose(&self->inputs);
	{
		psy_TableIterator it;	
	
		for (it = psy_table_begin(&self->trackconfigs); 
				!psy_tableiterator_equal(&it, psy_table_end());		
					psy_tableiterator_inc(&it)) {
			free(psy_tableiterator_value(&it));
		}			
		psy_table_dispose(&self->trackconfigs);
	}
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

void trackerview_onalign(TrackerView* self)
{
	psy_ui_Size size;
	psy_ui_Size menusize;	
	int headerheight = 30;	
	int griddefaultheight = 
		workspace_showgriddefaults(self->workspace)
			? self->metrics.lineheight
			: 0;
	int linenumberwidth = self->showlinenumbers ? 		
		self->metrics.textwidth * 
		(workspace_showbeatoffset(self->workspace) ? 8 : 5)	:0;
	int zoomboxheight;
	size = psy_ui_component_size(&self->component);
	menusize.width = self->blockmenu.component.visible
		? psy_ui_component_preferredsize(&self->blockmenu.component, &size).width
		: 0;	
	menusize.height = size.height;	
	psy_ui_component_setposition(&self->blockmenu.component,
		size.width - menusize.width,
		0, menusize.width,
		size.height);
	zoomboxheight = psy_ui_component_size(&self->zoombox.component).height;
	// header
	psy_ui_component_setposition(&self->header.component,
		linenumberwidth, 0,
		size.width - linenumberwidth - menusize.width,
		headerheight);
	// grid defaults
	if (workspace_showgriddefaults(self->workspace)) {
		psy_ui_component_setposition(&self->griddefaults.component,
			linenumberwidth, headerheight,
			size.width - linenumberwidth - menusize.width,
			griddefaultheight);
	}
	// grid
	psy_ui_component_setposition(&self->grid.component,
		linenumberwidth,
		headerheight + griddefaultheight,
		size.width - linenumberwidth - menusize.width,
		size.height - headerheight - griddefaultheight);	
	// line numbers
	psy_ui_component_setposition(&self->linenumbers.component,
		0, headerheight + griddefaultheight,
		linenumberwidth,
		size.height - headerheight - griddefaultheight -
		zoomboxheight);
	// line number label
	psy_ui_component_resize(&self->linenumberslabel.component,
		linenumberwidth, headerheight +
		griddefaultheight);
	// zoombox	
	psy_ui_component_setposition(&self->zoombox.component,
		0,
		size.height - zoomboxheight,		
		linenumberwidth,
		zoomboxheight);
	trackerview_computemetrics(self);
	trackergrid_adjustscroll(&self->grid);
}

// trackerheader
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackerView* view)
{			
	psy_ui_component_init(&self->component, parent);
	self->view = view;
	self->skin = &view->skin;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_draw, self,
		trackerheader_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		trackerheader_onmousedown);
	self->dx = 0;
	self->numtracks = 16;	
	self->classic = 1;
}

void trackerheader_ondraw(TrackerHeader* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	psy_ui_Size size;
	psy_ui_Rectangle r;
	int cpx = self->dx;
	uintptr_t track;

	size = psy_ui_component_size(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->skin->background);
	
	for (track = 0; track < self->numtracks; ++track) {		
		int trackx0 = track / 10;
		int track0x = track % 10;		
		SkinCoord digitx0 = self->skin->headercoords.digitx0;
		SkinCoord digit0x = self->skin->headercoords.digit0x;		
		digitx0.srcx += trackx0 * digitx0.srcwidth;
		digit0x.srcx += track0x * digit0x.srcwidth;
		skin_blitpart(g, &self->skin->bitmap, cpx, 0,
			&self->skin->headercoords.background);
		skin_blitpart(g, &self->skin->bitmap, cpx, 0, &digitx0);
		skin_blitpart(g, &self->skin->bitmap, cpx, 0, &digit0x);
		if (self->view->workspace->song) {
			if (patterns_istrackmuted(&self->view->workspace->song->patterns,
					track)) {
				skin_blitpart(g, &self->skin->bitmap, cpx, 0,
					&self->skin->headercoords.mute);
			}
			if (patterns_istracksoloed(&self->view->workspace->song->patterns,
					track)) {
				skin_blitpart(g, &self->skin->bitmap, cpx, 0,
					&self->skin->headercoords.solo);
			}
		}
		cpx += trackerview_trackwidth(self->view, track);
	}		
}

void trackerheader_onmousedown(TrackerHeader* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->view->workspace->song) {
		psy_ui_Rectangle r;
		uintptr_t track;
		int track_x;

		track = trackerview_screentotrack(self->view, ev->x - self->dx);
		track_x = trackerview_track_x(self->view, track);
		psy_ui_setrectangle(&r,
			self->skin->headercoords.mute.destx + track_x,
			self->skin->headercoords.mute.desty,
			self->skin->headercoords.mute.destwidth,
			self->skin->headercoords.mute.destheight);
		if (psy_ui_rectangle_intersect(&r, ev->x - self->dx, ev->y)) {
			if (patterns_istrackmuted(&self->view->workspace->song->patterns,
					track)) {
				patterns_unmutetrack(&self->view->workspace->song->patterns,
					track);
			} else {
				patterns_mutetrack(&self->view->workspace->song->patterns,
					track);
			}
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_setrectangle(&r,
			self->skin->headercoords.solo.destx + track_x,
			self->skin->headercoords.solo.desty,
			self->skin->headercoords.solo.destwidth,
			self->skin->headercoords.solo.destheight);

		if (psy_ui_rectangle_intersect(&r, ev->x - self->dx, ev->y)) {
			if (patterns_istracksoloed(&self->view->workspace->song->patterns,
					track)) {
				patterns_deactivatesolotrack(
					&self->view->workspace->song->patterns);
			} else {
				patterns_activatesolotrack(
					&self->view->workspace->song->patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void trackerlinenumbers_init(TrackerLineNumbers* self, psy_ui_Component* parent,
	TrackerView* view)
{			
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);		
	psy_signal_connect(&self->component.signal_draw, self,
		trackerlinenumbers_ondraw);	
	self->view = view;
	self->dy = 0;
	self->skin = &view->skin;
	psy_ui_component_setbackgroundcolor(&self->component,
		self->view->skin.background);
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	if (self->view->grid.pattern) {
		psy_ui_Size size;
		char buffer[20];		
		int cpy = self->dy;
		int line;		
		double offset;	
		TrackerGridBlock clip;

		size = psy_ui_component_size(&self->component);
		trackergrid_clipblock(&self->view->grid, &g->clip, &clip);		
		cpy = (clip.topleft.line) *
			self->view->metrics.lineheight + self->dy;
		offset = clip.topleft.offset;				
		line = clip.topleft.line;
		while (offset <= clip.bottomright.offset &&
			   offset < self->view->grid.pattern->length) {
			psy_ui_Rectangle r;			
			TrackerColumnFlags columnflags;			
			int ystart;
			int drawbeat;
			
			drawbeat = workspace_showbeatoffset(self->view->workspace);
			columnflags.playbar = trackergrid_testplaybar(&self->view->grid,
				offset);
			columnflags.mid = 0;
			columnflags.cursor = self->view->grid.drawcursor &&
				trackergrid_testcursor(&self->view->grid, 
				self->view->grid.cursor.track, offset);
			columnflags.beat = fmod(offset, 1.0f) == 0.0f;
			columnflags.beat4 = fmod(offset, 4.0f) == 0.0f;			
			columnflags.selection = 0; // trackergrid_testselection(
				//&self->view->grid, 0, offset) && self->view->grid.hasselection;
			setcolumncolor(self->skin, g, columnflags);
			if (self->view->showlinenumbersinhex) {
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
			psy_ui_setrectangle(&r, 0, cpy, size.width - 2,
				self->view->metrics.tm.tmHeight);
			psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_OPAQUE, r, buffer,
				strlen(buffer));		
			cpy += self->view->metrics.lineheight;
			ystart = cpy;			
			offset += self->view->grid.bpl;
			++line;
		}		
	}
}

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers* self,
	const PatternEditPosition* cursor)
{
	int line;		
	psy_ui_Rectangle r;		

	line = trackerview_offsettoscreenline(self->view, cursor->offset);	
	psy_ui_setrectangle(&r,
		0,
		self->view->metrics.lineheight * line + self->view->grid.dy,
		psy_ui_component_size(&self->component).width,
		self->view->metrics.lineheight);
	psy_ui_component_invalidaterect(&self->component, &r);	
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, psy_dsp_beat_t offset)
{
	int line;	
	psy_ui_Rectangle r;	
	
	if (offset >= self->view->sequenceentryoffset &&
			offset < self->view->sequenceentryoffset + self->view->grid.pattern->length) {
		line = (int) ((offset - self->view->sequenceentryoffset) 
			/ self->view->grid.bpl);	
		psy_ui_setrectangle(&r,
			self->view->grid.dx,
			self->view->metrics.lineheight * line + self->view->grid.dy,
			psy_ui_component_size(&self->component).width - self->view->grid.dx,
			self->view->metrics.lineheight);
		psy_ui_component_invalidaterect(&self->component, &r);
	}
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerView* view)
{		
	self->view = view;
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_draw, self,
		OnLineNumbersLabelDraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		OnLineNumbersLabelMouseDown);
}

void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel* self,
	psy_ui_Component* sender)
{
	self->view->header.classic = !self->view->header.classic;
	if (self->view->header.classic) {
		trackerview_setclassicheadercoords(self->view);
	} else {
		trackerview_setheadercoords(self->view);
	}
	psy_ui_component_invalidate(&self->view->header.component);
}

void OnLineNumbersLabelDraw(TrackerLineNumbersLabel* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	psy_ui_Size size;
	psy_ui_Rectangle r;

	size = psy_ui_component_size(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);	
	psy_ui_setbackgroundcolor(g, self->view->skin.background);
	psy_ui_settextcolor(g, self->view->skin.font);	
	psy_ui_textoutrectangle(g, r.left, r.top, 0, r, "Line", strlen("Line"));
}

void trackerview_ontimer(TrackerView* self, psy_ui_Component* sender, int timerid)
{
	if (timerid == TIMERID_TRACKERVIEW && self->grid.pattern) {
		if (self->doseqtick && player_playing(self->grid.player)) {
			if (!workspace_followingsong(self->workspace)) {
				trackerview_invalidateline(self, self->lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
				self->lastplayposition = player_position(&self->workspace->player);
				trackerview_invalidateline(self, self->lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
			}
		} else {
			if (self->lastplayposition != -1) {				
				trackerview_invalidateline(self, self->lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
				self->lastplayposition = -1;
			}
		}
		if (self->grid.pattern && self->grid.pattern->opcount != self->opcount &&
				self->syncpattern) {
			psy_ui_component_invalidate(&self->grid.component);
			psy_ui_component_invalidate(&self->linenumbers.component);
		}
		self->opcount = self->grid.pattern ? self->grid.pattern->opcount : 0;		
	}
}

void trackerview_onseqlinetick(TrackerView* self, psy_audio_Sequencer* sender)
{	
	if (psy_audio_sequencer_playing(sender)) {
		self->doseqtick = 1;
	}
}

int trackerview_numlines(TrackerView* self)
{
	return self->grid.pattern ?
		(int)(pattern_length(self->grid.pattern) / self->grid.bpl) : 0;
}

int trackerview_offsettoscreenline(TrackerView* self, psy_dsp_big_beat_t offset)
{
	return (int) (offset / self->grid.bpl);
}

void trackerview_onlpbchanged(TrackerView* self, psy_audio_Player* sender, uintptr_t lpb)
{
	self->grid.bpl = 1 / (psy_dsp_big_beat_t) lpb;
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
		} else {
			psy_ui_component_hide(&self->griddefaults.component);
		}
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
	if (strcmp(psy_properties_key(property), "font") == 0) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;		

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_properties_valuestring(property));
		psy_ui_font_init(&font, &fontinfo);		
		trackerview_setfont(self, &font);
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
			psy_ui_component_show(&self->griddefaults.component);
		} else {
			psy_ui_component_hide(&self->griddefaults.component);
		}
		trackerview_showlinenumbers(self, psy_properties_bool(pv, "linenumbers", 1));
		trackerview_showlinenumbercursor(self, psy_properties_bool(pv, "linenumberscursor", 1));
		trackerview_showlinenumbersinhex(self, psy_properties_bool(pv, "linenumbersinhex", 1));
		self->wraparound = psy_properties_bool(pv, "wraparound", 1);
		trackerview_showemptydata(self, psy_properties_bool(pv, "drawemptydata", 1));
		trackerview_setcentermode(self, psy_properties_bool(pv, "centercursoronscreen", 1));
		{
			psy_ui_FontInfo fontinfo;
			psy_ui_Font font;
			
			psy_ui_fontinfo_init_string(&fontinfo,
				psy_properties_readstring(pv, "font", "tahoma;-16"));
			psy_ui_font_init(&font, &fontinfo);		
			trackerview_setfont(self, &font);
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
	trackergrid_adjustscroll(&self->grid);
}

void trackerview_showlinenumbers(TrackerView* self, int showstate)
{
	self->showlinenumbers = showstate;
	if (self->showlinenumbers != 0) {		
		psy_ui_component_show(&self->linenumbers.component);
		psy_ui_component_show(&self->linenumberslabel.component);
	} else {
		psy_ui_component_hide(&self->linenumbers.component);
		psy_ui_component_hide(&self->linenumberslabel.component);
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
	trackergrid_adjustscroll(&self->grid);
	if (mode) {
		trackerview_centeroncursor(self);
	} else {
		self->grid.dy = 0;
		self->linenumbers.dy = 0;
		psy_ui_component_setverticalscrollposition(&self->grid.component, 0);
	}
}

void trackerview_initinputs(TrackerView* self)
{
	inputs_init(&self->inputs);	
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_UP, 0, 0), CMD_NAVUP);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_DOWN, 0, 0),CMD_NAVDOWN);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_LEFT, 0, 0),CMD_NAVLEFT);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_RIGHT, 0, 0),CMD_NAVRIGHT);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_PRIOR, 0, 0),CMD_NAVPAGEUP);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_NEXT, 0, 0),CMD_NAVPAGEDOWN);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_HOME, 0, 0), CMD_NAVTOP);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_END, 0, 0), CMD_NAVBOTTOM);	
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_TAB, 1, 0), CMD_COLUMNPREV);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_TAB, 0, 0), CMD_COLUMNNEXT);	
		
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_INSERT, 0, 0), CMD_ROWINSERT);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_BACK, 0, 0), CMD_ROWDELETE);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_DELETE, 0, 0), CMD_ROWCLEAR);

	inputs_define(&self->inputs, encodeinput('B', 0, 1), CMD_BLOCKSTART);
	inputs_define(&self->inputs, encodeinput('E', 0, 1), CMD_BLOCKEND);
	inputs_define(&self->inputs, encodeinput('U', 0, 1), CMD_BLOCKUNMARK);
	inputs_define(&self->inputs, encodeinput('X', 0, 1), CMD_BLOCKCUT);
	inputs_define(&self->inputs, encodeinput('C', 0, 1), CMD_BLOCKCOPY);
	inputs_define(&self->inputs, encodeinput('V', 0, 1), CMD_BLOCKPASTE);
	inputs_define(&self->inputs, encodeinput('M', 0, 1), CMD_BLOCKMIX);
	
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_F12, 0, 1), CMD_TRANSPOSEBLOCKINC);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_F11, 0, 1), CMD_TRANSPOSEBLOCKDEC);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_F12, 1, 1), CMD_TRANSPOSEBLOCKINC12);
	inputs_define(&self->inputs, encodeinput(psy_ui_KEY_F11, 1, 1), CMD_TRANSPOSEBLOCKDEC12);
	
	inputs_define(&self->inputs, encodeinput('A', 0, 1), CMD_SELECTALL);
	inputs_define(&self->inputs, encodeinput('R', 0, 1), CMD_SELECTCOL);
	inputs_define(&self->inputs, encodeinput('K', 0, 1), CMD_SELECTBAR);	
		
	inputs_define(&self->inputs, encodeinput('Z', 0, 1), CMD_UNDO);
	inputs_define(&self->inputs, encodeinput('Z', 1, 1), CMD_REDO);
	inputs_define(&self->inputs, encodeinput('F', 0, 1), CMD_FOLLOWSONG);
}

void trackerview_setpattern(TrackerView* self, psy_audio_Pattern* pattern)
{		
	if (pattern) {
		self->opcount = pattern->opcount;
	}			
	trackergrid_setpattern(&self->grid, pattern);
	self->linenumbers.dy = 0;	
	psy_ui_component_invalidate(&self->linenumbers.component);
	psy_ui_component_invalidate(&self->header.component);
	
}

void trackergrid_setpattern(TrackerGrid* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;	
	self->cursor.offset = 0;
	self->dy = 0;	
	trackergrid_adjustscroll(self);	
}

void trackergrid_oninterpolatelinear(TrackerGrid* self)
{
	pattern_blockinterpolatelinear(self->pattern,
		self->selection.topleft,
		self->selection.bottomright,
		(psy_dsp_beat_t)self->bpl);
}

void trackergrid_oninterpolatecurve(TrackerGrid* self)
{
}

void trackergrid_onchangegenerator(TrackerGrid* self)
{
	if (self->pattern && self->view->workspace->song) {
			pattern_changemachine(self->pattern, 
			self->selection.topleft,
			self->selection.bottomright,
			self->view->workspace->song->machines.slot);
		psy_ui_component_invalidate(&self->view->component);
	}
}

void trackergrid_onchangeinstrument(TrackerGrid* self)
{
	if (self->pattern && self->view->workspace->song) {
			pattern_changeinstrument(self->pattern, 
			self->selection.topleft,
			self->selection.bottomright,
			self->view->workspace->song->instruments.slot);
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
		PatternNode* begin;		
		PatternNode* p;
		PatternNode* q;
		PatternNode* prev = 0;
		psy_dsp_beat_t offset;
		int trackoffset;

		begin = pattern_greaterequal(self->pattern, 
			(psy_dsp_beat_t) self->selection.topleft.offset);
		offset = (psy_dsp_beat_t) self->selection.topleft.offset;
		trackoffset = self->selection.topleft.track;
		pattern_dispose(&self->view->workspace->patternpaste);
		pattern_init(&self->view->workspace->patternpaste);
		p = begin;
		while (p != 0) {			
			psy_audio_PatternEntry* entry;
			q = p->next;

			entry = (psy_audio_PatternEntry*) p->entry;
			if (entry->offset < self->selection.bottomright.offset) {
				if (entry->track >= self->selection.topleft.track &&
						entry->track < self->selection.bottomright.track) {						
					prev = pattern_insert(&self->view->workspace->patternpaste,
						prev, entry->track - trackoffset, 
						entry->offset - offset,
						patternentry_front(entry));
				}
			} else {
				break;
			}
			p = q;
		}
		pattern_setmaxsongtracks(&self->view->workspace->patternpaste, 
			self->selection.bottomright.track -
			self->selection.topleft.track);
		pattern_setlength(&self->view->workspace->patternpaste,
			(psy_dsp_beat_t)(self->selection.bottomright.offset -
			self->selection.topleft.offset));

	}
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_onblockpaste(TrackerGrid* self)
{
	PatternNode* p;
	PatternNode* prev = 0;
	psy_dsp_beat_t offset;
	int trackoffset;
	PatternEditPosition begin;
	PatternEditPosition end;

	offset = (psy_dsp_beat_t) self->cursor.offset;
	trackoffset = self->cursor.track;
	p = self->view->workspace->patternpaste.events;	

	begin = end = self->cursor;
	end.track += self->view->workspace->patternpaste.maxsongtracks;
	end.offset += self->view->workspace->patternpaste.length;
	pattern_blockremove(self->pattern, begin, end);
	// sequencer_checkiterators(&self->workspace->player.sequencer,
	//	node);
	while (p != 0) {			
		psy_audio_PatternEntry* pasteentry;
		PatternNode* node;

		pasteentry = (psy_audio_PatternEntry*) p->entry;
		node = pattern_findnode(self->pattern, 
			pasteentry->track + trackoffset,
			pasteentry->offset + offset,			
			(psy_dsp_beat_t) self->bpl,
			&prev);
		if (node) {
			psy_audio_PatternEntry* entry;

			entry = (psy_audio_PatternEntry*) node->entry;
			*patternentry_front(entry) = *patternentry_front(pasteentry);
		} else {
			pattern_insert(self->pattern,
				prev, 
				pasteentry->track + trackoffset, 
				pasteentry->offset + offset, 
				patternentry_front(pasteentry));
		}						
		p = p->next;
	}	
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_onblockmixpaste(TrackerGrid* self)
{
	PatternNode* p;
	PatternNode* prev = 0;
	psy_dsp_beat_t offset;
	int trackoffset;

	offset = (psy_dsp_beat_t) self->cursor.offset;
	trackoffset = self->cursor.track;
	p = self->view->workspace->patternpaste.events;
	while (p != 0) {			
		psy_audio_PatternEntry* pasteentry;		

		pasteentry = (psy_audio_PatternEntry*) p->entry;
		if (!pattern_findnode(self->pattern, pasteentry->track + trackoffset,
				pasteentry->offset + offset, (psy_dsp_beat_t) self->bpl,
				&prev)) {
			pattern_insert(self->pattern,
					prev, 
					pasteentry->track + trackoffset, 
					pasteentry->offset + offset, 
					patternentry_front(pasteentry));
		}						
		p = p->next;
	}	
	psy_ui_component_invalidate(&self->view->component);
}

void trackergrid_onblockdelete(TrackerGrid* self)
{
	if (self->hasselection) {
		pattern_blockremove(self->pattern, 
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
		undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->pattern,
				self->selection,
				self->cursor, +1, self->view->workspace)->command);
	}
}

void trackergrid_onblocktransposedown(TrackerGrid* self)
{
	if (self->hasselection) {		
		undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->pattern,
				self->selection,
				self->cursor, -1, self->view->workspace)->command);
	}
}

void trackergrid_onblocktransposeup12(TrackerGrid* self)
{
	if (self->hasselection) {		
		undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->pattern,
				self->selection,
				self->cursor, 12, self->view->workspace)->command);
	}
}

void trackergrid_onblocktransposedown12(TrackerGrid* self)
{
	if (self->hasselection) {		
		undoredo_execute(&self->view->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->pattern,
				self->selection,
				self->cursor, -12, self->view->workspace)->command);
	}
}

void trackerview_onpatternimport(TrackerView* self)
{
	if (self->grid.pattern) {
		psy_ui_OpenDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.psb";

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", filter, "PSB",
			workspace_songs_directory(self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(self->grid.pattern,
				psy_ui_opendialog_filename(&dialog),
				1.f / player_lpb(&self->workspace->player));
		}
		psy_ui_opendialog_dispose(&dialog);		
	}
}

void trackerview_onpatternexport(TrackerView* self)
{
	if (self->grid.pattern) {
		psy_ui_SaveDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.PSB";

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", filter, "PSB",
			workspace_songs_directory(self->workspace));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(self->grid.pattern,
				psy_ui_savedialog_filename(&dialog),
				1.f / player_lpb(&self->workspace->player),
				player_numsongtracks(&self->workspace->player));
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
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;

		psy_ui_fontinfo_init(&fontinfo, "Tahoma", 
			(int) (psy_properties_int(pv, "fontsize", 80) *
			zoombox_rate(sender)));
		psy_ui_font_init(&font, &fontinfo);		
		trackerview_setfont(self, &font);
		psy_ui_font_dispose(&font);		
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
		psy_ui_component_children(&self->component, 0), 
		psy_ui_ALIGN_TOP, 0));	
}

void trackerview_onpatterneditpositionchanged(TrackerView* self, Workspace* sender)
{	
	PatternEditPosition oldcursor;

	oldcursor = self->grid.cursor;	
	self->grid.cursor = workspace_patterneditposition(sender);
	if (!patterneditposition_equal(&self->grid.cursor, &oldcursor)) {
		trackerview_invalidatecursor(self, &oldcursor);
		if (player_playing(&sender->player) && workspace_followingsong(sender)) {
			int scrolldown;

			scrolldown = self->lastplayposition < 
				player_position(&self->workspace->player);
			trackerview_invalidateline(self, self->lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
			self->lastplayposition = player_position(&self->workspace->player);
			trackerview_invalidateline(self, self->lastplayposition);
			trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);			
			if (self->lastplayposition >= self->sequenceentryoffset &&
					self->lastplayposition < self->sequenceentryoffset +
					self->grid.pattern->length) {
				if (scrolldown) {
					trackerview_scrolldown(self);
				} else {
					trackerview_scrollup(self);
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

		machine = machines_at(&self->workspace->song->machines, slot);
		assert(machine);
		value = machine_parametervalue_scaled(machine, tweak, normvalue);
		patternevent_init_all(&event,
			(unsigned char) (
				workspace_recordtweaksastws(sender)
				? NOTECOMMANDS_TWEAKSLIDE
				: NOTECOMMANDS_TWEAK),
			NOTECOMMANDS_INST_EMPTY,
			(unsigned char) machines_slot(&self->workspace->song->machines),
			NOTECOMMANDS_VOL_EMPTY,
			(unsigned char) ((value & 0xFF00) >> 8),
			(unsigned char) (value & 0xFF));
		event.inst = (unsigned char) tweak;
		trackerview_preventsync(self);
		undoredo_execute(&self->workspace->undoredo,
			&InsertCommandAlloc(self->grid.pattern, self->grid.bpl,
				self->grid.cursor, event, self->workspace)->command);
		if (workspace_advancelineonrecordtweak(sender) &&
				!(workspace_followingsong(sender) && 
				  player_playing(&sender->player))) {			
			trackerview_advanceline(self);
		} else {
			trackerview_invalidatecursor(self, &self->grid.cursor);
		}
		trackerview_enablesync(self);
	}
}

int trackergrid_preferredtrackwidth(TrackerGrid* self)
{	
	return trackdef_width(&self->view->defaulttrackdef,
		self->view->metrics.textwidth);
}

int trackerheader_preferredtrackwidth(TrackerHeader* self)
{
	return self->view->skin.headercoords.background.destwidth;
}

void trackerview_initmetrics(TrackerView* self)
{
	self->metrics.textwidth = 9;
	self->metrics.textleftedge = 2;
	self->metrics.tm.tmHeight = 12;
	self->metrics.lineheight = 12 + 1;	
	self->metrics.patterntrackident = 0;
	self->metrics.headertrackident = 0;	
	self->metrics.visilines = 25;
}

void trackerview_computemetrics(TrackerView* self)
{
	psy_ui_Size gridsize;	
	int trackwidth;

	gridsize = psy_ui_component_size(&self->grid.component);	
	self->metrics.tm = psy_ui_component_textmetric(&self->component);	
	self->metrics.textwidth = (int)(self->metrics.tm.tmAveCharWidth * 1.5) + 2;
	self->metrics.textleftedge = 2;
	trackergrid_computecolumns(&self->grid, self->metrics.textwidth);	
	self->metrics.lineheight = self->metrics.tm.tmHeight + 1;
	trackwidth = max(
		trackergrid_preferredtrackwidth(&self->grid),
		trackerheader_preferredtrackwidth(&self->header));	
	self->metrics.patterntrackident =
		(trackwidth -
		trackergrid_preferredtrackwidth(&self->grid)) / 2;
	if (self->metrics.patterntrackident < 0) {
		self->metrics.patterntrackident = 0;
	}	
	self->metrics.headertrackident = 0;	
	self->metrics.visilines = gridsize.height / self->metrics.lineheight;
}

void trackerview_setfont(TrackerView* self, psy_ui_Font* font)
{	
	psy_ui_component_setfont(&self->component, font);
	psy_ui_component_setfont(&self->griddefaults.component, font);
	psy_ui_component_setfont(&self->grid.component, font);
	psy_ui_component_setfont(&self->linenumbers.component, font);
	trackerview_computemetrics(self);	
	trackerview_updatescrollstep(self);	
	psy_ui_component_align(&self->component);
}

void trackerview_updatescrollstep(TrackerView* self)
{
	self->grid.component.scrollstepx = trackerview_trackwidth(self, 
		trackerview_track_x(self,
			trackerview_screentotrack(self, -self->grid.dx)));
	self->grid.component.scrollstepy = self->metrics.lineheight;
}

int trackerview_track_x(TrackerView* self, uintptr_t track)
{
	int rv = 0;
	uintptr_t t;

	for (t = 0; t < track; ++t) {
		rv += trackerview_trackwidth(self, t);
	}
	return rv;
}

int trackerview_trackwidth(TrackerView* self, uintptr_t track)
{	
	return trackdef_width(trackerview_trackdef(self, track),
		self->metrics.textwidth) + 1;
}

uintptr_t trackerview_screentotrack(TrackerView* self, int x)
{
	int curr = 0;
	uintptr_t rv = 0;

	while (rv < player_numsongtracks(&self->workspace->player)) {
		curr += trackerview_trackwidth(self, rv);
		if (curr > x) {
			break;
		}
		++rv;
	}
	return rv;
}

TrackDef* trackerview_trackdef(TrackerView* self, uintptr_t track)
{
	TrackDef* rv;

	rv = psy_table_at(&self->trackconfigs, track);
	if (!rv) {
		rv = &self->defaulttrackdef;
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
	
	unsigned int column;
	int focus;
	int cpx;
	TrackDef* trackdef;
	psy_audio_PatternEvent* event;
	TrackerColumnFlags currcolumnflags;	
	currcolumnflags = columnflags;
	event = patternentry_front(entry);		
	trackdef = trackerview_trackdef(self->view, entry->track);
	focus = psy_ui_component_hasfocus(&self->component);
	currcolumnflags.cursor = self->drawcursor && focus && columnflags.cursor && self->cursor.column == 0;
	setcolumncolor(&self->view->skin, g, currcolumnflags);
	cpx = 0;	
	// draw note	
	psy_ui_setrectangle(&r, x + cpx, y,
		self->view->metrics.textwidth * 3,
		self->view->metrics.tm.tmHeight);
	notestr = (event->note != NOTECOMMANDS_EMPTY || !self->view->showemptydata) 
		  ? psy_dsp_notetostr(event->note, self->notestabmode)
		  : emptynotestr;		
	psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
		notestr, strlen(notestr));		
	cpx += trackdef_columnwidth(trackdef, 0, self->view->metrics.textwidth);
	// draw digit columns
	for (column = 1; column < trackdef_numcolumns(trackdef); ++column) {
		uintptr_t digit;
		uintptr_t value;
		uintptr_t empty;
		uintptr_t num;

		value = trackdef_value(trackdef, column, entry);
		empty = trackdef_emptyvalue(trackdef, column) == value;
		num = trackdef_numdigits(trackdef, column);
		for (digit = 0; digit < num; ++digit) {
			uint8_t digitvalue;
			
			digitvalue = ((value >> ((num - digit - 1) * 4)) & 0x0F);			
			currcolumnflags.cursor = self->drawcursor && focus &&
					columnflags.cursor && self->cursor.column == column &&
					self->cursor.digit == digit;
			setcolumncolor(&self->view->skin, g, currcolumnflags);
			trackergrid_drawdigit(self, g, x + cpx + digit * 
				self->view->metrics.textwidth, y, digitvalue,
				empty);
		}
		cpx += trackdef_columnwidth(trackdef, column,
			self->view->metrics.textwidth);
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

			event = (psy_audio_PatternEvent*) p->entry;
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

			event = (psy_audio_PatternEvent*) p->entry;
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
