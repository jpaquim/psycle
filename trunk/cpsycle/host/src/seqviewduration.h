/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQVIEWDURATION_H)
#define SEQVIEWDURATION_H

/* host */
#include "seqviewstate.h"
/* ui */
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SeqView;

/* SeqviewDuration */
typedef struct SeqviewDuration {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label desc;
	psy_ui_Label duration;			
	psy_dsp_big_seconds_t duration_ms;
	psy_dsp_big_beat_t duration_bts;
	bool calcduration;
	/* references */
	Workspace* workspace;
} SeqviewDuration;

void seqviewduration_init(SeqviewDuration*, psy_ui_Component* parent,
	Workspace*);
void seqviewduration_update(SeqviewDuration*, bool force);
void seqviewduration_stopdurationcalc(SeqviewDuration*);

INLINE psy_ui_Component* seqviewduration_base(SeqviewDuration* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQVIEWDURATION_H */
