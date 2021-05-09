// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MAINSTATUSBAR_H)
#define MAINSTATUSBAR_H

// platform
#include "../../detail/os.h"
// host
#include "clockbar.h"
#include "interpreter.h"
#include "workspace.h"
#include "zoombox.h"
// ui
#include <uilabel.h>
#include <uibutton.h>
#include <uinotebook.h>
#include <uiprogressbar.h>

#ifdef __cplusplus
extern "C" {
#endif

// MainStatusBar

typedef enum {
	TERMINALMSGTYPE_ERROR,
	TERMINALMSGTYPE_WARNING,
	TERMINALMSGTYPE_MESSAGE,
	TERMINALMSGTYPE_NONE,
	TERMINALMSGTYPE_NUM
} TerminalMsgType;

typedef struct MainStatusBar {
	// inherits
	psy_ui_Component component;
	// internal
	psy_ui_ProgressBar progressbar;
	psy_ui_Button toggleterminal;	
	psy_ui_Button togglekbdhelp;
	psy_ui_Button turnoff;
	ClockBar clockbar;
	psy_ui_Notebook viewstatusbars;
	psy_ui_Label statusbarlabel;
	ZoomBox zoombox;
	uintptr_t terminalstyleid;
	// references
	Workspace* workspace;
} MainStatusBar;

void mainstatusbar_init(MainStatusBar*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* mainstatusbar_base(MainStatusBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINSTATUSBAR_H */
