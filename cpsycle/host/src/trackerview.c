// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackerview.h"
#include <pattern.h>
#include "cmdsnotes.h"
#include "skinio.h"
#include "skingraphics.h"
#include "resources/resource.h"

#include <string.h>
#include <math.h>

#include <dir.h>
#include <portable.h>

#define TIMERID_TRACKERVIEW 600
static const psy_dsp_big_beat_t epsilon = 0.0001;

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
static void trackergrid_onkeydown(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void trackergrid_onkeyup(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static PatternEditPosition trackergrid_makecursor(TrackerGrid* self, int x, int y);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender,
	int stepx, int stepy);
static int trackergrid_testcursor(TrackerGrid*, unsigned int track,
	psy_dsp_big_beat_t offset);
static int trackergrid_testselection(TrackerGrid*, unsigned int track, double offset);
static int trackergrid_testplaybar(TrackerGrid* self, psy_dsp_big_beat_t offset);
static void trackergrid_clipblock(TrackerGrid*, const ui_rectangle*,
	TrackerGridBlock*);
static void trackergrid_drawdigit(TrackerGrid* self, psy_ui_Graphics*,
	int x, int y, int value, int empty);
static void trackergrid_adjustscroll(TrackerGrid*);
static double trackergrid_offset(TrackerGrid*, int y, unsigned int* lines);
static void trackergrid_numtrackschanged(TrackerGrid*, psy_audio_Player*,
	unsigned int numtracks);

static void trackerview_initfont(self);
static void trackerview_setfont(TrackerView*, ui_font*);
static void trackerview_initmetrics(TrackerView*);
static void trackerview_computemetrics(TrackerView*);
static void trackerview_ondestroy(TrackerView*, psy_ui_Component* sender);
static void trackerview_onkeydown(TrackerView*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void trackerview_onkeyup(TrackerView*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void trackerview_ontimer(TrackerView*, psy_ui_Component* sender, 
	int timerid);
static void trackerview_onalign(TrackerView*, psy_ui_Component* sender);
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
static void trackerview_onchangegenerator(TrackerView* self);
static void trackerview_onchangeinstrument(TrackerView* self);
static void trackerview_blockstart(TrackerView*);
static void trackerview_blockend(TrackerView*);
static void trackerview_blockunmark(TrackerView*);
static void trackerview_showblockmenu(TrackerView*);
static void trackerview_hideblockmenu(TrackerView*);
static void trackerview_toggleblockmenu(TrackerView*);
static void trackerview_centeroncursor(TrackerView*);
static void trackerview_onblockcut(TrackerView*);
static void trackerview_onblockcopy(TrackerView*);
static void trackerview_onblockpaste(TrackerView*);
static void trackerview_onblockmixpaste(TrackerView*);
static void trackerview_onblockdelete(TrackerView*);
static void trackerview_onblocktransposeup(TrackerView*);
static void trackerview_onblocktransposedown(TrackerView*);
static void trackerview_onblocktransposeup12(TrackerView*);
static void trackerview_onblocktransposedown12(TrackerView*);
static void trackerview_onlpbchanged(TrackerView*, psy_audio_Player* sender,
	uintptr_t lpb);
static void trackerview_onpatterneditpositionchanged(TrackerView*,
	Workspace* sender);
static void trackerview_onparametertweak(TrackerView*,
	Workspace* sender, int slot, int tweak, int value);
static void trackerview_onskinchanged(TrackerView*, Workspace*, psy_Properties*);
static int trackerview_trackwidth(TrackerView*, uintptr_t track);
static int trackerview_track_x(TrackerView*, uintptr_t track);
static int trackerview_screentotrack(TrackerView*, int x);
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

static void trackerconfigheader_ondraw(TrackerConfigHeader*,
	psy_ui_Component* sender, psy_ui_Graphics*);
static void trackerconfigheader_onmousedown(TrackerConfigHeader*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void trackerconfigheader_drawtrack(TrackerConfigHeader*,
	psy_ui_Graphics*, int cpx);

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
		trackergrid_vtable.draw = (psy_ui_fp_draw) trackergrid_ondraw;
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
	PatternEditPosition cursor, psy_audio_PatternEvent event, Workspace* workspace)
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
	self->view = view;
	self->editmode = TRACKERGRID_EDITMODE_SONG;
	self->hasselection = 0;
	self->midline = 1;
	self->view->workspace->chordmode = 0;
	self->chordbegin = 0;
	self->pattern = 0;
	ui_component_init(&self->component, parent);
	trackergrid_vtable_init(self);
	self->component.vtable = &trackergrid_vtable;
	psy_signal_connect(&player->signal_numsongtrackschanged, self,
		trackergrid_numtrackschanged);
	psy_signal_connect(&self->component.signal_keydown,self, 
		trackergrid_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self, 
		trackergrid_onkeyup);
	psy_signal_connect(&self->component.signal_mousedown, self,
		trackergrid_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		trackergrid_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		trackergrid_onmouseup);
	psy_signal_connect(&self->component.signal_mousedoubleclick, self,
		trackergrid_onmousedoubleclick);	
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);
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
	self->component.doublebuffered = 1;
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
	self->view->configheader.numtracks = numsongtracks;
	trackergrid_adjustscroll(self);
	ui_component_invalidate(&self->view->component);
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
	ui_component_setbackgroundcolor(
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
	ui_component_invalidate(&self->component);
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
		ui_drawsolidrectangle(g, g->clip, self->view->skin.background);	
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

void trackergrid_clipblock(TrackerGrid* self, const ui_rectangle* clip,
	TrackerGridBlock* block)
{	
	int lines;
	
	block->topleft.track = trackerview_screentotrack(self->view,
		clip->left - self->dx);	
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
	ui_rectangle r;
	unsigned int track;
	ui_size size;
	int trackswidth;

	size = ui_component_size(&self->component);
	for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
		trackergrid_drawtrackbackground(self, g, track);
	}
	trackswidth = trackerview_track_x(self->view, self->numtracks - 1) +
		trackerview_trackwidth(self->view, self->numtracks - 1);
	ui_setrectangle(&r,
		trackswidth + self->dx, 0, 
		size.width - trackswidth + self->dx,
		size.height);
	ui_drawsolidrectangle(g, r, self->view->skin.background);
}

void trackergrid_drawtrackbackground(TrackerGrid* self, psy_ui_Graphics* g, int track)
{
	ui_rectangle r;
	ui_size size;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, trackerview_track_x(self->view, track) + self->dx, 0,
		trackerview_trackwidth(self->view, track), size.height);
	ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

int trackergrid_testselection(TrackerGrid* self, unsigned int track, double offset)
{
	return self->hasselection &&
		track >= self->selection.topleft.track &&
		track < self->selection.bottomright.track &&
		offset >= self->selection.topleft.offset &&
		offset < self->selection.bottomright.offset;
}

int trackergrid_testcursor(TrackerGrid* self, unsigned int track, double offset)
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
	ui_size size;
	int halfy;	
	psy_audio_PatternEntry empty;

	patternentry_init(&empty);
	size = ui_component_size(&self->component);
	halfy = (self->view->metrics.visilines / 2) * self->view->metrics.lineheight;	
	cpy = clip->topleft.line * self->view->metrics.lineheight + self->dy;
	offset = clip->topleft.offset;	
	node = pattern_greaterequal(self->pattern, (psy_dsp_beat_t)offset);
	while (offset <= clip->bottomright.offset && offset < self->pattern->length) {
		int fill;
		
		columnflags.beat = fabs(fmod(offset, 1.0f)) < 0.01f ;
		columnflags.beat4 = fabs(fmod(offset, 4.0f)) < 0.01f;
		columnflags.mid = self->midline && cpy >= halfy && cpy < halfy + self->view->metrics.lineheight;
		
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
		ui_setbackgroundcolor(g, skin->cursor);
		ui_settextcolor(g, skin->fontCur);
	} else
	if (flags.playbar) {
		ui_setbackgroundcolor(g, skin->playbar);
		ui_settextcolor(g, skin->fontPlay);		
	} else		
	if (flags.selection != 0) {		
		ui_setbackgroundcolor(g, skin->cursor);
		ui_settextcolor(g, skin->fontCur);		
	} else 
	if (flags.mid) {
		ui_setbackgroundcolor(g, skin->midline);
		if (flags.cursor != 0) {
			ui_settextcolor(g, skin->fontCur);
		} else {
			ui_settextcolor(g, skin->font);
		}
	} else {	
		if (flags.beat4) {
			ui_setbackgroundcolor(g, skin->row4beat);			
			ui_settextcolor(g, skin->font);
		} else
		if (flags.beat) {
			ui_setbackgroundcolor(g, skin->rowbeat);
			ui_settextcolor(g, skin->font);
		} else {
			ui_setbackgroundcolor(g, skin->row);
			ui_settextcolor(g, skin->font);
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
	ui_rectangle r;
	ui_setrectangle(&r, x, y, self->view->metrics.textwidth,
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
	ui_textoutrectangle(g, r.left + self->view->metrics.textleftedge, r.top,
		ETO_OPAQUE | ETO_CLIPPED, r, buffer, strlen(buffer));	
}

void trackergrid_adjustscroll(TrackerGrid* self)
{
	ui_size size;	
	int vscrollmax;
	int stepx;
	size = ui_component_size(&self->component);		
	ui_component_sethorizontalscrollrange(&self->component, 0,
		self->numtracks - self->view->metrics.visitracks);
	vscrollmax = trackerview_numlines(self->view);
	if (!self->midline) {
		vscrollmax -= self->view->metrics.visilines;
	} else {
		vscrollmax -= 1;
	}
	ui_component_setverticalscrollrange(&self->component, 0, vscrollmax);
	if (self->midline) {		
		trackerview_centeroncursor(self->view);
	}
	stepx = -self->dx / self->view->metrics.trackwidth;
	ui_component_sethorizontalscrollposition(&self->component,
		stepx > 0 ? stepx : 0);
}

void trackerview_centeroncursor(TrackerView* self)
{
	int line;		
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset);
	self->grid.dy = (self->metrics.visilines / 2 - line) * self->metrics.lineheight;
	self->linenumbers.dy = self->grid.dy;
	ui_component_setverticalscrollposition(&self->grid.component,
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

void trackergrid_onkeydown(TrackerGrid* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{	
	if (self->editmode == TRACKERGRID_EDITMODE_LOCAL) {
		int cmd;

		cmd = inputs_cmd(&self->view->inputs, encodeinput(ev->keycode, 
			GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0));
		if (cmd == CMD_NAVLEFT) {
			trackergrid_prevcol(self);
			ui_component_invalidate(&self->component);
		} else
		if (cmd == CMD_NAVRIGHT) {
			trackergrid_nextcol(self);
			ui_component_invalidate(&self->component);
		} else {
			if (self->cursor.column != TRACKER_COLUMN_NOTE) {
				int digit = keycodetoint(ev->keycode);
				if (digit != -1) {
					trackergrid_inputvalue(self, digit, 1);
					ui_component_invalidate(&self->component);
					return;
				}			
			}				
			{	
				EventDriver* kbd;
				EventDriverCmd cmd;
				EventDriverData input;

				cmd.id = -1;			
				kbd = workspace_kbddriver(self->view->workspace);
				input.message = EVENTDRIVER_KEYDOWN;
				input.param1 = encodeinput(ev->keycode, 0, /* GetKeyState(VK_SHIFT) < 0 */
					GetKeyState(VK_CONTROL) < 0);
				kbd->cmd(kbd, input, &cmd);			
				trackergrid_inputnote(self,
					(psy_dsp_note_t)(cmd.id + workspace_octave(self->view->workspace) * 12),
					1);
				ui_component_invalidate(&self->component);
				return;					
			}
		}

	} else
	if (self->editmode == TRACKERGRID_EDITMODE_SONG) {
		sender->propagateevent = 1;
	}
}

void trackergrid_onkeyup(TrackerGrid* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* keyevent)
{
	sender->propagateevent = 1;	
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
		ui_component_invalidate(&self->component);
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
		ui_component_invalidate(&self->component);
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
		ui_component_invalidate(&self->component);
	}		
}

int trackerview_scrollup(TrackerView* self)
{
	int line;
	int topline;
	int rv = 1;
	ui_rectangle r;		
	
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset);		
	ui_setrectangle(&r,
		self->grid.cursor.track * self->metrics.trackwidth,
		self->metrics.lineheight * line,		
		self->metrics.trackwidth,
		self->metrics.lineheight);
	if (self->grid.midline) {
		topline = ui_component_size(&self->grid.component).height  
			/ self->metrics.lineheight / 2;		
	} else {
		topline = 0;
	}
	if (-self->grid.dy + topline * self->metrics.lineheight > r.top) {
		int dlines = (-self->grid.dy + topline * self->metrics.lineheight - r.top) /
			(self->metrics.lineheight);
		psy_signal_emit(&self->grid.component.signal_scroll,
			&self->grid.component, 2, 0, dlines);
		ui_component_scrollstep(&self->grid.component, 0, dlines);
		ui_component_setverticalscrollposition(
		&self->grid.component, 
		ui_component_verticalscrollposition(&self->grid.component) - dlines);
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
		ui_component_scrollstep(&self->grid.component, 0, -dlines);
		ui_component_setverticalscrollposition(
			&self->grid.component, 
			ui_component_verticalscrollposition(&self->grid.component) + dlines);
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
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

int trackerview_scrollleft(TrackerView* self)
{	
	int tracks;
	int invalidate = 1;
	
	tracks = self->grid.cursor.track;
	if (-self->grid.dx / self->metrics.trackwidth > tracks) {
		self->grid.dx = -tracks * self->metrics.trackwidth;
		self->header.dx = self->grid.dx;
		self->configheader.dx = self->grid.dx;
		ui_component_invalidate(&self->grid.component);
		ui_component_update(&self->grid.component);
		ui_component_invalidate(&self->configheader.component);
		ui_component_update(&self->configheader.component);
		ui_component_invalidate(&self->header.component);
		ui_component_update(&self->header.component);
		ui_component_sethorizontalscrollposition(&self->grid.component, 0);
		invalidate = 0;
	}
	return invalidate;
}

int trackerview_scrollright(TrackerView* self)
{
	int invalidate = 1;
	int visitracks;
	int tracks;
		
	visitracks = ui_component_size(&self->component).width /
		self->metrics.trackwidth;
	tracks = self->grid.cursor.track + 2;
	if (visitracks - self->grid.dx / self->metrics.trackwidth < tracks) {
		self->grid.dx = -(tracks - visitracks) * self->metrics.trackwidth;
		self->header.dx = self->grid.dx;
		self->configheader.dx = self->grid.dx;
		ui_component_invalidate(&self->configheader.component);
		ui_component_update(&self->configheader.component);
		ui_component_invalidate(&self->header.component);		
		ui_component_update(&self->header.component);
		ui_component_invalidate(&self->grid.component);		
		ui_component_update(&self->grid.component);
		ui_component_sethorizontalscrollposition(&self->grid.component, 
			tracks - visitracks);
		invalidate = 0;
	}			
	return invalidate;
}

void trackerview_onkeydown(TrackerView* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{		
	int cmd;

	cmd = inputs_cmd(&self->inputs, encodeinput(ev->keycode, 
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0));
	if (cmd == CMD_NAVUP) {
		trackerview_prevline(self);
	} else
	if (cmd == CMD_NAVPAGEUP) {		
		trackerview_prevlines(self, 
			player_lpb(&self->workspace->player), 0);
	}
	if (cmd == CMD_NAVDOWN) {		
		trackerview_advanceline(self);
	} else
	if (cmd == CMD_NAVPAGEDOWN) {
		trackerview_advancelines(self, 
			player_lpb(&self->workspace->player), 0);
	} else
	if (cmd == CMD_NAVLEFT) {
		trackergrid_prevcol(&self->grid);
	} else
	if (cmd == CMD_NAVRIGHT) {
		trackergrid_nextcol(&self->grid);
	} else
	if (cmd == CMD_NAVTOP) {
		trackerview_home(self);
	} else
	if (cmd == CMD_NAVBOTTOM) {
		trackerview_end(self);
	} else
	if (cmd == CMD_COLUMNPREV) {
		trackerview_prevtrack(self);
	} else
	if (cmd == CMD_COLUMNNEXT) {
		trackerview_nexttrack(self);
	} else
	if (cmd == CMD_BLOCKSTART) {
		trackerview_blockstart(self);
	} else	
	if (cmd == CMD_BLOCKEND) {
		trackerview_blockend(self);
	} else
	if (cmd == CMD_BLOCKUNMARK) {
		trackerview_blockunmark(self);
	} else
	if (cmd == CMD_BLOCKCUT) {
		trackerview_onblockcut(self);
	} else
	if (cmd == CMD_BLOCKCOPY) {
		trackerview_onblockcopy(self);
	} else
	if (cmd == CMD_BLOCKPASTE) {
		trackerview_onblockpaste(self);
	} else
	if (cmd == CMD_BLOCKMIX) {
		trackerview_onblockmixpaste(self);
	} else
	if (cmd == CMD_BLOCKMIX) {
		trackerview_onblockmixpaste(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKINC) {
		trackerview_onblocktransposeup(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKDEC) {
		trackerview_onblocktransposedown(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKINC12) {
		trackerview_onblocktransposeup12(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKDEC12) {
		trackerview_onblocktransposedown12(self);
	} else
	if (cmd == CMD_ROWDELETE) {
		if (self->grid.cursor.offset - self->grid.bpl >= 0) {
			PatternNode* prev;
			PatternNode* p;
			PatternNode* q;
			PatternNode* node;	

			trackerview_prevline(self);		
			node = pattern_findnode(self->grid.pattern, self->grid.cursor.track,
				(psy_dsp_beat_t)self->grid.cursor.offset, (psy_dsp_beat_t)self->grid.bpl, &prev);		
			if (node) {			
				pattern_remove(self->grid.pattern, node);
				ui_component_invalidate(&self->linenumbers.component);
			}
			if (prev) {
				p = prev->next;
			} else {
				p = self->grid.pattern->events;
			}
			for (; p != 0; p = q) {
				psy_audio_PatternEntry* entry;

				q = p->next;
				entry = (psy_audio_PatternEntry*) p->entry;								
				if (entry->track == self->grid.cursor.track) {
					psy_audio_PatternEvent event;
					psy_dsp_beat_t offset;
					uintptr_t track;
					PatternNode* node;
					PatternNode* prev;

					event = *patternentry_front(entry);
					offset = entry->offset;
					track = entry->track;
					pattern_remove(self->grid.pattern, p);
					offset -= (psy_dsp_beat_t) self->grid.bpl;
					node = pattern_findnode(self->grid.pattern, track,
						offset,						
						(psy_dsp_beat_t)self->grid.bpl,
						&prev);
					if (node) {
						psy_audio_PatternEntry* entry;

						entry = (psy_audio_PatternEntry*) node->entry;
						*patternentry_front(entry) = event;
					} else {
						pattern_insert(self->grid.pattern, prev, track, 
							(psy_dsp_beat_t)offset, &event);
					}					
				}
			}
			ui_component_invalidate(&self->component);
		}
	} else
	if (cmd == CMD_ROWCLEAR) {
		if (self->grid.cursor.column == TRACKER_COLUMN_NOTE) {
			PatternNode* prev;
			PatternNode* node;
			
			node = pattern_findnode(self->grid.pattern,
				self->grid.cursor.track,
				(psy_dsp_beat_t)self->grid.cursor.offset,
				(psy_dsp_beat_t)self->grid.bpl, &prev);
			if (node) {
				pattern_remove(self->grid.pattern, node);
				ui_component_invalidate(&self->linenumbers.component);
			}
			trackerview_advanceline(self);	
		} else {
			//TrackDef* trackdef;

			//trackerview_inputvalue(self, 
			//	self->coldefs[self->grid.cursor.column].emptyvalue, 0);
			return;
		}
	} else
	if (cmd == CMD_SELECTALL) {	
		trackerview_selectall(self);
	} else
	if (cmd == CMD_SELECTCOL) {
		trackerview_selectcol(self);
	} else
	if (cmd == CMD_SELECTBAR) {
		trackerview_selectbar(self);
	} else
	if (cmd == CMD_UNDO) {
		workspace_undo(self->workspace);
	} else
	if (cmd == CMD_REDO) {
		workspace_redo(self->workspace);
	} else
	if (cmd == CMD_FOLLOWSONG) {
		self->workspace->followsong = !self->workspace->followsong;
	} else {
		if (self->grid.cursor.column != TRACKER_COLUMN_NOTE) {			
			int digit = keycodetoint(ev->keycode);
			if (digit != -1) {
				trackergrid_inputvalue(&self->grid, digit, 1);
				return;
			}			
		}				
		{	
			EventDriver* kbd;
			EventDriverCmd cmd;
			EventDriverData input;

			cmd.id = -1;			
			kbd = workspace_kbddriver(self->workspace);			
			input.message = EVENTDRIVER_KEYDOWN;
			input.param1 = encodeinput(ev->keycode, 0, /* GetKeyState(VK_SHIFT) < 0 */
				GetKeyState(VK_CONTROL) < 0);
			kbd->cmd(kbd, input, &cmd);
			if (cmd.id == NOTECOMMANDS_RELEASE) {
				trackergrid_inputnote(&self->grid, NOTECOMMANDS_RELEASE,
					self->workspace->chordmode);
				return;
			}
			if (cmd.id != -1 && cmd.id <= NOTECOMMANDS_RELEASE &&
					ev->shift && !self->workspace->chordmode
					&& ev->keycode != VK_SHIFT) {
				self->workspace->chordmode = 1;
				self->grid.chordbegin = self->grid.cursor.track;
			}
		}
	}		
	ui_component_propagateevent(sender);
}

void trackerview_onkeyup(TrackerView* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	if (self->workspace->chordmode && ev->keycode == VK_SHIFT) {
		self->grid.cursor.track = self->grid.chordbegin;
		trackerview_scrollleft(self);
		trackerview_advanceline(self);
		self->workspace->chordmode = 0;
	} else {
		ui_component_propagateevent(sender);
	}
}

void trackerview_oninput(TrackerView* self, psy_audio_Player* sender,
	psy_audio_PatternEvent* event)
{
	if (ui_component_hasfocus(&self->grid.component) &&
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
	psy_audio_PatternEvent* event)
{
	PatternNode* node;
	PatternNode* prev;
				
	node = pattern_findnode(patterndefaults,
		self->grid.cursor.track,
		0,
		(psy_dsp_beat_t)self->grid.bpl,
		&prev);
	if (node) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*) node->entry;
		if (event->inst != NOTECOMMANDS_INST_EMPTY) {
			event->inst = patternentry_front(entry)->inst;
		}
		if (event->mach != NOTECOMMANDS_MACH_EMPTY) {
			event->mach = patternentry_front(entry)->mach;
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
				
	patternevent_init(&event,
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
		trackerview_invalidatecursor(self->view, &self->cursor);
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

void trackerview_invalidatecursor(TrackerView* self, const PatternEditPosition* cursor)
{
	int line;		
	ui_rectangle r;		

	line = trackerview_offsettoscreenline(self, cursor->offset);		
	ui_setrectangle(&r,
		trackerview_track_x(self, cursor->track) + self->grid.dx,
		self->metrics.lineheight * line + self->grid.dy,		
		trackerview_trackwidth(self, cursor->track),
		self->metrics.lineheight);
	ui_component_invalidaterect(&self->grid.component, &r);
	trackerlinenumbers_invalidatecursor(&self->linenumbers, cursor);	
}

void trackerview_invalidateline(TrackerView* self, psy_dsp_beat_t offset)
{
	int line;	
	ui_rectangle r;	
	
	if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset + self->grid.pattern->length) {
		line = (int) ((offset - self->sequenceentryoffset) 
			/ self->grid.bpl);
		ui_setrectangle(&r,
			self->grid.dx,
			self->metrics.lineheight * line + self->grid.dy,
			ui_component_size(&self->component).width - self->grid.dx,
			self->metrics.lineheight);
		ui_component_invalidaterect(&self->grid.component, &r);
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

	if (stepx != 0) {
		self->view->configheader.dx = self->dx;
		self->view->header.dx = self->dx;		
		ui_component_invalidate(&self->view->configheader.component);
		ui_component_invalidate(&self->view->header.component);		
	}
	if (stepy != 0) {
		self->view->linenumbers.dy = self->dy;
		ui_component_invalidate(&self->view->linenumbers.component);		
		ui_component_update(&self->view->linenumbers.component);
	}
	if (self->midline) {
		ui_size size;
		int halfvisilines;	
		int restoremidline;		
		ui_rectangle r;		
			
		size = ui_component_size(&self->component);		
		halfvisilines = self->view->metrics.visilines / 2;
		ui_setrectangle(&r, 0, halfvisilines * self->view->metrics.lineheight, size.width,
			2 * self->view->metrics.lineheight);
		restoremidline = self->midline;
		self->midline = 0;
		ui_component_invalidaterect(&self->component, &r);
		self->midline = restoremidline;
	}
}

void trackergrid_onmousedown(TrackerGrid* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (ev->button == 2) {
		trackerview_toggleblockmenu(self->view);
	} else
	if (self->pattern) {
		if (ev->button == 1) {			
			PatternEditPosition oldcursor;

			oldcursor = self->cursor;
			self->cursor = trackergrid_makecursor(self, ev->x, ev->y);			
			self->selection.topleft = self->cursor;
			workspace_setpatterneditposition(self->view->workspace, self->cursor);			
			if (self->hasselection) {				
				self->hasselection = 0;
				ui_component_invalidate(&self->component);
			}
			self->hasselection = 0;
			trackerview_invalidatecursor(self->view, &oldcursor);
			trackerview_invalidatecursor(self->view, &self->cursor);			
			ui_component_setfocus(&self->component);
			ui_component_capture(&self->component);
		}		
	}
}

void trackergrid_onmousemove(TrackerGrid* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
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

			ui_component_invalidate(&self->component);		
		}*/
	}
}

void trackergrid_onmousedoubleclick(TrackerGrid* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	trackerview_selectcol(self->view);
}

PatternEditPosition trackergrid_makecursor(TrackerGrid* self, int x, int y)
{
	TrackDef* trackdef;

	PatternEditPosition rv;
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
	
void trackergrid_onmouseup(TrackerGrid* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	ui_component_releasecapture(&self->component);
}

// trackerview
void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	self->workspace = workspace;
	self->opcount = 0;
	self->cursorstep = 1;
	self->syncpattern = 1;
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	self->wraparound = 1;
	trackerview_initcolumns(self);
	psy_table_init(&self->trackconfigs);
	trackerview_initmetrics(self);
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);	
	trackerview_initinputs(self);	
	self->grid.pattern = 0;
	psy_ui_bitmap_init(&self->skin.bitmap);
	psy_ui_bitmap_loadresource(&self->skin.bitmap, IDB_HEADERSKIN);
	trackerview_initdefaultskin(self);	
	trackerconfigheader_init(&self->configheader, &self->component, self);
	trackerheader_init(&self->header, &self->component, self);
	self->header.numtracks = player_numsongtracks(&workspace->player);	
	self->configheader.numtracks = player_numsongtracks(&workspace->player);	
	self->linenumbers.skin = &self->skin;
	// pattern insert defaults
	trackergrid_init(&self->griddefaults, &self->component, self, &workspace->player);
	self->griddefaults.editmode = TRACKERGRID_EDITMODE_LOCAL;
	self->griddefaults.pattern = &self->workspace->player.patterndefaults;
	// pattern grid
	trackergrid_init(&self->grid, &self->component, self, &workspace->player);
	trackerview_computemetrics(self);
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component, self);	
	trackerlinenumbers_init(&self->linenumbers, &self->component, self);
	trackerview_initfont(self);	
	self->header.skin = &self->skin;
	self->configheader.skin = &self->skin;
	self->showlinenumbers = 1;
	self->showlinenumbercursor = 1;
	self->showlinenumbersinhex = 1;
	self->showemptydata = 0;
	psy_signal_connect(&self->component.signal_destroy, self,
		trackerview_ondestroy);
	psy_signal_connect(&self->component.signal_align, self,
		trackerview_onalign);
	psy_signal_connect(&self->component.signal_timer, self,
		trackerview_ontimer);
	psy_signal_connect(&self->component.signal_keydown, self,
		trackerview_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self,
		trackerview_onkeyup);
	patternblockmenu_init(&self->blockmenu, &self->component);
	psy_signal_connect(&self->blockmenu.changegenerator.signal_clicked, self,
		trackerview_onchangegenerator);
	psy_signal_connect(&self->blockmenu.changeinstrument.signal_clicked, self,
		trackerview_onchangeinstrument);
	psy_signal_connect(&self->blockmenu.cut.signal_clicked, self,
		trackerview_onblockcut);
	psy_signal_connect(&self->blockmenu.copy.signal_clicked, self,
		trackerview_onblockcopy);
	psy_signal_connect(&self->blockmenu.paste.signal_clicked, self,
		trackerview_onblockpaste);
	psy_signal_connect(&self->blockmenu.mixpaste.signal_clicked, self,
		trackerview_onblockmixpaste);
	psy_signal_connect(&self->blockmenu.del.signal_clicked, self,
		trackerview_onblockdelete);
	psy_signal_connect(&self->blockmenu.blocktransposeup.signal_clicked, self,
		trackerview_onblocktransposeup);
	psy_signal_connect(&self->blockmenu.blocktransposedown.signal_clicked,
		self, trackerview_onblocktransposedown);
	psy_signal_connect(&self->blockmenu.blocktransposeup12.signal_clicked,
		self, trackerview_onblocktransposeup12);
	psy_signal_connect(&self->blockmenu.blocktransposedown12.signal_clicked,
		self, trackerview_onblocktransposedown12);
	ui_component_hide(&self->blockmenu.component);
	TrackerViewApplyProperties(self, 0);		
	trackergrid_adjustscroll(&self->grid);
	ui_component_starttimer(&self->component, TIMERID_TRACKERVIEW, 50);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		trackerview_onlpbchanged);
	psy_signal_connect(&workspace->signal_configchanged, self,
		trackerview_onconfigchanged);
	psy_signal_connect(&workspace->player.signal_inputevent, self,
		trackerview_oninput);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		trackerview_onpatterneditpositionchanged);
	psy_signal_connect(&workspace->signal_parametertweak, self,
		trackerview_onparametertweak);
	trackerview_readconfig(self);	
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		trackerview_onskinchanged);
}

void trackerview_initfont(TrackerView* self)
{
	ui_fontinfo fontinfo;
	ui_font font;

	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&font, &fontinfo);		
	trackerview_setfont(self, &font);
	ui_font_dispose(&font);	
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

void trackerview_onalign(TrackerView* self, psy_ui_Component* sender)
{
	ui_size size;
	ui_size menusize;	
	int headerheight = 30;
	int configheaderheight = 15;
	int griddefaultheight = 15;
	int linenumberwidth = self->showlinenumbers ? 		
		self->metrics.textwidth * 
		(workspace_showbeatoffset(self->workspace) ? 8 : 4)	:0;
	size = ui_component_size(&self->component);
	menusize.width = self->blockmenu.component.visible
		? ui_component_preferredsize(&self->blockmenu.component, &size).width
		: 0;	
	menusize.height = size.height;	
	ui_component_setposition(&self->blockmenu.component,
		size.width - menusize.width,
		0, menusize.width,
		size.height);
	// header
	ui_component_setposition(&self->header.component,
		linenumberwidth, 0,
		size.width - linenumberwidth - menusize.width,
		headerheight);
	// configheader
	ui_component_setposition(&self->configheader.component,
		linenumberwidth, headerheight,
		size.width - linenumberwidth - menusize.width,
		configheaderheight);
	// grid defaults
	ui_component_setposition(&self->griddefaults.component,
		linenumberwidth, headerheight + configheaderheight,
		size.width - linenumberwidth - menusize.width,
		griddefaultheight);
	// grid
	ui_component_setposition(&self->grid.component,
		linenumberwidth,
		configheaderheight + headerheight + griddefaultheight,
		size.width - linenumberwidth - menusize.width,
		size.height - headerheight -  configheaderheight - griddefaultheight);	
	// line numbers
	ui_component_setposition(&self->linenumbers.component,
		0, headerheight + configheaderheight + griddefaultheight,
		linenumberwidth,
		size.height - headerheight);
	// line number label
	ui_component_resize(&self->linenumberslabel.component,
		linenumberwidth, headerheight + configheaderheight +
		griddefaultheight);
	trackerview_computemetrics(self);
	trackergrid_adjustscroll(&self->grid);
}

// trackerconfigheader
void trackerconfigheader_init(TrackerConfigHeader* self,
	psy_ui_Component* parent, TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	// ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	psy_signal_connect(&self->component.signal_draw, self,
		trackerconfigheader_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		trackerconfigheader_onmousedown);
	self->dx = 0;
	self->numtracks = 16;	
	self->classic = 1;
}

void trackerconfigheader_ondraw(TrackerConfigHeader* self,
	psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	ui_size size;
	ui_rectangle r;
	int cpx = self->dx;
	uintptr_t track;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);		
	ui_drawsolidrectangle(g, r, self->skin->background);
			
	for (track = 0; track < self->numtracks; ++track) {		
		trackerconfigheader_drawtrack(self, g, cpx);
		cpx += trackerview_trackwidth(self->view, track);			
	}		
}

void trackerconfigheader_drawtrack(TrackerConfigHeader* self,
	psy_ui_Graphics* g, int x)
{
	uintptr_t column;
	ui_rectangle r;
	TrackerColumnFlags columnflags = { 0, 0, 0, 0, 0, 0 };
	TrackDef* trackdef;	
	int colx;

	trackdef = &self->view->defaulttrackdef;
	colx = 0;
	for (column = 0; column < trackdef_numcolumns(trackdef); ++column) {		
		char text[10];
		TrackColumnDef* columndef;

		columndef = trackdef_columndef(trackdef, column);		
		ui_setrectangle(&r, x + colx, 0, columndef->numchars *
			self->view->metrics.textwidth, self->view->metrics.tm.tmHeight);		
		if (column == 4) {
			psy_snprintf(text, 10, "-");
		} else 
		if (column == 5) {
			psy_snprintf(text, 10, "+");
		} else {
			psy_snprintf(text, 10, "");
		}		
		setcolumncolor(&self->view->skin, g, columnflags);
		ui_textoutrectangle(g, x + colx + 1, 0, ETO_OPAQUE | ETO_CLIPPED,
			r, text, strlen(text));
		colx += trackdef_columnwidth(trackdef, column,
			self->view->metrics.textwidth);
	}
}

void trackerconfigheader_onmousedown(TrackerConfigHeader* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (self->view->workspace->song) {		
		uintptr_t track;
		TrackDef* trackdef;
		int offset;

		track = trackerview_screentotrack(self->view, ev->x - self->dx);
		trackdef = trackerview_trackdef(self->view, track);
		offset = ev->x - self->dx - trackerview_track_x(self->view, track);
		offset -= self->view->metrics.textwidth * 8;
		if (offset > self->view->metrics.textwidth * 2) {
			if (trackdef != &self->view->defaulttrackdef) {
				trackdef->numfx++;
			} else {
				trackdef = malloc(sizeof(TrackDef));
				trackdef_init(trackdef);
				trackdef->numfx = 2;
				psy_table_insert(&self->view->trackconfigs, track, trackdef);				
			}
		} else {		
			if (trackdef != &self->view->defaulttrackdef) {
				trackdef->numfx--;
				if (trackdef->numfx == 1) {
					free(trackdef);
					psy_table_remove(&self->view->trackconfigs, track);
				}
			}
		}
		ui_component_invalidate(&self->view->component);
	}
}

// trackerheader
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_draw, self,
		trackerheader_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		trackerheader_onmousedown);
	self->dx = 0;
	self->numtracks = 16;	
	self->classic = 1;
}

void trackerheader_ondraw(TrackerHeader* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	ui_size size;
	ui_rectangle r;
	int cpx = self->dx;
	uintptr_t track;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);		
	ui_drawsolidrectangle(g, r, self->skin->background);
	
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
		ui_rectangle r;
		uintptr_t track;

		track = (ev->x - self->dx) / self->view->metrics.trackwidth;
		ui_setrectangle(&r,
			self->skin->headercoords.mute.destx + track *
				self->view->metrics.trackwidth,
			self->skin->headercoords.mute.desty,
			self->skin->headercoords.mute.destwidth,
			self->skin->headercoords.mute.destheight);
		if (ui_rectangle_intersect(&r, ev->x - self->dx, ev->y)) {
			if (patterns_istrackmuted(&self->view->workspace->song->patterns,
					track)) {
				patterns_unmutetrack(&self->view->workspace->song->patterns,
					track);
			} else {
				patterns_mutetrack(&self->view->workspace->song->patterns,
					track);
			}
			ui_component_invalidate(&self->component);
		}
		ui_setrectangle(&r,
			self->skin->headercoords.solo.destx + track *
				self->view->metrics.trackwidth,
			self->skin->headercoords.solo.desty,
			self->skin->headercoords.solo.destwidth,
			self->skin->headercoords.solo.destheight);

		if (ui_rectangle_intersect(&r, ev->x - self->dx, ev->y)) {
			if (patterns_istracksoloed(&self->view->workspace->song->patterns,
					track)) {
				patterns_deactivatesolotrack(
					&self->view->workspace->song->patterns);
			} else {
				patterns_activatesolotrack(
					&self->view->workspace->song->patterns, track);
			}
			ui_component_invalidate(&self->component);
		}
	}
}


void trackerlinenumbers_init(TrackerLineNumbers* self, psy_ui_Component* parent,
	TrackerView* view)
{		
	self->view = view;
	self->dy = 0;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_setbackgroundcolor(&self->component, 
		self->view->skin.background);	
	psy_signal_connect(&self->component.signal_draw, self,
		trackerlinenumbers_ondraw);	
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	if (self->view->grid.pattern) {
		ui_size size;
		char buffer[20];		
		int cpy = self->dy;
		int line;		
		double offset;	
		TrackerGridBlock clip;

		size = ui_component_size(&self->component);
		trackergrid_clipblock(&self->view->grid, &g->clip, &clip);		
		cpy = (clip.topleft.line) *
			self->view->metrics.lineheight + self->dy;
		offset = clip.topleft.offset;				
		line = clip.topleft.line;
		while (offset <= clip.bottomright.offset &&
			   offset < self->view->grid.pattern->length) {
			ui_rectangle r;			
			TrackerColumnFlags columnflags;			
			int ystart;
			int drawbeat;
			
			drawbeat = workspace_showbeatoffset(self->view->workspace);
			columnflags.playbar = trackergrid_testplaybar(&self->view->grid,
				offset);
			columnflags.mid = 0;
			columnflags.cursor = trackergrid_testcursor(&self->view->grid, 
				self->view->grid.cursor.track, offset);
			columnflags.beat = fmod(offset, 1.0f) == 0.0f;
			columnflags.beat4 = fmod(offset, 4.0f) == 0.0f;			
			columnflags.selection = trackergrid_testselection(
				&self->view->grid, 0, offset) && self->view->grid.hasselection;
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
			ui_setrectangle(&r, 0, cpy, size.width - 2,
				self->view->metrics.tm.tmHeight);
			ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer,
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
	ui_rectangle r;		

	line = trackerview_offsettoscreenline(self->view, cursor->offset);	
	ui_setrectangle(&r,
		0,
		self->view->metrics.lineheight * line + self->view->grid.dy,
		ui_component_size(&self->component).width,
		self->view->metrics.lineheight);
	ui_component_invalidaterect(&self->component, &r);	
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, psy_dsp_beat_t offset)
{
	int line;	
	ui_rectangle r;	
	
	if (offset >= self->view->sequenceentryoffset &&
			offset < self->view->sequenceentryoffset + self->view->grid.pattern->length) {
		line = (int) ((offset - self->view->sequenceentryoffset) 
			/ self->view->grid.bpl);	
		ui_setrectangle(&r,
			self->view->grid.dx,
			self->view->metrics.lineheight * line + self->view->grid.dy,
			ui_component_size(&self->component).width - self->view->grid.dx,
			self->view->metrics.lineheight);
		ui_component_invalidaterect(&self->component, &r);
	}
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);
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
	ui_component_invalidate(&self->view->header.component);
}

void OnLineNumbersLabelDraw(TrackerLineNumbersLabel* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	ui_size size;
	ui_rectangle r;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);	
	ui_setbackgroundcolor(g, self->view->skin.background);
	ui_settextcolor(g, self->view->skin.font);	
	ui_textoutrectangle(g, r.left, r.top, 0, r, "Line", strlen("Line"));
}

void trackerview_ontimer(TrackerView* self, psy_ui_Component* sender, int timerid)
{
	if (timerid == TIMERID_TRACKERVIEW && self->grid.pattern) {		
		if (player_playing(self->grid.player)) {
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
			ui_component_invalidate(&self->grid.component);
			ui_component_invalidate(&self->linenumbers.component);
		}
		self->opcount = self->grid.pattern ? self->grid.pattern->opcount : 0;		
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
	if (strcmp(psy_properties_key(property), "wraparound") == 0) {
		self->wraparound = psy_properties_value(property);
		ui_component_invalidate(&self->component);
	} else
	if (strcmp(psy_properties_key(property), "beatoffset") == 0) {
		ui_component_align(&self->component);
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
	if (strcmp(psy_properties_key(property), "fontsize") == 0) {
		ui_fontinfo fontinfo;
		ui_font font;

		ui_fontinfo_init(&fontinfo, "Tahoma", psy_properties_value(property));
		ui_font_init(&font, &fontinfo);		
		trackerview_setfont(self, &font);
		ui_font_dispose(&font);
	}
}

void trackerview_readconfig(TrackerView* self)
{
	psy_Properties* pv;
	
	pv = psy_properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {		
		trackerview_showlinenumbers(self, psy_properties_bool(pv, "linenumbers", 1));
		trackerview_showlinenumbercursor(self, psy_properties_bool(pv, "linenumberscursor", 1));
		trackerview_showlinenumbersinhex(self, psy_properties_bool(pv, "linenumbersinhex", 1));
		self->wraparound = psy_properties_bool(pv, "wraparound", 1);
		trackerview_showemptydata(self, psy_properties_bool(pv, "drawemptydata", 1));
		trackerview_setcentermode(self, psy_properties_bool(pv, "centercursoronscreen", 1));
		{
			ui_fontinfo fontinfo;
			ui_font font;

			ui_fontinfo_init(&fontinfo, "Tahoma", 
				psy_properties_int(pv, "fontsize", 80));
			ui_font_init(&font, &fontinfo);		
			trackerview_setfont(self, &font);
			ui_font_dispose(&font);
		}
		trackerview_initcolumns(self);
		trackerview_computemetrics(self);
	}
}

void trackerview_showlinenumbers(TrackerView* self, int showstate)
{
	self->showlinenumbers = showstate;
	if (self->showlinenumbers != 0) {		
		ui_component_show(&self->linenumbers.component);
		ui_component_show(&self->linenumberslabel.component);
	} else {
		ui_component_hide(&self->linenumbers.component);
		ui_component_hide(&self->linenumberslabel.component);
	}
	ui_component_align(&self->component);
	ui_component_invalidate(&self->component);
}

void trackerview_showlinenumbercursor(TrackerView* self, int showstate)
{
	self->showlinenumbercursor = showstate;	
	ui_component_invalidate(&self->component);
}

void trackerview_showlinenumbersinhex(TrackerView* self, int showstate)
{
	self->showlinenumbersinhex = showstate;	
	ui_component_invalidate(&self->component);
}

void trackerview_showemptydata(TrackerView* self, int showstate)
{
	self->showemptydata = showstate;	
	ui_component_invalidate(&self->component);
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
		ui_component_setverticalscrollposition(&self->grid.component, 0);
	}
}

void trackerview_initinputs(TrackerView* self)
{
	inputs_init(&self->inputs);	
	inputs_define(&self->inputs, encodeinput(VK_UP, 0, 0), CMD_NAVUP);
	inputs_define(&self->inputs, encodeinput(VK_DOWN, 0, 0),CMD_NAVDOWN);
	inputs_define(&self->inputs, encodeinput(VK_LEFT, 0, 0),CMD_NAVLEFT);
	inputs_define(&self->inputs, encodeinput(VK_RIGHT, 0, 0),CMD_NAVRIGHT);
	inputs_define(&self->inputs, encodeinput(VK_PRIOR, 0, 0),CMD_NAVPAGEUP);
	inputs_define(&self->inputs, encodeinput(VK_NEXT, 0, 0),CMD_NAVPAGEDOWN);
	inputs_define(&self->inputs, encodeinput(VK_HOME, 0, 0), CMD_NAVTOP);
	inputs_define(&self->inputs, encodeinput(VK_END, 0, 0), CMD_NAVBOTTOM);	
	inputs_define(&self->inputs, encodeinput(VK_TAB, 1, 0), CMD_COLUMNPREV);
	inputs_define(&self->inputs, encodeinput(VK_TAB, 0, 0), CMD_COLUMNNEXT);	
		
	inputs_define(&self->inputs, encodeinput(VK_INSERT, 0, 0), CMD_ROWINSERT);
	inputs_define(&self->inputs, encodeinput(VK_BACK, 0, 0), CMD_ROWDELETE);
	inputs_define(&self->inputs, encodeinput(VK_DELETE, 0, 0), CMD_ROWCLEAR);

	inputs_define(&self->inputs, encodeinput('B', 0, 1), CMD_BLOCKSTART);
	inputs_define(&self->inputs, encodeinput('E', 0, 1), CMD_BLOCKEND);
	inputs_define(&self->inputs, encodeinput('U', 0, 1), CMD_BLOCKUNMARK);
	inputs_define(&self->inputs, encodeinput('X', 0, 1), CMD_BLOCKCUT);
	inputs_define(&self->inputs, encodeinput('C', 0, 1), CMD_BLOCKCOPY);
	inputs_define(&self->inputs, encodeinput('V', 0, 1), CMD_BLOCKPASTE);
	inputs_define(&self->inputs, encodeinput('M', 0, 1), CMD_BLOCKMIX);
	
	inputs_define(&self->inputs, encodeinput(VK_F12, 0, 1), CMD_TRANSPOSEBLOCKINC);
	inputs_define(&self->inputs, encodeinput(VK_F11, 0, 1), CMD_TRANSPOSEBLOCKDEC);
	inputs_define(&self->inputs, encodeinput(VK_F12, 1, 1), CMD_TRANSPOSEBLOCKINC12);
	inputs_define(&self->inputs, encodeinput(VK_F11, 1, 1), CMD_TRANSPOSEBLOCKDEC12);
	
	inputs_define(&self->inputs, encodeinput('A', 0, 1), CMD_SELECTALL);
	inputs_define(&self->inputs, encodeinput('R', 0, 1), CMD_SELECTCOL);
	inputs_define(&self->inputs, encodeinput('K', 0, 1), CMD_SELECTBAR);	
		
	inputs_define(&self->inputs, encodeinput('Z', 0, 1), CMD_UNDO);
	inputs_define(&self->inputs, encodeinput('Z', 1, 1), CMD_REDO);
	inputs_define(&self->inputs, encodeinput('F', 0, 1), CMD_FOLLOWSONG);
}

void trackerview_setpattern(TrackerView* self, psy_audio_Pattern* pattern)
{	
	self->grid.pattern = pattern;
	if (pattern) {
		self->opcount = pattern->opcount;
	}	
	self->grid.cursor.offset = 0;	
	self->grid.dy = 0;	
	self->linenumbers.dy = 0;
	trackergrid_adjustscroll(&self->grid);
	ui_component_invalidate(&self->linenumbers.component);
	ui_component_invalidate(&self->header.component);	
}

void trackerview_onchangegenerator(TrackerView* self)
{
	if (self->grid.pattern && self->workspace->song) {
			pattern_changemachine(self->grid.pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright,
			self->workspace->song->machines.slot);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_onchangeinstrument(TrackerView* self)
{
	if (self->grid.pattern && self->workspace->song) {
			pattern_changeinstrument(self->grid.pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright,
			self->workspace->song->instruments.slot);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_onblockcut(TrackerView* self)
{
	if (self->grid.hasselection) {

		trackerview_onblockcopy(self);
		trackerview_onblockdelete(self);
	}
}

void trackerview_onblockcopy(TrackerView* self)
{
	if (self->grid.hasselection) {
		PatternNode* begin;		
		PatternNode* p;
		PatternNode* q;
		PatternNode* prev = 0;
		psy_dsp_beat_t offset;
		int trackoffset;

		begin = pattern_greaterequal(self->grid.pattern, 
			(psy_dsp_beat_t) self->grid.selection.topleft.offset);
		offset = (psy_dsp_beat_t) self->grid.selection.topleft.offset;
		trackoffset = self->grid.selection.topleft.track;
		pattern_dispose(&self->workspace->patternpaste);
		pattern_init(&self->workspace->patternpaste);
		p = begin;
		while (p != 0) {			
			psy_audio_PatternEntry* entry;
			q = p->next;

			entry = (psy_audio_PatternEntry*) p->entry;
			if (entry->offset < self->grid.selection.bottomright.offset) {
				if (entry->track >= self->grid.selection.topleft.track &&
						entry->track < self->grid.selection.bottomright.track) {						
					prev = pattern_insert(&self->workspace->patternpaste,
						prev, entry->track - trackoffset, 
						entry->offset - offset,
						patternentry_front(entry));
				}
			} else {
				break;
			}
			p = q;
		}
		pattern_setmaxsongtracks(&self->workspace->patternpaste, 
			self->grid.selection.bottomright.track -
			self->grid.selection.topleft.track);
		pattern_setlength(&self->workspace->patternpaste,
			(psy_dsp_beat_t)(self->grid.selection.bottomright.offset -
			self->grid.selection.topleft.offset));

	}
	ui_component_invalidate(&self->component);
}

void trackerview_onblockpaste(TrackerView* self)
{
	PatternNode* p;
	PatternNode* prev = 0;
	psy_dsp_beat_t offset;
	int trackoffset;
	PatternEditPosition begin;
	PatternEditPosition end;

	offset = (psy_dsp_beat_t) self->grid.cursor.offset;
	trackoffset = self->grid.cursor.track;
	p = self->workspace->patternpaste.events;	

	begin = end = self->grid.cursor;
	end.track += self->workspace->patternpaste.maxsongtracks;
	end.offset += self->workspace->patternpaste.length;
	pattern_blockremove(self->grid.pattern, begin, end);
	while (p != 0) {			
		psy_audio_PatternEntry* pasteentry;
		PatternNode* node;

		pasteentry = (psy_audio_PatternEntry*) p->entry;
		node = pattern_findnode(self->grid.pattern, 
			pasteentry->track + trackoffset,
			pasteentry->offset + offset,			
			(psy_dsp_beat_t) self->grid.bpl,
			&prev);
		if (node) {
			psy_audio_PatternEntry* entry;

			entry = (psy_audio_PatternEntry*) node->entry;
			*patternentry_front(entry) = *patternentry_front(pasteentry);
		} else {
			pattern_insert(self->grid.pattern,
					prev, 
					pasteentry->track + trackoffset, 
					pasteentry->offset + offset, 
					patternentry_front(pasteentry));
		}						
		p = p->next;
	}	
	ui_component_invalidate(&self->component);
}

void trackerview_onblockmixpaste(TrackerView* self)
{
	PatternNode* p;
	PatternNode* prev = 0;
	psy_dsp_beat_t offset;
	int trackoffset;

	offset = (psy_dsp_beat_t) self->grid.cursor.offset;
	trackoffset = self->grid.cursor.track;
	p = self->workspace->patternpaste.events;
	while (p != 0) {			
		psy_audio_PatternEntry* pasteentry;		

		pasteentry = (psy_audio_PatternEntry*) p->entry;
		if (!pattern_findnode(self->grid.pattern, pasteentry->track + trackoffset,
				pasteentry->offset + offset, (psy_dsp_beat_t) self->grid.bpl,
				&prev)) {
			pattern_insert(self->grid.pattern,
					prev, 
					pasteentry->track + trackoffset, 
					pasteentry->offset + offset, 
					patternentry_front(pasteentry));
		}						
		p = p->next;
	}	
	ui_component_invalidate(&self->component);
}

void trackerview_onblockdelete(TrackerView* self)
{
	if (self->grid.hasselection) {
		pattern_blockremove(self->grid.pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright);
		ui_component_invalidate(&self->component);
	}	
}

void trackerview_blockstart(TrackerView* self)
{
	self->grid.selection.topleft = self->grid.cursor;
	self->grid.selection.bottomright = self->grid.cursor;
	++self->grid.selection.bottomright.track;
	self->grid.selection.bottomright.offset += self->grid.bpl;
	self->grid.hasselection = 1;
	ui_component_invalidate(&self->component);
}

void trackerview_blockend(TrackerView* self)
{
	self->grid.selection.bottomright = self->grid.cursor;
	++self->grid.selection.bottomright.track;
	self->grid.selection.bottomright.offset += self->grid.bpl;
	ui_component_invalidate(&self->component);
}

void trackerview_blockunmark(TrackerView* self)
{
	self->grid.hasselection = 0;
	ui_component_invalidate(&self->component);
}

void trackerview_onblocktransposeup(TrackerView* self)
{
	if (self->grid.hasselection) {		
		undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->grid.pattern,
				self->grid.selection,
				self->grid.cursor, +1, self->workspace)->command);
	}
}

void trackerview_onblocktransposedown(TrackerView* self)
{
	if (self->grid.hasselection) {		
		undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->grid.pattern,
				self->grid.selection,
				self->grid.cursor, -1, self->workspace)->command);
	}
}

void trackerview_onblocktransposeup12(TrackerView* self)
{
	if (self->grid.hasselection) {		
		undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->grid.pattern,
				self->grid.selection,
				self->grid.cursor, 12, self->workspace)->command);
	}
}

void trackerview_onblocktransposedown12(TrackerView* self)
{
	if (self->grid.hasselection) {		
		undoredo_execute(&self->workspace->undoredo,
			&BlockTransposeCommandAlloc(self->grid.pattern,
				self->grid.selection,
				self->grid.cursor, -12, self->workspace)->command);
	}
}

void trackerview_showblockmenu(TrackerView* self)
{
	ui_component_show(&self->blockmenu.component);
	ui_component_align(&self->component);
	ui_component_invalidate(&self->linenumbers.component);
}

void trackerview_hideblockmenu(TrackerView* self)
{
	ui_component_hide(&self->blockmenu.component);
	ui_component_align(&self->component);
	ui_component_invalidate(&self->linenumbers.component);
}

void trackerview_toggleblockmenu(TrackerView* self)
{
	if (self->blockmenu.component.visible) {
		trackerview_hideblockmenu(self);
	} else {		
		trackerview_showblockmenu(self);
	}
}

void patternblockmenu_init(PatternBlockMenu* self, psy_ui_Component* parent)
{
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
		ui_button_init(&self->cut, &self->component);
	ui_button_settext(&self->cut, "Cut");
	ui_button_init(&self->copy, &self->component);	
	ui_button_settext(&self->copy, "Copy");
	ui_button_init(&self->paste, &self->component);	
	ui_button_settext(&self->paste, "Paste");
	ui_button_init(&self->mixpaste, &self->component);
	ui_button_settext(&self->mixpaste, "MixPaste");
	ui_button_init(&self->del, &self->component);
	ui_button_settext(&self->del, "Delete");

	ui_button_init(&self->changegenerator, &self->component);
	ui_button_settext(&self->changegenerator, "Change Generator");
	ui_button_init(&self->changeinstrument, &self->component);
	ui_button_settext(&self->changeinstrument, "Change psy_audio_Instrument");

	ui_button_init(&self->blocktransposeup, &self->component);
	ui_button_settext(&self->blocktransposeup, "Transpose +1");
	ui_button_init(&self->blocktransposedown, &self->component);
	ui_button_settext(&self->blocktransposedown, "Transpose -1");
	ui_button_init(&self->blocktransposeup12, &self->component);
	ui_button_settext(&self->blocktransposeup12, "Transpose +12");	
	ui_button_init(&self->blocktransposedown12, &self->component);
	ui_button_settext(&self->blocktransposedown12, "Transpose -12");
	psy_list_free(ui_components_setalign(ui_component_children(&self->component, 0), 
		UI_ALIGN_TOP, 0));	
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

void trackerview_onskinchanged(TrackerView* self, Workspace* sender,
	psy_Properties* properties)
{
	TrackerViewApplyProperties(self, properties);
}

void trackerview_onparametertweak(TrackerView* self, Workspace* sender,
	int slot, int tweak, int value)
{
	if (workspace_recordingtweaks(sender)) {		
		psy_audio_PatternEvent event;				
		
		patternevent_init(&event,
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
	self->metrics.trackwidth = 120;
	self->metrics.patterntrackident = 0;
	self->metrics.headertrackident = 0;
	self->metrics.visitracks = 4;
	self->metrics.visilines = 25;
}

void trackerview_computemetrics(TrackerView* self)
{
	ui_size gridsize;	

	gridsize = ui_component_size(&self->grid.component);	
	self->metrics.tm = ui_component_textmetric(&self->component);	
	self->metrics.textwidth = (int)(self->metrics.tm.tmAveCharWidth * 1.5) + 2;
	self->metrics.textleftedge = 2;
	trackergrid_computecolumns(&self->grid, self->metrics.textwidth);	
	self->metrics.lineheight = self->metrics.tm.tmHeight + 1;
	self->metrics.trackwidth = max(
		trackergrid_preferredtrackwidth(&self->grid),
		trackerheader_preferredtrackwidth(&self->header));	
	self->metrics.patterntrackident =
		(self->metrics.trackwidth -
			trackergrid_preferredtrackwidth(&self->grid)) / 2;
	if (self->metrics.patterntrackident < 0) {
		self->metrics.patterntrackident = 0;
	}
	self->metrics.trackwidth += 1;
	self->metrics.headertrackident = 0;
	self->metrics.visitracks = gridsize.width / self->metrics.trackwidth;
	self->metrics.visilines = gridsize.height / self->metrics.lineheight;
}

void trackerview_setfont(TrackerView* self, ui_font* font)
{	
	ui_component_setfont(&self->component, font);
	ui_component_setfont(&self->griddefaults.component, font);
	ui_component_setfont(&self->grid.component, font);
	ui_component_setfont(&self->linenumbers.component, font);
	trackerview_computemetrics(self);	
		self->grid.component.scrollstepx = self->metrics.trackwidth;
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

int trackerview_screentotrack(TrackerView* self, int x)
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
	ui_rectangle r;	
	
	unsigned int column;
	int cpx;
	TrackDef* trackdef;
	psy_audio_PatternEvent* event;
	TrackerColumnFlags currcolumnflags;	
	currcolumnflags = columnflags;
	event = patternentry_front(entry);		
	trackdef = trackerview_trackdef(self->view, entry->track);
	currcolumnflags.cursor = columnflags.cursor && self->cursor.column == 0;
	setcolumncolor(&self->view->skin, g, currcolumnflags);
	cpx = 0;
	// draw note	
	ui_setrectangle(&r, x + cpx, y, 
		self->view->metrics.textwidth * 3,
		self->view->metrics.tm.tmHeight);
	notestr = (event->note != 255 || !self->view->showemptydata) 
		  ? psy_dsp_notetostr(event->note, self->notestabmode)
		  : emptynotestr;		
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, r,
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
			currcolumnflags.cursor =
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
	TrackColumnDef* def;

	def = trackdef_columndef(self, column);
	return def ? def->numdigits : 0;	
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
	uintptr_t rv;
	TrackColumnDef* columndef;

	columndef = trackdef_columndef(self, column);
	if (columndef) {
		rv = columndef->emptyvalue;
	} else {
		rv = 0;
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
	int rv;
	TrackColumnDef* columndef;

	columndef = trackdef_columndef(self, column);
	if (columndef) {
		rv = columndef->numchars * textwidth + columndef->marginright;
	} else {
		rv = 0;
	}
	return rv;
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
