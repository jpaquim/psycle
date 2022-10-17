/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiclipboard.h"
/* platform */
#include "../../detail/portable.h"

void psy_ui_clipboard_init(psy_ui_Clipboard* self)
{
	assert(self);
	
	self->text = NULL;
}

void psy_ui_clipboard_dispose(psy_ui_Clipboard* self)
{
	assert(self);
	
	free(self->text);
	self->text = NULL;
}

void psy_ui_clipboard_set_text(psy_ui_Clipboard* self, const char* text)
{
	assert(self);
	
	psy_strreset(&self->text, text);
}

const char* psy_ui_clipboard_text(const psy_ui_Clipboard* self)
{
	assert(self);
	
	return self->text;
}
