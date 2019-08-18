// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "patternview.h"
#include <pattern.h>
#include <string.h>
#include <math.h>

static void Draw(PatternView* self, ui_graphics* g);
static void DrawBackground(PatternView* self, ui_graphics* g);
static void DrawTrackBackground(PatternView* self, ui_graphics* g, int track);
static void DrawEvents(PatternView* self, ui_graphics* g);
static void DrawPatternEvent(PatternView* self, ui_graphics* g, PatternEvent* event, int x, int y, int cursor, int beat, int beat4);
static int OnKeyDown(PatternView* self, int keycode, int keydata);
static void OnSize(PatternView* self, int width, int height);
static void OnScroll(PatternView* self, int cx, int cy);
static void SetDefaultSkin(PatternView* self);
static void OnMouseDown(PatternView* self, int x, int y, int button);

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

void InitPatternView(PatternView* self, ui_component* parent, Player* player)
{		
	if (self->skin.hfont == NULL) {
		self->skin.hfont = ui_createfont("Tahoma", 12);
	}
	ui_component_init(self, &self->component, parent);
	self->component.events.size = OnSize;
	self->component.events.keydown = OnKeyDown;
	self->component.events.mousedown = OnMouseDown;
	self->component.events.draw = Draw;
	self->component.events.scroll = OnScroll;
	ui_component_move(&self->component, 0, 0);

	self->player = player;
	self->numtracks = 16;
	self->numlines = 64;
	self->lpb = 4;
	self->bpl = 1.0f / self->lpb;
	self->notestab = notes_tab_a220;	
	self->cursor.track = 0;
	self->cursor.offset = 0;	
	self->cursor.col = 0;
	self->cursorstep = 0.25;
	self->dy = 0;
		
	self->textwidth = 10;
	self->colx[0] = 0;
	self->colx[1] = (self->textwidth*3)+2;
	self->colx[2] = self->colx[1]+self->textwidth;
	self->colx[3] = self->colx[2]+self->textwidth+1;
	self->colx[4] = self->colx[3]+self->textwidth;
	self->colx[5] = self->colx[4]+self->textwidth+1;
	self->colx[6] = self->colx[5]+self->textwidth;
	self->colx[7] = self->colx[6]+self->textwidth;
	self->colx[8] = self->colx[7]+self->textwidth;
	self->colx[9] = self->colx[8]+self->textwidth+1;
	self->trackwidth = self->colx[9];
	self->lineheight = 12;	
	ui_component_showverticalscrollbar(&self->component);
	self->component.doublebuffered = 1;
	PatternViewApplyProperties(self, 0);	
}

void PatternViewApplyProperties(PatternView* self, Properties* properties)
{
	properties_readint(properties, "pvc_separator", &self->skin.separator, 0x00400000);
	properties_readint(properties, "pvc_separator2", &self->skin.separator2, 0x00004000);
	properties_readint(properties, "pvc_background", &self->skin.background, 0x009a887c);
	properties_readint(properties, "pvc_background2", &self->skin.background2, 0x00aa786c);
	properties_readint(properties, "pvc_row4beat", &self->skin.row4beat, 0x00d5ccc6);
	properties_readint(properties, "pvc_row4beat2", &self->skin.row4beat2, 0x00fdfcf6);
	properties_readint(properties, "pvc_rowbeat", &self->skin.rowbeat, 0x00c9beb8);
	properties_readint(properties, "pvc_rowbeat2", &self->skin.rowbeat2, 0x00f9eee8);
	properties_readint(properties, "pvc_row", &self->skin.row, 0x00c1b5aa);
	properties_readint(properties, "pvc_row2", &self->skin.row2, 0x00f1e5da);
	properties_readint(properties, "pvc_font", &self->skin.font, 0x00000000);
	properties_readint(properties, "pvc_font2", &self->skin.font2, 0x00000000);
	properties_readint(properties, "pvc_fontplay", &self->skin.fontPlay, 0x00ffffff);
	properties_readint(properties, "pvc_fontplay2", &self->skin.fontPlay2, 0x00ffffff);
	properties_readint(properties, "pvc_fontcur", &self->skin.fontCur, 0x00ffffff);
	properties_readint(properties, "pvc_fontcur2", &self->skin.fontCur2, 0x00ffffff);
	properties_readint(properties, "pvc_fontsel", &self->skin.fontSel, 0x00ffffff);
	properties_readint(properties, "pvc_fontsel2", &self->skin.fontSel2, 0x00ffffff);
	properties_readint(properties, "pvc_selection", &self->skin.selection, 0x00e00000);
	properties_readint(properties, "pvc_selection2", &self->skin.selection2, 0x00ff5050);
	properties_readint(properties, "pvc_playbar", &self->skin.playbar, 0x0000e000);
	properties_readint(properties, "pvc_playbar2", &self->skin.playbar2, 0x0000e000);
	properties_readint(properties, "pvc_cursor", &self->skin.cursor, 0x005050e0);
	properties_readint(properties, "pvc_cursor2", &self->skin.cursor2, 0x005050ff);
	ui_invalidate(&self->component);
}

void Draw(PatternView* self, ui_graphics* g)
{	   	
	if (self->skin.hfont) {
		ui_setfont(g, self->skin.hfont);
	}
	DrawBackground(self, g);
	DrawEvents(self, g);
}

void DrawBackground(PatternView* self, ui_graphics* g)
{
	int track;
	for (track = 0; track < self->numtracks; ++track) {
		DrawTrackBackground(self, g, track);
	}
}

void DrawTrackBackground(PatternView* self, ui_graphics* g, int track)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, track * self->trackwidth, 0, self->trackwidth, self->cy);
	ui_drawsolidrectangle(g, r, self->skin.background);
	//ui_setrectangle(&r, r.left, 0, 1, r.bottom);
	// ui_drawsolidrectangle(g, r, 0x000000);
}

int TestCursor(PatternView* self, int track, float offset)
{
	return self->cursor.track == track && 
		self->cursor.offset >= offset && self->cursor.offset < offset + self->bpl;
}

void DrawEvents(PatternView* self, ui_graphics* g)
{
	int line;	
	int track;
	int cpx = 0;
	int cpy = self->dy;

	self->curr_event = self->pattern->events;
	for (line = 0; line < self->numlines; ++line) {
		float offset = line* self->bpl;
		int beat = fmod(offset, 1.0f) == 0.0f;
		int beat4 = fmod(offset, 4.0f) == 0.0f;
		for (track = 0; track < self->numtracks; ++track) {						
			int hasevent = 0;
			int cursor = -1;
			if (TestCursor(self, track, offset)) {
				cursor = 0;
			}
			while (self->curr_event && self->curr_event->track == track &&
				self->curr_event->offset >= offset && 				
				self->curr_event->offset < offset + self->bpl) {				
				DrawPatternEvent(self, g, &self->curr_event->event, cpx, cpy, cursor, beat, beat4);
				self->curr_event = self->curr_event->next;
				hasevent = 1;
			}
			if (!hasevent) {
				PatternEvent event;
				memset(&event, 0, sizeof(PatternEvent));			
				event.note = 255;
				DrawPatternEvent(self, g, &event, cpx, cpy, cursor, beat, beat4);
			}
			cpx += self->trackwidth;			
		}
		cpx = 0;
		cpy += self->lineheight;
	}
}

void SetColColor(PatternView* self, ui_graphics* g, int col, int cursor, int beat, int beat4)
{
	if (cursor != -1 && col == self->cursor.col) {
		if (beat4) {
			ui_setbackgroundcolor(g, self->skin.cursor);
			ui_settextcolor(g, self->skin.fontCur);			
		} else
		if (beat) {
			ui_setbackgroundcolor(g, self->skin.cursor);
			ui_settextcolor(g, self->skin.fontCur);			
		} else {
			ui_setbackgroundcolor(g, self->skin.cursor);
			ui_settextcolor(g, self->skin.fontCur);
		}
	} else {
		if (beat4) {
			ui_setbackgroundcolor(g, self->skin.row4beat);			
			ui_settextcolor(g, self->skin.font);
		} else
		if (beat) {
			ui_setbackgroundcolor(g, self->skin.rowbeat);
			ui_settextcolor(g, self->skin.font);
		} else {
			ui_setbackgroundcolor(g, self->skin.row);
			ui_settextcolor(g, self->skin.font);
		}
	}	
}

void DrawPatternEvent(PatternView* self, ui_graphics* g, PatternEvent* event, int x, int y, int cursor, int beat, int beat4)
{					
	ui_rectangle r;						
	int c;

	SetColColor(self, g, 0, cursor, beat, beat4);
	ui_setrectangle(&r, x + self->colx[0], y, self->textwidth*3, self->lineheight);
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r,
	self->notestab[event->note],
	strlen(self->notestab[event->note]));			
	
	for (c=1; c < 9; ++c) {
		SetColColor(self, g, 1, cursor, beat, beat4);
		ui_setrectangle(&r, x + self->colx[c], y, self->textwidth, self->lineheight);
		ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, "", strlen(""));
	}	
}

void OnSize(PatternView* self, int width, int height)
{
	self->cx = width;
	self->cy = height;
}

int OnKeyDown(PatternView* self, int keycode, int keydata)
{		
	int cmd;

	if (keycode == VK_F5) {
		player_start(self->player);
		return 0;
	} else
	if (keycode == VK_UP) {
		self->cursor.offset -= self->bpl;		
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_DOWN) {
		self->cursor.offset += self->bpl;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_TAB && GetKeyState (VK_SHIFT) < 0) {
		self->cursor.track -= 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_TAB) {
		self->cursor.track += 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_F8) {
		player_stop(self->player);
		return 0;
	} else {
		cmd = Cmd(&self->noteinputs->map, keycode);
		if (cmd != -1) {		
			float offset;
			PatternEvent ev = { 0, 0, 0, 0, 0 };
			int base = 48;

			ev.note = (unsigned char)(base + cmd);
			offset = self->cursor.offset;
			pattern_write(self->pattern, self->cursor.track, offset, ev);
			self->cursor.offset += self->cursorstep;
			ui_invalidate(&self->component);			
		}		
	}
	return 1;
}

void OnScroll(PatternView* self, int cx, int cy)
{
	self->dy += cy;
}

void OnMouseDown(PatternView* self, int x, int y, int button)
{
	ui_component_setfocus(&self->component);
}
