// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(KBDHELP_H)
#define KBDHELP_H

#include "kbdbox.h"
#include "workspace.h"

#include <uieditor.h>
#include <uinotebook.h>

typedef struct KbdHelp {
	psy_ui_Component component;
	KbdBox kbdbox;	
	Workspace* workspace;
} KbdHelp;

void kbdhelp_init(KbdHelp*, psy_ui_Component* parent, Workspace*);

void kbdhelp_markpatterncmds(KbdHelp*);

INLINE psy_ui_Component* kbdhelp_base(KbdHelp* self) { return &self->component; }

#endif /* KBDHELP_H */
