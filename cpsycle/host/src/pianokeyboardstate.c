/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianokeyboardstate.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* KeyboardState */
void keyboardstate_init(KeyboardState* self, PatternViewSkin* skin)
{
	assert(self);

	self->keymin = 0;
	self->keymax = 119;
	self->defaultkeyheight = psy_ui_value_make_eh(1.0);
	self->keyheight = self->defaultkeyheight;
	self->keyheightpx = 13;
	self->keyboardheightpx = 13 * (self->keymax - self->keymin);
	self->notemode = psy_dsp_NOTESTAB_A220;
	self->drawpianokeys = TRUE;
	self->skin = skin;
}
