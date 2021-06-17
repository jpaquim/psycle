/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITHEADERDESC_H)
#define SEQEDITHEADERDESC_H

/* host */
#include "seqeditorstate.h"
#include "zoombox.h"
/* ui */
#include <uibutton.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SeqEditorHeaderDescBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component top;	
	ZoomBox hzoom;
	psy_ui_Button insert;
	psy_ui_Button del;
	psy_ui_Button up;
	psy_ui_Button down;
	/* references */
	SeqEditState* state;
} SeqEditorHeaderDescBar;

void seqeditorheaderdescbar_init(SeqEditorHeaderDescBar*,
	psy_ui_Component* parent, SeqEditState* state);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITHEADERDESC_H */
