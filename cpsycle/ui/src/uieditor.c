// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uieditor.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiapp.h"
#include "uilabel.h"
#include "uiwincomponentimp.h"
#include "uiimpfactory.h"
#include "scintilla/include/scintilla.h"

#include <stdio.h>
#include <stdlib.h>

extern psy_ui_App app;

static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp;
}


#define SCI_ENABLED 1

static HMODULE scimodule = 0;

#define MAXTEXTLENGTH 65535

static int loadscilexer(void);
static void onappdestroy(void*, psy_ui_App* sender);
static void psy_ui_editor_setfont(psy_ui_Editor*, psy_ui_Font*);
static void psy_ui_editor_styleclearall(psy_ui_Editor*);
static void psy_ui_editor_setcaretcolor(psy_ui_Editor*, uint32_t color);
static intptr_t sci(psy_ui_Editor*, uintptr_t msg, uintptr_t wparam,
	uintptr_t lparam);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;
static psy_ui_fp_component_setfont super_setfont;

static void vtable_init(psy_ui_Editor* self)
{
	if (!vtable_initialized) {		
		vtable = *(self->component.vtable);
		super_setfont = vtable.setfont;
		vtable.setfont = (psy_ui_fp_component_setfont)psy_ui_editor_setfont;
		vtable_initialized = 1;
	}
}
void psy_ui_editor_init(psy_ui_Editor* self, psy_ui_Component* parent)
{  					
#ifdef SCI_ENABLED
	int err;

	if ((err = loadscilexer()) == 0) {
		psy_ui_win_ComponentImp* imp;

		imp = (psy_ui_win_ComponentImp*)malloc(sizeof(psy_ui_win_ComponentImp));
		if (imp) {
			psy_ui_win_componentimp_init(imp,
				&self->component,
				parent ? parent->imp : 0,
				TEXT("Scintilla"),
				0, 0, 100, 20,
				WS_CHILD | WS_VISIBLE,
				0);
			if (imp->hwnd) {
				psy_ui_component_init_imp(&self->component, parent, &imp->imp);
				vtable_init(self);
				self->component.vtable = &vtable;
				psy_ui_editor_setcolor(self, psy_ui_defaults_color(&app.defaults));
				psy_ui_editor_setcaretcolor(self, psy_ui_defaults_color(&app.defaults));
				psy_ui_editor_setbackgroundcolor(self,
					psy_ui_defaults_backgroundcolor(&app.defaults));
				psy_ui_editor_setfont(self, NULL);
				psy_ui_editor_styleclearall(self);				
			}
		}
	} else
#endif	
	{
		psy_ui_LabelImp* imp;
		
		imp = psy_ui_impfactory_allocinit_labelimp(psy_ui_app_impfactory(&app), &self->component, parent);
		psy_ui_component_init_imp(&self->component, parent, &imp->component_imp);
		imp->vtable->dev_setstyle(imp, SS_CENTER);		
#ifdef SCI_ENABLED
		imp->vtable->dev_settext(imp,		
			"Editor can't be used.\n"
			"LoadLibrary SciLexer.dll failure\n"
			"Check if 'SciLexer.dll' is in the psycle bin directory or\n"
			"if you have the right version for your system.");
		psy_ui_error("LoadLibrary SciLexer.dll failure ...",
			"Error - Psycle Ui - Editor");
#else
		imp->vtable->dev_settext(imp,
			"Editor can't be used. Scintilla disabled in build\n");
#endif
	}
}

int loadscilexer(void)
{
#ifdef SCI_ENABLED
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

intptr_t sci(psy_ui_Editor* self, uintptr_t msg, uintptr_t wparam,
	uintptr_t lparam)
{
	return SendMessage((HWND) psy_ui_win_component_details(&self->component)->hwnd,
		msg, (WPARAM) wparam, (LPARAM) lparam);
}

void psy_ui_editor_load(psy_ui_Editor* self, const char* path)
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
		psy_ui_editor_settext(self, text);		
	}
}

void psy_ui_editor_save(psy_ui_Editor* self, const char* path)
{
	FILE* fp;
	
	int size = sci(self, SCI_GETLENGTH, 0, 0) + 1;
	fp = fopen(path, "wb");
	if (fp) {		
		char* buffer;		

		buffer = malloc(sizeof(char) * (size + 1));
		if (buffer) {
			sci(self, SCI_GETTEXT, size, (LPARAM)buffer);			
			fwrite(buffer, sizeof(char), (size - 1) / sizeof(char), fp);
		}				
		fclose(fp);
		free(buffer);
	}	
}

void psy_ui_editor_settext(psy_ui_Editor* self, const char* text)
{
	sci(self, SCI_SETTEXT, strlen(text), (uintptr_t) text);		
}

void psy_ui_editor_addtext(psy_ui_Editor* self, const char* text)
{
	sci(self, SCI_ADDTEXT, strlen(text), (uintptr_t) text);
}

void psy_ui_editor_clear(psy_ui_Editor* self)
{
	sci(self, SCI_CLEARALL, 0, 0);
}

void psy_ui_editor_setcolor(psy_ui_Editor* self, uint32_t color)
{
	sci(self, SCI_STYLESETFORE, STYLE_DEFAULT, color);  
}

void psy_ui_editor_setbackgroundcolor(psy_ui_Editor* self, uint32_t color)
{
	sci(self, SCI_STYLESETBACK, STYLE_DEFAULT, color);  
}

void psy_ui_editor_styleclearall(psy_ui_Editor* self)
{
	sci(self, SCI_STYLECLEARALL, 0, 0);
}

void psy_ui_editor_setcaretcolor(psy_ui_Editor* self, uint32_t color)
{
	sci(self, SCI_SETCARETFORE, color, 0);	
}

void psy_ui_editor_preventedit(psy_ui_Editor* self)
{	
	sci(self, SCI_SETREADONLY, 1, 0);
}

void psy_ui_editor_enableedit(psy_ui_Editor* self)
{
	sci(self, SCI_SETREADONLY, 0, 0);	
}

void psy_ui_editor_setfont(psy_ui_Editor* self, psy_ui_Font* source)
{
	psy_ui_FontInfo fontinfo;
	psy_ui_Font* font;

	if (source) {
		super_setfont(&self->component, source);
	}
	font = psy_ui_component_font(&self->component);
	fontinfo = psy_ui_font_fontinfo(font);
	if (fontinfo.lfHeight < 0) {
		HDC hdc;
		int devcap;

		hdc = GetDC(NULL);
		devcap = GetDeviceCaps(hdc, LOGPIXELSY);
		fontinfo.lfHeight = -fontinfo.lfHeight * 72 / devcap;
		ReleaseDC(NULL, hdc);
	}
	sci(self, SCI_STYLESETSIZE, STYLE_DEFAULT, fontinfo.lfHeight);
	sci(self, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)fontinfo.lfFaceName);
	psy_ui_editor_styleclearall(self);
}

#else

void psy_ui_editor_init(psy_ui_Editor* self, psy_ui_Component* parent)
{

}
void psy_ui_editor_load(psy_ui_Editor* self, const char* path)
{

}

void psy_ui_editor_save(psy_ui_Editor* self, const char* path)
{
    
}

void psy_ui_editor_settext(psy_ui_Editor* self, const char* text)
{

}
void psy_ui_editor_addtext(psy_ui_Editor* self, const char* text)
{

}
void psy_ui_editor_clear(psy_ui_Editor* self)
{

}
void psy_ui_editor_setcolor(psy_ui_Editor* self, uint32_t color)
{

}
void psy_ui_editor_setbackgroundcolor(psy_ui_Editor* v, uint32_t color)
{

}
void psy_ui_editor_enableedit(psy_ui_Editor* self)
{

}
void psy_ui_editor_preventedit(psy_ui_Editor* self)
{

}

#endif

