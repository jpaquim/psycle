// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIDEF_H)
#define UIDEF_H

#include "../../detail/prefix.h"
#include "../../detail/stdint.h"
#include <stddef.h>

// target win98 or nt 4 or later systems
#define _WIN32_WINNT 0x400

#include <windows.h>
#if defined min
#undef min
#endif

typedef TEXTMETRIC ui_textmetric;

typedef enum {
	UI_UNIT_EH,
	UI_UNIT_EW,
	UI_UNIT_PX,
	UI_UNIT_PE
} UiUnit;

typedef struct {
	union {
		intptr_t integer;
		double real;
	} quantity;
	UiUnit unit;
} ui_value;

ui_value ui_value_makepx(intptr_t px);
ui_value ui_value_makeew(double em);
ui_value ui_value_makeeh(double em);
intptr_t ui_value_px(ui_value*, const ui_textmetric*);

typedef struct { 
	int x;
	int y;
} ui_point;

ui_point ui_point_make(int x, int y);

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
} ui_rectangle;

typedef struct {
	int width;
	int height;
} ui_size;

typedef struct {
	ui_value top;
	ui_value right;
	ui_value bottom;
	ui_value left;
} ui_margin;

void ui_margin_init(ui_margin*, ui_value top, ui_value right, ui_value bottom,
	ui_value left);
intptr_t ui_margin_width_px(ui_margin*, const ui_textmetric*);
intptr_t ui_margin_height_px(ui_margin*, const ui_textmetric*);


typedef struct {
	LOGFONT lf; 
} ui_fontinfo;

void ui_fontinfo_init(ui_fontinfo*, const char* family, int height);

typedef struct {
	HFONT hfont;
	int stock;
} ui_font;

typedef enum {
	UI_CURSOR_DEFAULT,
	UI_CURSOR_COLRESIZE
} ui_cursor;

typedef enum {
	UI_ALIGNMENT_NONE				= 0,	
	UI_ALIGNMENT_LEFT				= 2,
	UI_ALIGNMENT_RIGHT				= 4,
	UI_ALIGNMENT_CENTER_HORIZONTAL	= UI_ALIGNMENT_LEFT | UI_ALIGNMENT_RIGHT,
	UI_ALIGNMENT_TOP				= 8,
	UI_ALIGNMENT_BOTTOM				= 16,
	UI_ALIGNMENT_CENTER_VERTICAL	= UI_ALIGNMENT_TOP | UI_ALIGNMENT_BOTTOM
} UiAlignment;

void ui_setrectangle(ui_rectangle*, int left, int top, int width, int height);
int ui_rectangle_intersect(ui_rectangle*, int x, int y);
int ui_rectangle_intersect_rectangle(const ui_rectangle*, const ui_rectangle* other);
void ui_rectangle_union(ui_rectangle*, const ui_rectangle* other);
void ui_error(const char* err, const char* shorterr);

void ui_font_init(ui_font*, const ui_fontinfo* info);
void ui_font_copy(ui_font*, const ui_font* other);
void ui_font_dispose(ui_font*);

#endif
