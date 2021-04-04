// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCERBAR_H)
#define SEQUENCERBAR_H

// host
#include "sequencetrackbox.h"
#include "sequencebuttons.h"
// audio
#include <patterns.h>
#include <sequence.h>
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uiscroller.h>
#include <uisplitbar.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SequencerBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_CheckBox followsong;
	psy_ui_CheckBox shownames;	
	psy_ui_CheckBox recordnoteoff;
	psy_ui_CheckBox recordtweak;
	psy_ui_CheckBox multichannelaudition;
	psy_ui_CheckBox allownotestoeffect;
	psy_ui_Component seqedit;
	psy_ui_Button toggleseqedit;	
	psy_ui_Component stepseq;
	psy_ui_Button togglestepseq;	
	// references
	Workspace* workspace;
} SequencerBar;

void sequencerbar_init(SequencerBar* self, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* sequencerbar_base(SequencerBar* self)
{
	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif /* SEQUENCERBAR_H */
