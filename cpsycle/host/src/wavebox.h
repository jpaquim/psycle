// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(WAVEBOX_H)
#define WAVEBOX_H

#include "uicomponent.h"
#include <sample.h>

typedef struct {	
	psy_ui_Component component;
	psy_audio_Sample* sample;
	bool hasselection;
	uintptr_t selectionstart;
	uintptr_t selectionend;
	float zoomleft;
	float zoomright;
	double offsetstep;	
	int dragmode;
	int dragoffset;
	psy_Signal selectionchanged;
	char* nowavetext;
	bool preventdrawonselect;
} WaveBox;

void wavebox_init(WaveBox*, psy_ui_Component* parent);
void wavebox_setnowavetext(WaveBox*, const char* text);
void wavebox_setsample(WaveBox*, psy_audio_Sample*);
void wavebox_setzoom(WaveBox*, psy_dsp_beat_t left, psy_dsp_beat_t right);
void wavebox_setselection(WaveBox* self, uintptr_t selectionstart,
	uintptr_t selectionend);
void wavebox_clearselection(WaveBox* self);
INLINE bool wavebox_hasselection(WaveBox* self)
{
	return self->hasselection != FALSE;
}

#endif
