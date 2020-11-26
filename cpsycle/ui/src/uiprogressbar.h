// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_PROGRESSBAR_H
#define psy_ui_PROGRESSBAR_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   psy_ui_Component component;   
   char* text;   
   float progress;
} psy_ui_ProgressBar;

void psy_ui_progressbar_init(psy_ui_ProgressBar*, psy_ui_Component* parent);
void psy_ui_progressbar_settext(psy_ui_ProgressBar*, const char* text);
void psy_ui_progressbar_setprogress(psy_ui_ProgressBar*, float progress);
void psy_ui_progressbar_tick(psy_ui_ProgressBar*);

INLINE psy_ui_Component* progressbar_base(psy_ui_ProgressBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_PROGRESSBAR_H */
