/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERCOLUMN_H)
#define TRACKERCOLUMN_H

/* host */
#include "trackergridstate.h"
#include "workspace.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TrackerColumn */
typedef struct TrackerColumn {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	uintptr_t track;
	psy_ui_RealSize digitsize;		
	psy_ui_RealSize size;
	psy_ui_RealSize line_size;
	psy_ui_Colour draw_restore_fg_colour;
	psy_ui_Colour draw_restore_bg_colour;
	/* references */
	TrackerState* state;	
	Workspace* workspace;
} TrackerColumn;

void trackercolumn_init(TrackerColumn*, psy_ui_Component* parent,
	uintptr_t index, TrackerState*, Workspace*);

TrackerColumn* trackercolumn_alloc(void);
TrackerColumn* trackercolumn_allocinit(psy_ui_Component* parent,
	uintptr_t index, TrackerState*, Workspace*);

INLINE psy_ui_Component* trackercolumn_base(TrackerColumn* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERCOLUMN_H */
