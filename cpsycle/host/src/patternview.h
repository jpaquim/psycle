// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNVIEW)
#define PATTERNVIEW

#include <uicomponent.h>
#include "trackerview.h"
#include "pianoroll.h"
#include "workspace.h"

typedef struct PatternView {
	ui_component component;
	TrackerView trackerview;
	Pianoroll pianoroll;
	TabBar tabbar;
} PatternView;

void InitPatternView(PatternView*, ui_component* parent, Workspace* workspace);

#endif