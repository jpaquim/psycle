/* This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_APP_H
#define psy_ui_APP_H

/* local */
#include "uieventdispatch.h"
#include "uidefaults.h"
#include "timers.h"
/* container */
#include <signal.h>
/* file */
#include <translator.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_AppImp;
struct psy_ui_Component;
struct psy_ui_ImpFactory;

/* psy_ui_AppZoom */
typedef struct psy_ui_AppZoom {
	/* signals */
	psy_Signal signal_zoom;
	/* internal data */
	double rate;
	int basefontsize;	
} psy_ui_AppZoom;

void psy_ui_appzoom_init(psy_ui_AppZoom*);
void psy_ui_appzoom_dispose(psy_ui_AppZoom*);

void psy_ui_appzoom_setrate(psy_ui_AppZoom*, double rate);
double psy_ui_appzoom_rate(const psy_ui_AppZoom*);

void psy_ui_appzoom_update_base_fontsize(psy_ui_AppZoom*, psy_ui_Font*);

struct psy_ui_Component;

/* psy_ui_App */
typedef struct psy_ui_App {
	/* signals */
	psy_Signal signal_dispose;
	psy_Signal signal_mousehook;
	/* internal data */
	struct psy_ui_AppImp* imp;
	struct psy_ui_ImpFactory* impfactory;
	psy_ui_AppZoom zoom;
	psy_ui_Defaults defaults;
	psy_Translator translator;	
	bool alignvalid;
	bool mousetracking;
	bool setpositioncacheonly;	
	psy_Timers wintimers;
	/* references */
	struct psy_ui_Component* main;
	struct psy_ui_Component* capture_;
	struct psy_ui_Component* hover;
	struct psy_ui_Component* focus;
	psy_ui_DragEvent dragevent;	
	psy_ui_EventDispatch eventdispatch;
	psy_Table components;
} psy_ui_App;

psy_ui_App* psy_ui_app(void);

void psy_ui_app_init(psy_ui_App*, bool dark, uintptr_t instance);
void psy_ui_app_dispose(psy_ui_App*);

void psy_ui_app_setmain(psy_ui_App*, struct psy_ui_Component* main);
struct psy_ui_Component* psy_ui_app_main(psy_ui_App*);
int psy_ui_app_run(psy_ui_App*);
void psy_ui_app_stop(psy_ui_App*);
void psy_ui_app_close(psy_ui_App*);
void psy_ui_app_startmousehook(psy_ui_App*);
void psy_ui_app_stopmousehook(psy_ui_App*);

void psy_ui_app_setzoomrate(psy_ui_App*, double rate);
double psy_ui_app_zoomrate(const psy_ui_App*);
psy_ui_AppZoom* psy_ui_app_zoom(psy_ui_App*);
void psy_ui_app_set_default_font(psy_ui_App*, psy_ui_Font*);
void psy_ui_app_lighttheme(psy_ui_App*);
void psy_ui_app_darktheme(psy_ui_App*);
bool psy_ui_app_hasdarktheme(const psy_ui_App*);
psy_ui_Style* psy_ui_app_style(psy_ui_App*, uintptr_t styletype);
const psy_ui_Style* psy_ui_app_style_const(const psy_ui_App*, uintptr_t styletype);
void psy_ui_app_set_hover(psy_ui_App*, struct psy_ui_Component* hover);
void psy_ui_app_startdrag(psy_ui_App*);
void psy_ui_app_stopdrag(psy_ui_App*);
void psy_ui_app_starttimer(psy_ui_App*, struct psy_ui_Component*, uintptr_t id,
	uintptr_t interval);
void psy_ui_app_stoptimer(psy_ui_App*, struct psy_ui_Component*, uintptr_t id);


INLINE struct psy_ui_Component* psy_ui_app_capture(psy_ui_App* self)
{
	return self->capture_;
}

INLINE void psy_ui_app_setcapture(psy_ui_App* self, struct psy_ui_Component* component)
{
	self->capture_ = component;
}

INLINE struct psy_ui_Component* psy_ui_app_hover(psy_ui_App* self)
{
	return self->hover;
}

INLINE struct psy_ui_ImpFactory* psy_ui_app_impfactory(psy_ui_App* self)
{
	assert(self);

	return self->impfactory;
}

INLINE struct psy_ui_Component* psy_ui_app_focus(psy_ui_App* self)
{
	return self->focus;
}

void psy_ui_app_set_focus(psy_ui_App*, struct psy_ui_Component*);

psy_ui_Style* psy_ui_style(uintptr_t styletype);
const psy_ui_Style* psy_ui_style_const(uintptr_t styletype);
psy_List* psy_ui_app_toplevel(psy_ui_App* self);

/* psy_ui_AppImp */
typedef void (*psy_ui_fp_appimp_dispose)(struct psy_ui_AppImp*);
typedef int (*psy_ui_fp_appimp_run)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_stop)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_close)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_sendevent)(struct psy_ui_AppImp*,
	struct psy_ui_Component*, psy_ui_Event*);
typedef struct psy_ui_Component* (*psy_ui_fp_appimp_component)(struct psy_ui_AppImp*,
	uintptr_t platformhandle);
typedef psy_List* (*psy_ui_fp_appimp_toplevel)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_onappdefaultschange)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_startmousehook)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_stopmousehook)(struct psy_ui_AppImp*);

typedef struct psy_ui_AppImpVTable {
	psy_ui_fp_appimp_dispose dev_dispose;
	psy_ui_fp_appimp_run dev_run;
	psy_ui_fp_appimp_stop dev_stop;
	psy_ui_fp_appimp_close dev_close;	
	psy_ui_fp_appimp_onappdefaultschange dev_onappdefaultschange;
	psy_ui_fp_appimp_startmousehook dev_startmousehook;
	psy_ui_fp_appimp_stopmousehook dev_stopmousehook;
	psy_ui_fp_appimp_sendevent dev_sendevent;
	psy_ui_fp_appimp_component dev_component;
	psy_ui_fp_appimp_toplevel dev_toplevel;
} psy_ui_AppImpVTable;

typedef struct psy_ui_AppImp {
	psy_ui_AppImpVTable* vtable;
} psy_ui_AppImp;

void psy_ui_appimp_init(psy_ui_AppImp*);

const struct psy_ui_Defaults* psy_ui_defaults(void);
struct psy_ui_Defaults* psy_ui_appdefaults(void);
const struct psy_ui_Defaults* psy_ui_appdefaults_const(void);

int psy_ui_logpixelsx(void);
int psy_ui_logpixelsy(void);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_APP_H */
