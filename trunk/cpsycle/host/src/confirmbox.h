/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CONFIRMBOX_H)
#define CONFIRMBOX_H

/* host */
#include "viewindex.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CONFIRM_YES = 1,
	CONFIRM_NO = 2,
	CONFIRM_CONTINUE = 4
} ConfirmBoxButton;

typedef struct ConfirmBox {
	/* inherits */
	psy_ui_Component component;		
	/* callbacks */
	psy_Slot slot_accept;
	psy_Slot slot_reject;
	psy_Slot slot_cont;
	/* internal */
	psy_ui_Component view;
	psy_ui_Label title;
	psy_ui_Label header;
	psy_ui_Button yes;
	psy_ui_Button no;
	psy_ui_Button cont;	
} ConfirmBox;

void confirmbox_init(ConfirmBox*, psy_ui_Component* parent);

void confirmbox_set_labels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr);	
void confirmbox_set_callbacks(ConfirmBox*, psy_Slot accept, psy_Slot reject,
	psy_Slot cont);

INLINE psy_ui_Component* confirmbox_base(ConfirmBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIRMBOX */
