// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinlistboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwincomponentimp.h"
#include "../../uicomponent.h"
#include "../../uiapp.h"
#include "uiwinapp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

static int windowstyle(HWND hwnd);

static void oncommand(psy_ui_ListBox*, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam);

/* ListBoxImp VTable */

static intptr_t dev_addtext(psy_ui_win_ComponentImp*, const char* text);
static void dev_settext(psy_ui_win_ComponentImp*, const char* text, intptr_t index);
static void dev_text(psy_ui_win_ComponentImp*, char* text, intptr_t index);
static void dev_setstyle(psy_ui_win_ComponentImp*, int style);
static void dev_clear(psy_ui_win_ComponentImp*);
static void dev_setcursel(psy_ui_win_ComponentImp*, intptr_t index);
static intptr_t dev_cursel(psy_ui_win_ComponentImp*);
static void dev_addsel(psy_ui_win_ComponentImp*, intptr_t index);
static void dev_selitems(psy_ui_win_ComponentImp*, intptr_t* items,
	intptr_t maxitems);
static intptr_t dev_selcount(psy_ui_win_ComponentImp*);
static intptr_t dev_count(psy_ui_win_ComponentImp*);

static psy_ui_ListBoxImpVTable listboximp_vtable;
static bool listboximp_vtable_initialized = FALSE;

static void listboximp_imp_vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!listboximp_vtable_initialized) {
		listboximp_vtable = *((psy_ui_ListBoxImpVTable*)self->imp.extended_vtable);
		listboximp_vtable.dev_addtext = (psy_ui_fp_listboximp_dev_addtext)dev_addtext;
		listboximp_vtable.dev_settext = (psy_ui_fp_listboximp_dev_settext)dev_settext;
		listboximp_vtable.dev_text = (psy_ui_fp_listboximp_dev_text)dev_text;
		listboximp_vtable.dev_setstyle = (psy_ui_fp_listboximp_dev_setstyle)dev_setstyle;
		listboximp_vtable.dev_clear = (psy_ui_fp_listboximp_dev_clear)dev_clear;
		listboximp_vtable.dev_setcursel = (psy_ui_fp_listboximp_dev_setcursel)dev_setcursel;
		listboximp_vtable.dev_cursel = (psy_ui_fp_listboximp_dev_cursel)dev_cursel;
		listboximp_vtable.dev_addsel = (psy_ui_fp_listboximp_dev_addsel)dev_addsel;
		listboximp_vtable.dev_selitems = (psy_ui_fp_listboximp_dev_selitems)dev_selitems;
		listboximp_vtable.dev_selcount = (psy_ui_fp_listboximp_dev_selcount)dev_selcount;
		listboximp_vtable.dev_count = (psy_ui_fp_listboximp_dev_count)dev_count;
		listboximp_vtable_initialized = TRUE;
	}
	self->imp.extended_vtable = (void*)&listboximp_vtable;
}

void psy_ui_win_listboximp_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{	
	psy_ui_win_componentimp_init(self,
		component,
		parent,
		TEXT("LISTBOX"),
		0, 0, 100, 200,
		(WS_CHILD | WS_VISIBLE | LBS_STANDARD & ~LBS_SORT | LBS_NOTIFY) & ~WS_BORDER,
		1);
	/* vtable_init(self); */
	psy_ui_listboximp_extend(&self->imp);
	listboximp_imp_vtable_init(self);
	self->imp.extended_imp = self;
	psy_signal_connect(&self->imp.signal_command, component, oncommand);
}

void psy_ui_win_listboximp_multiselect_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(self,
		component,
		parent,
		TEXT("LISTBOX"),
		0, 0, 100, 20,
		(WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL & ~LBS_SORT | LBS_NOTIFY)
		& ~WS_BORDER,
		1);
	/* imp_vtable_init(self); */
	psy_ui_listboximp_extend(&self->imp);
	listboximp_imp_vtable_init(self);
	self->imp.extended_imp = self;
	psy_signal_connect(&self->imp.signal_command, component, oncommand);
}

psy_ui_win_ComponentImp* psy_ui_win_listboximp_alloc(void)
{
	return (psy_ui_win_ComponentImp*)malloc(sizeof(psy_ui_win_ComponentImp));
}

psy_ui_win_ComponentImp* psy_ui_win_listboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_listboximp_alloc();
	if (rv) {
		psy_ui_win_listboximp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_win_ComponentImp* psy_ui_win_listboximp_multiselect_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_listboximp_alloc();
	if (rv) {
		psy_ui_win_listboximp_multiselect_init(rv, component, parent);
	}
	return rv;
}

intptr_t dev_addtext(psy_ui_win_ComponentImp* self, const char* text)
{
	return SendMessage(self->hwnd, LB_ADDSTRING, 0, (LPARAM)text);
}

void dev_settext(psy_ui_win_ComponentImp* self, const char* text, intptr_t index)
{
	intptr_t sel;
	char* currtext;
	intptr_t numchars;

	numchars = SendMessage(self->hwnd, LB_GETTEXTLEN, (WPARAM)index, (LPARAM)0);
	currtext = malloc(sizeof(char) * (numchars + 1));
	SendMessage(self->hwnd, LB_GETTEXT, (WPARAM)index, (LPARAM)currtext);
	if (strcmp(currtext, text) != 0) {
		sel = SendMessage(self->hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		if (sel != -1) {
			SendMessage(self->hwnd, LB_DELETESTRING, (WPARAM)index, (LPARAM)text);
			SendMessage(self->hwnd, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
			SendMessage(self->hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);
			SetWindowText(self->hwnd, text);
		}
	}
	free(currtext);
}

void dev_setstyle(psy_ui_win_ComponentImp* self, int style)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE, style);
#else
	SetWindowLong(self->hwnd, GWL_STYLE, style);
#endif
}

void dev_text(psy_ui_win_ComponentImp* self, char* text, intptr_t index)
{
	SendMessage(self->hwnd, LB_GETTEXT, (WPARAM)index, (LPARAM)text);
}

void dev_clear(psy_ui_win_ComponentImp* self)
{
	SendMessage(self->hwnd, LB_RESETCONTENT, 0, (LPARAM)0);
}

void dev_setcursel(psy_ui_win_ComponentImp* self, intptr_t index)
{
	RECT rect;

	if ((windowstyle(self->hwnd) & LBS_EXTENDEDSEL) == LBS_EXTENDEDSEL) {
		SendMessage(self->hwnd, LB_SETSEL, (WPARAM)0, (LPARAM)-1);
		if (index != -1) {
			SendMessage(self->hwnd, LB_SETSEL, (WPARAM)1, (LPARAM)index);
		}
	} else {				
		SendMessage(self->hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);		
	}
	GetClientRect(self->hwnd, &rect);
	if (rect.bottom - rect.top < 20) {
		SendMessage(self->hwnd, LB_SETTOPINDEX, (WPARAM)0, (LPARAM)0);
	}
}

void dev_addsel(psy_ui_win_ComponentImp* self, intptr_t index)
{
	if ((windowstyle(self->hwnd) & LBS_EXTENDEDSEL) == LBS_EXTENDEDSEL) {
		if (index != -1) {
			SendMessage(self->hwnd, LB_SETSEL, (WPARAM)1, (LPARAM)index);
		}
	}
}

intptr_t dev_cursel(psy_ui_win_ComponentImp* self)
{
	return SendMessage(self->hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}

void dev_selitems(psy_ui_win_ComponentImp* self, intptr_t* items, intptr_t maxitems)
{
	if (maxitems > 0) {
		int32_t* win32_items;
		int32_t win32_maxitems;
		int32_t i;

		if (maxitems < INT32_MAX) {
			win32_maxitems = (int32_t)maxitems;
		} else {
			win32_maxitems = INT32_MAX;
		}
		win32_items = (int32_t*)malloc(sizeof(int32_t) * (intptr_t)(maxitems));
		if (win32_items) {
			for (i = 0; i < win32_maxitems; ++i) {
				win32_items[i] = (int32_t)items[i];
			}
			SendMessage(self->hwnd, LB_GETSELITEMS, (WPARAM)win32_maxitems,
				(LPARAM)win32_items);
			for (i = 0; i < win32_maxitems; ++i) {
				items[i] = win32_items[i];
			}
			for (i = win32_maxitems; i < maxitems; ++i) {
				items[i] = -1;
			}
		}
	}
}

intptr_t dev_selcount(psy_ui_win_ComponentImp* self)
{
	return SendMessage(self->hwnd, LB_GETSELCOUNT,
		(WPARAM)0, (LPARAM)0);
}

static intptr_t dev_count(psy_ui_win_ComponentImp* self)
{
	return SendMessage(self->hwnd, LB_GETCOUNT,
		(WPARAM)0, (LPARAM)0);
}

void oncommand(psy_ui_ListBox* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch (HIWORD(wParam))
	{
	case LBN_SELCHANGE:
	{
		if (self->signal_selchanged.slots) {
			intptr_t sel = psy_ui_listbox_cursel(self);
			psy_signal_emit(&self->signal_selchanged, self, 1, sel);
		}
	}
	break;
	default:
		break;
	}
}

int windowstyle(HWND hwnd)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(hwnd, GWL_STYLE);
#else
	rv = (int)GetWindowLong(hwnd, GWL_STYLE);
#endif
	return rv;
}

#endif
