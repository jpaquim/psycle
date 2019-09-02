// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(WAVEBOX_H)
#define WAVEBOX_H

#include "uicomponent.h"
#include <sample.h>

typedef struct {	
	ui_component component;
	Sample* sample;	
} WaveBox;

void InitWaveBox(WaveBox*, ui_component* parent);

#endif
