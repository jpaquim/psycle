// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "trackerview.h"
#include <pattern.h>
#include "cmdsnotes.h"
#include <string.h>
#include <math.h>
#include "resources/resource.h"

#define TIMERID_TRACKERVIEW 600

static void OnDraw(TrackerGrid*, ui_component* sender, ui_graphics* g);
static void OnHeaderDraw(TrackerHeader*, ui_component* sender, ui_graphics* g);
static void DrawBackground(TrackerGrid*, ui_graphics* g, TrackerGridBlock* clip);
static void DrawTrackBackground(TrackerGrid*, ui_graphics* g, int track);
static void DrawEvents(TrackerGrid*, ui_graphics* g, TrackerGridBlock* clip);
static void DrawPatternEvent(TrackerGrid*, ui_graphics* g, PatternEvent* event, int x, int y, int playbar, int cursor, int beat, int beat4);
static void OnKeyDown(TrackerView*, ui_component* sender, int keycode, int keydata);
static void OnGridKeyDown(TrackerGrid*, ui_component* sender, int keycode, int keydata);
static void OnGridSize(TrackerGrid*, ui_component* sender, int width, int height);
static void OnViewSize(TrackerView*, ui_component* sender, int width, int height);
static void Align(TrackerView*, ui_component* sender);
static void OnScroll(TrackerGrid*, ui_component* sende, int cx, int cy);
static void SetDefaultSkin(TrackerGrid*);
static void OnMouseDown(TrackerGrid*, ui_component* sender, int x, int y, int button);
static void ClipBlock(TrackerGrid*, ui_graphics*, TrackerGridBlock* block);
static void DrawDigit(TrackerGrid*, ui_graphics*, int digit, int col, int x, int y);
static void HandlePatternEventInput(TrackerView*, unsigned int keycode,
	int shift, int ctrl);
static void InputNote(TrackerView*, note_t note);
static void InputDigit(TrackerView*, int value);
static void EnterDigitColumn(PatternEvent*, int column, int value);
static void EnterDigit(int digit, int newval, unsigned char* val);
static int chartoint(char c);
unsigned char NoteCmdToNote(TrackerView*, int cmd);
static void OnLineNumbersDraw(TrackerLineNumbers*, ui_component* sender, ui_graphics* g);
static void OnLineNumbersLabelDraw(TrackerLineNumbersLabel*, ui_component* sender, ui_graphics* g);
static void InitDefaultSkin(TrackerView*);
static void BlitSkinPart(TrackerHeader*, ui_graphics*, int x, int y, SkinCoord*);
static void LineNumbersDrawBackground(TrackerLineNumbers*, ui_graphics*);
static void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel*, ui_component* sender);
static void ShowLineNumbers(TrackerView*, int showstate);
static void ShowEmptyData(TrackerView*, int showstate);
static void OnTimer(TrackerView*, ui_component* sender, int timerid);
static int NumLines(TrackerView*);
static void AdjustScrollranges(TrackerGrid*);
static double Offset(TrackerGrid*, int y, unsigned int* lines, unsigned int* sublines, unsigned int* subline);
static int TestCursor(TrackerGrid*, unsigned int track, double offset, unsigned int subline);
static int TestRange(double position, double offset, double  width);
static void OnSongTracksNumChanged(TrackerGrid*, Player*,
	unsigned int numsongtracks);
static void SetClassicHeaderCoords(TrackerView*);
static void SetHeaderCoords(TrackerView*);
static void OnConfigChanged(TrackerView*, Workspace*, Properties*);
static void ReadConfig(TrackerView* self);
static PatternNode* FindNode(Pattern*, unsigned int track, beat_t offset,
	unsigned int subline, float bpl, PatternNode** prev);

static void InitTrackerInputs(TrackerView*);

enum {
	CMD_NAVUP,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_NAVPAGEUP,	///< pgup
	CMD_NAVPAGEDOWN,///< pgdn
	CMD_NAVTOP,		///< home
	CMD_NAVBOTTOM,	///< end
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
		FindNode(self->pattern,
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
		OnSongTracksNumChanged);
	self->view->font = ui_createfont("Tahoma", 12);	
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_size, self, OnGridSize);
	signal_connect(&self->component.signal_keydown,self, OnGridKeyDown);
	signal_connect(&self->component.signal_mousedown, self,OnMouseDown);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_scroll, self, OnScroll);	
	ui_component_move(&self->component, 0, 0);

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
	AdjustScrollranges(self);
	self->component.doublebuffered = 1;		
}

void OnSongTracksNumChanged(TrackerGrid* self, Player* player,
	unsigned int numsongtracks)
{	
	self->numtracks = numsongtracks;	
	self->view->header.numtracks = numsongtracks;
	AdjustScrollranges(self);
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
	ui_invalidate(&self->component);
}

void OnDraw(TrackerGrid* self, ui_component* sender, ui_graphics* g)
{	 
  	TrackerGridBlock clip;
	if (self->view->pattern) {
		self->bpl = 1.0 / player_lpb(self->player);
		ui_setfont(g, &self->view->font);		
		ClipBlock(self, g, &clip);
		DrawBackground(self, g, &clip);
		DrawEvents(self, g, &clip);
	} else {
		ui_drawsolidrectangle(g, g->clip, self->view->skin.background);	
	}
}

double Offset(TrackerGrid* self, int y, unsigned int* lines,
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
			if (!curr || (*lines + *sublines >= count)) {
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

void ClipBlock(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* block)
{	
	int lines;
	int sublines;
	int subline;
	block->topleft.track = (g->clip.left - self->dx) / self->trackwidth;
	block->topleft.col = 0;
	block->topleft.offset =  Offset(self, g->clip.top - self->dy, &lines, &sublines, &subline);
	block->topleft.line = lines;
	block->topleft.subline = subline;
	block->topleft.totallines = lines + sublines;
	block->bottomright.track = (g->clip.right - self->dx + self->trackwidth) / self->trackwidth;
	if (block->bottomright.track > self->numtracks) {
		block->bottomright.track = self->numtracks;
	}
	block->bottomright.col = 0;
	block->bottomright.offset = Offset(self, g->clip.bottom - self->dy, &lines, &sublines, &subline);
	block->bottomright.line = lines;
	block->bottomright.totallines = lines + sublines;
	block->bottomright.subline = subline;
}

void DrawBackground(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{
	ui_rectangle r;
	unsigned int track;
	for (track = clip->topleft.track; track < clip->bottomright.track; ++track) {
		DrawTrackBackground(self, g, track);
	}
	ui_setrectangle(&r, self->numtracks * self->trackwidth + self->dx, 0, self->cx - (self->numtracks * self->trackwidth + self->dx), self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

void DrawTrackBackground(TrackerGrid* self, ui_graphics* g, int track)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, track * self->trackwidth + self->dx, 0, self->trackwidth, self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

int TestCursor(TrackerGrid* self, unsigned int track, double offset,
	unsigned int subline)
{
	return self->cursor.track == track && 
		self->cursor.offset >= offset && self->cursor.offset < offset + self->bpl
		&& self->cursor.subline == subline;
}

void DrawEvents(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{	
	unsigned int track;
	int cpx = 0;	
	int cpy;
	double offset;
	double threshold = 0.0001;	
	int subline;	
	int line = 0;	
	
	cpy = (clip->topleft.totallines - clip->topleft.subline) * self->lineheight + self->dy;	
	offset = clip->topleft.offset;	
	self->curr_event = pattern_greaterequal(self->view->pattern, (beat_t)offset);
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
			for (track =  clip->topleft.track; track < clip->bottomright.track; ++track) {
				int hasevent = 0;
				int cursor = TestCursor(self, track, offset, subline);
				int playbar = 0;
				playbar = self->player->playing && TestRange(
								self->player->sequencer.position, offset, self->bpl)
						   ? 1 : 0;				
				while (!fill && self->curr_event &&
					((PatternEntry*)(self->curr_event->entry))->track <= track &&
					((PatternEntry*)(self->curr_event->entry))->offset + 2* threshold >= offset &&
					((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl - threshold) {
					PatternEntry* entry;
										
					entry = (PatternEntry*)(self->curr_event->entry);					
					if (((PatternEntry*)(self->curr_event->entry))->track == track) {
						DrawPatternEvent(self, g, &((PatternEntry*)(self->curr_event->entry))->event, cpx, cpy, playbar, cursor, beat, beat4);
						hasevent = 1;
						self->curr_event = self->curr_event->next;
						break;
					}
					self->curr_event = self->curr_event->next;
				}
				if (!hasevent) {
					PatternEvent event;
					memset(&event, 0xFF, sizeof(PatternEvent));
					event.cmd = 0;
					event.parameter = 0;
					DrawPatternEvent(self, g, &event, cpx, cpy, playbar, cursor, beat, beat4);
				} else
				if (self->curr_event && ((PatternEntry*)(self->curr_event->entry))->track <= track) {
					fill = 1;
				}
				cpx += self->trackwidth;			
			}
			// skip remaining tracks
			while (self->curr_event &&
				((PatternEntry*)(self->curr_event->entry))->track > 0 &&
				((PatternEntry*)(self->curr_event->entry))->offset + 2* threshold >= offset && 
				((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl - threshold) {
				self->curr_event = self->curr_event->next;
			}
			cpy += self->lineheight;
			++line;
			subline++;
		} while (self->curr_event &&
			((PatternEntry*)(self->curr_event->entry))->offset + 2*threshold < offset + self->bpl);
		offset += self->bpl;
		subline = 0;
	}
}

int TestRange(double position, double offset, double width)
{
	return position >= offset && position < offset + width; 
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

void DrawPatternEvent(TrackerGrid* self, ui_graphics* g, PatternEvent* event, int x, int y, int playbar, int cursor, int beat, int beat4)
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
		DrawDigit(self, g, hi, 1, x, y);
		SetColColor(&self->view->skin, g, 2, playbar, cursor && (self->cursor.col == 2), beat, beat4);
		DrawDigit(self, g, lo, 2, x, y);
	}
	{	// draw mach
		int hi = (event->mach & 0xF0) >> 4;
		int lo = event->mach & 0x0F;
		if (event->mach == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 3, playbar, cursor && (self->cursor.col == 3), beat, beat4);
		DrawDigit(self, g, hi, 3, x, y);
		SetColColor(&self->view->skin, g, 4, playbar, cursor && (self->cursor.col == 4), beat, beat4);
		DrawDigit(self, g, lo, 4, x, y);
	}
	{	// draw cmd
		int hi = (event->cmd & 0xF0) >> 4;
		int lo = event->cmd & 0x0F;				
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 5, playbar, cursor && (self->cursor.col == 5), beat, beat4);
		DrawDigit(self, g, hi, 5, x, y);
		SetColColor(&self->view->skin, g, 6, playbar, cursor && (self->cursor.col == 6), beat, beat4);
		DrawDigit(self, g, lo, 6, x, y);
	}
	{	// draw parameter
		int hi = (event->parameter & 0xF0) >> 4;
		int lo = event->parameter & 0x0F;		
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 7, playbar, cursor && (self->cursor.col == 7), beat, beat4);
		DrawDigit(self, g, hi, 7, x, y);
		SetColColor(&self->view->skin, g, 8, playbar, cursor && (self->cursor.col == 8), beat, beat4);
		DrawDigit(self, g, lo, 8, x, y);
	}		
}

void DrawDigit(TrackerGrid* self, ui_graphics* g, int digit, int col, int x, int y)
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

void OnGridSize(TrackerGrid* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;
	AdjustScrollranges(self);
}

void AdjustScrollranges(TrackerGrid* self)
{
	ui_size size;
	int visibletracks;
	int visiblelines;

	size = ui_component_size(&self->component);	
	visibletracks = size.width / self->trackwidth;
	visiblelines = size.height / self->lineheight;
	ui_component_sethorizontalscrollrange(&self->component, 0, self->numtracks - visibletracks);
	ui_component_setverticalscrollrange(&self->component, 0, NumLines(self->view) - visiblelines);	
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

PatternNode* FindNode(Pattern* pattern, unsigned int track, float offset, 
	unsigned int subline, beat_t bpl, PatternNode** prev)
{
	unsigned int currsubline = 0;
	int first = 1;
	PatternNode* node = pattern_greaterequal(pattern, offset);	
	if (node) {
		*prev = node->prev;
	} else {
		*prev = pattern_last(pattern);
	}	
	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			node = 0;
			break;
		}
		if (entry->track == 0 && !first) {
			++currsubline;				
		}
		if (subline < currsubline) {			
			node = 0;
			break;
		}
		if (entry->track > track && subline == currsubline) {			
			node = 0;
			break;
		}		
		if (entry->track == track && subline == currsubline) {
			break;
		}				
		*prev = node;		
		node = node->next;
		first = 0;
	}
	return node;
}

void OnGridKeyDown(TrackerGrid* self, ui_component* sender, int keycode, int keydata)
{
	sender->propagateevent = 1;	
}

void AdvanceCursor(TrackerView* self)
{
	if (self->grid.cursor.subline < 
		NumSublines(self->pattern, self->grid.cursor.offset, self->grid.bpl)) {
		++self->grid.cursor.subline;
	} else {
		self->grid.cursor.offset += self->cursorstep * self->grid.bpl;
		self->grid.cursor.subline = 0;
	}
	workspace_seteditposition(self->workspace, self->grid.cursor);
}

void OnKeyDown(TrackerView* self, ui_component* sender, int keycode, int keydata)
{		
	int cmd;

	cmd = inputs_cmd(&self->inputs, encodeinput(keycode, 0, 0));
		
	if (cmd == CMD_NAVUP) {
		if (self->grid.cursor.subline > 0) {
			--self->grid.cursor.subline;
		} else {
			self->grid.cursor.offset -= self->grid.bpl;
		}
		workspace_seteditposition(self->workspace, self->grid.cursor);
		ui_invalidate(&self->component);
	} else
	if (cmd == CMD_NAVDOWN) {		
		AdvanceCursor(self);
		workspace_seteditposition(self->workspace, self->grid.cursor);
		ui_invalidate(&self->component);
	} else
	if (cmd == CMD_NAVLEFT) {
		if (self->grid.cursor.col == 0) {
			self->grid.cursor.col = TRACKERGRID_NUMCOLS - 2;
			--self->grid.cursor.track;
		} else {
			--self->grid.cursor.col;
		}
		workspace_seteditposition(self->workspace, self->grid.cursor);
		ui_invalidate(&self->component);
	} else
	if (cmd == CMD_NAVRIGHT) {
		if (self->grid.cursor.col == TRACKERGRID_NUMCOLS - 2) {
			self->grid.cursor.col = 0;
			++self->grid.cursor.track;
		} else {
			++self->grid.cursor.col;
		}
		workspace_seteditposition(self->workspace, self->grid.cursor);
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_TAB && GetKeyState (VK_SHIFT) < 0) {
		self->grid.cursor.track -= 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_TAB) {
		self->grid.cursor.track += 1;
		workspace_seteditposition(self->workspace, self->grid.cursor);
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_DELETE) {
		PatternNode* prev;
		PatternNode* node = FindNode(self->pattern, self->grid.cursor.track,
			(beat_t)self->grid.cursor.offset, self->grid.cursor.subline, (beat_t)self->grid.bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
			ui_invalidate(&self->component);
		}

	} else
	if (keycode == VK_RETURN) {		
		PatternNode* prev;
		PatternNode* node = FindNode(self->pattern, 0,
			(beat_t)self->grid.cursor.offset, self->grid.cursor.subline + 1, (beat_t)self->grid.bpl, &prev);		
		if (prev && ((PatternEntry*)prev->entry)->offset >= self->grid.cursor.offset) {
			PatternEvent ev = { 255, 255, 255, 0, 0 };
			double offset;
			++self->grid.cursor.subline;
			offset = self->grid.cursor.offset + self->grid.cursor.subline*self->grid.bpl/4;
			pattern_insert(self->pattern, prev, 0, (beat_t)offset, &ev);			
			AdjustScrollranges(&self->grid);
			workspace_seteditposition(self->workspace, self->grid.cursor);
			ui_invalidate(&self->component);			
		}	
	} else {
		HandlePatternEventInput(self, 
			keycode, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
	}	
	ui_component_propagateevent(sender);
}

void HandlePatternEventInput(TrackerView* self, unsigned int keycode,
	int shift, int ctrl)
{
	if (self->grid.cursor.col == TRACKER_COLUMN_NOTE) {
		int cmd;

		cmd = inputs_cmd(self->grid.noteinputs, encodeinput(keycode, shift, ctrl));
		if (cmd != -1) {
			InputNote(self, NoteCmdToNote(self, cmd));
		}
	} else {								
		InputDigit(self, chartoint((char)keycode));			
	}		
}

unsigned char NoteCmdToNote(TrackerView* self, int cmd)
{
	int rv = 255;	
		
	if (cmd == CMD_NOTE_TWEAKM) {
		rv = NOTECOMMANDS_TWEAK;
	} else
	if (cmd == CMD_NOTE_TWEAKS) {
		rv = NOTECOMMANDS_TWEAKSLIDE;
	} else
	if (cmd == CMD_NOTE_STOP) {
		rv = NOTECOMMANDS_RELEASE;
	} else {
		unsigned char base;

		base = (unsigned char)workspace_octave(self->workspace) * 12;
		rv = base + cmd;
	}
	return rv;
}

void InputNote(TrackerView* self, note_t note)
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
	undoredo_execute(&self->workspace->undoredo,
		&InsertCommandAlloc(self->pattern, self->grid.bpl,
			self->grid.cursor, event)->command);				
	AdvanceCursor(self);
}

void InputDigit(TrackerView* self, int value)
{
	if (self->pattern && value != -1) {
		PatternNode* prev;	
		PatternEvent event;
				
		PatternNode* node = FindNode(self->pattern,
			self->grid.cursor.track,
			(beat_t)self->grid.cursor.offset,
			self->grid.cursor.subline,
			(beat_t)self->grid.bpl,
			&prev);						
		event = pattern_event(self->pattern, node);						
		EnterDigitColumn(&event, self->grid.cursor.col, value);				
		undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(self->pattern, self->grid.bpl,
					self->grid.cursor, event)->command);
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

void EnterDigitColumn(PatternEvent* event, int column, int value)
{
	switch (column) {
		case 1: 
			if ((event->inst == 0xFF) && (value != 0x0F)) {
				event->inst = 0;
			}
			EnterDigit(0, value, &event->inst);
		break;
		case 2:
			if ((event->inst == 0xFF) && (value != 0x0F)) {
				event->inst = 0;
			}
			EnterDigit(1, value, &event->inst);
		break;
		case 3:
			if ((event->mach == 0xFF) && (value != 0x0F)) {
				event->mach = 0;
			}
			EnterDigit(0, value, &event->mach);
		break;
		case 4:
			if ((event->mach == 0xFF) && (value != 0x0F)) {
				event->mach = 0;
			}
			EnterDigit(1, value, &event->mach);
		break;
		case 5:							
			EnterDigit(0, value, &event->cmd);
		break;
		case 6:
			EnterDigit(1, value, &event->cmd);
		break;
		case 7:
			EnterDigit(0, value, &event->parameter);
		break;
		case 8:
			EnterDigit(1, value, &event->parameter);
		break;
		default:
		break;
	}
}

void EnterDigit(int digit, int newval, unsigned char* val)
{	
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else
	if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

void OnScroll(TrackerGrid* self, ui_component* sender, int cx, int cy)
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
		UpdateWindow(self->linenumbers->component.hwnd);
	}
}

void OnMouseDown(TrackerGrid* self, ui_component* sender, int x, int y, int button)
{
	if (self->view->pattern) {
		if (button == 1) {
			int lines;
			int sublines;
			int subline;		
			int coloffset;				
			self->cursor.offset = Offset(self, y - self->dy, &lines, &sublines, &subline);
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

void InitTrackerView(TrackerView* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	self->opcount = 0;
	self->cursorstep = 1;
	ui_component_init(&self->component, parent);
	InitTrackerInputs(self);
	self->pattern = 0;
	ui_bitmap_loadresource(&self->skin.bitmap, IDB_HEADERSKIN);
	InitDefaultSkin(self);	
	InitTrackerHeader(&self->header, &self->component);	
	self->header.numtracks = player_numsongtracks(&workspace->player);
	self->header.trackwidth = self->skin.headercoords.background.destwidth;
	self->linenumbers.skin = &self->skin;
	InitTrackerGrid(&self->grid, &self->component, self, &workspace->player);
	self->grid.noteinputs = workspace_noteinputs(workspace);
	InitTrackerLineNumbersLabel(&self->linenumberslabel, &self->component, self);	
	InitTrackerLineNumbers(&self->linenumbers, &self->component);
	self->linenumbers.view = self;
	self->grid.header = &self->header;
	self->grid.linenumbers = &self->linenumbers;	
	self->header.skin = &self->skin;
	self->linenumbers.lineheight = 13;
	self->showlinenumbers = 1;
	self->showemptydata = 0;
	signal_connect(&self->component.signal_size, self, OnViewSize);
	signal_connect(&self->component.signal_timer, self, OnTimer);
	signal_connect(&self->component.signal_keydown,self, OnKeyDown);
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
	SetTimer(self->component.hwnd, TIMERID_TRACKERVIEW, 200, 0);
	signal_connect(&workspace->signal_configchanged, self, OnConfigChanged);
	ShowLineNumbers(self, workspace_showlinenumbers(workspace));
}

void InitDefaultSkin(TrackerView* self)
{
	SetClassicHeaderCoords(self);
}

void SetClassicHeaderCoords(TrackerView* self)
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

void SetHeaderCoords(TrackerView* self)
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

void OnViewSize(TrackerView* self, ui_component* sender, int width, int height)
{
	Align(self, sender);
}

void Align(TrackerView* self, ui_component* sender)
{
	ui_size size;
	int width;
	int height;
	int headerheight = 20;
	int linenumberwidth = self->showlinenumbers ? 45 :0;
	int hscrollbarheight = 20;
	size = ui_component_size(&self->component);
	width = size.width;
	height = size.height;
	ui_component_move(&self->header.component, linenumberwidth, 0);
	ui_component_resize(&self->header.component, width - linenumberwidth, headerheight);
	ui_component_move(&self->grid.component, linenumberwidth, headerheight);
	ui_component_resize(&self->linenumberslabel.component, linenumberwidth, headerheight);
	ui_component_resize(&self->grid.component, width - linenumberwidth, height - headerheight);
	ui_component_move(&self->linenumbers.component, 0, headerheight);
	ui_component_resize(&self->linenumbers.component, linenumberwidth, height - headerheight);
}

void InitTrackerHeader(TrackerHeader* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnHeaderDraw);
	self->dx = 0;
	self->numtracks = 16;
	self->trackwidth = 102;
	self->classic = 1;
}

void OnHeaderDraw(TrackerHeader* self, ui_component* sender, ui_graphics* g)
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
		BlitSkinPart(self, g, cpx, 0, &self->skin->headercoords.background);
		BlitSkinPart(self, g, cpx, 0, &digitx0);
		BlitSkinPart(self, g, cpx, 0, &digit0x);
		cpx += self->trackwidth;
	}		
}

void BlitSkinPart(TrackerHeader* self, ui_graphics* g, int x, int y,
	SkinCoord* coord)
{
	ui_drawbitmap(g, &self->skin->bitmap, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}

void InitTrackerLineNumbers(TrackerLineNumbers* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnLineNumbersDraw);
	self->dy = 0;
	self->textheight = 12;
	self->lineheight = self->textheight + 1;
	self->component.doublebuffered = 1;
}

void OnLineNumbersDraw(TrackerLineNumbers* self, ui_component* sender, ui_graphics* g)
{		
	LineNumbersDrawBackground(self, g);	
	if (self->view->pattern) {
		ui_size size;
		char buffer[20];		
		int cpy = self->dy;
		int line;		
		double offset;	
		TrackerGridBlock clip;

		size = ui_component_size(&self->component);
		ClipBlock(&self->view->grid, g, &clip);		
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

void LineNumbersDrawBackground(TrackerLineNumbers* self, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, self->view->skin.background);
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
		SetClassicHeaderCoords(self->view);
	} else {
		SetHeaderCoords(self->view);
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
	ui_textoutrectangle(g, r.left, r.top, 0, r, "Line",
		strlen("Line"));	
}

void OnTimer(TrackerView* self, ui_component* sender, int timerid)
{
	if (timerid == TIMERID_TRACKERVIEW) {
		if (self->grid.player->playing) {
			ui_invalidate(&self->grid.component);
		}
		if (self->pattern && self->pattern->opcount != self->opcount) {
			ui_invalidate(&self->grid.component);
			self->opcount = self->pattern->opcount;
		}
	}
}

int NumLines(TrackerView* self)
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

void OnConfigChanged(TrackerView* self, Workspace* workspace, Properties*
	property)
{
	if (property == workspace->config) {
		ReadConfig(self);
	} else
	if (strcmp(properties_key(property), "linenumbers") == 0) {
		ShowLineNumbers(self, properties_value(property));
	} else
	if (strcmp(properties_key(property), "drawemptydata") == 0) {
		ShowEmptyData(self, properties_value(property));
	}
}

void ReadConfig(TrackerView* self)
{
	Properties* pv;
	
	pv = properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {		
		ShowLineNumbers(self, properties_bool(pv, "linenumbers", 1));
		ShowEmptyData(self, properties_bool(pv, "drawemptydata", 1));
	}
}

void ShowLineNumbers(TrackerView* self, int showstate)
{
	self->showlinenumbers = showstate;
	if (self->showlinenumbers != 0) {		
		ui_component_show(&self->linenumbers.component);
		ui_component_show(&self->linenumberslabel.component);
	} else {
		ui_component_hide(&self->linenumbers.component);
		ui_component_hide(&self->linenumberslabel.component);
	}
	Align(self, &self->component);
	ui_invalidate(&self->component);
}

void ShowEmptyData(TrackerView* self, int showstate)
{
	self->showemptydata = showstate;	
	ui_invalidate(&self->component);
}

void InitTrackerInputs(TrackerView* self)
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
}
