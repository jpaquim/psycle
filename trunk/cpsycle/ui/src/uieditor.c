// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uieditor.h"
#include "uiapp.h"
#include "scintilla/include/scintilla.h"

#include <stdio.h>

#define SCINTILLA_ENABLED 1

static HMODULE scimodule = 0;

#define MAXTEXTLENGTH 65535

static int loadscilexer(void);
static void onappdestroy(void*, psy_ui_App* sender);
static void ui_editor_styleclearall(psy_ui_Editor*);
static void ui_editor_setcaretcolor(psy_ui_Editor*, uint32_t color);

void ui_editor_init(psy_ui_Editor* self, psy_ui_Component* parent)
{  		
#ifdef SCINTILLA_ENABLED
	int err;

	if ((err = loadscilexer()) == 0) {	
		ui_win32_component_init(&self->component, parent, TEXT("Scintilla"), 
			0, 0, 100, 20, WS_CHILD | WS_VISIBLE, 0);
		if (self->component.hwnd) {
			extern psy_ui_App app;

			ui_editor_setcolor(self, ui_defaults_color(&app.defaults));
			ui_editor_setcaretcolor(self, ui_defaults_color(&app.defaults));
			ui_editor_setbackgroundcolor(self, 
				ui_defaults_backgroundcolor(&app.defaults));
			ui_editor_styleclearall(self);
		}
	} else
#endif	
	{
		ui_win32_component_init(&self->component, parent, TEXT("STATIC"),
			0, 0, 100, 20,
			WS_CHILD | WS_VISIBLE | SS_CENTER, 0);
#ifdef SCINTILLA_ENABLED
		SetWindowText((HWND)self->component.hwnd, 
			"Editor can't be used.\n"
			"LoadLibrary SciLexer.dll failure\n"
			"Check if 'SciLexer.dll' is in the psycle bin directory or\n"
			"if you have the right version for your system.");
		ui_error("LoadLibrary SciLexer.dll failure ...",
			"Error - Psycle Ui - Editor");
#else
	SetWindowText((HWND)self->component.hwnd, 
			"Editor can't be used. Scintilla disabled in build\n");
#endif
	}
}

int loadscilexer(void)
{
#ifdef SCINTILLA_ENABLED
	if (scimodule == 0) {
		scimodule = LoadLibrary ("SciLexer.dll");
		if (scimodule != NULL) {		
			psy_signal_connect(&app.signal_dispose, 0, onappdestroy);
		}		
	}
	return scimodule == NULL;
#endif
}

void onappdestroy(void* context, psy_ui_App* sender)
{	
	if (scimodule != NULL) {
		FreeLibrary(scimodule);
		scimodule = 0;	
	}
}

void ui_editor_load(psy_ui_Editor* self, const char* path)
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

void ui_editor_settext(psy_ui_Editor* self, const char* text)
{
	SendMessage((HWND) self->component.hwnd,
		SCI_SETTEXT,
		(WPARAM)(intptr_t)strlen(text),
		(LPARAM)(intptr_t) text);		
}

void ui_editor_addtext(psy_ui_Editor* self, const char* text)
{
	SendMessage((HWND) self->component.hwnd,
		SCI_ADDTEXT,
		(WPARAM)(intptr_t)strlen(text),
		(LPARAM)(intptr_t) text);		
}

void ui_editor_clear(psy_ui_Editor* self)
{
	SendMessage((HWND) self->component.hwnd, SCI_CLEARALL, 0, 0);
}

void ui_editor_setcolor(psy_ui_Editor* self, uint32_t color)
{
	SendMessage((HWND) self->component.hwnd, 
		SCI_STYLESETFORE, STYLE_DEFAULT, color);  
}

void ui_editor_setbackgroundcolor(psy_ui_Editor* self, uint32_t color)
{
	SendMessage((HWND) self->component.hwnd, 
		SCI_STYLESETBACK, STYLE_DEFAULT, color);  
}

void ui_editor_styleclearall(psy_ui_Editor* self)
{
	SendMessage((HWND) self->component.hwnd, SCI_STYLECLEARALL, 0, 0);
}

void ui_editor_setcaretcolor(psy_ui_Editor* self, uint32_t color)
{
	SendMessage((HWND) self->component.hwnd, SCI_SETCARETFORE, color, 0);
}

void ui_editor_preventedit(psy_ui_Editor* self)
{
	SendMessage((HWND) self->component.hwnd, SCI_SETREADONLY, 1, 0);
}

void ui_editor_enableedit(psy_ui_Editor* self)
{
	SendMessage((HWND) self->component.hwnd, SCI_SETREADONLY, 0, 0);
}


