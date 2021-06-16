/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTRACKDESC_H)
#define SEQEDITTRACKDESC_H

/* host */
#include "seqeditorstate.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditTrackDesc */
typedef struct SeqEditTrackDesc {
	/* inherits */
	psy_ui_Component component;	
	/* signals */
	psy_Signal signal_resize;
	/* references */
	SeqEditState* state;
	Workspace* workspace;	
} SeqEditTrackDesc;

void seqedittrackdesc_init(SeqEditTrackDesc*, psy_ui_Component* parent,
	SeqEditState*, Workspace*);

void seqedittrackdesc_build(SeqEditTrackDesc*);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTRACKDESC_H */
