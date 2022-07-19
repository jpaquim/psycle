/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEBOX_H)
#define FILEBOX_H

/* host */
/* ui */
// #include <uitextarea.h>
#include <uibutton.h>
#include <uitextarea.h>

/* FileBox */

typedef struct FileBox {
	/* inherits */
	psy_ui_Component component;
	/* inrernal */
	psy_ui_Component colour;	
	psy_ui_TextArea edit;
	psy_ui_Button dialog;
	/* references */
	psy_Property* property;
} FileBox;

void filebox_init(FileBox*, psy_ui_Component* parent);

FileBox* filebox_alloc(void);
FileBox* filebox_allocinit(psy_ui_Component* parent);

void filebox_data_exchange(FileBox*, psy_Property*);

INLINE psy_ui_Component* filebox_base(FileBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* FILEBOX_H */
