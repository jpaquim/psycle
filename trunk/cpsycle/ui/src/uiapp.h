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

// psy_ui_App
typedef struct psy_ui_App {
	// signals
	psy_Signal signal_dispose;
	// internal data
	struct psy_ui_AppImp* imp;	
	struct psy_ui_ImpFactory* imp_factory;
	psy_ui_AppZoom zoom;
	psy_ui_Defaults defaults;
	psy_Translator translator;	
	// references
	struct psy_ui_Component* main;
} psy_ui_App;

psy_ui_App* psy_ui_app(void);

void psy_ui_app_init(psy_ui_App*, bool dark, uintptr_t instance);
void psy_ui_app_dispose(psy_ui_App*);
struct psy_ui_Component* psy_ui_app_main(psy_ui_App*);
int psy_ui_app_run(psy_ui_App*);
void psy_ui_app_stop(psy_ui_App*);
void psy_ui_app_close(psy_ui_App*);
void psy_ui_app_setzoomrate(psy_ui_App*, double rate);
double psy_ui_app_zoomrate(const psy_ui_App*);
psy_ui_AppZoom* psy_ui_app_zoom(psy_ui_App*);
void psy_ui_app_setdefaultfont(psy_ui_App*, psy_ui_Font*);
void psy_ui_app_lighttheme(psy_ui_App*);
void psy_ui_app_darktheme(psy_ui_App*);
bool psy_ui_app_hasdarktheme(const psy_ui_App*);

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

typedef struct psy_ui_AppImpVTable {
	psy_ui_fp_appimp_dispose dev_dispose;
	psy_ui_fp_appimp_run dev_run;
	psy_ui_fp_appimp_stop dev_stop;
	psy_ui_fp_appimp_close dev_close;	
	psy_ui_fp_appimp_onappdefaultschange dev_onappdefaultschange;
} psy_ui_AppImpVTable;

typedef struct psy_ui_AppImp {
	psy_ui_AppImpVTable* vtable;
} psy_ui_AppImp;

void psy_ui_appimp_init(psy_ui_AppImp*);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_APP_H */
