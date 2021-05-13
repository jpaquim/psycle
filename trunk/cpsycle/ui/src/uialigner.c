/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uialigner.h"
/* std */
#include <math.h>
#include <stdio.h>

/* prototypes */
static void psy_ui_aligner_dispose_virtual(psy_ui_Aligner* self) { }
static void psy_ui_aligner_align_virtual(psy_ui_Aligner* self) { }
static void psy_ui_aligner_preferredsize_virtual(psy_ui_Aligner* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
}
/* vtable */
static psy_ui_AlignerVTable aligner_vtable;
static bool aligner_vtable_initialized = FALSE;

static void aligner_vtable_init(void)
{
	if (!aligner_vtable_initialized) {
		aligner_vtable.dispose = psy_ui_aligner_dispose_virtual;
		aligner_vtable.align = psy_ui_aligner_align_virtual;
		aligner_vtable.preferredsize = psy_ui_aligner_preferredsize_virtual;
		aligner_vtable_initialized = TRUE;
	}	
}
/* implementation */
void psy_ui_aligner_init(psy_ui_Aligner* self)
{
	aligner_vtable_init();
	self->vtable = &aligner_vtable;
}
