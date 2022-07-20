/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(RANGEEDIT_H)
#define RANGEEDIT_H

/* ui */
#include <uilabel.h>
#include <uitextarea.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float realedit_real_t;

typedef struct RangeEdit {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	psy_ui_Label from;
	psy_ui_TextArea edit_from;
	psy_ui_Label to;
	psy_ui_TextArea edit_to;	
} RangeEdit;

void rangeedit_init(RangeEdit*, psy_ui_Component* parent);

RangeEdit* rangeedit_alloc(void);
RangeEdit* rangeedit_allocinit(psy_ui_Component* parent);
RangeEdit* rangeedit_allocinit_range(psy_ui_Component* parent, psy_Property* from,
	psy_Property* to);

void rangeedit_data_exchange(RangeEdit*, psy_Property* from, psy_Property* to);

INLINE psy_ui_Component* rangeedit_base(RangeEdit* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* RANGEEDIT_H */
