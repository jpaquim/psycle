/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MASTERUI_H)
#define MASTERUI_H

/* host */
#include "machineui.h" /* vst view */
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MasterUi */
typedef struct MasterUi {
	/* inherits */
	psy_ui_Component component;
	bool preventmachinepos;
	/* internal */	
	psy_audio_Machine* machine;
	ParamViews* paramviews;
} MasterUi;

void masterui_init(MasterUi*, psy_ui_Component* parent, ParamViews*,
	Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* MASTERUI_H */
