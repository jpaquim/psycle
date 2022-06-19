/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_PROGRESSBAR_H
#define psy_ui_PROGRESSBAR_H

/* local */
#include "uicomponent.h"

/*
** psy_ui_ProgressBar
**
** psy_ui_Component
**        ^
**        |
** psy_ui_ProgressBar
** 
** Draws a filled rectangle bar indicating the progress of an operation.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_ProgressBar {
	/* inherits */
	psy_ui_Component component;
	/* internal	*/
	double progress;
} psy_ui_ProgressBar;

void psy_ui_progressbar_init(psy_ui_ProgressBar*, psy_ui_Component* parent);

/* set the progress: 0..1 (0% - 100%) */
void psy_ui_progressbar_set_progress(psy_ui_ProgressBar*, double progress);
/* advance progress of 10% */
void psy_ui_progressbar_tick(psy_ui_ProgressBar*);

INLINE psy_ui_Component* progressbar_base(psy_ui_ProgressBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_PROGRESSBAR_H */
