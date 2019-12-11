// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uieditor.h"
#include "uiapp.h"
#include "scintilla/include/scintilla.h"

#include <stdio.h>

static HMODULE scimodule = 0;

#define MAXTEXTLENGTH 65535

static int loadscilexer(void);
static void onappdestroy(void*, ui_app* sender);

void ui_editor_init(ui_editor* self, ui_component* parent)
{  	
	int err;

	if ((err = loadscilexer()) == 0) {	
		ui_win32_component_init(&self->component, parent, TEXT("Scintilla"), 
			0, 0, 100, 20, WS_CHILD | WS_VISIBLE, 0);
	} else {
		ui_win32_component_init(&self->component, parent, TEXT("STATIC"),
			0, 0, 100, 20,
			WS_CHILD | WS_VISIBLE | SS_CENTER, 0);
		SetWindowText((HWND)self->component.hwnd, 
			"Editor can't be used.\n"
			"LoadLibrary SciLexer.dll failure\n"
			"Check if 'SciLexer.dll' is in the psycle bin directory or\n"
			"if you have the right version for your system.");
		ui_error("LoadLibrary SciLexer.dll failure ...",
			"Error - Psycle Ui - Editor");		
	}
}

int loadscilexer(void)
{
	if (scimodule == 0) {
		scimodule = LoadLibrary ("SciLexer.dll");
		if (scimodule != NULL) {		
			psy_signal_connect(&app.signal_dispose, 0, onappdestroy);
		}		
	}
	return scimodule == NULL;
}

void onappdestroy(void* context, ui_app* sender)
{	
	if (scimodule != NULL) {
		FreeLibrary(scimodule);
		scimodule = 0;	
	}
}

void ui_editor_load(ui_editor* self, const char* path)
{	
	FILE* fp;

	fp = fopen(path, "rb");
	if (fp) {
		char c;
		int pos = 0;
		char text[MAXTEXTLENGTH];

		memset(text, 0, MAXTEXTLENGTH);
		while ((c = fgetc(fp)) != EOF && pos < MAXTEXTLENGTH) {
			text[pos] = c;
			++pos;
		}		
		fclose(fp);		
		SendMessage((HWND) self->component.hwnd,
			SCI_ADDTEXT,
			(WPARAM)(intptr_t)strlen(text),
			(LPARAM)(intptr_t) text);		
	}
}
