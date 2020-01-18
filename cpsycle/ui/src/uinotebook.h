// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_NOTEBOOK_H
#define psy_ui_NOTEBOOK_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   psy_ui_Component splitbar;
   int pageindex;
   int split;
   int splitx;
} psy_ui_Notebook;

void psy_ui_notebook_init(psy_ui_Notebook*, psy_ui_Component* parent);
void psy_ui_notebook_setpageindex(psy_ui_Notebook*, int page);
int psy_ui_notebook_pageindex(psy_ui_Notebook*);
psy_ui_Component* psy_ui_notebook_activepage(psy_ui_Notebook*);
void psy_ui_notebook_split(psy_ui_Notebook*);
int psy_ui_notebook_splitactivated(psy_ui_Notebook*);
void psy_ui_notebook_full(psy_ui_Notebook*);
void psy_ui_notebook_connectcontroller(psy_ui_Notebook*, 
	psy_Signal* controllersignal);
psy_ui_Component* psy_ui_notebook_base(psy_ui_Notebook*);

#endif /* psy_ui_NOTEBOOK_H */
