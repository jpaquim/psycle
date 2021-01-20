// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include "uicomponent.h"
// platform
#include "../../detail/psyconf.h"
#include "../../detail/os.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#include "imps/win32/uiwinimpfactory.h"
#elif PSYCLE_USE_TK == PSYCLE_TK_CURSES
#include <curses.h>
#include "imps/curses/uicursesimpfactory.h"
#elif PSYCLE_USE_TK == PSYCLE_TK_XT
#include "uix11app.h"
#include "uix11impfactory.h"
#else
	#error "Platform not supported"
#endif
// std
#include <stdlib.h>

static psy_ui_App* app = NULL;

void psy_ui_app_onlanguagechanged(psy_ui_App*, psy_Translator* sender);

static void ui_app_initimpfactory(psy_ui_App*);
static void ui_app_initimp(psy_ui_App*, uintptr_t instance);

psy_ui_App* psy_ui_app(void)
{
	assert(app);

	return app;
}

void psy_ui_app_init(psy_ui_App* self, uintptr_t instance)
{	
	assert(self);

	app = self;
	self->main = NULL;
	psy_signal_init(&self->signal_dispose);	
	ui_app_initimpfactory(self);
	psy_ui_defaults_init(&self->defaults);
	ui_app_initimp(self, instance);
	psy_translator_init(&self->translator);
	psy_signal_connect(&self->translator.signal_languagechanged, self,
		psy_ui_app_onlanguagechanged);
}

void ui_app_initimpfactory(psy_ui_App* self)
{
	assert(self);

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32	
	self->imp_factory = (psy_ui_ImpFactory*)
		psy_ui_win_impfactory_allocinit();
#elif PSYCLE_USE_TK == PSYCLE_TK_CURSES
	// todo
	//initscr();
	//refresh();
	//self->imp_factory = (psy_ui_ImpFactory*)
		//psy_ui_curses_impfactory_allocinit();
#elif PSYCLE_USE_TK == PSYCLE_TK_XT
	// todo
	//self->platform = (psy_ui_X11App*)malloc(sizeof(psy_ui_X11App));
	//psy_ui_x11app_init(app.platform, 0);
	//self->imp_factory = (psy_ui_ImpFactory*)
	//	psy_ui_x11_impfactory_allocinit();
#else
	#error "Platform not supported"
#endif	
}

void ui_app_initimp(psy_ui_App* self, uintptr_t instance)
{
	assert(self);

	if (self->imp_factory) {
		self->imp = psy_ui_impfactory_allocinit_appimp(self->imp_factory, self,
			instance);
	} else {
		self->imp = NULL;
	}
}

void psy_ui_app_dispose(psy_ui_App* self)
{	
	assert(self);

	psy_signal_emit(&self->signal_dispose, self, 0);
	psy_signal_dispose(&self->signal_dispose);
	if (self->imp) {
		self->imp->vtable->dev_dispose(self->imp);
		free(self->imp);
		self->imp = NULL;
	}
	if (self->imp_factory) {
		free(self->imp_factory);
		self->imp_factory = NULL;
	}
	psy_ui_defaults_dispose(&self->defaults);
	psy_translator_dispose(&self->translator);
}

struct psy_ui_Component* psy_ui_app_main(psy_ui_App* self)
{
	assert(self);

	return self->main;	
}

int psy_ui_app_run(psy_ui_App* self) 
{
	assert(self);

	if (self->imp) {
		return self->imp->vtable->dev_run(self->imp);
	}
	return PSY_ERRRUN;
}

void psy_ui_app_stop(psy_ui_App* self)
{
	assert(self);

	if (self->imp) {
		self->imp->vtable->dev_stop(self->imp);
	}
}

void psy_ui_app_close(psy_ui_App* self)
{
	assert(self);

	if (self->imp) {
		self->imp->vtable->dev_close(self->imp);
	}
}

void psy_ui_app_onlanguagechanged(psy_ui_App* self, psy_Translator* translator)
{	
	assert(self);

	if (self->main) {
		psy_List* p;
		psy_List* q;

		// notify all components language changed
		psy_ui_component_updatelanguage(self->main);
		for (p = q = psy_ui_component_children(self->main, psy_ui_RECURSIVE);
				p != NULL; psy_list_next(&p)) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)psy_list_entry(p);
			psy_ui_component_updatelanguage(component);
		}
		psy_list_free(q);
		psy_ui_component_alignall(self->main);
	}	
}

// psy_ui_AppImp
static void psy_ui_appimp_dispose(psy_ui_AppImp* self) { }
static int psy_ui_appimp_run(psy_ui_AppImp* self) { return PSY_ERRRUN; }
static void psy_ui_appimp_stop(psy_ui_AppImp* self) { }
static void psy_ui_appimp_close(psy_ui_AppImp* self) { }

static psy_ui_AppImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_appimp_dispose;		
		imp_vtable.dev_run = psy_ui_appimp_run;
		imp_vtable.dev_stop = psy_ui_appimp_stop;
		imp_vtable.dev_close = psy_ui_appimp_close;		
		imp_vtable_initialized = TRUE;
	}
}

void psy_ui_appimp_init(psy_ui_AppImp* self)
{
	assert(self);

	imp_vtable_init();
	self->vtable = &imp_vtable;
}
