/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(EXPORTVIEW_H)
#define EXPORTVIEW_H

/* host */
#include "propertiesview.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include <uitextarea.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Module export view */

typedef struct ExportView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	PropertiesView view;	
	psy_Property* properties;
	/* references */
	Workspace* workspace;	
} ExportView;

void exportview_init(ExportView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* EXPORTVIEW_H */
