// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uiapp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#include "uiwinapp.h"	
#include "uiwinimpfactory.h"
#elif PSYCLE_USE_TK == PSYCLE_TK_CURSES
#include <curses.h>
#include "uicursesimpfactory.h"
#elif PSYCLE_USE_TK == PSYCLE_TK_XT
#include "uixtapp.h"
#include "uixtimpfactory.h"
#else
	#error "Platform not supported"
#endif
#include <stdlib.h>

psy_ui_App app;

static void ui_app_initplatform(psy_ui_App*, uintptr_t instance);

void psy_ui_app_init(psy_ui_App* self, uintptr_t instance)
{	
	self->main = 0;
	psy_signal_init(&self->signal_dispose);
	ui_app_initplatform(self, instance);
	psy_ui_defaults_init(&self->defaults);	
}

void ui_app_initplatform(psy_ui_App* self, uintptr_t instance)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	self->platform = (psy_ui_WinApp*) malloc(sizeof(psy_ui_WinApp));
	psy_ui_winapp_init(app.platform, (HINSTANCE) instance);
	self->imp_factory = (psy_ui_ImpFactory*) psy_ui_win_impfactory_allocinit();
#elif PSYCLE_USE_TK == PSYCLE_TK_CURSES
	initscr();
	refresh();
	self->imp_factory = (psy_ui_ImpFactory*) psy_ui_curses_impfactory_allocinit();
#elif PSYCLE_USE_TK == PSYCLE_TK_XT
	self->platform = (psy_ui_XtApp*) malloc(sizeof(psy_ui_XtApp));
	psy_ui_xtapp_init(app.platform, 0);
	self->imp_factory = (psy_ui_ImpFactory*) psy_ui_xt_impfactory_allocinit();
#else
	#error "Platform not supported"
#endif
}

void psy_ui_app_dispose(psy_ui_App* self)
{	
	psy_signal_emit(&self->signal_dispose, self, 0);
	psy_signal_dispose(&self->signal_dispose);
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	psy_ui_winapp_dispose(self->platform);
#elif PSYCLE_USE_TK == PSYCLE_TK_CURSES
	endwin();
#elif PSYCLE_USE_TK == PSYCLE_TK_XT
	psy_ui_xtapp_dispose(self->platform);
#else
#error "Platform not supported"
#endif
	free(self->platform);
	self->platform = 0;
	free(self->imp_factory);
	self->imp_factory = 0;
}

struct psy_ui_Component* psy_ui_app_main(psy_ui_App* self)
{
	return self->main;	
}

int psy_ui_app_run(psy_ui_App* self) 
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	return psy_ui_winapp_run((psy_ui_WinApp*)self->platform);
#elif PSYCLE_USE_TK == PSYCLE_TK_XT
	return psy_ui_xtapp_run((psy_ui_XtApp*)self->platform);
#endif
}

void psy_ui_app_stop(psy_ui_App* self)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	psy_ui_winapp_stop((psy_ui_WinApp*)self->platform);
#endif	
}

void psy_ui_app_close(psy_ui_App* self)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	psy_ui_winapp_close((psy_ui_WinApp*)self->platform, self->main);
#endif	
}
