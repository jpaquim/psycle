// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UIEVENTS)
#define UIEVENTS

#include "graphics.h"

typedef struct {		
   void (*draw)(void*, ui_graphics*);
   void (*timer)(void*, int timer);
   void (*size)(void*, int width, int height);
   int (*keydown)(void*, int keycode, int keydata);
   void (*keyup)(void*, int keycode, int keydata);
   void (*mousedown)(void*, int x, int y, int button);
   void (*mouseup)(void*, int x, int y, int button);
   void (*mousemove)(void*, int x, int y, int button);
   void (*mousedoubleclick)(void*, int x, int y, int button);   
   void (*scroll)(void*, int dx, int dy);
   void (*create)(void*);
   void (*destroy)(void*, void*);
   void (*command)(void*, WPARAM wparam, LPARAM lparam);
   int (*childenum)(void*, void*);
   void* target;
   void* cmdtarget;
} ui_events;


#endif
