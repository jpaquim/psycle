// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "kbdhelp.h"
#include "inputmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void kbdhelp_init(KbdHelp* self, psy_ui_Component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(kbdhelp_base(self), parent);		
	psy_ui_component_enablealign(kbdhelp_base(self));
	kbdbox_init(&self->kbdbox, kbdhelp_base(self));	
	psy_ui_component_setalign(kbdbox_base(&self->kbdbox),
		psy_ui_ALIGN_CLIENT);
	kbdhelp_markpatterncmds(self);
}

void kbdhelp_markpatterncmds(KbdHelp* self)
{
	psy_EventDriver* kbd;
	psy_Properties* p;

	kbd = workspace_kbddriver(self->workspace);
	if (kbd) {
		p = psy_properties_find(kbd->properties, "notes");
		if (p && p->children) {
			for (p = p->children; p != NULL; p = p->next) {
				uintptr_t keycode;
				int shift;
				int ctrl;

				decodeinput(p->item.value.i, &keycode, &shift, &ctrl);
				kbdbox_setcolor(&self->kbdbox, keycode, 0x00B1C8B0);
				kbdbox_setdescription(&self->kbdbox, keycode, shift,
					ctrl, psy_properties_shorttext(p));
			}
		}
		p = psy_properties_find(self->workspace->cmds, "generalcmds");
		if (p && p->children) {
			for (p = p->children; p != NULL; p = p->next) {
				uintptr_t keycode;
				int shift;
				int ctrl;

				decodeinput(p->item.value.i, &keycode, &shift, &ctrl);
				kbdbox_setcolor(&self->kbdbox, keycode, 0x00D1C5B6);
				kbdbox_setdescription(&self->kbdbox, keycode, shift,
					ctrl, psy_properties_shorttext(p));
			}
		}
	}
}
