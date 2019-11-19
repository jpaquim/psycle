// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(INSTRUMENTSVIEW_H)
#define INSTRUMENTSVIEW_H

#include <uinotebook.h>
#include "tabbar.h"
#include "player.h"
#include "sampulseinstrumentview.h"
#include "samplerinstrumentview.h"
#include "workspace.h"

typedef struct {
	ui_component component;
	ui_notebook notebook;
	SamplerInstrumentView samplerview;
	SampulseInstrumentView sampulseview;
	TabBar tabbar;
} InstrumentsView;

void InitInstrumentsView(InstrumentsView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);

#endif
