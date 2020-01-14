// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIALIGNER_H)
#define UIALIGNER_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component* component;
} psy_ui_Aligner;

void psy_ui_aligner_init(psy_ui_Aligner*, psy_ui_Component*);
void psy_ui_aligner_align(psy_ui_Aligner*);
void psy_ui_aligner_preferredsize(psy_ui_Aligner*, psy_ui_Size* limit,
	psy_ui_Size* rv);

#endif
