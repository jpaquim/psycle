// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiborder.h"

psy_ui_Size psy_ui_border_size(const psy_ui_Border* self)
{
	psy_ui_Size rv;
	psy_ui_RealSize maxsize;

	psy_ui_realsize_init(&maxsize);
	if (self->left == psy_ui_BORDER_SOLID) {
		++(maxsize.width);
	}
	if (self->top == psy_ui_BORDER_SOLID) {
		++(maxsize.height);
	}
	if (self->right == psy_ui_BORDER_SOLID) {
		++(maxsize.width);
	}
	if (self->bottom == psy_ui_BORDER_SOLID) {
		++(maxsize.height);
	}
	return psy_ui_size_makereal(maxsize);	
}
