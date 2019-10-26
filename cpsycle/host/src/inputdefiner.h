// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INPUTDEFINER_H)
#define INPUTDEFINER_H

#include <uicomponent.h>

typedef struct {
   ui_component component;   
   unsigned int input;
   int regularkey;
} InputDefiner;

void inputdefiner_init(InputDefiner*, ui_component* parent);
void inputdefiner_setinput(InputDefiner*, unsigned int input);
void inputdefiner_text(InputDefiner*, char* text);

#endif
