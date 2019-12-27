// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UINOTEBOOK_H)
#define UINOTEBOOK_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   psy_ui_Component splitbar;
   int pageindex;
   int split;
   int splitx;
} ui_notebook;

void ui_notebook_init(ui_notebook*, psy_ui_Component* parent);
void ui_notebook_setpageindex(ui_notebook*, int page);
int ui_notebook_pageindex(ui_notebook*);
psy_ui_Component* ui_notebook_activepage(ui_notebook*);
void ui_notebook_split(ui_notebook*);
void ui_notebook_full(ui_notebook*);
void ui_notebook_connectcontroller(ui_notebook*, 
	psy_Signal* controllersignal);

#endif
