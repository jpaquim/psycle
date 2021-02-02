// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uieditor.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#define BLOCKSIZE 128 * 1024

#include "uiapp.h"
#include "uilabel.h"
#include "imps/win32/uiwincomponentimp.h"
#include "uiimpfactory.h"
#include "scintilla/include/scintilla.h"
// std
#include <stdio.h>
#include <stdlib.h>

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
static void psy_ui_editor_setcaretcolour(psy_ui_Editor*, psy_ui_Colour colour);
static intptr_t sci(psy_ui_Editor*, uintptr_t msg, uintptr_t wparam,
	uintptr_t lparam);
static void psy_ui_editor_getrange(psy_ui_Editor*, intptr_t start, intptr_t end, char* text);
static void setstyle(psy_ui_Editor*, int style, COLORREF fore, COLORREF back,
	int size, const char* face);
static void psy_ui_editor_onstylesupdate(psy_ui_Editor* self);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;
static psy_ui_fp_component_setfont super_setfont;

static void vtable_init(psy_ui_Editor* self)
{
	if (!vtable_initialized) {		
		vtable = *(self->component.vtable);
		super_setfont = vtable.setfont;
		vtable.setfont = (psy_ui_fp_component_setfont)psy_ui_editor_setfont;
		vtable.onupdatestyles =
			(psy_ui_fp_component_onupdatestyles)
			psy_ui_editor_onstylesupdate;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
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
				psy_ui_editor_setcolour(self, psy_ui_style(psy_ui_STYLE_COMMON)->colour);
				psy_ui_editor_setcaretcolour(self,
					psy_ui_style(psy_ui_STYLE_COMMON)->colour);
				psy_ui_editor_setbackgroundcolour(self,
					psy_ui_style(psy_ui_STYLE_COMMON)->backgroundcolour);
				psy_ui_editor_setfont(self, NULL);
				sci(self, SCI_SETMARGINWIDTHN, 0, 0);
				sci(self, SCI_SETMARGINWIDTHN, 1, 0);
				sci(self, SCI_SETMARGINWIDTHN, 2, 0);
				sci(self, SCI_SETMARGINWIDTHN, 3, 0);
				sci(self, SCI_SETMARGINWIDTHN, 4, 0);
				sci(self, SCI_SETMARGINWIDTHN, 5, 0);
				// sci(self, SCI_SETCARETSTYLE, CARETSTYLE_BLOCK, 0);
				psy_ui_editor_styleclearall(self);				
			}
		}
	} else
#endif	
	{	
		psy_ui_component_init(&self->component, parent);
		/*psy_ui_LabelImp* imp;
		
		imp = psy_ui_impfactory_allocinit_labelimp(psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
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
#endif*/
	}
}

int loadscilexer(void)
{
#ifdef SCI_ENABLED
	if (scimodule == 0) {
		scimodule = LoadLibrary ("SciLexer.dll");
		if (scimodule != NULL) {		
			psy_signal_connect(&psy_ui_app()->signal_dispose, 0, onappdestroy);
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
		(UINT)msg, (WPARAM)wparam, (LPARAM)lparam);
}

void setstyle(psy_ui_Editor* self, int style, COLORREF fore, COLORREF back,
	int size, const char* face)
{
	sci(self, SCI_STYLESETFORE, (uintptr_t)style, (uintptr_t)fore);
	sci(self, SCI_STYLESETBACK, (uintptr_t)style, (uintptr_t)back);
	if (size >= 1)
		sci(self, SCI_STYLESETSIZE, (uintptr_t)style, (uintptr_t)size);
	if (face)
		sci(self, SCI_STYLESETFONT, (uintptr_t)style, (uintptr_t)face);
}

void psy_ui_editor_load(psy_ui_Editor* self, const char* path)
{	
	FILE* fp;
	
	sci(self, SCI_CANCEL, 0, 0);
	sci(self, SCI_SETUNDOCOLLECTION, 0, 0);
	fp = fopen(path, "rb");
	if (fp) {
		char data[BLOCKSIZE];
		uintptr_t lenfile;
		
		lenfile = fread(data, 1, sizeof(data), fp);
		while (lenfile > 0) {
			sci(self, SCI_ADDTEXT, lenfile, (LPARAM)(char*)data);
			lenfile = fread(data, 1, sizeof(data), fp);
		}
		fclose(fp);
	}	
	sci(self, SCI_SETUNDOCOLLECTION, 1, 0);
	sci(self, EM_EMPTYUNDOBUFFER, 0, 0);
	sci(self, SCI_SETSAVEPOINT, 0, 0);
	sci(self, SCI_GOTOPOS, 0, 0);
}

void psy_ui_editor_save(psy_ui_Editor* self, const char* path)
{
	FILE* fp;

	fp = fopen(path, "wb");
	if (fp) {
		char data[BLOCKSIZE + 1];
		intptr_t lengthdoc;
		intptr_t i;

		lengthdoc = sci(self, SCI_GETLENGTH, 0, 0);
		for (i = 0; i < lengthdoc; i += BLOCKSIZE) {
			intptr_t grabsize;
			
			grabsize = lengthdoc - i;
			if (grabsize > BLOCKSIZE)
				grabsize = BLOCKSIZE;
			psy_ui_editor_getrange(self, i, i + grabsize, data);
			fwrite(data, grabsize, 1, fp);
		}
		fclose(fp);
		sci(self, SCI_SETSAVEPOINT, 0, 0);
	}
}

void psy_ui_editor_getrange(psy_ui_Editor* self, intptr_t start, intptr_t end, char* text)
{
	struct TextRange tr;

	tr.chrg.cpMin = (int32_t)start;
	tr.chrg.cpMax = (int32_t)end;
	tr.lpstrText = text;
	sci(self, SCI_GETTEXTRANGE, 0, (LPARAM)(&tr));
}

void psy_ui_editor_settext(psy_ui_Editor* self, const char* text)
{
	if (text) {
		sci(self, SCI_SETTEXT, strlen(text), (uintptr_t)text);
	}
}

void psy_ui_editor_addtext(psy_ui_Editor* self, const char* text)
{
	if (text) {
		sci(self, SCI_ADDTEXT, strlen(text), (uintptr_t)text);
	}
}

char* psy_ui_editor_text(psy_ui_Editor* self, uintptr_t maxlength, char* text)
{
	uintptr_t length;
	
	length = psy_ui_editor_length(self);
	if (length > maxlength) {
		length = maxlength;
	}
	sci(self, SCI_GETTEXT, length, (LPARAM)text);
	return text;
}

uintptr_t psy_ui_editor_length(psy_ui_Editor* self)
{
	return (uintptr_t)sci(self, SCI_GETLENGTH, 0, 0);
}

void psy_ui_editor_clear(psy_ui_Editor* self)
{
	sci(self, SCI_CLEARALL, 0, 0);
}

void psy_ui_editor_setcolour(psy_ui_Editor* self, psy_ui_Colour colour)
{
	sci(self, SCI_STYLESETFORE, STYLE_DEFAULT, colour.value);  
}

void psy_ui_editor_setbackgroundcolour(psy_ui_Editor* self, psy_ui_Colour colour)
{
	sci(self, SCI_STYLESETBACK, STYLE_DEFAULT, colour.value);  
}

void psy_ui_editor_styleclearall(psy_ui_Editor* self)
{
	sci(self, SCI_STYLECLEARALL, 0, 0);
}

void psy_ui_editor_setcaretcolour(psy_ui_Editor* self, psy_ui_Colour colour)
{
	sci(self, SCI_SETCARETFORE, colour.value, 0);	
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
	const psy_ui_Font* font;

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

void psy_ui_editor_enablewrap(psy_ui_Editor* self)
{
	sci(self, SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
}

void psy_ui_editor_disablewrap(psy_ui_Editor* self)
{
	sci(self, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
}

void psy_ui_editor_onstylesupdate(psy_ui_Editor* self)
{
	psy_ui_editor_setcolour(self, psy_ui_style(psy_ui_STYLE_COMMON)->colour);
	psy_ui_editor_setcaretcolour(self,
		psy_ui_style(psy_ui_STYLE_COMMON)->colour);
	psy_ui_editor_setbackgroundcolour(self,
		psy_ui_style(psy_ui_STYLE_COMMON)->backgroundcolour);
	psy_ui_editor_setfont(self, NULL);
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

char* psy_ui_editor_text(psy_ui_Editor* self, uintptr_t maxlength, char* text)
{
}

void psy_ui_editor_clear(psy_ui_Editor* self)
{

}
void psy_ui_editor_setcolour(psy_ui_Editor* self, psy_ui_Colour colour)
{

}
void psy_ui_editor_setbackgroundcolour(psy_ui_Editor* v, psy_ui_Colour colour)
{

}
void psy_ui_editor_enableedit(psy_ui_Editor* self)
{

}
void psy_ui_editor_preventedit(psy_ui_Editor* self)
{

}

void psy_ui_editor_enablewrap(psy_ui_Editor* self)
{
	
}

void psy_ui_editor_disablewrap(psy_ui_Editor* self)
{
	
}

#endif

