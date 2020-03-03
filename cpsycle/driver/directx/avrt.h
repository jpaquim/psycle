// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_AVRT_H
#define psy_audio_AVRT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <mmsystem.h>

// AVRT is the new "multimedia scheduling stuff"
typedef HANDLE(WINAPI* FAvSetMmThreadCharacteristics)(LPCTSTR, LPDWORD);
typedef BOOL(WINAPI* FAvRevertMmThreadCharacteristics)(HANDLE);

extern FAvSetMmThreadCharacteristics pAvSetMmThreadCharacteristics;
extern FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics;

// Dynamic load and unload of avrt.dll, so the executable can run on
// Windows 2K and XP.
BOOL SetupAVRT(void);
void CloseAVRT(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_AVRT_H */
