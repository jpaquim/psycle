// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCETRACKBOX_H)
#define SEQUENCETRACKBOX_H

// host
#include "trackbox.h"
// ui
#include <uiedit.h>
// audio
#include <sequence.h>

#ifdef __cplusplus
extern "C" {
#endif

// SequenceTrackBox
//
// Discribes and controls a sequence track(used by SequenceView and SeqEditor).

typedef struct SequenceTrackBox {
	// inherits
	TrackBox trackbox;
	uintptr_t trackidx;
	// references
	psy_audio_Sequence* sequence;
	psy_ui_Edit* edit;
	bool preventedit;	
} SequenceTrackBox;

void sequencetrackbox_init(SequenceTrackBox*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence*, uintptr_t trackidx,
	psy_ui_Edit*);

SequenceTrackBox* sequencetrackbox_alloc(void);
SequenceTrackBox* sequencetrackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence*, uintptr_t trackidx,
	psy_ui_Edit*);

void sequencetrackbox_showtrackname(SequenceTrackBox*);

INLINE psy_ui_Component* sequencetrackbox_base(SequenceTrackBox* self)
{
	return &self->trackbox.component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCETRACKBOX_H */
