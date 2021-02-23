// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(ENVELOPEVIEW_H)
#define ENVELOPEVIEW_H

// host
#include "scrollzoom.h"
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
// dsp
#include <envelope.h>
// container
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

// This modifies the envelope for volume and filters shown on a graph.

typedef struct {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_tweaked;
	// internal
	double cx;
	double cy;
	psy_dsp_Envelope* settings;
	psy_List* dragpoint;
	uintptr_t dragpointindex;
	int sustainstage;
	int dragrelative;
	psy_ui_Margin spacing;	
	double zoomleft;
	double zoomright;
	psy_dsp_amp_t modamount;
	psy_ui_Size ptsize;
	psy_ui_Size ptsize2;
	psy_ui_Colour pointcolour;
	psy_ui_Colour curvecolour;
	psy_ui_Colour gridcolour;
	psy_ui_Colour sustaincolour;
	psy_ui_Colour rulercolour;
} EnvelopeBox;

void envelopebox_init(EnvelopeBox*, psy_ui_Component* parent);
void envelopebox_setenvelope(EnvelopeBox*, psy_dsp_Envelope*);
void envelopebox_setzoom(EnvelopeBox*, double zoomleft, double zoomright);

INLINE void envelopebox_setmodamount(EnvelopeBox* self, psy_dsp_amp_t amount)
{
	self->modamount = amount;
}

void envelopebox_update(EnvelopeBox*);

INLINE psy_ui_Component* envelopebox_base(EnvelopeBox* self)
{
	return &self->component;
}

typedef struct EnvelopeBar {
	psy_ui_Component component;
	psy_ui_CheckBox enabled;
	psy_ui_CheckBox carry;
	psy_ui_Button millisec;
	psy_ui_Button ticks;
	psy_ui_Button adsr;
} EnvelopeBar;

void envelopebar_init(EnvelopeBar*, psy_ui_Component* parent);
void envelopebar_settext(EnvelopeBar*, const char* text);

INLINE psy_ui_Component* envelopebar_base(EnvelopeBar* self)
{
	return &self->component;
}

typedef struct EnvelopeView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_tweaked;
	// ui elements
	EnvelopeBar bar;
	EnvelopeBox envelopebox;
	ScrollZoom zoom;	
} EnvelopeView;

void envelopeview_init(EnvelopeView*, psy_ui_Component* parent);
void envelopeview_setenvelope(EnvelopeView*, psy_dsp_Envelope* settings);

INLINE void envelopeview_setmodamount(EnvelopeView* self, psy_dsp_amp_t amount)
{	
	envelopebox_setmodamount(&self->envelopebox, amount);
}

void envelopeview_update(EnvelopeView*);
void envelopeview_settext(EnvelopeView*, const char* text);

INLINE psy_ui_Component* envelopeview_base(EnvelopeView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
