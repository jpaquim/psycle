// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_APP_H
#define psy_ui_APP_H

// local
#include "uidefaults.h"
// container
#include <signal.h>
// file
#include <translator.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_AppImp;
struct psy_ui_Component;
struct psy_ui_ImpFactory;

// psy_ui_AppZoom
typedef struct psy_ui_AppZoom {
	// signals
	psy_Signal signal_zoom;
	// internal data
	double rate;
	int basefontsize;
} psy_ui_AppZoom;

void psy_ui_appzoom_init(psy_ui_AppZoom*);
void psy_ui_appzoom_dispose(psy_ui_AppZoom*);

void psy_ui_appzoom_setrate(psy_ui_AppZoom*, double rate);
double psy_ui_appzoom_rate(const psy_ui_AppZoom*);

void psy_ui_appzoom_updatebasefontsize(psy_ui_AppZoom*, psy_ui_Font*);

struct psy_ui_Component;

// psy_ui_App
typedef struct psy_ui_App {
	// signals
	psy_Signal signal_dispose;
	psy_Signal signal_mousehook;
	// internal data
	struct psy_ui_AppImp* imp;
	struct psy_ui_ImpFactory* imp_factory;
	psy_ui_AppZoom zoom;
	psy_ui_Defaults defaults;
	psy_Translator translator;	
	bool alignvalid;
	bool mousetracking;
	// references
	struct psy_ui_Component* main;
	struct psy_ui_Component* capture;	
	struct psy_ui_Component* hover;
} psy_ui_App;

psy_ui_App* psy_ui_app(void);
const psy_ui_Style* psy_ui_style(uintptr_t styletype);

void psy_ui_app_init(psy_ui_App*, bool dark, uintptr_t instance);
void psy_ui_app_dispose(psy_ui_App*);
struct psy_ui_Component* psy_ui_app_main(psy_ui_App*);
int psy_ui_app_run(psy_ui_App*);
void psy_ui_app_stop(psy_ui_App*);
void psy_ui_app_close(psy_ui_App*);
void psy_ui_app_startmousehook(psy_ui_App*);
void psy_ui_app_stopmousehook(psy_ui_App*);

void psy_ui_app_setzoomrate(psy_ui_App*, double rate);
double psy_ui_app_zoomrate(const psy_ui_App*);
psy_ui_AppZoom* psy_ui_app_zoom(psy_ui_App*);
void psy_ui_app_setdefaultfont(psy_ui_App*, psy_ui_Font*);
void psy_ui_app_lighttheme(psy_ui_App*);
void psy_ui_app_darktheme(psy_ui_App*);
bool psy_ui_app_hasdarktheme(const psy_ui_App*);
const psy_ui_Style* psy_ui_app_style(const psy_ui_App*, uintptr_t styletype);
void psy_ui_app_updatesyles(psy_ui_App*);
void psy_ui_app_sethover(psy_ui_App*, struct psy_ui_Component* hover);

INLINE struct psy_ui_Component* psy_ui_app_capture(psy_ui_App* self)
{
	return self->capture;
}


INLINE struct psy_ui_ImpFactory* psy_ui_app_impfactory(psy_ui_App* self)
{
	assert(self);

	return self->imp_factory;
}

// psy_ui_AppImp
typedef void (*psy_ui_fp_appimp_dispose)(struct psy_ui_AppImp*);
typedef int (*psy_ui_fp_appimp_run)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_stop)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_close)(struct psy_ui_AppImp*);
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
