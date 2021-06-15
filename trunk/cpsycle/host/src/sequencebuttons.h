/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQUENCEBUTTONS_H)
#define SEQUENCEBUTTONS_H

/* host */
#include "sequencetrackbox.h"
#include "sequencehostcmds.h"
/* audio */
#include <patterns.h>
#include <sequence.h>
/* ui */
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

/* SequenceButtons */
typedef struct SequenceButtons {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component standard;
	psy_ui_Component row0;
	psy_ui_Button incpattern;
	psy_ui_Button insertentry;
	psy_ui_Button decpattern;
	psy_ui_Component row1;
	psy_ui_Component rowmore;
	psy_ui_Button newentry;		
	psy_ui_Button delentry;
	psy_ui_Button cloneentry;	
	psy_ui_Button more;
	psy_ui_Component block;
	psy_ui_Component row2;
	psy_ui_Button clear;
	psy_ui_Button rename;
	psy_ui_Edit edit;
	/* psy_ui_Button cut; */
	psy_ui_Button copy;
	psy_ui_Component row3;
	psy_ui_Button paste;
	psy_ui_Button singlesel;
	psy_ui_Button multisel;
	/* references */
	SequenceCmds* cmds;
} SequenceButtons;

void sequencebuttons_init(SequenceButtons*, psy_ui_Component* parent,
	SequenceCmds*);

INLINE psy_ui_Component* sequencebuttons_base(SequenceButtons* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCEBUTTONS_H */
