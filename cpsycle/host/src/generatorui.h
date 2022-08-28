/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(GENERATORUI_H)
#define GENERATORUI_H

/* host */
#include "machineui.h"
#include "paramviews.h"
/* ui */
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GeneratorUi */
typedef struct GeneratorUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component mute;
	psy_ui_Component solo;
	EditnameUi editname;
	PanUi pan;
	VuUi vu;
	bool preventmachinepos;
	/* references */
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	ParamViews* paramviews;	
} GeneratorUi;

void generatorui_init(GeneratorUi*, psy_ui_Component* parent, uintptr_t slot,
	ParamViews* paramviews, psy_audio_Machines*);

#ifdef __cplusplus
}
#endif

#endif /* GENERATORUI_H */
