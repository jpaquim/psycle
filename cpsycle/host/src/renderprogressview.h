/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(RENDERPROGRESSVIEW_H)
#define RENDERPROGRESSVIEW_H

/* host */
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RenderProgressView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component client;
	psy_ui_Label render;	
	psy_ui_Component abortbar;
	psy_ui_Button abort;
	/* references */
	Workspace* workspace;
} RenderProgressView;

void renderprogressview_init(RenderProgressView*, psy_ui_Component* parent,
	Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* RENDERPROGRESSVIEW_H */
