// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_APP_H
#define psy_ui_APP_H

#include "../../detail/stdint.h"
#include "../../detail/os.h"
#include "uidefaults.h"

#include <hashtbl.h>
#include <signal.h>

typedef struct {
	psy_Signal signal_dispose;
	struct psy_ui_Component* main;
	void* platform;
	psy_ui_Defaults defaults;
} psy_ui_App;

extern psy_ui_App app;

void psy_ui_app_init(psy_ui_App*, uintptr_t instance);
void psy_ui_app_dispose(psy_ui_App*);
struct psy_ui_Component* psy_ui_app_main(psy_ui_App*);
int psy_ui_app_run(psy_ui_App*);
void psy_ui_app_stop(psy_ui_App*);

#endif /* psy_ui_APP_H */
