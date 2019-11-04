// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PARAMVIEW)
#define PARAMVIEW

#include <uicomponent.h>
#include <machine.h>
#include <plugin_interface.h>

typedef struct {
   ui_component component;   
   int cx;
   int cy;
   Machine* machine;   
   int numparams;
   int numparametercols;   
   int numrows;
   int tweak;
   int tweakbase;
   int tweakval;
   int my;   
} ParamView;

void InitParamView(ParamView*, ui_component* parent, Machine* machine);
void ParamViewSize(ParamView*, int* width, int* height);


#endif