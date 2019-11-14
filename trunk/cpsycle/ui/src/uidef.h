// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIDEF_H)
#define UIDEF_H

#include "../../detail/prefix.h"
#include <stddef.h>

#include <windows.h>
#if defined min
#undef min
#endif

typedef struct { 
	int x;
	int y;
} ui_point;

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
	int top;
	int right;
	int bottom;
	int left;
} ui_margin;

void ui_margin_init(ui_margin*, int top, int right, int bottom, int left);

typedef struct {
	LOGFONT lf; 
} ui_fontinfo;

void ui_fontinfo_init(ui_fontinfo*, const char* family, int height);

typedef struct {
	HFONT hfont;
	int stock;
} ui_font;

void ui_setrectangle(ui_rectangle*, int left, int top, int width, int height);
int ui_rectangle_intersect(ui_rectangle*, int x, int y);
int ui_rectangle_intersect_rectangle(const ui_rectangle*, const ui_rectangle* other);
void ui_error(const char* err, const char* shorterr);

void ui_font_init(ui_font*, const ui_fontinfo* info);
void ui_font_dispose(ui_font*);

#ifdef _WIN64
typedef long long winid_t;
#else
typedef int winid_t;
#endif

#endif
