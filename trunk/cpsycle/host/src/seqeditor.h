/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

/* host */
#include "seqeditheader.h"
#include "seqeditheaderdesc.h"
#include "seqeditproperties.h"
#include "seqedittrackdesc.h"
#include "seqedittracks.h"
#include "seqedittoolbar.h"
/* ui */
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SeqEditor {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	SeqEditToolBar toolbar;
	psy_ui_Component spacer;	
	SeqEditHeader header;	
	psy_ui_Scroller scroller;	
	psy_ui_Component left;
	SeqEditorHeaderDescBar headerdescbar;
	ZoomBox vzoom;
	psy_ui_Component trackdescpane;
	SeqEditTrackDesc trackdesc;
	SeqEditorTracks tracks;
	SeqEditProperties properties;
	SeqEditState state;
	SequenceCmds cmds;	
	bool expanded;
	psy_ui_TextArea edit;
} SeqEditor;

void seqeditor_init(SeqEditor*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* seqeditor_base(SeqEditor* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITOR_H */
