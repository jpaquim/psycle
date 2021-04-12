// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMPONENTSIZEHINTS_H
#define psy_ui_COMPONENTSIZEHINTS_H

// local
#include "uigeometry.h"

typedef struct psy_ui_ComponentSizeHints {
	psy_ui_Margin margin;
	psy_ui_Margin spacing;
	psy_ui_Size preferredsize;
	psy_ui_Size minsize;
	psy_ui_Size maxsize;
} psy_ui_ComponentSizeHints;

void psy_ui_componentsizehints_init(psy_ui_ComponentSizeHints*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSIZEHINTS_H */
