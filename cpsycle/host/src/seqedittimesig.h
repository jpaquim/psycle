/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTIMESIG_H)
#define SEQEDITTIMESIG_H

/* host */
#include "seqeditorstate.h"
#include "workspace.h"
/* ui */
#include <uiedit.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditTimesig */
typedef struct SeqEditTimeSig {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	bool preventedit;
	bool editnominator;
	/* references */
	SeqEditState* state;
	psy_audio_PatternEvent* currtimesig;
} SeqEditTimeSig;

void seqedittimesig_init(SeqEditTimeSig*, psy_ui_Component* parent,
	SeqEditState*);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTIMESIG_H */
