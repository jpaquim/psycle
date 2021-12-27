/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MINMAXIMIZE_H)
#define MINMAXIMIZE_H

#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MinMaximize 
** 
** The host minimizes or maximizes the mainview with default keydefine Ctrl + Tab 
** This the list of the components that gets hidden when maximized
*/

struct psy_ui_Component;

typedef struct MinMaximize {	
	psy_List* minmaximize;
	struct psy_ui_Component* view;
} MinMaximize;

void minmaximize_init(MinMaximize*, struct psy_ui_Component* view);
void minmaximize_dispose(MinMaximize*);

/* adds a component to hide at maximized */
void minmaximize_add(MinMaximize*, struct psy_ui_Component*);
/* toggles the visibility of the stored components */
void minmaximize_toggle(MinMaximize*);

#ifdef __cplusplus
}
#endif

#endif /* MINMAXIMIZE_H */
