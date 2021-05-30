// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "avrt.h"

static HMODULE hDInputDLL = 0;
static int refcount = 0;

FAvSetMmThreadCharacteristics pAvSetMmThreadCharacteristics = NULL;
FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics = NULL;

#define _GetProc(fun, type, name) \
{                                                  \
    fun = (type) GetProcAddress(hDInputDLL,name);  \
    if (fun == NULL) { return FALSE; }             \
}

// Dynamic load and unload of avrt.dll, so the executable can run on
// Windows 2K and XP.
BOOL SetupAVRT(void)
{
	if (refcount == 0) {
		hDInputDLL = LoadLibraryA("avrt.dll");
		if (hDInputDLL == NULL)
			return FALSE;
		_GetProc(pAvSetMmThreadCharacteristics, FAvSetMmThreadCharacteristics,
			"AvSetMmThreadCharacteristicsA");
		_GetProc(pAvRevertMmThreadCharacteristics, FAvRevertMmThreadCharacteristics,
			"AvRevertMmThreadCharacteristics");
	}
	++refcount;
	return pAvSetMmThreadCharacteristics &&
		pAvRevertMmThreadCharacteristics;
}

void CloseAVRT(void)
{
	--refcount;
	if (refcount == 0) {
		if (hDInputDLL != NULL)
			FreeLibrary(hDInputDLL);
		hDInputDLL = NULL;
	}
}
