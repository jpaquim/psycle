// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uidef.h"

#include <windows.h>

void ui_setrectangle(ui_rectangle* self, int left, int top, int width, int height)
{
   self->left = left;
   self->top = top;
   self->right = left + width;
   self->bottom = top + height;   
}

void ui_setmargin(ui_margin* self, int top, int right, int bottom, int left)
{   
   self->top = top;
   self->right = right;
   self->bottom = bottom;   
   self->left = left;
}

void ui_error(const char* err, const char* shorterr)
{
	MessageBox(NULL, err, shorterr, MB_OK | MB_ICONERROR);
}