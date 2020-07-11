// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(KBDHELP_H)
#define KBDHELP_H

#include "kbdbox.h"
#include "tabbar.h"
#include "workspace.h"

#include <uieditor.h>
#include <uinotebook.h>

// aim: keyboard shortcut helpview for general and note commands.

typedef struct KbdHelp {
	psy_ui_Component component;
	TabBar tabbar;
	KbdBox kbdbox;	
	Workspace* workspace;
	psy_Properties* search;
} KbdHelp;

void kbdhelp_init(KbdHelp*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* kbdhelp_base(KbdHelp* self) { return &self->component; }

#endif /* KBDHELP_H */
