/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQVIEWTRACKHEADERS_H)
#define SEQVIEWTRACKHEADERS_H

/* host */
#include "sequencetrackbox.h"
#include "seqviewstate.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SeqviewTrackHeaders */
typedef struct SeqviewTrackHeaders {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_trackselected;	
	/* internal */
	psy_ui_Component client;	
	/* references */
	SeqViewState* state;	
} SeqviewTrackHeaders;

void seqviewtrackheaders_init(SeqviewTrackHeaders* self,
	psy_ui_Component* parent, SeqViewState*);

void seqviewtrackheaders_build(SeqviewTrackHeaders*);

INLINE psy_ui_Component* seqviewtrackheader_base(SeqviewTrackHeaders* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQVIEWTRACKHEADERS_H */
