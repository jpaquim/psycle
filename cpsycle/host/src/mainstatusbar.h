/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINSTATUSBAR_H)
#define MAINSTATUSBAR_H

/* platform */
#include "../../detail/os.h"
/* host */
#include "clockbar.h"
#include "interpreter.h"
#include "workspace.h"
#include "zoombox.h"
/* ui */
#include <uilabel.h>
#include <uibutton.h>
#include <uinotebook.h>
#include <uiprogressbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MainStatusBar */

typedef struct StatusLabel {
	/* inherits */
	psy_ui_Component component;
	psy_ui_ProgressBar progressbar;
	psy_ui_Label label;
} StatusLabel;

void statuslabel_init(StatusLabel*, psy_ui_Component* parent);

typedef enum {
	TERMINALMSGTYPE_ERROR,
	TERMINALMSGTYPE_WARNING,
	TERMINALMSGTYPE_MESSAGE,
	TERMINALMSGTYPE_NONE,
	TERMINALMSGTYPE_NUM
} TerminalMsgType;

typedef struct MainStatusBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component pane;
	StatusLabel statusbarlabel;	
	psy_ui_Button toggleterminal;	
	psy_ui_Button togglekbdhelp;
	psy_ui_Button turnoff;
	ClockBar clockbar;	
	ZoomBox zoombox;	
	intptr_t pluginscanprogress;	
	char* strbuffer;
	psy_Lock outputlock;
	uintptr_t terminal_button_mode;
	/* references */
	Workspace* workspace;
} MainStatusBar;

void mainstatusbar_init(MainStatusBar*, psy_ui_Component* parent, Workspace*);

void mainstatusbar_update_terminal_button(MainStatusBar*);
void mainstatusbar_set_default_status_text(MainStatusBar*, const char* text);
void mainstatusbar_idle(MainStatusBar*);

INLINE psy_ui_Component* mainstatusbar_base(MainStatusBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINSTATUSBAR_H */
