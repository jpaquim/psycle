/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(STYLEVIEW_H)
#define STYLEVIEW_H

/* host */
#include "propertiesview.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct StyleViewBar {
	psy_ui_Component component;	
	psy_ui_Button diskop;	
} StyleViewBar;

void styleviewbar_init(StyleViewBar*, psy_ui_Component* parent);


typedef struct StyleView {
	psy_ui_Component component;
	StyleViewBar bar;
	PropertiesView view;
	psy_Property* styles_property;
	Workspace* workspace;
} StyleView;

void styleview_init(StyleView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);


#ifdef __cplusplus
}
#endif

#endif /* STYLEVIEW_H */
