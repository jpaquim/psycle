/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiapp.h"
/* local */
#include "uicomponent.h"
/* std */
#include <stdlib.h>
#include <stdio.h>
/* platform */
#include "../../detail/psyconf.h"
#include "../../detail/os.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#include "imps/win32/uiwinimpfactory.h"
#include <Windows.h>
#elif PSYCLE_USE_TK == PSYCLE_TK_X11
#include "imps/x11/uix11app.h"
#include "imps/x11/uix11impfactory.h"
#else
	#error "Platform not supported"
#endif

/* global app reference set by app_init */
static psy_ui_App* app = NULL;

/* psy_ui_AppZoom */

void psy_ui_appzoom_init(psy_ui_AppZoom* self)
{
	assert(self);

	self->rate = 1.0;
	self->basefontsize = -16;
	psy_signal_init(&self->signal_zoom);
}

void psy_ui_appzoom_dispose(psy_ui_AppZoom* self)
{
	psy_signal_dispose(&self->signal_zoom);
}

void psy_ui_appzoom_setrate(psy_ui_AppZoom* self, double rate)
{
	assert(self);

	self->rate = rate;
	psy_signal_emit_float(&self->signal_zoom, self, 0);
}

double psy_ui_appzoom_rate(const psy_ui_AppZoom* self)
{
	assert(self);

	return self->rate;
}

void psy_ui_appzoom_updatebasefontsize(psy_ui_AppZoom* self, psy_ui_Font* basefont)
{
	psy_ui_FontInfo fontinfo;	

	assert(self);

	fontinfo = psy_ui_font_fontinfo(basefont);
	self->basefontsize = fontinfo.lfHeight;
}

/* psy_ui_App */
static void psy_ui_app_changedefaultfontsize(psy_ui_App*, int size);
static void psy_ui_app_onlanguagechanged(psy_ui_App*, psy_Translator* sender);

static void ui_app_initimpfactory(psy_ui_App*);
static void ui_app_initimp(psy_ui_App*, uintptr_t instance);

psy_ui_App* psy_ui_app(void)
{
	assert(app);

	return app;
}

void psy_ui_app_init(psy_ui_App* self, bool dark, uintptr_t instance)
{	
	assert(self);

	app = self;
	self->main = NULL;
	self->capture_ = NULL;	
	self->alignvalid = TRUE;
	self->mousetracking = FALSE;
	self->hover = NULL;
	self->setpositioncacheonly = FALSE;
	self->focus = NULL;
	psy_table_init(&self->components);
	psy_ui_geometry_init();
	psy_signal_init(&self->signal_dispose);	
	psy_signal_init(&self->signal_mousehook);
	psy_ui_appzoom_init(&self->zoom);
	ui_app_initimpfactory(self);
	ui_app_initimp(self, instance);
	psy_ui_defaults_init(&self->defaults, psy_ui_DARKTHEME);
	psy_ui_appzoom_updatebasefontsize(&self->zoom,
		psy_ui_defaults_font(&self->defaults));	
	psy_translator_init(&self->translator);
	psy_signal_connect(&self->translator.signal_languagechanged, self,
		psy_ui_app_onlanguagechanged);
	psy_ui_dragevent_init(&self->dragevent);
	self->deltaperline = 120;
	self->accumwheeldelta = 0;
	psy_ui_eventdispatch_init(&self->eventdispatch);
#ifdef DIVERSALIS__OS__MICROSOFT
	psy_ui_eventdispatch_disable_handle_doubleclick(&self->eventdispatch);
#endif
	psy_timers_init(&self->wintimers);
}

void ui_app_initimpfactory(psy_ui_App* self)
{
	assert(self);

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32	
	self->impfactory = (psy_ui_ImpFactory*)
		psy_ui_win_impfactory_allocinit();
#elif PSYCLE_USE_TK == PSYCLE_TK_CURSES
	/* todo
	   initscr();
	   refresh();
	   self->imp_factory = (psy_ui_ImpFactory*)
		  psy_ui_curses_impfactory_allocinit(); */
#elif PSYCLE_USE_TK == PSYCLE_TK_X11
	printf("Create X11 Impfactory\n");
	self->impfactory = (psy_ui_ImpFactory*)
		psy_ui_x11_impfactory_allocinit();	
	printf("X11 Impfactory created\n");
#elif PSYCLE_USE_TK == PSYCLE_TK_GTK
	self->impfactory = (psy_ui_ImpFactory*)
		psy_ui_gtk_impfactory_allocinit();
#else
	#error "Platform not supported"
#endif	
}

void ui_app_initimp(psy_ui_App* self, uintptr_t instance)
{
	assert(self);

	if (self->impfactory) {		
		self->imp = psy_ui_impfactory_allocinit_appimp(self->impfactory, self,
			instance);
		if (!self->imp) {
			printf("Create App Imp failed\n");
		}
	} else {
		self->imp = NULL;
	}
}

void psy_ui_app_dispose(psy_ui_App* self)
{	
	assert(self);

	psy_signal_emit(&self->signal_dispose, self, 0);
	psy_signal_dispose(&self->signal_dispose);
	psy_signal_dispose(&self->signal_mousehook);
	psy_ui_appzoom_dispose(&self->zoom);
	psy_ui_defaults_dispose(&self->defaults);
	if (self->imp) {
		self->imp->vtable->dev_dispose(self->imp);
		free(self->imp);
		self->imp = NULL;
	}
	if (self->impfactory) {
		free(self->impfactory);
		self->impfactory = NULL;
	}	
	psy_translator_dispose(&self->translator);
	psy_ui_dragevent_dispose(&self->dragevent);
	psy_ui_eventdispatch_dispose(&self->eventdispatch);
	psy_table_dispose(&self->components);
	psy_timers_dispose(&self->wintimers);
}

void psy_ui_app_setmain(psy_ui_App* self, psy_ui_Component* main)
{
	self->main = main;
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

void psy_ui_app_startmousehook(psy_ui_App* self)
{
	assert(self);

	if (self->imp) {
		self->imp->vtable->dev_startmousehook(self->imp);
	}
}

void psy_ui_app_stopmousehook(psy_ui_App* self)
{
	assert(self);

	if (self->imp) {
		self->imp->vtable->dev_stopmousehook(self->imp);
	}
}

void psy_ui_app_onlanguagechanged(psy_ui_App* self, psy_Translator* translator)
{	
	assert(self);

	if (self->main) {
		psy_List* p;
		psy_List* q;

		/* notify all components language changed */
		psy_ui_component_updatelanguage(self->main);
		for (p = q = psy_ui_component_children(self->main, psy_ui_RECURSIVE);
				p != NULL; psy_list_next(&p)) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)psy_list_entry(p);
			psy_ui_component_updatelanguage(component);
		}
		psy_list_free(q);
		psy_ui_component_align_full(self->main);		
	}	
}

void psy_ui_app_setzoomrate(psy_ui_App* self, double rate)
{
	psy_ui_appzoom_setrate(&self->zoom, rate);
	psy_ui_app_changedefaultfontsize(self,
		(int)(self->zoom.basefontsize * rate));
}

double psy_ui_app_zoomrate(const psy_ui_App* self)
{
	return psy_ui_appzoom_rate(&self->zoom);
}

psy_ui_AppZoom* psy_ui_app_zoom(psy_ui_App* self)
{
	return &self->zoom;
}

void psy_ui_app_changedefaultfontsize(psy_ui_App* self, int size)
{
	psy_ui_FontInfo fontinfo;
	psy_ui_Font font;

	assert(self);

	fontinfo = psy_ui_font_fontinfo(&psy_ui_style_const(psy_ui_STYLE_ROOT)->font);	
	fontinfo.lfHeight = size;	
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_replacedefaultfont(self->main, &font);
	psy_ui_component_invalidate(self->main);	
}

void psy_ui_app_setdefaultfont(psy_ui_App* self, psy_ui_Font* font)
{	
	psy_ui_FontInfo fontinfo;
	psy_ui_Font zoomedfont;

	assert(self);
	
	psy_ui_appzoom_updatebasefontsize(&self->zoom, font);
	if (self->zoom.rate != 1.0) {
		fontinfo = psy_ui_font_fontinfo(font);
		fontinfo.lfHeight = (int)(self->zoom.basefontsize * self->zoom.rate);
		psy_ui_font_init(&zoomedfont, &fontinfo);
		psy_ui_replacedefaultfont(self->main, &zoomedfont);
	} else {
		psy_ui_replacedefaultfont(self->main, font);
	}
	psy_ui_component_invalidate(self->main);
}

void psy_ui_app_lighttheme(psy_ui_App* self)
{
	assert(self);

	psy_ui_defaults_inittheme(&self->defaults, psy_ui_LIGHTTHEME, TRUE);
	if (self->imp) {
		self->imp->vtable->dev_onappdefaultschange(self->imp);
	}
}

void psy_ui_app_darktheme(psy_ui_App* self)
{
	assert(self);

	psy_ui_defaults_inittheme(&self->defaults, psy_ui_DARKTHEME, TRUE);
	if (self->imp) {
		self->imp->vtable->dev_onappdefaultschange(self->imp);
	}
}

bool psy_ui_app_hasdarktheme(const psy_ui_App* self)
{
	assert(self);

	return psy_ui_defaults()->styles.theme == psy_ui_DARKTHEME;
}

psy_ui_Style* psy_ui_app_style(psy_ui_App* self, uintptr_t styletype)
{
	return psy_ui_defaults_style(&self->defaults, styletype);
}

const psy_ui_Style* psy_ui_app_style_const(const psy_ui_App* self, uintptr_t styletype)
{
	return psy_ui_defaults_style_const(&self->defaults, styletype);
}

psy_ui_Style* psy_ui_style(uintptr_t styletype)
{
	return psy_ui_app_style(psy_ui_app(), styletype);
}

const psy_ui_Style* psy_ui_style_const(uintptr_t styletype)
{
	return psy_ui_app_style_const(psy_ui_app(), styletype);
}

void psy_ui_app_setfocus(psy_ui_App* self, psy_ui_Component* component)
{
	if (self->focus) {
		psy_ui_Event ev;

		psy_ui_event_init_stop_propagation(&ev, psy_ui_FOCUSOUT);			
		psy_ui_eventdispatch_send(&self->eventdispatch, self->focus, &ev);			
	}
	self->focus = component;
	if (self->focus) {
		psy_ui_Event ev;

		psy_ui_event_init_stop_propagation(&ev, psy_ui_FOCUS);		
		psy_ui_eventdispatch_send(&self->eventdispatch, self->focus, &ev);
	}	
}

/* psy_ui_AppImp */
static void psy_ui_appimp_dispose(psy_ui_AppImp* self) { }
static int psy_ui_appimp_run(psy_ui_AppImp* self) { return PSY_ERRRUN; }
static void psy_ui_appimp_stop(psy_ui_AppImp* self) { }
static void psy_ui_appimp_close(psy_ui_AppImp* self) { }
static void psy_ui_appimp_onappdefaultschange(psy_ui_AppImp* self) { }
static void psy_ui_appimp_startmousehook(psy_ui_AppImp* self) { }
static void psy_ui_appimp_stopmousehook(psy_ui_AppImp* self) { }
static void psy_ui_appimp_sendevent(psy_ui_AppImp* self,
	psy_ui_Component* component, psy_ui_Event* ev)
{
}

static psy_ui_Component* psy_ui_appimp_component(psy_ui_AppImp* self,
	uintptr_t platformhandle)
{
	return NULL;
}

static psy_ui_AppImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_appimp_dispose;		
		imp_vtable.dev_run = psy_ui_appimp_run;
		imp_vtable.dev_stop = psy_ui_appimp_stop;
		imp_vtable.dev_close = psy_ui_appimp_close;	
		imp_vtable.dev_onappdefaultschange = psy_ui_appimp_onappdefaultschange;
		imp_vtable.dev_startmousehook = psy_ui_appimp_startmousehook;
		imp_vtable.dev_stopmousehook = psy_ui_appimp_stopmousehook;
		imp_vtable.dev_sendevent = psy_ui_appimp_sendevent;
		imp_vtable.dev_component = psy_ui_appimp_component;
		imp_vtable_initialized = TRUE;
	}
}

void psy_ui_appimp_init(psy_ui_AppImp* self)
{
	assert(self);

	imp_vtable_init();
	self->vtable = &imp_vtable;
}

const struct psy_ui_Defaults* psy_ui_defaults(void)
{
	return &psy_ui_app()->defaults;
}

struct psy_ui_Defaults* psy_ui_appdefaults(void)
{
	return &psy_ui_app()->defaults;
}

const struct psy_ui_Defaults* psy_ui_appdefaults_const(void)
{
	return &psy_ui_app()->defaults;
}

int psy_ui_logpixelsx(void)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	int rv;
	HDC screen;	

	screen = GetDC(NULL);	
	rv = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(NULL, screen);
	return rv;
#endif
	return 72;
}

int psy_ui_logpixelsy(void)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	int rv;
	HDC screen;

	screen = GetDC(NULL);
	rv = GetDeviceCaps(screen, LOGPIXELSY);
	ReleaseDC(NULL, screen);
	return rv;
#endif
	return 72;
}

void psy_ui_app_sethover(psy_ui_App* self, psy_ui_Component* hover)
{
	self->hover = hover;
}

void psy_ui_app_startdrag(psy_ui_App* self)
{
	self->dragevent.active = TRUE;
	psy_ui_component_setcursor(self->main, psy_ui_CURSORSTYLE_GRAB);
}

void psy_ui_app_stopdrag(psy_ui_App* self)
{
	if (self->dragevent.active) {
		self->dragevent.active = FALSE;
		if (self->dragevent.dataTransfer) {
			psy_property_deallocate(self->dragevent.dataTransfer);
			self->dragevent.dataTransfer = NULL;
		}
	}
}

void psy_ui_app_starttimer(psy_ui_App* self, psy_ui_Component* component, uintptr_t id,
	uintptr_t interval)
{	
	psy_timers_addtimer(&self->wintimers, (uintptr_t)component, component,
		(psy_fp_timerwork)component->vtable->ontimer, 
		&component->signal_timer, id, interval);
}

void psy_ui_app_stoptimer(psy_ui_App* self, psy_ui_Component* component, uintptr_t id)
{
	psy_timers_removetimer(&self->wintimers, (uintptr_t)component, id);
}

/*
** Initial entry point for the startup of psycle
**
** Normally, main is the c entry point, but win32 uses WinMain, so we have
** to define for win32 a different entry point as the usual main method.
** We split the startup call with os defines of diversalis and call then
** psycle_run as main method
*/

#ifdef DIVERSALIS__OS__MICROSOFT

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>

int psycle_run(uintptr_t instance, int options);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ PSTR szCmdLine, _In_ int iCmdShow)
{
	int err;

	/* calls the platform independend startup code */
	err = psycle_run((uintptr_t)hInstance, iCmdShow);
#if defined _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif
	return err;
}

#else
#define _MAX_PATH 4096

int psycle_run(uintptr_t instance, int options);

int main(int argc, char** argv)
{
	return psycle_run(0, 0);
}
#endif
