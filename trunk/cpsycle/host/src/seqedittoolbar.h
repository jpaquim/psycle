/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTOOLBAR_H)
#define SEQEDITTOOLBAR_H

/* host */
#include "seqeditorentry.h"
#include "seqeditheader.h"
#include "zoombox.h"
/* ui */
#include <uicombobox.h>
#include <uicheckbox.h>
#include <uitextarea.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditToolBar */
typedef struct SeqEditToolBar {
	/* inherits */
	psy_ui_Component component;	
	psy_ui_Button move;
	psy_ui_Button reorder;
	psy_ui_Label desctype;
	psy_ui_ComboBox inserttype;
	psy_ui_Button assignsample;
	psy_ui_CheckBox usesamplerindex;	
	IntEdit samplerindex;
	psy_ui_Button configure;
	psy_ui_Button expand;
	psy_ui_Button timesig;
	psy_ui_Button loop;
	psy_ui_Label trackname;	
	psy_ui_TextArea trackedit;	
	/* references */	
	SeqEditState* state;
} SeqEditToolBar;

void seqedittoolbar_init(SeqEditToolBar*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditortoolbar_setdragtype(SeqEditToolBar*, SeqEditorDragType);

void seqeditortoolbar_updatetrackname(SeqEditToolBar*);

INLINE psy_ui_Component* seqedittoolbar_base(SeqEditToolBar* self)
{
	assert(self);

	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTOOLBAR_H */
