/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwineditimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwincomponentimp.h"
#include "../../uicomponent.h"
#include "../../uiapp.h"
#include "uiwinapp.h"
#include "../../detail/portable.h"

static void oncommand(psy_ui_Edit*, psy_ui_Component* sender, WPARAM wParam, LPARAM lParam);

/* EditImp VTable */
static void dev_settext(psy_ui_win_ComponentImp*, const char* text);
static void dev_text(psy_ui_win_ComponentImp* self, char* text);
static void dev_setstyle(psy_ui_win_ComponentImp*, int style);
static void dev_enableedit(psy_ui_win_ComponentImp*);
static void dev_preventedit(psy_ui_win_ComponentImp*);
static void dev_setsel(psy_ui_win_ComponentImp*, intptr_t cpmin, intptr_t cpmax);

static psy_ui_EditImpVTable editimp_vtable;
static bool editimp_vtable_initialized = FALSE;

static void editimp_imp_vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!editimp_vtable_initialized) {
		editimp_vtable = *((psy_ui_EditImpVTable*)self->imp.extended_vtable);
		editimp_vtable.dev_settext = (psy_ui_fp_editimp_dev_settext)dev_settext;
		editimp_vtable.dev_text = (psy_ui_fp_editimp_dev_text)dev_text;
		editimp_vtable.dev_setstyle = (psy_ui_fp_editimp_dev_setstyle)dev_setstyle;
		editimp_vtable.dev_enableedit = (psy_ui_fp_editimp_dev_enableedit)dev_enableedit;
		editimp_vtable.dev_preventedit = (psy_ui_fp_editimp_dev_preventedit)dev_preventedit;
		editimp_vtable.dev_setsel = (psy_ui_fp_editimp_dev_setsel)dev_setsel;
		editimp_vtable_initialized = TRUE;
	}
	self->imp.extended_vtable = (void*)&editimp_vtable;
}

void psy_ui_win_editimp_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{	
	psy_ui_win_componentimp_init(self,
		component,
		parent,
		TEXT("EDIT"),
		0, 0, 100, 20,		
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
		1);	
	psy_ui_editimp_extend(&self->imp);
	editimp_imp_vtable_init(self);
	self->imp.extended_imp = self;
	psy_signal_connect(&self->imp.signal_command, component, oncommand);
}

void psy_ui_win_editimp_multiline_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(self,
		component,
		parent,
		TEXT("EDIT"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_LEFT,
		1);	
	psy_ui_editimp_extend(&self->imp);
	editimp_imp_vtable_init(self);
	self->imp.extended_imp = self;
	psy_signal_connect(&self->imp.signal_command, component, oncommand);
}

psy_ui_win_ComponentImp* psy_ui_win_editimp_alloc(void)
{
	return (psy_ui_win_ComponentImp*) malloc(sizeof(psy_ui_win_ComponentImp));
}

psy_ui_win_ComponentImp* psy_ui_win_editimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_editimp_alloc();
	if (rv) {
		psy_ui_win_editimp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_win_ComponentImp* psy_ui_win_editimp_multiline_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_editimp_alloc();
	if (rv) {
		psy_ui_win_editimp_multiline_init(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_win_ComponentImp* self, const char* text)
{		
	SetWindowText(self->hwnd, (text) ? text : "");
}

void dev_setstyle(psy_ui_win_ComponentImp* self, int style)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE, style);
#else
	SetWindowLong(self->hwnd, GWL_STYLE, style);
#endif
}

void dev_text(psy_ui_win_ComponentImp* self, char* text)
{	
	GetWindowText(self->hwnd, text, 256);
}

void dev_enableedit(psy_ui_win_ComponentImp* self)
{
	SendMessage(self->hwnd, EM_SETREADONLY, (WPARAM)0,
		(LPARAM)0);
}

void dev_preventedit(psy_ui_win_ComponentImp* self)
{
	SendMessage(self->hwnd, EM_SETREADONLY, (WPARAM)1,
		(LPARAM)0);
}

void dev_setsel(psy_ui_win_ComponentImp* self, intptr_t cpmin, intptr_t cpmax)
{
	SendMessage(self->hwnd, EM_SETSEL, (WPARAM)cpmin,
		(LPARAM)cpmax);
}

void oncommand(psy_ui_Edit* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch (HIWORD(wParam))
	{
	case EN_SETFOCUS:		
		self->component.vtable->onfocus(&self->component);
		psy_signal_emit(&self->component.signal_focus, self, 0);
		break;
	case EN_CHANGE:
	{
		if (self->signal_change.slots) {
			psy_signal_emit(&self->signal_change, self, 0);
		}
	}
	break;
	default:
		break;
	}
}

#endif /* TK_WIN32 */
