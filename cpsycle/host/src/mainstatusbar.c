/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "mainstatusbar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"

/* prototypes */
static void mainstatusbar_initzoombox(MainStatusBar*);
static void mainstatusbar_initviewstatusbars(MainStatusBar*);
static void mainstatusbar_initstatuslabel(MainStatusBar*);
static void mainstatusbar_initturnoffbutton(MainStatusBar*);
static void mainstatusbar_initclockbar(MainStatusBar*);
static void mainstatusbar_initkbdhelpbutton(MainStatusBar*);
static void mainstatusbar_initterminalbutton(MainStatusBar*);
static void mainstatusbar_initprogressbar(MainStatusBar*);
static void mainstatusbar_onzoomboxchanged(MainStatusBar*, ZoomBox* sender);
static void mainstatusbar_onsongloadprogress(MainStatusBar*, Workspace* sender,
	intptr_t progress);
static void mainstatusbar_onpluginscanprogress(MainStatusBar*, Workspace*,
	int progress);
static void mainstatusbar_onstatus(MainStatusBar*, Workspace* sender,
	const char* text);
/* implementation */
void mainstatusbar_init(MainStatusBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	self->clockcounter = 20;
	psy_ui_component_setstyletype(&self->component, STYLE_STATUSBAR);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	mainstatusbar_initzoombox(self);
	mainstatusbar_initstatuslabel(self);
	mainstatusbar_initviewstatusbars(self);
	mainstatusbar_initturnoffbutton(self);
	mainstatusbar_initclockbar(self);
	mainstatusbar_initkbdhelpbutton(self);
	mainstatusbar_initterminalbutton(self);
	mainstatusbar_initprogressbar(self);
}

void mainstatusbar_initzoombox(MainStatusBar* self)
{
	zoombox_init(&self->zoombox, &self->component);
	psy_signal_connect(&self->zoombox.signal_changed,
		self, mainstatusbar_onzoomboxchanged);
}

void mainstatusbar_initstatuslabel(MainStatusBar* self)
{
	psy_ui_label_init(&self->statusbarlabel, &self->component, NULL);
	psy_ui_label_preventtranslation(&self->statusbarlabel);
	psy_ui_label_settext(&self->statusbarlabel, "Ready");	
	psy_ui_label_setcharnumber(&self->statusbarlabel, 40.0);
	workspace_connectstatus(self->workspace, self,
		(fp_workspace_output)mainstatusbar_onstatus);
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
	psy_ui_button_loadresource(&self->turnoff, IDB_EXIT_LIGHT,
		IDB_EXIT_DARK, psy_ui_colour_white());
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
	psy_ui_margin_setright(&margin, psy_ui_value_make_ew(4.0));
	psy_ui_component_setmargin(psy_ui_button_base(&self->togglekbdhelp),
		 margin);
	psy_ui_component_setalign(psy_ui_button_base(&self->togglekbdhelp),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_button_loadresource(&self->togglekbdhelp, IDB_KBD, IDB_KBD,
		psy_ui_colour_white());
}

void mainstatusbar_initterminalbutton(MainStatusBar* self)
{	
	self->terminalstyleid = STYLE_TERM_BUTTON;	
	psy_ui_button_init_text(&self->toggleterminal, &self->component,
		NULL, "Terminal");
	psy_ui_component_setalign(psy_ui_button_base(&self->toggleterminal),
		psy_ui_ALIGN_RIGHT);
	psy_ui_button_loadresource(&self->toggleterminal, IDB_TERM, IDB_TERM,
		psy_ui_colour_white());
}

void mainstatusbar_initprogressbar(MainStatusBar* self)
{
	self->pluginscanprogress = -1;
	psy_ui_progressbar_init(&self->progressbar, &self->component, NULL);
	psy_ui_component_setalign(progressbar_base(&self->progressbar),
		psy_ui_ALIGN_RIGHT);
	workspace_connectloadprogress(self->workspace, self,
		(fp_workspace_songloadprogress)mainstatusbar_onsongloadprogress);
	psy_signal_connect(&self->workspace->signal_scanprogress, self,
		mainstatusbar_onpluginscanprogress);
}

void mainstatusbar_onstatus(MainStatusBar* self, Workspace* sender,
	const char* text)
{
	psy_ui_label_settext(&self->statusbarlabel, text);
	psy_ui_label_fadeout(&self->statusbarlabel);
}

void mainstatusbar_updateterminalbutton(MainStatusBar* self)
{
	psy_ui_component_setstyletype(psy_ui_button_base(&self->toggleterminal),
		self->terminalstyleid);
	psy_ui_component_invalidate(psy_ui_button_base(&self->toggleterminal));
}

void mainstatusbar_setdefaultstatustext(MainStatusBar* self, const char* text)
{
	psy_ui_label_settext(&self->statusbarlabel,text);
	psy_ui_label_setdefaulttext(&self->statusbarlabel, text);
}

void mainstatusbar_onzoomboxchanged(MainStatusBar* self, ZoomBox* sender)
{
	psy_ui_app_setzoomrate(psy_ui_app(), zoombox_rate(sender));
}

void mainstatusbar_onsongloadprogress(MainStatusBar* self, Workspace* workspace,
	intptr_t progress)
{
	if (progress == -1) {
		workspace_output(self->workspace, "\n");
	}
	psy_ui_progressbar_setprogress(&self->progressbar, progress / 100.f);
}

void mainstatusbar_onpluginscanprogress(MainStatusBar* self, Workspace* workspace,
	int progress)
{
	self->pluginscanprogress = progress;
}

void mainstatusbar_idle(MainStatusBar* self)
{
	if (self->pluginscanprogress != -1) {
		if (self->pluginscanprogress == 0) {
			psy_ui_progressbar_setprogress(&self->progressbar, 0);
			self->pluginscanprogress = -1;
		} else {
			psy_ui_progressbar_tick(&self->progressbar);
		}
	}
	if (self->clockcounter == 0) {
		clockbar_idle(&self->clockbar);
		self->clockcounter = 20;
	}
	--self->clockcounter;
}
