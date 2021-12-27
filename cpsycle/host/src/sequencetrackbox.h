/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQUENCETRACKBOX_H)
#define SEQUENCETRACKBOX_H

/* host */
#include "trackbox.h"
#include "workspace.h"
/* ui */
#include <uitextinput.h>
/* audio */
#include <sequence.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** SequenceTrackBox
**
**
** Describes and controls a sequence track(used by SeqView and SeqEditor).
*/

typedef struct SequenceTrackBox {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_resize;
	/* internal */
	TrackBox trackbox;
	/* internal */
	uintptr_t trackidx;
	bool preventedit;
	/* references */
	psy_audio_Sequence* sequence;
	Workspace* workspace;
} SequenceTrackBox;

void sequencetrackbox_init(SequenceTrackBox*, psy_ui_Component* parent,
	psy_audio_Sequence*, uintptr_t trackidx, Workspace* workspace);

SequenceTrackBox* sequencetrackbox_alloc(void);
SequenceTrackBox* sequencetrackbox_allocinit(psy_ui_Component* parent,
	psy_audio_Sequence*, uintptr_t trackidx, Workspace*);

void sequencetrackbox_showtrackname(SequenceTrackBox*);

INLINE psy_ui_Component* sequencetrackbox_base(SequenceTrackBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCETRACKBOX_H */
