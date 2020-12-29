// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_NOTEBOOK_H
#define psy_ui_NOTEBOOK_H

// local
#include "uisplitbar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Notebook {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_SplitBar splitbar;
	// internal data;
	uintptr_t pageindex;
	int split;
} psy_ui_Notebook;

void psy_ui_notebook_init(psy_ui_Notebook*, psy_ui_Component* parent);
void psy_ui_notebook_select(psy_ui_Notebook*, uintptr_t page);
uintptr_t psy_ui_notebook_pageindex(psy_ui_Notebook*);
psy_ui_Component* psy_ui_notebook_activepage(psy_ui_Notebook*);
void psy_ui_notebook_split(psy_ui_Notebook*, psy_ui_Orientation);
int psy_ui_notebook_splitactivated(psy_ui_Notebook*);
void psy_ui_notebook_full(psy_ui_Notebook*);
void psy_ui_notebook_connectcontroller(psy_ui_Notebook*, 
	psy_Signal* controllersignal);

INLINE psy_ui_Component* psy_ui_notebook_base(psy_ui_Notebook* self)
{
	assert(self);

	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_NOTEBOOK_H */
