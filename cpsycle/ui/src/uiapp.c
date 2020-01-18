// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uiapp.h"

#ifdef DIVERSALIS__OS__MICROSOFT
	#include "uiwinapp.h"	
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
	psy_ui_defaults_init(&self->defaults);
	ui_app_initplatform(self, instance);
}

void ui_app_initplatform(psy_ui_App* self, uintptr_t instance)
{
#ifdef DIVERSALIS__OS__MICROSOFT
	self->platform = (psy_ui_WinApp*) malloc(sizeof(psy_ui_WinApp));
	psy_ui_winapp_init(app.platform, (HINSTANCE) instance);
#else
	#error "Platform not supported"
#endif
}

void psy_ui_app_dispose(psy_ui_App* self)
{	
	psy_signal_emit(&self->signal_dispose, self, 0);
	psy_signal_dispose(&self->signal_dispose);
	psy_ui_winapp_dispose(self->platform);
	free(self->platform);
	self->platform = 0;
}

struct psy_ui_Component* psy_ui_app_main(psy_ui_App* self)
{
	return self->main;	
}

int psy_ui_app_run(psy_ui_App* self) 
{
#ifdef DIVERSALIS__OS__MICROSOFT
	return psy_ui_winapp_run((psy_ui_WinApp*)self->platform);
#endif	
}

void psy_ui_app_stop(psy_ui_App* self)
{
#ifdef DIVERSALIS__OS__MICROSOFT
	psy_ui_winapp_stop((psy_ui_WinApp*)self->platform);
#endif	
}
