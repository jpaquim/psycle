// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(ENVELOPEVIEW_H)
#define ENVELOPEVIEW_H

#include "uicomponent.h"
#include "list.h"
#include <adsr.h>

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
	ui_margin spacing;	
	psy_dsp_ADSRSettings dummysettings;
	psy_dsp_ADSRSettings* adsr;
	ADSRPointMapper pointmapper;
} EnvelopeView;

void InitEnvelopeView(EnvelopeView*, psy_ui_Component* parent);
void EnvelopeViewSetAdsrEnvelope(EnvelopeView*, psy_dsp_ADSRSettings*);
void EnvelopeViewUpdate(EnvelopeView*);

#endif
