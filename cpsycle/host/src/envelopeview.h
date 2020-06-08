// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(ENVELOPEVIEW_H)
#define ENVELOPEVIEW_H

#include "scrollzoom.h"
#include <uicomponent.h>
#include <list.h>
#include <adsr.h>

// aim: This modifies the ADSR (Attack-Decay-Sustain-Release) envelope for the
//      volume and filters, which is shown on a graph.
//
// todo: add point free form envelopes

typedef struct {	
	psy_dsp_EnvelopePoint start;
	psy_dsp_EnvelopePoint attack;
	psy_dsp_EnvelopePoint decay;
	psy_dsp_EnvelopePoint release;
	psy_dsp_ADSRSettings* settings;
} ADSRPointMapper;

typedef struct {
	psy_ui_Component component;
	int cx;
	int cy;
	psy_List* points;
	psy_List* dragpoint;
	int sustainstage;
	int dragrelative;
	psy_ui_Margin spacing;	
	psy_dsp_ADSRSettings dummysettings;
	psy_dsp_ADSRSettings* adsr;
	ADSRPointMapper pointmapper;
	char* text;
	float zoomleft;
	float zoomright;
} EnvelopeBox;

void envelopebox_init(EnvelopeBox*, psy_ui_Component* parent);
void envelopebox_setadsrenvelope(EnvelopeBox*, psy_dsp_ADSRSettings*);
void envelopebox_update(EnvelopeBox*);
void envelopebox_settext(EnvelopeBox*, const char* text);

typedef struct {
	psy_ui_Component component;
	EnvelopeBox envelopebox;
	ScrollZoom zoom;	
} EnvelopeView;

void envelopeview_init(EnvelopeView*, psy_ui_Component* parent);
void envelopeview_setadsrenvelope(EnvelopeView*, psy_dsp_ADSRSettings*);
void envelopeview_update(EnvelopeView*);
void envelopeview_settext(EnvelopeView*, const char* text);

#endif
