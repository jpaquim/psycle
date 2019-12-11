// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIAPP_H)
#define UIAPP_H

#include "../../detail/stdint.h"
#include "../../detail/os.h"

#include <hashtbl.h>
#include <signal.h>

typedef struct {
	psy_Signal signal_dispose;
} ui_app;

extern ui_app app;

#if defined (DIVERSALIS__OS__MICROSOFT)

#define WIN32_LEAN_AND_MEAN
#define UIMAIN int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
#define UIINIT ui_init((uintptr_t) hInstance)
void ui_init(uintptr_t hInstance);
#else
#define UIMAIN int main(int argc, char **argv)
#define UIINIT ui_init()
void ui_init(void);
#endif
void ui_dispose();
int ui_run(void);
void ui_quit(void);

#endif
