// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIDEF)
#define UIDEF

typedef struct { int x, y; } ui_point;
typedef struct { int left, top, right, bottom; } ui_rectangle;
typedef struct { int width, height; } ui_size;
typedef struct { int top, right, bottom, left; } ui_margin;

void ui_setrectangle(ui_rectangle*, int left, int top, int width, int height);
void ui_setmargin(ui_margin*, int top, int right, int bottom, int left);
void ui_error(const char* err, const char* shorterr);

#endif