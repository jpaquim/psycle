// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "trackerview.h"
#include <pattern.h>
#include "cmdsnotes.h"
#include <string.h>
#include <math.h>
#include "resources/resource.h"

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
static void OnEditPositionChanged(TrackerGrid*, Sequence* sender);
static void ClipBlock(TrackerGrid*, ui_graphics* g, TrackerGridBlock* block);
static void DrawDigit(TrackerGrid*, ui_graphics* g, int digit, int col, int x, int y);
static void EnterDigit(int digit, int newval, unsigned char* val);
static void OnLineNumbersDraw(TrackerLineNumbers*, ui_component* sender, ui_graphics* g);
static void OnLineNumbersLabelDraw(TrackerLineNumbersLabel*, ui_component* sender, ui_graphics* g);
static void InitDefaultSkin(TrackerView*);
static void BlitSkinPart(TrackerHeader*, ui_graphics* g, int x, int y, SkinCoord* coord);
static void LineNumbersDrawBackground(TrackerLineNumbers*, ui_graphics* g);
static void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel*, ui_component* sender);
static void ShowLineNumbers(TrackerView* self, int showstate);
static void OnTimer(TrackerView*, ui_component* sender, int timerid);
static void OnPropertiesClose(TrackerView*, ui_component* sender);
static void OnPropertiesApply(TrackerView*, ui_component* sender);
static int NumLines(TrackerView*);
static void AdjustScrollranges(TrackerGrid*);
static float Offset(TrackerGrid*, int y, int* lines, int* sublines, int* subline);
static int TestCursor(TrackerGrid*, int track, double offset, int subline);
static int TestRange(double position, double offset, double  width);
static void OnSongTracksNumChanged(TrackerGrid*, Player* player,
	unsigned int numsongtracks);
static void SetClassicHeaderCoords(TrackerView* self);
static void SetHeaderCoords(TrackerView* self);
static void OnConfigChanged(TrackerView*, Workspace*, Properties*);

char* notes_tab_a440[256] = {
	"C-m","C#m","D-m","D#m","E-m","F-m","F#m","G-m","G#m","A-m","A#m","B-m", //0
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //1
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //2
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //3
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //4
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //5
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //6
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //7
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //8
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //9
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};
char* notes_tab_a220[256] = {
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //0
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //1
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //2
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //3
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //4
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //5
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //6
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //7
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //8
	"C-9","C#9","D-9","D#9","E-9","F-9","F#9","G-9","G#9","A-9","A#9","B-9", //9
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};

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
	self->lpb = self->player->lpb;
	self->bpl = 1.0f / self->lpb;
	self->notestab = notes_tab_a220;	
	self->cursor.track = 0;
	self->cursor.offset = 0;
	self->cursor.subline = 0;
	self->cursor.col = 0;
	self->cursorstep = 0.25;
	self->dx = 0;
	self->dy = 0;

	self->textheight = 12;
	self->textwidth = 9;
	self->textleftedge = 2;
	self->colx[0] = 0;
	self->colx[1] = (self->textwidth*3)+2;
	self->colx[2] = self->colx[1]+self->textwidth;
	self->colx[3] = self->colx[2]+self->textwidth+1;
	self->colx[4] = self->colx[3]+self->textwidth;
	self->colx[5] = self->colx[4]+self->textwidth+1;
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
	signal_connect(&self->player->song->sequence.signal_editpositionchanged,
		self, OnEditPositionChanged);	
}

void OnSongTracksNumChanged(TrackerGrid* self, Player* player,
	unsigned int numsongtracks)
{	
	self->numtracks = numsongtracks;	
	self->view->header.numtracks = numsongtracks;
	AdjustScrollranges(self);
	ui_invalidate(&self->view->component);
}

void TrackerViewSongChanged(TrackerView* self, Workspace* workspace)
{	
	signal_connect(&self->grid.player->song->sequence.signal_editpositionchanged,
		&self->grid, OnEditPositionChanged);	
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
		self->bpl = 1.0f / self->player->lpb;		
		ui_setfont(g, &self->view->font);		
		ClipBlock(self, g, &clip);
		DrawBackground(self, g, &clip);
		DrawEvents(self, g, &clip);
	} else {
		ui_drawsolidrectangle(g, g->clip, self->view->skin.background);	
	}
}

float Offset(TrackerGrid* self, int y, int* lines, int* sublines, int* subline)
{
	float offset = 0;	
	int cpy = 0;		
	int first = 1;
	int count = y / self->lineheight;
	int remaininglines = 0;	

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
	int track;
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

int TestCursor(TrackerGrid* self, int track, double offset, int subline)
{
	return self->cursor.track == track && 
		self->cursor.offset >= offset && self->cursor.offset < offset + self->bpl
		&& self->cursor.subline == subline;
}

void DrawEvents(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{	
	int track;
	int cpx = 0;	
	int cpy;
	double offset;
	int subline;	
	PatternNode* prev;
	
	cpy = (clip->topleft.totallines - clip->topleft.subline) * self->lineheight + self->dy;	
	offset = clip->topleft.offset;	
	self->curr_event = pattern_greaterequal(self->view->pattern, (float)offset, &prev);
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
					((PatternEntry*)(self->curr_event->entry))->offset >= offset && 				
					((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl) {					
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
				((PatternEntry*)(self->curr_event->entry))->offset >= offset && 				
				((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl) {					
				self->curr_event = self->curr_event->next;
			}
			cpy += self->lineheight;
			subline++;
		} while (self->curr_event &&
			((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl);
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
	
	SetColColor(&self->view->skin, g, 0, playbar, cursor && self->cursor.col == 0, beat, beat4);
	ui_setrectangle(&r, x + self->colx[0], y, self->textwidth*3, self->textheight);
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r,
	self->notestab[event->note],
	strlen(self->notestab[event->note]));			
	
	{	// inst
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
	{	// mach
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
	{	// cmd
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
	{	// parameter
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
		_snprintf(buffer, 2, "%s", "");	
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


int NumSublines(Pattern* pattern, float offset, float bpl)
{
	PatternNode* prev;
	PatternNode* node = pattern_greaterequal(pattern, offset, &prev);		
	int currsubline = -1;
	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			break;
		}				
		if (entry->track == 0) {
			++currsubline;					
		}
		node = node->next;
	}
	return currsubline;
}

PatternNode* FindNode(Pattern* pattern, int track, float offset, int subline, float bpl, PatternNode** prev)
{
	PatternNode* node = pattern_greaterequal(pattern, offset, prev);	
	int currsubline = 0;
	int first = 1;	
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
	if (self->grid.cursor.subline < NumSublines(self->pattern, self->grid.cursor.offset, self->grid.bpl)) {
		++self->grid.cursor.subline;
	} else {
		self->grid.cursor.offset += self->grid.bpl;
		self->grid.cursor.subline = 0;
	}
}

void OnKeyDown(TrackerView* self, ui_component* sender, int keycode, int keydata)
{		
	int cmd;
		
	if (keycode == VK_UP) {
		if (self->grid.cursor.subline > 0) {
			--self->grid.cursor.subline;
		} else {
			self->grid.cursor.offset -= self->grid.bpl;
		}
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_DOWN) {		
		AdvanceCursor(self);
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_LEFT) {
		if (self->grid.cursor.col == 0) {
			self->grid.cursor.col = TRACKERGRID_NUMCOLS - 2;
			--self->grid.cursor.track;
		} else {
			--self->grid.cursor.col;
		}
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_RIGHT) {
		if (self->grid.cursor.col == TRACKERGRID_NUMCOLS - 2) {
			self->grid.cursor.col = 0;
			++self->grid.cursor.track;
		} else {
			++self->grid.cursor.col;
		}
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_TAB && GetKeyState (VK_SHIFT) < 0) {
		self->grid.cursor.track -= 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_TAB) {
		self->grid.cursor.track += 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_DELETE) {
		PatternNode* prev;
		PatternNode* node = FindNode(self->pattern, self->grid.cursor.track,
			self->grid.cursor.offset, self->grid.cursor.subline, self->grid.bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
			ui_invalidate(&self->component);
		}
	} else
	if (keycode == VK_RETURN) {
		PatternNode* prev;
		PatternNode* node = FindNode(self->pattern, 0,
			self->grid.cursor.offset, self->grid.cursor.subline + 1, self->grid.bpl, &prev);		
		if (prev && ((PatternEntry*)prev->entry)->offset >= self->grid.cursor.offset) {
			PatternEvent ev = { 255, 255, 255, 0, 0 };
			float offset;
			++self->grid.cursor.subline;
			offset = self->grid.cursor.offset + self->grid.cursor.subline*self->grid.bpl/4;
			pattern_insert(self->pattern, prev, 0, offset, &ev);			
			AdjustScrollranges(&self->grid);
			ui_invalidate(&self->component);			
		}		
	} else {
		if (self->grid.cursor.col == 0) {			
			cmd = Cmd(&self->grid.noteinputs->map, keycode);
			if (cmd != -1) {		
				int base = 48;
				PatternNode* prev;
				PatternNode* node = FindNode(self->pattern, self->grid.cursor.track, self->grid.cursor.offset, self->grid.cursor.subline, self->grid.bpl, &prev);
				if (node) {					
					PatternEntry* entry = (PatternEntry*)(node->entry);
					if (cmd == CMD_NOTE_Stop) {
						entry->event.note = 120;
					} else {
						entry->event.note = (unsigned char)(base + cmd);					
					}
				} else {
					Machine* machine;
					float offset;
					PatternEvent ev = { 0, 255, 255, 0, 0 };

					if (cmd == CMD_NOTE_Stop) {
						ev.note = 120;
					} else {
						ev.note = (unsigned char)(base + cmd);
					}
					ev.mach = machines_slot(&self->grid.player->song->machines);
					machine = machines_at(&self->grid.player->song->machines, ev.mach);
					if (machine && machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
						ev.inst = self->grid.player->song->instruments.slot;
					}
					offset = self->grid.cursor.offset;					
					pattern_insert(self->pattern, prev, self->grid.cursor.track, offset, &ev);
				}
				AdvanceCursor(self);				
				ui_invalidate(&self->component);			
			}		
		} else {
			int val = -1;
			if (keycode >= '0' && keycode <='9') {
				val = keycode - '0';
			} else
			if (keycode >= 'A' && keycode <='Z') {
				val = keycode - 'A' + 10;
			}
			if (val != -1 && self->pattern) {
				PatternNode* prev;
				PatternEntry* entry = 0;
				PatternNode* node = FindNode(self->pattern, self->grid.cursor.track, self->grid.cursor.offset, self->grid.cursor.subline, self->grid.bpl, &prev);
				if (node) {					
					entry = (PatternEntry*)(node->entry);					
				} else {
					float offset;
					int base = 48;
					PatternEvent ev = { 255, 255, 255, 0, 0 };					
					offset = self->grid.cursor.offset;
					entry = pattern_write(self->pattern, self->grid.cursor.track, offset, ev);					
				}				
				if (entry) {					
					switch (self->grid.cursor.col) {
						case 1: 
							if ((entry->event.inst == 0xFF) && (val != 0x0F)) {
								entry->event.inst = 0;
							}
							EnterDigit(0, val, &entry->event.inst);
						break;
						case 2:
							if ((entry->event.inst == 0xFF) && (val != 0x0F)) {
								entry->event.inst = 0;
							}
							EnterDigit(1, val, &entry->event.inst);
						break;
						case 3:
							if ((entry->event.mach == 0xFF) && (val != 0x0F)) {
								entry->event.mach = 0;
							}
							EnterDigit(0, val, &entry->event.mach);
						break;
						case 4:
							if ((entry->event.mach == 0xFF) && (val != 0x0F)) {
								entry->event.mach = 0;
							}
							EnterDigit(1, val, &entry->event.mach);
						break;
						case 5:							
							EnterDigit(0, val, &entry->event.cmd);
						break;
						case 6:
							EnterDigit(1, val, &entry->event.cmd);
						break;
						case 7:
							EnterDigit(0, val, &entry->event.parameter);
						break;
						case 8:
							EnterDigit(1, val, &entry->event.parameter);
						break;
						default:
						break;
					}
					ui_invalidate(&self->component);
				}
			}
		}		
	}	
	ui_component_propagateevent(sender);
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
			ui_invalidate(&self->component);
			ui_component_setfocus(&self->component);
		} else
		if (button == 2) {
			ui_size size = ui_component_size(&self->view->component);
			if (ui_component_visible(&self->view->properties.component)) {			
				ui_component_hide(&self->view->properties.component);			
			} else {
				ui_component_show(&self->view->properties.component);
			}
			OnViewSize(self->view, &self->view->component, size.width, size.height);
		}
	}
}

void OnEditPositionChanged(TrackerGrid* self, Sequence* sender)
{	
	if (sender->editposition.trackposition.tracknode) {		
		SequenceEntry* entry = (SequenceEntry*)sender->editposition.trackposition.tracknode->entry;	
		TrackerViewSetPattern(self->view, patterns_at(&self->player->song->patterns, entry->pattern));
	} else {
		TrackerViewSetPattern(self->view, 0);		
	}
	ui_invalidate(&self->component);
}

void InitTrackerView(TrackerView* self, ui_component* parent, Workspace* workspace)
{		
	ui_component_init(&self->component, parent);
	self->pattern = 0;
	ui_bitmap_loadresource(&self->skin.skinbmp, IDB_HEADERSKIN);
	InitDefaultSkin(self);	
	InitTrackerHeader(&self->header, &self->component);	
	self->header.numtracks = player_numsongtracks(&workspace->player);
	self->header.trackwidth = self->skin.headercoords.background.destwidth;
	self->linenumbers.skin = &self->skin;
	InitTrackerGrid(&self->grid, &self->component, self, &workspace->player);
	InitTrackerLineNumbersLabel(&self->linenumberslabel, &self->component, self);	
	InitTrackerLineNumbers(&self->linenumbers, &self->component);
	self->linenumbers.view = self;
	self->grid.header = &self->header;
	self->grid.linenumbers = &self->linenumbers;	
	self->header.skin = &self->skin;
	self->linenumbers.lineheight = 13;	

	properties_readbool(workspace->config, "linenumbers",
		&self->showlinenumbers, 1);
	if (self->showlinenumbers == 0) {		
		ui_component_hide(&self->linenumbers.component);
		ui_component_hide(&self->linenumberslabel.component);
	}
	signal_connect(&self->component.signal_size, self, OnViewSize);
	InitPatternProperties(&self->properties, &self->component, 0);	
	ui_component_hide(&self->properties.component);	
	signal_connect(&self->properties.closebutton.signal_clicked, self, OnPropertiesClose);
	signal_connect(&self->properties.applybutton.signal_clicked, self, OnPropertiesApply);
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
	SetTimer(self->component.hwnd, 200, 50, 0);
	signal_connect(&workspace->signal_configchanged, self, OnConfigChanged);
	ShowLineNumbers(self, workspace_showlinenumbers(workspace));
}

void TrackerViewSetPattern(TrackerView* self, Pattern* pattern)
{	
	self->pattern = pattern;
	self->grid.dx = 0;
	self->grid.dy = 0;
	self->header.dx = 0;
	self->linenumbers.dy = 0;
	PatternPropertiesSetPattern(&self->properties, pattern);
	AdjustScrollranges(&self->grid);
	ui_invalidate(&self->linenumbers.component);
	ui_invalidate(&self->header.component);
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
	int propertyheight = ui_component_visible(&self->properties.component) ? 60 : 0;
	size = ui_component_size(&self->component);
	width = size.width;
	height = size.height;
	ui_component_move(&self->header.component, linenumberwidth, 0);
	ui_component_resize(&self->header.component, width - linenumberwidth, headerheight);
	ui_component_move(&self->grid.component, linenumberwidth, headerheight);
	ui_component_resize(&self->linenumberslabel.component, linenumberwidth, headerheight);
	ui_component_resize(&self->grid.component, width - linenumberwidth, height - headerheight -  propertyheight);
	ui_component_move(&self->linenumbers.component, 0, headerheight);
	ui_component_resize(&self->linenumbers.component, linenumberwidth, height - headerheight -  propertyheight);
	ui_component_move(&self->properties.component, 0, height - propertyheight);
	ui_component_resize(&self->properties.component, width, propertyheight);
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
	int track;

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

void BlitSkinPart(TrackerHeader* self, ui_graphics* g, int x, int y, SkinCoord* coord)
{
	ui_drawbitmap(g, &self->skin->skinbmp, x + coord->destx, y + coord->desty,
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
		float offset;	
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
	if (self->grid.player->playing) {
		ui_invalidate(&self->grid.component);
	}
}

void OnPropertiesClose(TrackerView* self, ui_component* sender)
{
	ui_size size = ui_component_size(&self->component);
	OnViewSize(self, &self->component, size.width, size.height);	
}

void OnPropertiesApply(TrackerView* self, ui_component* sender)
{
	SCROLLINFO		si;	
	ui_size size = ui_component_size(&self->component);	
	OnViewSize(self, &self->grid.component, size.width, size.height);	
	size = ui_component_size(&self->grid.component);
	AdjustScrollranges(&self->grid);	
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (self->grid.component.hwnd, SB_VERT, &si) ;	
	self->grid.dy = -si.nPos * self->grid.lineheight;	
	self->linenumbers.dy = self->grid.dy;
	ui_invalidate(&self->component);
}

int NumLines(TrackerView* self)
{
	int lines = 0;
	int sublines = 0;	
	int remaininglines = 0;
	float offset = 0;

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
		remaininglines = (int)(offset * self->grid.player->lpb);
	}
	return lines + sublines + remaininglines;
}

void OnConfigChanged(TrackerView* self, Workspace* workspace, Properties* property)
{
	if (strcmp(properties_key(property), "linenumbers") == 0) {
		ShowLineNumbers(self, properties_value(property));
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
