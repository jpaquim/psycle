// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIEVENTS_H)
#define UIEVENTS_H

#include "uigraphics.h"
#include <signal.h>

typedef struct {		               
   int (*childenum)(void*, void*);
   void* target;
   void* cmdtarget;
} ui_events;

#endif

