/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(EFFECTUI_H)
#define EFFECTUI_H

/* host */
#include "machineui.h"
#include "paramviews.h"
/* ui */
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EffectUi */
typedef struct EffectUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component mute;
	psy_ui_Component bypass;
	EditnameUi editname;
	PanUi pan;
	VuUi vu;
	bool preventmachinepos;
	/* references */
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	ParamViews* paramviews;	
} EffectUi;

void effectui_init(EffectUi*, psy_ui_Component* parent, uintptr_t slot,
	ParamViews*, psy_audio_Machines*);

#ifdef __cplusplus
}
#endif

#endif /* EFFECTUI_H */
