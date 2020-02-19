// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MIDIVIEW_H)
#define MIDIVIEW_H

#include "labelpair.h"
#include <uicheckbox.h>
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Label title;
	psy_ui_Label coreinfo;
	psy_ui_Component top;
	psy_ui_Component resources;
	psy_ui_Label resourcestitle;
	LabelPair resources_win;
	LabelPair resources_mem;
	LabelPair resources_swap;
	LabelPair resources_vmem;		
	psy_ui_Component performance;
	LabelPair audiothreads;
	LabelPair totaltime;
	LabelPair machines;
	LabelPair routing;
	psy_ui_CheckBox cpucheck;
	psy_ui_Component performancelist;
	psy_ui_Margin topmargin;
	Workspace* workspace;
} MidiView;

void midiview_init(MidiView*, psy_ui_Component* parent, Workspace*);

#endif
