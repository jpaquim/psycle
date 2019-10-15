// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "ui_app.h"
#include <windows.h>

int ui_run(void) 
{
	MSG msg; 

	while (GetMessage (&msg, NULL, 0, 0))
    {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
    }
    return msg.wParam ;
}

void ui_quit(void)
{
	PostQuitMessage(0);
}