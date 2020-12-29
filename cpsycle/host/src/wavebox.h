// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(WAVEBOX_H)
#define WAVEBOX_H

#include "workspace.h"

// audio
#include <sample.h>
// ui
#include <uicomponent.h>

// WaveBox
//
// Shows PCM wave data, allows frame and loop point selection. The view can
// optionally doubled at the loop slice points. The resampler interpolates
// the frames with the specific quality. Todo: sinc not working correctly

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	WAVEBOX_DRAG_NONE,
	WAVEBOX_DRAG_LEFT,
	WAVEBOX_DRAG_RIGHT,
	WAVEBOX_DRAG_MOVE,
	WAVEBOX_DRAG_LOOP_CONT_LEFT,
	WAVEBOX_DRAG_LOOP_CONT_RIGHT,
	WAVEBOX_DRAG_LOOP_SUSTAIN_LEFT,
	WAVEBOX_DRAG_LOOP_SUSTAIN_RIGHT
} WaveBoxDragMode;

typedef enum {
	WAVEBOX_LOOPVIEW_CONT_SINGLE,
	WAVEBOX_LOOPVIEW_CONT_DOUBLE,
	WAVEBOX_LOOPVIEW_SUSTAIN_SINGLE,
	WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE
} WaveBoxLoopViewMode;

typedef struct {
	bool hasselection;
	uintptr_t start;
	uintptr_t end;
} WaveBoxSelection;

void waveboxselection_init(WaveBoxSelection*);
void waveboxselection_setrange(WaveBoxSelection*, uintptr_t start,
	uintptr_t end);
void waveboxselection_setstart(WaveBoxSelection*, uintptr_t start, bool* swapped);
void waveboxselection_setend(WaveBoxSelection*, uintptr_t end, bool* swapped);

INLINE uintptr_t waveboxselection_numframes(WaveBoxSelection* self)
{
	return (self->hasselection)
		? self->end - self->start + 1
		: 0;	
}

typedef struct {
	double offsetstep;
	psy_ui_Size size;
	psy_dsp_beat_t zoomleft;
	psy_dsp_beat_t zoomright;
	WaveBoxLoopViewMode loopviewmode;	
	WaveBoxSelection selection;
	WaveBoxSelection oldselection;
	psy_audio_Sample* sample;
	uintptr_t channel;
	psy_ui_Component* component;
	psy_dsp_MultiResampler resampler;	
} WaveBoxContext;

void waveboxcontext_init(WaveBoxContext*, psy_ui_Component* component);
void waveboxcontext_setsample(WaveBoxContext*, psy_audio_Sample*,
	uintptr_t channel);

INLINE psy_audio_Sample* waveboxcontext_sample(WaveBoxContext* self)
{
	return self->sample;
}

void waveboxcontext_setsize(WaveBoxContext*, const psy_ui_Size*);
void waveboxcontext_setzoom(WaveBoxContext*, float zoomleft,
	float zoomright);
uintptr_t waveboxcontext_numframes(WaveBoxContext*);
int waveboxcontext_frametoscreen(WaveBoxContext*, uintptr_t frame);
uintptr_t waveboxcontext_realframe(WaveBoxContext*, uintptr_t frame);
void waveboxcontext_setselection(WaveBoxContext*, uintptr_t selectionstart,
	uintptr_t selectionend);
void waveboxcontext_clearselection(WaveBoxContext*);
void waveboxcontext_refreshdisplay(WaveBoxContext*);
psy_dsp_amp_t waveboxcontext_frame_at(WaveBoxContext*, float frame);

typedef struct {	
	psy_ui_Component component;	
	WaveBoxDragMode dragmode;
	bool dragstarted;
	intptr_t dragoffset;
	psy_Signal selectionchanged;
	char* nowavetext;
	bool preventdrawonselect;	
	bool drawline;	
	WaveBoxContext context;
} WaveBox;

void wavebox_init(WaveBox*, psy_ui_Component* parent, Workspace*);
WaveBox* wavebox_alloc(void);
WaveBox* wavebox_allocinit(psy_ui_Component* parent, Workspace*);

void wavebox_setnowavetext(WaveBox*, const char* text);
void wavebox_setsample(WaveBox*, psy_audio_Sample*, uintptr_t channel);
void wavebox_setzoom(WaveBox*, psy_dsp_beat_t left, psy_dsp_beat_t right);
void wavebox_setselection(WaveBox*, uintptr_t selectionstart,
	uintptr_t selectionend);
void wavebox_clearselection(WaveBox*);
void wavebox_refresh(WaveBox*);
void wavebox_setquality(WaveBox*, psy_dsp_ResamplerQuality);


INLINE WaveBoxContext* wavebox_metric(WaveBox* self)
{
	return &self->context;
}

INLINE bool wavebox_hasselection(WaveBox* self)
{
	return self->context.selection.hasselection != FALSE;
}

INLINE void wavebox_setloopviewmode(WaveBox* self, WaveBoxLoopViewMode mode)
{
	self->context.loopviewmode = mode;
}

INLINE void wavebox_drawlines(WaveBox* self)
{
	self->drawline = TRUE;
}

INLINE void wavebox_drawbars(WaveBox* self)
{
	self->drawline = FALSE;
}

INLINE psy_audio_Sample* wavebox_sample(WaveBox* self)
{
	return self->context.sample;
}

#ifdef __cplusplus
}
#endif

#endif
