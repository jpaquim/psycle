/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(ARROWUI_H)
#define ARROWUI_H

/* host */
#include "machineui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ArrowUi */
typedef struct ArrowUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_audio_Wire wire;	
	Workspace* workspace;
} ArrowUi;

void arrowui_init(ArrowUi*, psy_ui_Component* parent,	
	psy_audio_Wire, Workspace*);

ArrowUi* arrowui_alloc(void);
ArrowUi* arrowui_allocinit(psy_ui_Component* parent,
	psy_audio_Wire, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* ARROWUI_H */
