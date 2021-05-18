/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANORULER_H)
#define PIANORULER_H

/* host */
#include "pianogridstate.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PianoRuler */
typedef struct {
	/* inherits */
	psy_ui_Component component;	
	/* references */
	PianoGridState* gridstate;
	PianoGridState defaultgridstate;
} PianoRuler;

void pianoruler_init(PianoRuler*, psy_ui_Component* parent, PianoGridState*);
void pianoruler_setsharedgridstate(PianoRuler*, PianoGridState*);

INLINE psy_ui_Component* pianoruler_base(PianoRuler* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PIANORULER_H */
