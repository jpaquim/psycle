// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(INTEDIT_H)
#define INTEDIT_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uiedit.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IntEdit {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_changed;
	// ui elements
	psy_ui_Label desc;
	psy_ui_Edit edit;
	psy_ui_Button less;
	psy_ui_Button more;
	// internal data
	int minval;
	int maxval;
	int restore;	
} IntEdit;

void intedit_init(IntEdit*, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval);
void intedit_init_connect(IntEdit*, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval,
	void* context, void* fp);

IntEdit* intedit_alloc(void);
IntEdit* intedit_allocinit(psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval);

int intedit_value(IntEdit*);
void intedit_setvalue(IntEdit*, int value);
void intedit_enableedit(IntEdit*);
void intedit_preventedit(IntEdit*);
void intedit_seteditcharnumber(IntEdit*, int charnumber);
void intedit_setdesccharnumber(IntEdit*, int charnumber);

INLINE psy_ui_Component* intedit_base(IntEdit* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* INTEDIT_H */
