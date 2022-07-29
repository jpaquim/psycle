/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEEDIT_H)
#define FILEEDIT_H

/* host */
#include "filebox.h"
/* ui */
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
	FileBox* filebox;
} FileEdit;

void fileedit_init(FileEdit*, psy_ui_Component* parent);

FileEdit* fileedit_alloc(void);
FileEdit* fileedit_allocinit(psy_ui_Component* parent);

void fileedit_data_exchange(FileEdit*, psy_Property*);
void fileedit_set_file_box(FileEdit*, FileBox*);

INLINE psy_ui_Component* fileedit_base(FileEdit* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* FILEEDIT_H */
