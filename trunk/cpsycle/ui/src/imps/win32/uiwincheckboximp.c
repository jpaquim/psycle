// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwincheckboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwincomponentimp.h"
#include "../../uicomponent.h"
#include "../../uiapp.h"
#include "uiwinapp.h"
#include "../../detail/portable.h"

static psy_ui_Size dev_preferredsize(psy_ui_win_ComponentImp*,
	const psy_ui_Size* limits);
// vtable
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.dev_preferredsize =
			(psy_ui_fp_componentimp_dev_preferredsize)
			dev_preferredsize;
		vtable_initialized = 0;
	}
	self->imp.vtable = &vtable;
}

static void oncommand(psy_ui_CheckBox*, psy_ui_Component*,
	WPARAM wParam, LPARAM lParam);

// CheckBoxImp VTable
static void dev_settext(psy_ui_win_ComponentImp*, const char* text);
static void dev_text(psy_ui_win_ComponentImp*, char* text);
static void dev_check(psy_ui_win_ComponentImp*);
static void dev_disablecheck(psy_ui_win_ComponentImp*);
static int dev_checked(psy_ui_win_ComponentImp*);

static psy_ui_CheckBoxImpVTable checkboximp_vtable;
static bool checkboximp_vtable_initialized = FALSE;

static void checkboximp_imp_vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!checkboximp_vtable_initialized) {
		checkboximp_vtable = *((psy_ui_CheckBoxImpVTable*)self->imp.extended_vtable);
		checkboximp_vtable.dev_settext =
			(psy_ui_fp_checkboximp_dev_settext)
			dev_settext;
		checkboximp_vtable.dev_text =
			(psy_ui_fp_checkboximp_dev_text)
			dev_text;
		checkboximp_vtable.dev_check =
			(psy_ui_fp_checkboximp_dev_check)
			dev_check;
		checkboximp_vtable.dev_disablecheck =
			(psy_ui_fp_checkboximp_dev_disablecheck)
			dev_disablecheck;
		checkboximp_vtable.dev_checked =
			(psy_ui_fp_checkboximp_dev_checked)
			dev_checked;
		checkboximp_vtable_initialized = TRUE;
	}
	self->imp.extended_vtable = (void*)&checkboximp_vtable;
}

void psy_ui_win_checkboximp_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(self,
		component,
		parent,
		TEXT("BUTTON"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_FLAT,
		1);
	vtable_init(self);
	psy_ui_checkboximp_extend(&self->imp);
	checkboximp_imp_vtable_init(self);
	self->imp.extended_imp = self;
	psy_signal_connect(&self->imp.signal_command, component, oncommand);
}

void psy_ui_win_checkboximp_init_multiline(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(self,
		component,
		parent,
		TEXT("BUTTON"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_MULTILINE | BS_FLAT,
		1);	
	vtable_init(self);
	psy_ui_checkboximp_extend(&self->imp);
	self->imp.extended_imp = self;
	checkboximp_imp_vtable_init(self);	
	psy_signal_connect(&self->imp.signal_command, component, oncommand);
}

psy_ui_win_ComponentImp* psy_ui_win_checkboximp_alloc(void)
{
	return (psy_ui_win_ComponentImp*)malloc(sizeof(psy_ui_win_ComponentImp));
}

psy_ui_win_ComponentImp* psy_ui_win_checkboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_checkboximp_alloc();
	if (rv) {
		psy_ui_win_checkboximp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_win_ComponentImp* psy_ui_win_checkboximp_allocinit_multiline(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_checkboximp_alloc();
	if (rv) {
		psy_ui_win_checkboximp_init_multiline(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_win_ComponentImp* self, const char* text)
{
	SetWindowText(self->hwnd, text);		
}

void dev_text(psy_ui_win_ComponentImp* self, char* text)
{	
	GetWindowText(self->hwnd, text, 256);
}

void dev_check(psy_ui_win_ComponentImp* self)
{
	if (!dev_checked(self)) {
		SendMessage(self->hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED,
			(LPARAM)0);
	}
}

void dev_disablecheck(psy_ui_win_ComponentImp* self)
{
	if (dev_checked(self)) {
		SendMessage(self->hwnd, BM_SETCHECK, (WPARAM)0, (LPARAM)0);
	}
}

int dev_checked(psy_ui_win_ComponentImp* self)
{
	return SendMessage(self->hwnd, BM_GETCHECK, (WPARAM)0,
		(LPARAM)0) != 0;
}

psy_ui_Size dev_preferredsize(psy_ui_win_ComponentImp* self, const psy_ui_Size* limits)
{
	psy_ui_Size rv;
	psy_ui_Graphics g;
	HDC hdc;
	HWND hwnd;
	char text[512];
	RECT rect = { 0 };
	const psy_ui_TextMetric* tm;

	hwnd = self->hwnd;
	hdc = GetDC(NULL);	
	psy_ui_graphics_init(&g, hdc);
	// psy_ui_setfont(&g, psy_ui_component_font(self->win_component_imp.component));
	dev_text(self, text);	
	tm = psy_ui_component_textmetric(self->component);
	rect.left = 0;
	// todo: 30 only estimated (size of the checkbox)
	rect.right = (int)psy_ui_value_px(&limits->width, tm, NULL) - 30;
	DrawText(hdc, text, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);	
	psy_ui_graphics_dispose(&g);	
	ReleaseDC(NULL, hdc);
	rv.width = psy_ui_value_make_px(rect.right);
	rv.height = psy_ui_value_make_px(rect.bottom);
	return rv;
}

void oncommand(psy_ui_CheckBox* self, psy_ui_Component* sender,
	WPARAM wParam, LPARAM lParam)
{
	switch (HIWORD(wParam))
	{
	case BN_CLICKED:
	{
		if (self->signal_clicked.slots) {
			psy_signal_emit(&self->signal_clicked, self, 0);
		}
	}
	break;
	default:
		break;
	}
}

#endif
