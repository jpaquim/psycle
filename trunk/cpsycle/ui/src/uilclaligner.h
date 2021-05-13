/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_LCLALIGNER_H
#define psy_ui_LCLALIGNER_H

/* local */
#include "uialigner.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_ui_LCLAligner
**
**  The psy_ui_LCLAligner works pretty much like in Lazarus/Delphi and can be
**  used to quickly fill an area and is the default aligner in psycle. Use
**  psy_ui_component_setalign to control the placement.
**  (see: https://wiki.freepascal.org/Autosize_/_Layout#Align)
*/

struct psy_ui_Component;

typedef struct psy_ui_LCLAligner {
	psy_ui_Aligner aligner;
	struct psy_ui_Component* component;
} psy_ui_LCLAligner;

void psy_ui_lclaligner_init(psy_ui_LCLAligner*, struct psy_ui_Component*);

INLINE psy_ui_Aligner* psy_ui_lclaligner_base(psy_ui_LCLAligner* self)
{
	return &self->aligner;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LCLALIGNER_H */
