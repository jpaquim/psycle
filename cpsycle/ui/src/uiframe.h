/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_FRAME_H
#define psy_ui_FRAME_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef psy_ui_Component psy_ui_Frame;

void psy_ui_frame_init(psy_ui_Frame*, psy_ui_Component* parent);
void psy_ui_toolframe_init(psy_ui_Frame* self, psy_ui_Component* parent);
void psy_ui_frame_init_main(psy_ui_Frame*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_FRAME_H */
