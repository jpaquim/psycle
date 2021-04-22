// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_GridALIGNER_H
#define psy_ui_GridALIGNER_H

// local
#include "uialigner.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_ui_GridAligner
//

struct psy_ui_Component;

typedef struct psy_ui_GridAligner {
	psy_ui_Aligner aligner;
	struct psy_ui_Component* component;
	uintptr_t numcols;
} psy_ui_GridAligner;

void psy_ui_gridaligner_init(psy_ui_GridAligner*, struct psy_ui_Component*);

INLINE psy_ui_Aligner* psy_ui_gridaligner_base(psy_ui_GridAligner* self)
{
	return &self->aligner;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GridALIGNER_H */
