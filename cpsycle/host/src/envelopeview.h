// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(ENVELOPEVIEW_H)
#define ENVELOPEVIEW_H

#include "uicomponent.h"
#include "list.h"
#include <adsr.h>

typedef struct {	
	EnvelopePoint start;
	EnvelopePoint attack;
	EnvelopePoint decay;
	EnvelopePoint release;
	ADSRSettings* settings;
} ADSRPointMapper;

typedef struct {	
	ui_component component;
	int cx;
	int cy;
	List* points;
	List* dragpoint;
	int sustainstage;
	int dragrelative;
	ui_margin spacing;	
	ADSRSettings dummysettings;
	ADSRSettings* adsr;
	ADSRPointMapper pointmapper;
} EnvelopeView;

void InitEnvelopeView(EnvelopeView*, ui_component* parent);
void EnvelopeViewSetAdsrEnvelope(EnvelopeView*, ADSRSettings*);
void EnvelopeViewUpdate(EnvelopeView*);


#endif
