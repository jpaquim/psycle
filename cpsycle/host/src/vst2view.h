// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(VST2VIEW)
#define VST2VIEW

#include <uicomponent.h>
#include <machine.h>


typedef struct {
   ui_component component;   
   Machine* plugin;      
} Vst2View;

void InitVst2View(Vst2View*, ui_component* parent, Machine*);

#endif
