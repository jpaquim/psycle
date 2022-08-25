/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTRACK_H)
#define SEQEDITTRACK_H

/* host */
#include "seqeditorentry.h"
#include "seqeditorstate.h"
#include "sequencehostcmds.h"
#include "sequencetrackbox.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditTrack */
struct SeqEditTrack;
struct SeqEditorTracks;

typedef struct SeqEditTrack {
	psy_ui_Component component;	
	psy_audio_SequenceTrack* currtrack;	
	uintptr_t trackindex;
	SeqEditState* state;		
	//psy_ui_Component* view;
	psy_List* entries;			
} SeqEditTrack;

void seqedittrack_init(SeqEditTrack*, psy_ui_Component* parent,
	SeqEditState*, psy_audio_SequenceTrack*, uintptr_t trackindex);	
void seqedittrack_dispose(SeqEditTrack*);

SeqEditTrack* seqedittrack_alloc(void);
SeqEditTrack* seqedittrack_allocinit(psy_ui_Component* parent,
	SeqEditState*, psy_audio_SequenceTrack*, uintptr_t trackindex);


INLINE psy_ui_Component* seqedittrack_base(SeqEditTrack* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTRACK_H */
