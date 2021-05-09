// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainstatusbar.h"
// host
#include "resources/resource.h"
#include "styles.h"

// prototypes
static void mainstatusbar_initviewstatusbars(MainStatusBar*);
static void mainstatusbar_initstatuslabel(MainStatusBar*);
static void mainstatusbar_initturnoffbutton(MainStatusBar*);
static void mainstatusbar_initclockbar(MainStatusBar*);
static void mainstatusbar_initkbdhelpbutton(MainStatusBar*);
static void mainstatusbar_initterminalbutton(MainStatusBar*);
static void mainstatusbar_initprogressbar(MainStatusBar*);
// implementation
void mainstatusbar_init(MainStatusBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_setstyletype(&self->component, STYLE_STATUSBAR);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.25, 0.0));
	mainstatusbar_initstatuslabel(self);
	mainstatusbar_initviewstatusbars(self);
	mainstatusbar_initturnoffbutton(self);
	mainstatusbar_initclockbar(self);
	mainstatusbar_initkbdhelpbutton(self);
	mainstatusbar_initterminalbutton(self);
	mainstatusbar_initprogressbar(self);
}

void mainstatusbar_initstatuslabel(MainStatusBar* self)
{
	psy_ui_label_init(&self->statusbarlabel, &self->component, NULL);
	psy_ui_label_preventtranslation(&self->statusbarlabel);
	psy_ui_label_settext(&self->statusbarlabel, "Ready");
	psy_ui_label_preventwrap(&self->statusbarlabel);
	psy_ui_label_setcharnumber(&self->statusbarlabel, 40.0);
}

void mainstatusbar_initviewstatusbars(MainStatusBar* self)
{
	psy_ui_notebook_init(&self->viewstatusbars, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->viewstatusbars),
		psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setdefaultalign(
		psy_ui_notebook_base(&self->viewstatusbars),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
}

void mainstatusbar_initturnoffbutton(MainStatusBar* self)
{
	psy_ui_button_init_text(&self->turnoff, &self->component,
		NULL, "main.exit");
	psy_ui_component_setalign(psy_ui_button_base(&self->turnoff),
		psy_ui_ALIGN_RIGHT);		
}

void mainstatusbar_initclockbar(MainStatusBar* self)
{
	clockbar_init(&self->clockbar, &self->component, self->workspace);
	psy_ui_component_setalign(clockbar_base(&self->clockbar),
		psy_ui_ALIGN_RIGHT);		
}

void mainstatusbar_initkbdhelpbutton(MainStatusBar* self)
{	
	psy_ui_Margin margin;

	psy_ui_button_init_text(&self->togglekbdhelp, &self->component,
		NULL, "main.kbd");
	margin = psy_ui_component_margin(psy_ui_button_base(&self->togglekbdhelp));
	margin.right = psy_ui_value_make_ew(4.0);
	psy_ui_component_setmargin(psy_ui_button_base(&self->togglekbdhelp),
		 margin);
	psy_ui_component_setalign(psy_ui_button_base(&self->togglekbdhelp),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_button_setbitmapresource(&self->togglekbdhelp, IDB_KBD);
	psy_ui_button_setbitmaptransparency(&self->togglekbdhelp,
		psy_ui_colour_white());	
}

void mainstatusbar_initterminalbutton(MainStatusBar* self)
{	
	self->terminalstyleid = STYLE_TERM_BUTTON;	
	psy_ui_button_init_text(&self->toggleterminal, &self->component,
		NULL, "Terminal");
	psy_ui_component_setalign(psy_ui_button_base(&self->toggleterminal),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_button_setbitmapresource(&self->toggleterminal, IDB_TERM);
	psy_ui_button_setbitmaptransparency(&self->toggleterminal,
		psy_ui_colour_white());
}

void mainstatusbar_initprogressbar(MainStatusBar* self)
{
	psy_ui_progressbar_init(&self->progressbar, &self->component, NULL);
	psy_ui_component_setalign(progressbar_base(&self->progressbar),
		psy_ui_ALIGN_RIGHT);	
}
