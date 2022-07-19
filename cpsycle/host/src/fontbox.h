/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FONTBOX_H)
#define FONTBOX_H

/* host */
#include "intedit.h"
/* ui */
#include <uilabel.h>
#include <uibutton.h>
#include <uicombobox.h>

/* FontBox */

typedef struct FontBox {
	/* inherits */
	psy_ui_Component component;
	/* inrernal */
	psy_ui_ComboBox family;
	IntEdit size;
	psy_ui_Button bold;
	psy_ui_Button unterline;
	psy_ui_Button italic;
	psy_ui_Button dialog;
	/* references */
	psy_Property* property;
} FontBox;

void fontbox_init(FontBox*, psy_ui_Component* parent);

FontBox* fontbox_alloc(void);
FontBox* fontbox_allocinit(psy_ui_Component* parent);

void fontbox_data_exchange(FontBox*, psy_Property*);

INLINE psy_ui_Component* fontbox_base(FontBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* FONTBOX_H */
