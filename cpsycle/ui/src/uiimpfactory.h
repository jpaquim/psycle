// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_IMPFACTORY_H
#define psy_ui_IMPFACTORY_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_componentimp(struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_LabelImp* psy_ui_impfactory_allocinit_labelimp(struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp(struct psy_ui_Component*, struct psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_IMPFACTORY_H */
