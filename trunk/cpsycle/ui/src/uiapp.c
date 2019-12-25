// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include <windows.h>
#include <excpt.h>

ui_app app;

static int FilterException(const char* msg, int code, struct _EXCEPTION_POINTERS *ep) 
{	
	// char txt[512];	
			
	MessageBox(0, msg, "Psycle Ui Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

void ui_app_init(ui_app* self)
{
	psy_signal_init(&self->signal_dispose);
	self->main = 0;
}

void ui_app_dispose(ui_app* self)
{
	psy_signal_dispose(&self->signal_dispose);
}

struct ui_component* ui_app_main(ui_app* self)
{
	return self->main;	
}


int ui_run(void) 
{
	MSG msg; 

	// __try
	// {
		while (GetMessage (&msg, NULL, 0, 0))
		{
			  TranslateMessage (&msg) ;
			  DispatchMessage (&msg) ;
		}
	// }
	// __except(FilterException("app loop", GetExceptionCode(), GetExceptionInformation())) {			
	// }
    return (int) msg.wParam ;
}

void ui_quit(void)
{
	PostQuitMessage(0);
}
