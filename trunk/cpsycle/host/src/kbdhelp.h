// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(KBDHELP_H)
#define KBDHELP_H

#include "kbdbox.h"
#include <uitabbar.h>
#include "workspace.h"

#include <uieditor.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: keyboard shortcut helpview for general and note commands.

typedef struct KbdHelp {
	psy_ui_Component component;
	psy_ui_TabBar tabbar;
	KbdBox kbdbox;	
	Workspace* workspace;
} KbdHelp;

void kbdhelp_init(KbdHelp*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* kbdhelp_base(KbdHelp* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* KBDHELP_H */
