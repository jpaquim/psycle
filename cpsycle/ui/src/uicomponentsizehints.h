/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMPONENTSIZEHINTS_H
#define psy_ui_COMPONENTSIZEHINTS_H

/* local */
#include "uigeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_SizeHints {
	psy_ui_Size preferredsize;	
	psy_ui_Size minsize;
	psy_ui_Size maxsize;
} psy_ui_SizeHints;

void psy_ui_sizehints_init(psy_ui_SizeHints*);

psy_ui_SizeHints* psy_ui_sizehints_alloc(void);
psy_ui_SizeHints* psy_ui_sizehints_allocinit(void);
void psy_ui_sizehints_deallocate(psy_ui_SizeHints*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSIZEHINTS_H */
