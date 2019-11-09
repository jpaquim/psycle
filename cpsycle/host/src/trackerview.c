// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackerview.h"
#include <pattern.h>
#include "cmdsnotes.h"
#include <string.h>
#include <math.h>
#include "resources/resource.h"
#include "skingraphics.h"

#define TIMERID_TRACKERVIEW 600
static const big_beat_t epsilon = 0.0001;

static void trackergrid_ondraw(TrackerGrid*, ui_component* sender, ui_graphics*);
static void trackergrid_drawbackground(TrackerGrid*, ui_graphics*, TrackerGridBlock* clip);
static void trackergrid_drawtrackbackground(TrackerGrid*, ui_graphics*, int track);
static void trackergrid_drawevents(TrackerGrid*, ui_graphics*,
	TrackerGridBlock* clip);
static void trackergrid_drawevent(TrackerGrid*, ui_graphics* g, PatternEvent*,
	int x, int y, int playbar, int cursor, int beat, int beat4);
static void trackergrid_onkeydown(TrackerGrid*, ui_component* sender,
	int keycode, int keydata);
static void trackergrid_onmousedown(TrackerGrid*, ui_component* sender,
	int x, int y, int button);
static void trackergrid_onsize(TrackerGrid*, ui_component* sender, ui_size* size);
static void trackergrid_onscroll(TrackerGrid*, ui_component* sender,
	int cx, int cy);
static int trackergrid_testcursor(TrackerGrid*, unsigned int track,
	big_beat_t offset, unsigned int subline);
static int trackergrid_testplaybar(TrackerGrid* self, big_beat_t offset);
static void trackergrid_clipblock(TrackerGrid*, const ui_rectangle*,
	TrackerGridBlock*);
static void trackergrid_drawdigit(TrackerGrid*, ui_graphics*, int digit,
	int col, int x, int y);
static void trackergrid_adjustscroll(TrackerGrid*);
static double trackergrid_offset(TrackerGrid*, int y, unsigned int* lines,
	unsigned int* sublines, unsigned int* subline);
static void trackergrid_numtrackschanged(TrackerGrid*, Player*,
	unsigned int numtracks);
static void trackerview_onsize(TrackerView*, ui_component* sender, ui_size*);
static void trackerview_ondestroy(TrackerView*, ui_component* sender);
static void trackerview_onkeydown(TrackerView*, ui_component* sender,
	int keycode, int keydata);
static void trackerview_ontimer(TrackerView*, ui_component* sender, 
	int timerid);
static void trackerview_align(TrackerView*, ui_component* sender);
static void trackerview_inputnote(TrackerView*, note_t);
static void trackerview_inputdigit(TrackerView*, int value);
static void enterdigitcolumn(PatternEvent*, int column, int value);
static void enterdigit(int digit, int newval, unsigned char* val);
static int chartoint(char c);
static void trackerview_prevcol(TrackerView*);
static void trackerview_nextcol(TrackerView*);
static void trackerview_prevline(TrackerView*);
static void trackerview_advanceline(TrackerView*);
static void trackerview_prevtrack(TrackerView*);
static void trackerview_nexttrack(TrackerView*);
static void trackerview_enablesync(TrackerView*);
static void trackerview_preventsync(TrackerView*);
static int trackerview_scrolleft(TrackerView*);
static int trackerview_scrollright(TrackerView*);
static void trackerview_showlinenumbers(TrackerView*, int showstate);
static void trackerview_showemptydata(TrackerView*, int showstate);
static int trackerview_numlines(TrackerView*);
static void trackerview_setclassicheadercoords(TrackerView*);
static void trackerview_setheadercoords(TrackerView*);
static void trackerview_onconfigchanged(TrackerView*, Workspace*, Properties*);
static void trackerview_readconfig(TrackerView*);
static void trackerview_oninput(TrackerView*, Player*, PatternEvent*);
static void trackerview_initinputs(TrackerView*);
static void trackerview_invalidatecursor(TrackerView*, const TrackerCursor*);
static void trackerview_invalidateline(TrackerView*, beat_t offset);
static void trackerview_initdefaultskin(TrackerView*);
static int trackerview_offsettoscreenline(TrackerView*, big_beat_t);

static void trackerheader_ondraw(TrackerHeader*, ui_component* sender, ui_graphics* g);

static void OnLineNumbersLabelDraw(TrackerLineNumbersLabel*, ui_component* sender, ui_graphics*);
static void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel*, ui_component* sender);

static void trackerlinenumbers_ondraw(TrackerLineNumbers*,
	ui_component* sender, ui_graphics*);

static int testrange(big_beat_t position, big_beat_t offset, big_beat_t width);
static int testrange_e(big_beat_t position, big_beat_t offset, big_beat_t width);
static int colgroupstart(int col);

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
};

enum {
	TRACKER_COLUMN_NOTE	 = 0,
	TRACKER_COLUMN_INST	 = 1,
	TRACKER_COLUMN_MACH	 = 3,
	TRACKER_COLUMN_CMD	 = 5,
	TRACKER_COLUMN_PARAM = 7,
	TRACKER_COLUMN_END	 = 9
};

/// Commands
typedef struct {
	Command command;
	TrackerCursor cursor;
	Pattern* pattern;
	double bpl;
	PatternNode* node;	
	PatternEvent event;
	PatternEvent oldevent;
	int insert;
} InsertCommand;

static void InsertCommandDispose(InsertCommand*);
static void InsertCommandExecute(InsertCommand*);
static void InsertCommandRevert(InsertCommand*);

InsertCommand* InsertCommandAlloc(Pattern* pattern, double bpl,
	TrackerCursor cursor, PatternEvent event)
{
	InsertCommand* rv;
	
	rv = malloc(sizeof(InsertCommand));
	rv->command.dispose = InsertCommandDispose;
	rv->command.execute = InsertCommandExecute;
	rv->command.revert = InsertCommandRevert;
	rv->cursor = cursor;	
	rv->bpl = bpl;
	rv->event = event;
	rv->node = 0;
	rv->insert = 0;
	rv->pattern = pattern;	
	return rv;
}

void InsertCommandDispose(InsertCommand* self) { }

void InsertCommandExecute(InsertCommand* self)
{		
	PatternNode* prev;
	self->node = 
		pattern_findnode(self->pattern,
			self->cursor.track,
			(beat_t)self->cursor.offset,
			self->cursor.subline,
			(beat_t)self->bpl, &prev);	
	if (self->node) {
		self->oldevent = pattern_event(self->pattern, self->node);
		pattern_setevent(self->pattern, self->node, &self->event);
		self->insert = 0;
	} else {
		self->node = pattern_insert(self->pattern,
			prev,
			self->cursor.track, 
			(beat_t)self->cursor.offset,
			&self->event);
		self->insert = 1;
	}
}

void InsertCommandRevert(InsertCommand* self)
{		
	if (self->insert) {
		pattern_remove(self->pattern, self->node);
	} else {
		pattern_setevent(self->pattern, self->node, &self->oldevent);		
	}
}

/// TrackerGrid
void InitTrackerGrid(TrackerGrid* self, ui_component* parent, TrackerView* view, Player* player)
{		
	self->view = view;
	self->header = 0;	
	signal_connect(&player->signal_numsongtrackschanged, self,
		trackergrid_numtrackschanged);
	self->view->font = ui_createfont("Tahoma", 12);	
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_size, self, trackergrid_onsize);
	signal_connect(&self->component.signal_keydown,self, 
		trackergrid_onkeydown);
	signal_connect(&self->component.signal_mousedown, self,
		trackergrid_onmousedown);
	signal_connect(&self->component.signal_draw, self,
		trackergrid_ondraw);
	signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);	
	self->player = player;	
	self->numtracks = player_numsongtracks(player);
	self->lpb = player_lpb(self->player);
	self->bpl = 1.0 / player_lpb(self->player);
	self->notestabmode = NOTESTAB_DEFAULT;	
	self->cursor.track = 0;
	self->cursor.offset = 0;
	self->cursor.subline = 0;
	self->cursor.col = 0;
	workspace_seteditposition(self->view->workspace, self->cursor);
	self->cursorstep = 0.25;
	self->dx = 0;
	self->dy = 0;

	self->textheight = 12;
	self->textwidth = 9;
	self->textleftedge = 2;
	self->colx[0] = 0;
	self->colx[1] = (self->textwidth*3)+2;
	self->colx[2] = self->colx[1]+self->textwidth;
	self->colx[3] = self->colx[2]+self->textwidth + 1;
	self->colx[4] = self->colx[3]+self->textwidth;
	self->colx[5] = self->colx[4]+self->textwidth + 1;
	self->colx[6] = self->colx[5]+self->textwidth;
	self->colx[7] = self->colx[6]+self->textwidth;
	self->colx[8] = self->colx[7]+self->textwidth;
	self->colx[9] = self->colx[8]+self->textwidth + 1;
	self->trackwidth = self->colx[9];		
	
	self->lineheight = self->textheight + 1;
	self->component.scrollstepx = self->trackwidth;
	self->component.scrollstepy = self->lineheight;
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);
	trackergrid_adjustscroll(self);
	self->component.doublebuffered = 1;		
}

void trackergrid_numtrackschanged(TrackerGrid* self, Player* player,
	unsigned int numsongtracks)
{	
	self->numtracks = numsongtracks;	
	self->view->header.numtracks = numsongtracks;
	trackergrid_adjustscroll(self);
	ui_invalidate(&self->view->component);
}

void TrackerViewApplyProperties(TrackerView* self, Properties* p)
{
	self->skin.separator = properties_int(p, "pvc_separator", 0x00292929);
	self->skin.separator2 = properties_int(p, "pvc_separator2", 0x00292929);
	self->skin.background = properties_int(p, "pvc_background", 0x00292929);
	self->skin.background2 = properties_int(p, "pvc_background2", 0x00292929);
	self->skin.row4beat = properties_int(p, "pvc_row4beat", 0x00595959);
	self->skin.row4beat2 = properties_int(p, "pvc_row4beat2", 0x00595959);
	self->skin.rowbeat = properties_int(p, "pvc_rowbeat", 0x00363636);
	self->skin.rowbeat2 = properties_int(p, "pvc_rowbeat2", 0x00363636);
	self->skin.row = properties_int(p, "pvc_row", 0x003E3E3E);
	self->skin.row2 = properties_int(p, "pvc_row2", 0x003E3E3E);
	self->skin.font = properties_int(p, "pvc_font", 0x00CACACA);
	self->skin.font2 = properties_int(p, "pvc_font2", 0x00CACACA );
	self->skin.fontPlay = properties_int(p, "pvc_fontplay", 0x00FFFFFF);
	self->skin.fontCur2 = properties_int(p, "pvc_fontcur2", 0x00FFFFFF);
	self->skin.fontSel = properties_int(p, "pvc_fontsel", 0x00FFFFFF);
	self->skin.fontSel2 = properties_int(p, "pvc_fontsel2", 0x00FFFFFF);
	self->skin.selection = properties_int(p, "pvc_selection", 0x009B7800);
	self->skin.selection2 = properties_int(p, "pvc_selection2", 0x009B7800);
	self->skin.playbar = properties_int(p, "pvc_playbar", 0x009F7B00);
	self->skin.playbar2 = properties_int(p, "pvc_playbar2", 0x009F7B00);
	self->skin.cursor = properties_int(p, "pvc_cursor", 0x009F7B00);
	self->skin.cursor2 = properties_int(p, "pvc_cursor2", 0x009F7B00);
	ui_component_setbackgroundcolor(
		&self->linenumbers.component, self->skin.background);	
	ui_invalidate(&self->component);
}

void trackergrid_ondraw(TrackerGrid* self, ui_component* sender, ui_graphics* g)
{	 
  	TrackerGridBlock clip;
	if (self->view->pattern) {
		self->bpl = 1.0 / player_lpb(self->player);
		ui_setfont(g, &self->view->font);		
		trackergrid_clipblock(self, &g->clip, &clip);
		trackergrid_drawbackground(self, g, &clip);
		trackergrid_drawevents(self, g, &clip);
	} else {
		ui_drawsolidrectangle(g, g->clip, self->view->skin.background);	
	}
}

big_beat_t trackergrid_offset(TrackerGrid* self, int y, unsigned int* lines,
	unsigned int* sublines, unsigned int* subline)
{
	double offset = 0;	
	int cpy = 0;		
	int first = 1;
	unsigned int count = y / self->lineheight;
	unsigned int remaininglines = 0;	

	if (self->view->pattern) {
		PatternNode* curr = self->view->pattern->events;
		*lines = 0;
		*sublines = 0;	
		*subline = 0;
		while (curr) {		
			PatternEntry* entry;		
			first = 1;
			do {
				entry = (PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->bpl))
				{
					if (*lines + *sublines >= count) {
						break;
					}
					if (entry->track == 0 && !first) {
						++(*sublines);
						++*subline;
					}							
					first = 0;
					curr = curr->next;
				} else {
					*subline = 0;
					break;
				}
				if (*lines + *sublines >= count) {
					break;
				}
			} while (curr);
			if ((*lines + *sublines >= count)) {
				break;
			}
			++(*lines);
			*subline = 0;		
			offset += self->bpl;
		}	
		remaininglines =  (count - (*lines + *sublines));
		*lines += remaininglines;
	}
	return offset + remaininglines * self->bpl;
}

void trackergrid_clipblock(TrackerGrid* self, const ui_rectangle* clip,
	TrackerGridBlock* block)
{	
	int lines;
	int sublines;
	int subline;
	block->topleft.track = (clip->left - self->dx) / self->trackwidth;
	block->topleft.col = 0;
	block->topleft.offset =  trackergrid_offset(self, clip->top - self->dy,
		&lines, &sublines, &subline);
	block->topleft.line = lines;
	block->topleft.subline = subline;
	block->topleft.totallines = lines + sublines;
	block->bottomright.track = (clip->right - self->dx + self->trackwidth) / self->trackwidth;
	if (block->bottomright.track > self->numtracks) {
		block->bottomright.track = self->numtracks;
	}
	block->bottomright.col = 0;
	block->bottomright.offset = trackergrid_offset(self, clip->bottom - self->dy,
		&lines, &sublines, &subline);
	block->bottomright.line = lines;
	block->bottomright.totallines = lines + sublines;
	block->bottomright.subline = subline;
}

void trackergrid_drawbackground(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{
	ui_rectangle r;
	unsigned int track;

	for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
		trackergrid_drawtrackbackground(self, g, track);
	}
	ui_setrectangle(&r, self->numtracks * self->trackwidth + self->dx, 0, 
		self->cx - (self->numtracks * self->trackwidth + self->dx), self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);
}

void trackergrid_drawtrackbackground(TrackerGrid* self, ui_graphics* g, int track)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, track * self->trackwidth + self->dx, 0,
		self->trackwidth, self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

int trackergrid_testcursor(TrackerGrid* self, unsigned int track, double offset,
	unsigned int subline)
{
	return self->cursor.track == track && 
		testrange(self->cursor.offset, offset, self->bpl) &&
		self->cursor.subline == subline;
}

int trackergrid_testplaybar(TrackerGrid* self, big_beat_t offset)
{
	return player_playing(self->player) && 
		testrange(self->view->lastplayposition -
				      self->view->sequenceentryoffset,
				  offset, self->bpl); 	
}

void trackergrid_drawevents(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{	
	unsigned int track;
	int cpx = 0;	
	int cpy;
	double offset;	
	int subline;	
	int line = 0;
	PatternNode* node;
	
	cpy = (clip->topleft.totallines - clip->topleft.subline) * self->lineheight + self->dy;	
	offset = clip->topleft.offset;	
	node = pattern_greaterequal(self->view->pattern, (beat_t)offset);
	subline = 0;	
	while (offset <= clip->bottomright.offset && offset < self->view->pattern->length) {	
		int beat;
		int beat4;
		int fill;
		
		beat = fabs(fmod(offset, 1.0f)) < 0.01f ;
		beat4 = fabs(fmod(offset, 4.0f)) < 0.01f;
		do {
			fill = 0;
			cpx = clip->topleft.track * self->trackwidth + self->dx;						
			for (track =  clip->topleft.track; track < clip->bottomright.track;
					++track) {
				int hasevent = 0;
				int cursor;
				int playbar;

				cursor = trackergrid_testcursor(self, track, offset, subline);
				playbar = trackergrid_testplaybar(self, offset);
				while (!fill && node &&
					   ((PatternEntry*)(node->entry))->track <= track &&
					   testrange_e(((PatternEntry*)(node->entry))->offset,
						  offset,
						  self->bpl)) {
					PatternEntry* entry;
										
					entry = (PatternEntry*)(node->entry);					
					if (entry->track == track) {
						trackergrid_drawevent(self, g, &entry->event, 
							cpx, cpy, playbar, cursor, beat, beat4);						
						node = node->next;
						hasevent = 1;
						break;
					}
					node = node->next;
				}
				if (!hasevent) {
					PatternEvent event;
					memset(&event, 0xFF, sizeof(PatternEvent));
					event.cmd = 0;
					event.parameter = 0;
					trackergrid_drawevent(self, g, &event, cpx, cpy, playbar, cursor, beat, beat4);
				} else
				if (node && ((PatternEntry*)(node->entry))->track <= track) {
					fill = 1;
				}
				cpx += self->trackwidth;			
			}
			// skip remaining tracks
			while (node && ((PatternEntry*)(node->entry))->track > 0 &&
					testrange_e(((PatternEntry*)(node->entry))->offset,
						offset, self->bpl)) {
				node = node->next;
			}
			cpy += self->lineheight;
			++line;
			++subline;
		} while (node &&
			((PatternEntry*)(node->entry))->offset + 2*epsilon < offset + self->bpl);
		offset += self->bpl;
		subline = 0;
	}
}

int testrange(big_beat_t position, big_beat_t offset, big_beat_t width)
{
	return position >= offset && position < offset + width; 
}

int testrange_e(big_beat_t position, big_beat_t offset, big_beat_t width)
{
	return position + 2*epsilon >= offset &&
		position < offset + width - epsilon;
}

void SetColColor(TrackerSkin* skin, ui_graphics* g, int col, int playbar, int cursor, int beat, int beat4)
{
	if (cursor != 0) {
		if (beat4) {
			ui_setbackgroundcolor(g, skin->cursor);
			ui_settextcolor(g, skin->fontCur);			
		} else
		if (beat) {
			ui_setbackgroundcolor(g, skin->cursor);
			ui_settextcolor(g, skin->fontCur);			
		} else {
			ui_setbackgroundcolor(g, skin->cursor);
			ui_settextcolor(g, skin->fontCur);
		}
	} else 
	if (playbar) {
		ui_setbackgroundcolor(g, skin->playbar);
		ui_settextcolor(g, skin->fontPlay);		
	} else	{
		if (beat4) {
			ui_setbackgroundcolor(g, skin->row4beat);			
			ui_settextcolor(g, skin->font);
		} else
		if (beat) {
			ui_setbackgroundcolor(g, skin->rowbeat);
			ui_settextcolor(g, skin->font);
		} else {
			ui_setbackgroundcolor(g, skin->row);
			ui_settextcolor(g, skin->font);
		}
	}	
}

void trackergrid_drawevent(TrackerGrid* self, ui_graphics* g, PatternEvent* event, int x, int y, int playbar, int cursor, int beat, int beat4)
{					
	ui_rectangle r;
	static const char* emptynotestr = "- - -";
	const char* notestr;
		
	SetColColor(&self->view->skin, g, 0, playbar, cursor && self->cursor.col == 0, beat, beat4);
	{	// draw note
		ui_setrectangle(&r, x + self->colx[0], y, self->textwidth*3, self->textheight);
		notestr = (event->note != 255 || !self->view->showemptydata) 
			  ? notetostr(event->note, self->notestabmode)
			  : emptynotestr;		
		ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, notestr,
		strlen(notestr));	
	}
	{	// draw inst
		int hi = (event->inst & 0xF0) >> 4;
		int lo = event->inst & 0x0F;
		if (event->inst == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 1, playbar, cursor && (self->cursor.col == 1), beat, beat4);
		trackergrid_drawdigit(self, g, hi, 1, x, y);
		SetColColor(&self->view->skin, g, 2, playbar, cursor && (self->cursor.col == 2), beat, beat4);
		trackergrid_drawdigit(self, g, lo, 2, x, y);
	}
	{	// draw mach
		int hi = (event->mach & 0xF0) >> 4;
		int lo = event->mach & 0x0F;
		if (event->mach == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 3, playbar, cursor && (self->cursor.col == 3), beat, beat4);
		trackergrid_drawdigit(self, g, hi, 3, x, y);
		SetColColor(&self->view->skin, g, 4, playbar, cursor && (self->cursor.col == 4), beat, beat4);
		trackergrid_drawdigit(self, g, lo, 4, x, y);
	}
	{	// draw cmd
		int hi = (event->cmd & 0xF0) >> 4;
		int lo = event->cmd & 0x0F;				
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 5, playbar, cursor && (self->cursor.col == 5), beat, beat4);
		trackergrid_drawdigit(self, g, hi, 5, x, y);
		SetColColor(&self->view->skin, g, 6, playbar, cursor && (self->cursor.col == 6), beat, beat4);
		trackergrid_drawdigit(self, g, lo, 6, x, y);
	}
	{	// draw parameter
		int hi = (event->parameter & 0xF0) >> 4;
		int lo = event->parameter & 0x0F;		
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 7, playbar, cursor && (self->cursor.col == 7), beat, beat4);
		trackergrid_drawdigit(self, g, hi, 7, x, y);
		SetColColor(&self->view->skin, g, 8, playbar, cursor && (self->cursor.col == 8), beat, beat4);
		trackergrid_drawdigit(self, g, lo, 8, x, y);
	}		
}

void trackergrid_drawdigit(TrackerGrid* self, ui_graphics* g, int digit, int col, int x, int y)
{
	char buffer[20];	
	ui_rectangle r;
	ui_setrectangle(&r, x + self->colx[col], y, self->textwidth, self->textheight);
	if (digit != -1) {
		_snprintf(buffer, 2, "%X", digit);	
	} else {
		if (self->view->showemptydata) {
			_snprintf(buffer, 2, "%s", ".");
		} else {
			_snprintf(buffer, 2, "%s", "");	
		}
	}
	ui_textoutrectangle(g, r.left + self->textleftedge, r.top,
		ETO_OPAQUE | ETO_CLIPPED, r, buffer, strlen(buffer));	
}

void trackergrid_onsize(TrackerGrid* self, ui_component* sender, ui_size* size)
{
	self->cx = size->width;
	self->cy = size->height;
	trackergrid_adjustscroll(self);
}

void trackergrid_adjustscroll(TrackerGrid* self)
{
	ui_size size;
	int visitracks;
	int visilines;

	size = ui_component_size(&self->component);	
	visitracks = size.width / self->trackwidth;
	visilines = size.height / self->lineheight;
	ui_component_sethorizontalscrollrange(&self->component, 0,
		self->numtracks - visitracks);
	ui_component_setverticalscrollrange(&self->component, 0,
		trackerview_numlines(self->view) - visilines);	
}

unsigned int NumSublines(Pattern* pattern, double offset, double bpl)
{
	PatternNode* node = pattern_greaterequal(pattern, (beat_t)offset);	
	unsigned int currsubline = 0;
	int first = 1;

	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
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

void trackergrid_onkeydown(TrackerGrid* self, ui_component* sender, int keycode, int keydata)
{
	sender->propagateevent = 1;	
}

void trackerview_prevcol(TrackerView* self)
{
	int invalidate = 1;
	TrackerCursor oldcursor;
	oldcursor = self->grid.cursor;

	if (self->grid.cursor.col == 0) {
		self->grid.cursor.col = TRACKERGRID_numparametercols - 2;
		if (self->grid.cursor.track > 0) {
			--self->grid.cursor.track;
			trackerview_scrolleft(self);
		} else {			
			self->grid.cursor.track = player_numsongtracks(
				&self->workspace->player) - 1;
			invalidate = trackerview_scrollright(self);
		}
	} else {
		--self->grid.cursor.col;
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);	
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

void trackerview_nextcol(TrackerView* self)
{
	int invalidate = 1;
	TrackerCursor oldcursor;
	oldcursor = self->grid.cursor;	

	if (self->grid.cursor.col == TRACKERGRID_numparametercols - 2) {
		self->grid.cursor.col = 0;
		if (self->grid.cursor.track < player_numsongtracks(
			&self->workspace->player) - 1) {
			++self->grid.cursor.track;
			invalidate = trackerview_scrollright(self);
		} else {			
			self->grid.cursor.track = 0;
			invalidate = trackerview_scrolleft(self);
		}
	} else {
		++self->grid.cursor.col;
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

void trackerview_prevline(TrackerView* self)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;
	if (self->grid.cursor.subline > 0) {
			--self->grid.cursor.subline;
	} else {
		self->grid.cursor.offset -= workspace_cursorstep(self->workspace)
			* self->grid.bpl;
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);		
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);
}

void trackerview_advanceline(TrackerView* self)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;
	if (self->grid.cursor.subline < 
		NumSublines(self->pattern, self->grid.cursor.offset, self->grid.bpl)) {
		++self->grid.cursor.subline;
	} else {
		self->grid.cursor.offset += 
			workspace_cursorstep(self->workspace) * self->grid.bpl;
		self->grid.cursor.subline = 0;
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);
}

void trackerview_prevtrack(TrackerView* self)
{
	TrackerCursor oldcursor;
	int invalidate = 1;

	oldcursor = self->grid.cursor;
	self->grid.cursor.col = 0;
	if (self->grid.cursor.track > 0) {
		--self->grid.cursor.track;
		trackerview_scrolleft(self);
	} else {
		self->grid.cursor.track = 
			player_numsongtracks(&self->workspace->player) - 1;
		invalidate = trackerview_scrollright(self);
	}
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);	
}

void trackerview_nexttrack(TrackerView* self)
{
	TrackerCursor oldcursor;
	int invalidate = 1;

	oldcursor = self->grid.cursor;
	self->grid.cursor.col = 0;
	if (self->grid.cursor.track < 
		player_numsongtracks(&self->workspace->player) - 1) {
			++self->grid.cursor.track;
		invalidate = trackerview_scrollright(self);
	} else {
		self->grid.cursor.track = 0;
		invalidate = trackerview_scrolleft(self);
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);		
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

int trackerview_scrolleft(TrackerView* self)
{	
	int tracks;
	int invalidate = 1;
	
	tracks = self->grid.cursor.track;
	if (-self->grid.dx / self->grid.trackwidth > tracks) {
		self->grid.dx = -tracks * self->grid.trackwidth;
		self->header.dx = self->grid.dx;
		ui_invalidate(&self->grid.component);
		UpdateWindow((HWND)self->grid.component.hwnd);
		ui_invalidate(&self->header.component);
		UpdateWindow((HWND)self->header.component.hwnd);
		ui_component_sethorizontalscrollposition(
			&self->grid.component, 0);		
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
		self->grid.trackwidth;
	tracks = self->grid.cursor.track + 2;
	if (visitracks - self->grid.dx / self->grid.trackwidth < tracks) {
		self->grid.dx = -(tracks - visitracks) * self->grid.trackwidth;
		self->header.dx = self->grid.dx;
		ui_invalidate(&self->header.component);
		UpdateWindow((HWND)self->header.component.hwnd);				
		ui_invalidate(&self->grid.component);	
		UpdateWindow((HWND)self->grid.component.hwnd);
		ui_component_sethorizontalscrollposition(&self->grid.component, 
			tracks - visitracks);
		invalidate = 0;
	}			
	return invalidate;
}

void trackerview_onkeydown(TrackerView* self, ui_component* sender, int keycode, int keydata)
{		
	int cmd;

	cmd = inputs_cmd(&self->inputs, encodeinput(keycode, 
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0));		
	if (cmd == CMD_NAVUP) {
		trackerview_prevline(self);
	} else
	if (cmd == CMD_NAVDOWN) {		
		trackerview_advanceline(self);
	} else
	if (cmd == CMD_NAVLEFT) {
		trackerview_prevcol(self);
	} else
	if (cmd == CMD_NAVRIGHT) {
		trackerview_nextcol(self);
	} else
	if (cmd == CMD_COLUMNPREV) {
		trackerview_prevtrack(self);
	} else
	if (cmd == CMD_COLUMNNEXT) {
		trackerview_nexttrack(self);
	} else
	if (keycode == VK_DELETE) {
		PatternNode* prev;
		PatternNode* node = pattern_findnode(self->pattern, self->grid.cursor.track,
			(beat_t)self->grid.cursor.offset, self->grid.cursor.subline, (beat_t)self->grid.bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
			ui_invalidate(&self->linenumbers.component);
		}

	} else
	if (keycode == VK_RETURN) {		
		PatternNode* prev;
		PatternNode* node = pattern_findnode(self->pattern, 0,
			(beat_t)self->grid.cursor.offset, self->grid.cursor.subline + 1, (beat_t)self->grid.bpl, &prev);		
		if (prev && ((PatternEntry*)prev->entry)->offset >= self->grid.cursor.offset) {
			PatternEvent ev = { 255, 255, 255, 0, 0 };
			double offset;
			++self->grid.cursor.subline;
			offset = self->grid.cursor.offset + self->grid.cursor.subline*self->grid.bpl/4;
			pattern_insert(self->pattern, prev, 0, (beat_t)offset, &ev);			
			trackergrid_adjustscroll(&self->grid);
			workspace_seteditposition(self->workspace, self->grid.cursor);
			ui_invalidate(&self->linenumbers.component);
		}	
	} else {
		if (self->grid.cursor.col != TRACKER_COLUMN_NOTE) {			
			trackerview_inputdigit(self, chartoint((char)keycode));
		}		
	}	
	ui_component_propagateevent(sender);
}

void trackerview_oninput(TrackerView* self, Player* sender, PatternEvent* event)
{
	if (ui_component_hasfocus(&self->grid.component) &&
			self->grid.cursor.col == TRACKER_COLUMN_NOTE) {		
		trackerview_inputnote(self, event->note);		
	}
}

void trackerview_enablesync(TrackerView* self)
{
	self->opcount = self->pattern->opcount;
	self->syncpattern = 1;
}

void trackerview_preventsync(TrackerView* self)
{
	self->opcount = self->pattern->opcount;
	self->syncpattern = 0;
}

void trackerview_inputnote(TrackerView* self, note_t note)
{
	Machine* machine;
	PatternEvent event;
				
	patternevent_init(&event,
		note,
		255,
		(unsigned char)machines_slot(&self->workspace->song->machines),
		0,
		0);
	machine = machines_at(&self->workspace->song->machines, event.mach);
	if (machine && 
			machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
		event.inst = self->workspace->song->instruments.slot;
	}
	trackerview_preventsync(self);
	undoredo_execute(&self->workspace->undoredo,
		&InsertCommandAlloc(self->pattern, self->grid.bpl,
			self->grid.cursor, event)->command);	
	trackerview_advanceline(self);
	trackerview_enablesync(self);
}

void trackerview_inputdigit(TrackerView* self, int value)
{
	if (self->pattern && value != -1) {
		PatternNode* prev;	
		PatternEvent event;
		PatternNode* node;
		
		node = pattern_findnode(self->pattern,
					self->grid.cursor.track,
					(beat_t)self->grid.cursor.offset,
					self->grid.cursor.subline,
					(beat_t)self->grid.bpl,
					&prev);						
		event = pattern_event(self->pattern, node);
		enterdigitcolumn(&event, self->grid.cursor.col, value);
		trackerview_preventsync(self);
		undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(self->pattern, self->grid.bpl,
					self->grid.cursor, event)->command);
		if (colgroupstart(self->grid.cursor.col + 1) != 
				colgroupstart(self->grid.cursor.col)) {
			self->grid.cursor.col = colgroupstart(self->grid.cursor.col);
			trackerview_advanceline(self);			
		} else {
			trackerview_nextcol(self);
		}
		trackerview_invalidatecursor(self, &self->grid.cursor);
		trackerview_enablesync(self);
	}
}

int colgroupstart(int col)
{
	if (col == 1 || col == 2) {
		return 1;
	} else
	if (col == 3 || col == 4) {
		return 3;
	} else
		if (col == 5 || col == 6 || col == 7 || col == 8) {
		return 5;
	} else {
		return 0;
	}
}

int chartoint(char c) {
	int rv = -1;

	if (c >= '0' && c <='9') {
		rv = c - '0';
	} else
	if (c >= 'A' && c <='Z') {
		rv = c - 'A' + 10;
	} else
	if (c >= 'a' && c <='z') {
		rv = c - 'a' + 10;
	}
	return rv;
}

void enterdigitcolumn(PatternEvent* event, int column, int value)
{
	switch (column) {
		case 1: 
			if ((event->inst == 0xFF) && (value != 0x0F)) {
				event->inst = 0;
			}
			enterdigit(0, value, &event->inst);
		break;
		case 2:
			if ((event->inst == 0xFF) && (value != 0x0F)) {
				event->inst = 0;
			}
			enterdigit(1, value, &event->inst);
		break;
		case 3:
			if ((event->mach == 0xFF) && (value != 0x0F)) {
				event->mach = 0;
			}
			enterdigit(0, value, &event->mach);
		break;
		case 4:
			if ((event->mach == 0xFF) && (value != 0x0F)) {
				event->mach = 0;
			}
			enterdigit(1, value, &event->mach);
		break;
		case 5:							
			enterdigit(0, value, &event->cmd);
		break;
		case 6:
			enterdigit(1, value, &event->cmd);
		break;
		case 7:
			enterdigit(0, value, &event->parameter);
		break;
		case 8:
			enterdigit(1, value, &event->parameter);
		break;
		default:
		break;
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

void trackerview_invalidatecursor(TrackerView* self, const TrackerCursor* cursor)
{
	int line;		
	ui_rectangle r;		

	line = trackerview_offsettoscreenline(self, cursor->offset)
		+ cursor->subline;	
	ui_setrectangle(&r,
		cursor->track * self->grid.trackwidth + self->grid.dx,
		self->grid.lineheight * line + self->grid.dy,		
		self->grid.trackwidth,
		self->grid.lineheight);
	ui_invalidaterect(&self->grid.component, &r);
}

void trackerview_invalidateline(TrackerView* self, beat_t offset)
{
	int line;	
	ui_rectangle r;	
	
	if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset + self->pattern->length) {
		line = (int) ((offset - self->sequenceentryoffset) 
			/ self->grid.bpl);	
		ui_setrectangle(&r,
			self->grid.dx,
			self->grid.lineheight * line + self->grid.dy,
			ui_component_size(&self->component).width - self->grid.dx,
			self->grid.lineheight);
		ui_invalidaterect(&self->grid.component, &r);
	}
}


void trackergrid_onscroll(TrackerGrid* self, ui_component* sender, int cx, int cy)
{	
	self->dx += cx;
	self->dy += cy;	

	if (self->header && cx != 0) {
		self->header->dx += cx;
		ui_invalidate(&self->header->component);
	}
	if (self->linenumbers && cy != 0) {
		self->linenumbers->dy += cy;
		ui_invalidate(&self->linenumbers->component);
		UpdateWindow((HWND)self->linenumbers->component.hwnd);
	}
}

void trackergrid_onmousedown(TrackerGrid* self, ui_component* sender, int x, int y, int button)
{
	if (self->view->pattern) {
		if (button == 1) {
			int lines;
			int sublines;
			int subline;		
			int coloffset;				
			self->cursor.offset = trackergrid_offset(self, y - self->dy, &lines, &sublines, &subline);
			self->cursor.totallines = lines + sublines;
			self->cursor.subline = subline;
			self->cursor.track = (x - self->dx) / self->trackwidth;
			coloffset = (x - self->dx) - self->cursor.track * self->trackwidth;
			if (coloffset < 3*self->textwidth) {
				self->cursor.col = 0;
			} else {
				self->cursor.col = coloffset / self->textwidth - 2;
			}
			workspace_seteditposition(self->view->workspace, self->cursor);
			ui_invalidate(&self->component);
			ui_component_setfocus(&self->component);
		}
	}
}

void trackerview_init(TrackerView* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	self->opcount = 0;
	self->cursorstep = 1;
	self->syncpattern = 1;
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	trackerview_initinputs(self);
	self->pattern = 0;
	ui_bitmap_loadresource(&self->skin.bitmap, IDB_HEADERSKIN);
	trackerview_initdefaultskin(self);	
	trackerheader_init(&self->header, &self->component);	
	self->header.numtracks = player_numsongtracks(&workspace->player);
	self->header.trackwidth = self->skin.headercoords.background.destwidth;
	self->linenumbers.skin = &self->skin;
	InitTrackerGrid(&self->grid, &self->component, self, &workspace->player);
	InitTrackerLineNumbersLabel(&self->linenumberslabel, &self->component, self);	
	trackerlinenumbers_init(&self->linenumbers, &self->component, self);	
	self->grid.header = &self->header;
	self->grid.linenumbers = &self->linenumbers;	
	self->header.skin = &self->skin;
	self->linenumbers.lineheight = 13;
	self->showlinenumbers = 1;
	self->showemptydata = 0;
	signal_connect(&self->component.signal_destroy, self, trackerview_ondestroy);
	signal_connect(&self->component.signal_size, self, trackerview_onsize);
	signal_connect(&self->component.signal_timer, self, trackerview_ontimer);
	signal_connect(&self->component.signal_keydown, self,
		trackerview_onkeydown);
	TrackerViewApplyProperties(self, 0);
	if (self->grid.trackwidth < self->header.trackwidth) {
		int i;
		int temp = (self->header.trackwidth - self->grid.trackwidth)/2;
		self->grid.trackwidth = self->header.trackwidth;
		for (i = 0; i < 10; ++i) {
			self->grid.colx[i] += temp;
		}				
	} else {
		self->header.trackwidth = self->grid.trackwidth;
	}
	self->grid.component.scrollstepx = self->grid.trackwidth;
	ui_component_starttimer(&self->component, TIMERID_TRACKERVIEW, 100);
	signal_connect(&workspace->signal_configchanged, self,
		trackerview_onconfigchanged);
	signal_connect(&workspace->player.signal_inputevent, self,
		trackerview_oninput);
	trackerview_showlinenumbers(self, workspace_showlinenumbers(workspace));
}

void trackerview_ondestroy(TrackerView* self, ui_component* sender)
{
	inputs_dispose(&self->inputs);
}

void trackerview_initdefaultskin(TrackerView* self)
{
	trackerview_setclassicheadercoords(self);
}

void trackerview_setclassicheadercoords(TrackerView* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };	
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 81, 18, 11, 11, 75, 3, 11, 11, 0 };
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

void trackerview_setheadercoords(TrackerView* self)
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

void trackerview_onsize(TrackerView* self, ui_component* sender, ui_size* size)
{
	trackerview_align(self, sender);
}

void trackerview_align(TrackerView* self, ui_component* sender)
{
	ui_size size;
	int width;
	int height;
	int headerheight = 30;
	int linenumberwidth = self->showlinenumbers ? 45 :0;

	size = ui_component_size(&self->component);
	width = size.width;
	height = size.height;
	ui_component_setposition(&self->header.component, linenumberwidth, 0,
		width - linenumberwidth, headerheight);
	ui_component_setposition(&self->grid.component, linenumberwidth, headerheight,
		width - linenumberwidth, height - headerheight);	
	ui_component_setposition(&self->linenumbers.component, 0, headerheight,
		linenumberwidth, height - headerheight);
	ui_component_resize(&self->linenumberslabel.component,
		linenumberwidth, headerheight);	
}

void trackerheader_init(TrackerHeader* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	signal_connect(&self->component.signal_draw, self, trackerheader_ondraw);
	self->dx = 0;
	self->numtracks = 16;
	self->trackwidth = 102;
	self->classic = 1;
}

void trackerheader_ondraw(TrackerHeader* self, ui_component* sender, ui_graphics* g)
{	
	ui_size size;
	ui_rectangle r;
	int cpx = self->dx;
	unsigned int track;

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
		cpx += self->trackwidth;
	}		
}

void trackerlinenumbers_init(TrackerLineNumbers* self, ui_component* parent,
	TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 
		self->view->skin.background);
	signal_connect(&self->component.signal_draw, self,
		trackerlinenumbers_ondraw);
	self->dy = 0;
	self->textheight = 12;
	self->lineheight = self->textheight + 1;
	self->component.doublebuffered = 1;
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, ui_component* sender,
	ui_graphics* g)
{	
	if (self->view->pattern) {
		ui_size size;
		char buffer[20];		
		int cpy = self->dy;
		int line;		
		double offset;	
		TrackerGridBlock clip;

		size = ui_component_size(&self->component);
		trackergrid_clipblock(&self->view->grid, &g->clip, &clip);
		ui_setfont(g, &self->view->font);				
		cpy = (clip.topleft.totallines - clip.topleft.subline) * self->lineheight + self->dy;
		offset = clip.topleft.offset;				
		line = clip.topleft.line;
		while (offset <= clip.bottomright.offset && offset < self->view->pattern->length) {
			ui_rectangle r;			
			int beat;
			int beat4;
			int subline;
			int numsublines;
			int ystart;
			beat = fmod(offset, 1.0f) == 0.0f;
			beat4 = fmod(offset, 4.0f) == 0.0f;
			SetColColor(self->skin, g, 0, 0, 0, beat, beat4);
			// %3i
			_snprintf(buffer, 10, "%.2X %.3f", line, offset);
			ui_setrectangle(&r, 0, cpy, size.width - 2, self->textheight);
			ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer,
				strlen(buffer));		
			cpy += self->lineheight;
			ystart = cpy;
			numsublines = NumSublines(self->view->pattern, offset, self->view->grid.bpl);
			for (subline = 0; subline < numsublines; ++subline) {
				_snprintf(buffer, 10, "  %.2X", subline);	
				ui_setrectangle(&r, 0, cpy, size.width - 2, self->textheight);
				ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer,
					strlen(buffer));		
				cpy += self->lineheight;
			}
			if (ystart != cpy) {
				ui_drawline(g, 1, ystart, 1, cpy - self->lineheight / 2);
				ui_drawline(g, 1, cpy - self->lineheight / 2, 4, cpy - self->lineheight / 2);
			}
			offset += self->view->grid.bpl;
			++line;
		}		
	}
}

void InitTrackerLineNumbersLabel(TrackerLineNumbersLabel* self, ui_component* parent, TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_connect(&self->component.signal_draw, self, OnLineNumbersLabelDraw);
	signal_connect(&self->component.signal_mousedown, self, OnLineNumbersLabelMouseDown);
}

void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel* self, ui_component* sender)
{
	self->view->header.classic = !self->view->header.classic;
	if (self->view->header.classic) {
		trackerview_setclassicheadercoords(self->view);
	} else {
		trackerview_setheadercoords(self->view);
	}
	ui_invalidate(&self->view->header.component);
}

void OnLineNumbersLabelDraw(TrackerLineNumbersLabel* self, ui_component* sender, ui_graphics* g)
{	
	ui_size size;
	ui_rectangle r;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);	
	ui_setbackgroundcolor(g, self->view->skin.background);
	ui_settextcolor(g, self->view->skin.font);	
	ui_textoutrectangle(g, r.left, r.top, 0, r, "Line", strlen("Line"));
}

void trackerview_ontimer(TrackerView* self, ui_component* sender, int timerid)
{
	if (timerid == TIMERID_TRACKERVIEW) {		
		if (player_playing(self->grid.player)) {
			trackerview_invalidateline(self, self->lastplayposition);
			self->lastplayposition = player_position(&self->workspace->player);			
			trackerview_invalidateline(self, self->lastplayposition);
		} else {
			if (self->lastplayposition != -1) {				
				trackerview_invalidateline(self, self->lastplayposition);
				self->lastplayposition = -1;
			}
		}
		if (self->pattern && self->pattern->opcount != self->opcount &&
				self->syncpattern) {
			ui_invalidate(&self->grid.component);			
		}
		self->opcount = self->pattern ? self->pattern->opcount : 0;		
	}
}

int trackerview_numlines(TrackerView* self)
{
	int lines = 0;
	int sublines = 0;	
	int remaininglines = 0;
	double offset = 0;

	if (!self->pattern) {
		return 0;
	} else {		
		int first = 1;		
		PatternNode* curr = self->pattern->events;
		int subline = 0;

		while (curr && offset < self->pattern->length) {		
			PatternEntry* entry;		
			first = 1;
			do {
				entry = (PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->grid.bpl))
				{					
					if (entry->track == 0 && !first) {
						++(sublines);
						++subline;
					}							
					first = 0;
					curr = curr->next;
				} else {
					subline = 0;
					break;
				}	
			} while (curr);		
			++(lines);
			subline = 0;		
			offset += self->grid.bpl;
		}
	}			
	offset = self->pattern->length - offset;
	if (offset > 0) {
		remaininglines = (int)(offset * player_lpb(self->grid.player));
	}
	return lines + sublines + remaininglines;
}

int trackerview_offsettoscreenline(TrackerView* self, big_beat_t offs)
{
	int lines = 0;
	int sublines = 0;	
	int remaininglines = 0;
	double offset = 0;
	
	if (!self->pattern) {
		return 0;
	} else {		
		int first = 1;		
		PatternNode* curr = self->pattern->events;
		int subline = 0;

		while (curr && offset < self->pattern->length) {		
			PatternEntry* entry;		
			first = 1;
			do {
				entry = (PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->grid.bpl))
				{						
					if (entry->track == 0 && !first) {
						++(sublines);
						++subline;
					}
					if (entry->offset >= offs) {
						break;
					}
					first = 0;
					curr = curr->next;
				} else {
					subline = 0;
					break;
				}	
			} while (curr);			
			if (offset >= offs) {
				subline = 0;
				break;
			}
			++(lines);
			subline = 0;			
			offset += self->grid.bpl;			
		}
	}			
	offset = offs - offset;
	if (offset > 0) {		
		remaininglines = (int)(offset * player_lpb(self->grid.player));		
	}
	return lines + sublines + remaininglines;
}

void trackerview_onconfigchanged(TrackerView* self, Workspace* workspace,
	Properties* property)
{
	if (property == workspace->config) {
		trackerview_readconfig(self);
	} else
	if (strcmp(properties_key(property), "linenumbers") == 0) {
		trackerview_showlinenumbers(self, properties_value(property));
	} else
	if (strcmp(properties_key(property), "drawemptydata") == 0) {
		trackerview_showemptydata(self, properties_value(property));
	}
}

void trackerview_readconfig(TrackerView* self)
{
	Properties* pv;
	
	pv = properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {		
		trackerview_showlinenumbers(self, properties_bool(pv, "linenumbers", 1));
		trackerview_showemptydata(self, properties_bool(pv, "drawemptydata", 1));
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
	trackerview_align(self, &self->component);
	ui_invalidate(&self->component);
}

void trackerview_showemptydata(TrackerView* self, int showstate)
{
	self->showemptydata = showstate;	
	ui_invalidate(&self->component);
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
}

void trackerview_setpattern(TrackerView* self, Pattern* pattern)
{
	self->pattern = pattern;
	if (pattern) {
		self->opcount = pattern->opcount;
	}
	self->grid.dx = 0;
	self->grid.dy = 0;
	self->header.dx = 0;
	self->linenumbers.dy = 0;
	trackergrid_adjustscroll(&self->grid);
	ui_invalidate(&self->linenumbers.component);
	ui_invalidate(&self->header.component);	
}
