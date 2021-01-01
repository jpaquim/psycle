// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(REALEDIT_H)
#define REALEDIT_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uiedit.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float realedit_real_t;

typedef struct RealEdit {
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
	realedit_real_t minval;
	realedit_real_t maxval;
	realedit_real_t restore;
} RealEdit;

void realedit_init(RealEdit*, psy_ui_Component* parent,
	const char* desc, realedit_real_t value, realedit_real_t minval, realedit_real_t maxval);
void realedit_init_connect(RealEdit*, psy_ui_Component* parent,
	const char* desc, realedit_real_t value, realedit_real_t minval, realedit_real_t maxval,
	void* context, void* fp);
realedit_real_t realedit_value(RealEdit*);
void realedit_setvalue(RealEdit*, realedit_real_t value);
void realedit_enableedit(RealEdit*);
void realedit_preventedit(RealEdit*);
void realedit_seteditcharnumber(RealEdit*, int charnumber);
void realedit_setdesccharnumber(RealEdit*, int charnumber);

INLINE psy_ui_Component* realedit_base(RealEdit* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* REALEDIT_H */
