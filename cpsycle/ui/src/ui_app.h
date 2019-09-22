// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIAPP)
#define UIAPP

#define WIN32_LEAN_AND_MEAN
#define UIMAIN int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,\
 PSTR szCmdLine, int iCmdShow)
#define UIINIT ui_init(hInstance)
#define UIDISPOSE ui_dispose()

int ui_run(void);
void ui_quit(void);

#endif
