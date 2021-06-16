/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITLOOPS_H)
#define SEQEDITLOOPS_H

/* host */
#include "seqeditorstate.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditLoops */
typedef struct SeqEditLoops {
	/* inherits */
	psy_ui_Component component;	
	/* signals */
	psy_Signal signal_changed;
	/* internal */	
	bool drag;
	psy_audio_PatternNode* nodebegin;
	psy_audio_PatternNode* nodeend;
	psy_audio_PatternEvent e;
	/* references */
	SeqEditState* state;
} SeqEditLoops;

void seqeditloops_init(SeqEditLoops*, psy_ui_Component* parent,
	SeqEditState*);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITLOOPS_H */
