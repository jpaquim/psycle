// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PARAMVIEW)
#define PARAMVIEW

#include "workspace.h"
#include <uicomponent.h>
#include <machine.h>
#include <plugin_interface.h>

typedef struct {
   psy_ui_Component component;   
   int cx;
   int cy;
   psy_audio_Machine* machine;   
   uintptr_t numparams;
   uintptr_t numparametercols;   
   uintptr_t numrows;
   int tweak;
   int tweakbase;
   int tweakval;
   int my;
   Workspace* workspace;
} ParamView;

void paramview_init(ParamView*, psy_ui_Component* parent, psy_audio_Machine*,
	Workspace*);
ParamView* paramview_alloc(void);
ParamView* paramview_allocinit(psy_ui_Component* parent, psy_audio_Machine*,
	Workspace*);


#endif
