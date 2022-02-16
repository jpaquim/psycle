/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_DROPDOWNBOX_H
#define psy_ui_DROPDOWNBOX_H

/* local */
#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DopDownBox */

struct psy_ui_ComponentImp;

typedef struct psy_ui_DropDownBox {
    /* inherits */
    psy_ui_Component component;    
} psy_ui_DropDownBox;

void psy_ui_dropdownbox_init(psy_ui_DropDownBox*, psy_ui_Component* parent);

void psy_ui_dropdownbox_show(psy_ui_DropDownBox*, psy_ui_Component* field);
void psy_ui_dropdownbox_hide(psy_ui_DropDownBox*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DROPDOWNBOX_H */
