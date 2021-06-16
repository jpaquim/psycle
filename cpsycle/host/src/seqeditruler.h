/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITRULER_H)
#define SEQEDITRULER_H

/* host */
#include "seqeditorstate.h"
#include "workspace.h"
/* ui */
#include <uiedit.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditRuler */
typedef struct SeqEditRuler {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	/* references */
	SeqEditState* state;	
} SeqEditRuler;

void seqeditruler_init(SeqEditRuler*, psy_ui_Component* parent,
	SeqEditState*);

INLINE psy_ui_Component* seqeditruler_base(SeqEditRuler* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITRULER_H */
