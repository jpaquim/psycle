// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(INTERPOLATECURVEVIEW_H)
#define INTERPOLATECURVEVIEW_H

#include "workspace.h"

#include <uibutton.h>
#include <uicheckbox.h>
#include <uicombobox.h>
#include <uiedit.h>

#include <list.h>

#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: specifies how the values of the parameters column will change from line
//      to line. Clicking the 'Interpolate' Button fills the parameters column
//      from the beginning to the end of a selection according to the curve in
//      this view.

typedef enum {
	INTERPOLATECURVETYPE_LINEAR,
	INTERPOLATECURVETYPE_HERMITE
} InterpolateCurveType;

typedef struct {
	psy_ui_Component component;
	psy_ui_Button checktwk;
	psy_ui_ComboBox combotwk;
	psy_ui_Edit pos;
	psy_ui_Edit value;
	psy_ui_Edit min;
	psy_ui_Edit max;
	psy_ui_ComboBox curvetype;
	psy_ui_Button ok;
	psy_ui_Button cancel;
} InterpolateCurveBar;

void interpolatecurvebar_init(InterpolateCurveBar*, psy_ui_Component* parent,
	Workspace*);

typedef struct KeyFrame {
	psy_dsp_big_beat_t offset;
	int value;
	InterpolateCurveType curve;
} KeyFrame;

INLINE void keyframe_init(KeyFrame* self, psy_dsp_big_beat_t offset, int value,
	InterpolateCurveType curve)
{
	self->offset = offset;
	self->value = value;
	self->curve = curve;
}

KeyFrame* keyframe_alloc(void);
KeyFrame* keyframe_allocinit(psy_dsp_big_beat_t offset, int value,
	InterpolateCurveType curve);

struct InterpolateCurveView;

typedef struct {
	psy_ui_Component component;
	psy_List* keyframes;
	psy_dsp_big_beat_t range;
	intptr_t valuerange;
	intptr_t minval;
	intptr_t maxval;
	psy_audio_Pattern* pattern;
	PatternSelection selection;
	psy_List* dragkeyframe;
	psy_List* selected;
	struct InterpolateCurveView* view;
} InterpolateCurveBox;

void interpolatecurvebox_init(InterpolateCurveBox*, psy_ui_Component* parent, struct InterpolateCurveView* view,
	Workspace*);

typedef struct InterpolateCurveView {
	psy_ui_Component component;	
	InterpolateCurveBox box;
	InterpolateCurveBar bar;
	psy_Signal signal_cancel;
} InterpolateCurveView;

void interpolatecurveview_init(InterpolateCurveView*, psy_ui_Component* parent,
	int startsel, int endsel, int lpb, Workspace*);
void interpolatecurveview_setselection(InterpolateCurveView*,
	PatternSelection);
void interpolatecurveview_setpattern(InterpolateCurveView*,
	psy_audio_Pattern*);

#ifdef __cplusplus
}
#endif

#endif
