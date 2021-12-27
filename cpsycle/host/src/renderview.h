/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(RENDERVIEW_H)
#define RENDERVIEW_H

/* host */
#include "propertiesview.h"
#include "renderprogressview.h"
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uitextinput.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* View for saving a song as a Windows PCM Wav. */

typedef struct RenderView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	PropertiesView view;	
	RenderProgressView progressview;
	psy_Property* properties;	
	psy_Property* driverconfigure;
	psy_AudioDriver* fileoutdriver;		
	int restoreloopmode;
	psy_dsp_DitherSettings restoredither;
	bool restoredodither;
	/* references */
	psy_AudioDriver* curraudiodriver;
	Workspace* workspace;
} RenderView;

void renderview_init(RenderView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* RENDERVIEW_H */
