/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "../../detail/psyconf.h"
#include "uidef.h"
#include "uiapp.h"

#include <math.h>

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

void psy_ui_error(const char* err, const char* shorterr)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	MessageBox(NULL, err, shorterr, MB_OK | MB_ICONERROR);
#endif
}
