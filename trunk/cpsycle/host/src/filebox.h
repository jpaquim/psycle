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

/* FileEdit */

typedef struct FileEdit {
	/* inherits */
	psy_ui_Component component;
	/* inrernal */	
	psy_ui_TextArea edit;
	psy_ui_Button dialog;
	/* references */
	psy_Property* property;
} FileEdit;

void fileedit_init(FileEdit*, psy_ui_Component* parent);

FileEdit* fileedit_alloc(void);
FileEdit* fileedit_allocinit(psy_ui_Component* parent);

void fileedit_data_exchange(FileEdit*, psy_Property*);

INLINE psy_ui_Component* fileedit_base(FileEdit* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* FILEBOX_H */
