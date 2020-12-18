// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	psy_ui_Component component;
	int cx;
	int cy;
	psy_dsp_EnvelopeSettings* settings;
	psy_List* dragpoint;
	int sustainstage;
	int dragrelative;
	psy_ui_Margin spacing;	
	float zoomleft;
	float zoomright;	
} EnvelopeBox;

void envelopebox_init(EnvelopeBox*, psy_ui_Component* parent);
void envelopebox_setenvelope(EnvelopeBox* self, psy_dsp_EnvelopeSettings*);
void envelopebox_update(EnvelopeBox*);

INLINE psy_ui_Component* envelopebox_base(EnvelopeBox* self)
{
	return &self->component;
}

typedef struct EnvelopeBar {
	psy_ui_Component component;
	psy_ui_CheckBox enabled;
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
	// ui elements
	EnvelopeBar bar;
	EnvelopeBox envelopebox;
	ScrollZoom zoom;
	// references
	Workspace* workspace;
} EnvelopeView;

void envelopeview_init(EnvelopeView*, psy_ui_Component* parent, Workspace*);
void envelopeview_setenvelope(EnvelopeView*, psy_dsp_EnvelopeSettings* settings);
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
