/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITHEADER_H)
#define SEQEDITHEADER_H

/* host */
#include "seqeditloops.h"
#include "seqeditruler.h"
#include "seqedittimesig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditHeader */
typedef struct SeqEditHeader {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component pane;
	SeqEditTimeSigs timesig;
	SeqEditLoops loops;
	SeqEditRuler ruler;
	/* references */
	SeqEditState* state;
} SeqEditHeader;

void seqeditheader_init(SeqEditHeader*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditheader_hide_timesig(SeqEditHeader*);
void seqeditheader_show_timesig(SeqEditHeader*);
void seqeditheader_hide_loops(SeqEditHeader*);
void seqeditheader_show_loops(SeqEditHeader*);


#ifdef __cplusplus
}
#endif

#endif /* SEQEDITHEADER_H */
