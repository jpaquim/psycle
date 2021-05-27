/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwincomboboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwincomponentimp.h"
/* local */
#include "../../uiapp.h"
#include "../../uicomponent.h"
#include "../../uiimpfactory.h"
#include "uiwinapp.h"
/* platform */
#include "../../detail/portable.h"

static void oncommand(psy_ui_ComboBox*, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam);
static psy_ui_RealPoint translatecoords(psy_ui_Component* src,
	psy_ui_Component* dst);

/* ComboBoxImp VTable */
static intptr_t dev_addtext(psy_ui_win_ComboBoxImp*, const char* text);
static void dev_settext(psy_ui_win_ComboBoxImp*, const char* text, intptr_t index);
static void dev_text(const psy_ui_win_ComboBoxImp*, char* text);
static void dev_clear(psy_ui_win_ComboBoxImp*);
static void dev_setcursel(psy_ui_win_ComboBoxImp*, intptr_t index);
static intptr_t dev_cursel(const psy_ui_win_ComboBoxImp*);
static void dev_selitems(psy_ui_win_ComboBoxImp*, intptr_t* items, intptr_t maxitems);
static intptr_t dev_selcount(const psy_ui_win_ComboBoxImp*);
static intptr_t dev_count(const psy_ui_win_ComboBoxImp*);
static void dev_showdropdown(psy_ui_win_ComboBoxImp*);

static psy_ui_ComboboxImpVTable comboboximp_vtable;
static bool comboboximp_vtable_initialized = FALSE;

static void comboboximp_imp_vtable_init(psy_ui_ComponentImp* self)
{
	if (!comboboximp_vtable_initialized) {
		comboboximp_vtable = *((psy_ui_ComboboxImpVTable*)self->extended_vtable);
		comboboximp_vtable.dev_addtext = (psy_ui_fp_comboboximp_dev_addtext)dev_addtext;
		comboboximp_vtable.dev_settext = (psy_ui_fp_comboboximp_dev_settext)dev_settext;
		comboboximp_vtable.dev_text = (psy_ui_fp_comboboximp_dev_text)dev_text;		
		comboboximp_vtable.dev_clear = (psy_ui_fp_comboboximp_dev_clear)dev_clear;
		comboboximp_vtable.dev_setcursel = (psy_ui_fp_comboboximp_dev_setcursel)dev_setcursel;
		comboboximp_vtable.dev_cursel = (psy_ui_fp_comboboximp_dev_cursel)dev_cursel;
		comboboximp_vtable.dev_count = (psy_ui_fp_comboboximp_dev_count)dev_count;
		comboboximp_vtable.dev_selitems = (psy_ui_fp_comboboximp_dev_selitems)dev_selitems;
		comboboximp_vtable.dev_selcount = (psy_ui_fp_comboboximp_dev_selcount)dev_selcount;
		comboboximp_vtable.dev_showdropdown = (psy_ui_fp_comboboximp_dev_showdropdown)dev_showdropdown;
		comboboximp_vtable_initialized = TRUE;
	}
	self->extended_vtable = (void*)&comboboximp_vtable;
}
/* implementation */
void psy_ui_win_comboboximp_init(psy_ui_win_ComboBoxImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view)
{	
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;	
	self->component = component;
	self->view = view;	
	psy_ui_win_componentimp_init(&self->win_combo_imp,
		0,
		(view)
		? view->imp
		: parent->imp,
		TEXT("COMBOBOX"),
		0, 0, 100, 20,
		WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);	
	psy_ui_comboboximp_extend(&self->win_combo_imp.imp);		
	comboboximp_imp_vtable_init(&self->win_combo_imp.imp);
	self->win_combo_imp.imp.extended_imp = self;
	psy_signal_connect(&self->win_combo_imp.imp.signal_command, component, oncommand);
}

psy_ui_win_ComboBoxImp* psy_ui_win_comboboximp_alloc(void)
{
	return (psy_ui_win_ComboBoxImp*)malloc(sizeof(psy_ui_win_ComboBoxImp));
}

psy_ui_win_ComboBoxImp* psy_ui_win_comboboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_win_ComboBoxImp* rv;

	rv = psy_ui_win_comboboximp_alloc();
	if (rv) {
		psy_ui_win_comboboximp_init(rv, component, parent, view);
	}
	return rv;
}

intptr_t dev_addtext(psy_ui_win_ComboBoxImp* self, const char* text)
{
	return SendMessage(self->win_combo_imp.hwnd, CB_ADDSTRING, 0, (LPARAM)text);
}

void dev_settext(psy_ui_win_ComboBoxImp* self, const char* text, intptr_t index)
{
	intptr_t sel;

	sel = SendMessage(self->win_combo_imp.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (sel != -1) {
		SendMessage(self->win_combo_imp.hwnd, CB_DELETESTRING, (WPARAM)index, (LPARAM)text);
		SendMessage(self->win_combo_imp.hwnd, CB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
		SendMessage(self->win_combo_imp.hwnd, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
		SetWindowText(self->win_combo_imp.hwnd, text);
	}
}

void dev_text(const psy_ui_win_ComboBoxImp* self, char* text)
{
	intptr_t sel;

	sel = dev_cursel(self);
	if (sel != -1) {
		intptr_t len;

		len = SendMessage(self->win_combo_imp.hwnd, CB_GETLBTEXTLEN, (WPARAM)sel, 0);
		if (len > 0) {
			SendMessage(self->win_combo_imp.hwnd, CB_GETLBTEXT, (WPARAM)sel,
				(LPARAM)text);
		} else {
			text[0] = '\0';
		}
	} else {
		text[0] = '\0';
	}
}

void dev_clear(psy_ui_win_ComboBoxImp* self)
{
	SendMessage(self->win_combo_imp.hwnd, CB_RESETCONTENT, 0, (LPARAM)0);
	psy_ui_component_invalidate(self->component);
}

void dev_setcursel(psy_ui_win_ComboBoxImp* self, intptr_t index)
{
	char text[512];
	intptr_t len;
	
	SendMessage(self->win_combo_imp.hwnd, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
	len = SendMessage(self->win_combo_imp.hwnd, CB_GETLBTEXTLEN, (WPARAM)index, 0);
	SendMessage(self->win_combo_imp.hwnd, CB_GETLBTEXT, (WPARAM)index,
		(LPARAM)text);	
	psy_ui_component_invalidate(self->component);
}

intptr_t dev_cursel(const psy_ui_win_ComboBoxImp* self)
{
	return SendMessage(self->win_combo_imp.hwnd, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
}

intptr_t dev_count(const psy_ui_win_ComboBoxImp* self)
{
	return SendMessage(self->win_combo_imp.hwnd, CB_GETCOUNT, 0, (LPARAM) 0);
}

void dev_selitems(psy_ui_win_ComboBoxImp* self, intptr_t* items, intptr_t maxitems)
{
	/*
	** SendMessage(self->win_combo_imp.hwnd, CB_GETSELITEMS, (WPARAM)maxitems,
	** (LPARAM)items);
	*/
}

intptr_t dev_selcount(const psy_ui_win_ComboBoxImp* self)
{
	return 0;
}

void oncommand(psy_ui_ComboBox* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch (HIWORD(wParam))
	{
		case CBN_SELCHANGE:
		{
			if (self->signal_selchanged.slots) {
				intptr_t sel = psy_ui_combobox_cursel(self);
				psy_signal_emit(&self->signal_selchanged, self, 1, sel);				
			}
			psy_ui_component_invalidate(&self->component);
		}
		break;
		default:
		break;
	}
}

void dev_showdropdown(psy_ui_win_ComboBoxImp* self)
{
	psy_ui_Size size;	
	psy_ui_RealRectangle position;

	size = psy_ui_component_scrollsize(self->component);
	position = psy_ui_component_position(self->component);
	if (self->view) {
		psy_ui_RealPoint translation;
		
		translation = translatecoords(self->component, self->view);		
		self->win_combo_imp.imp.vtable->dev_setposition(&self->win_combo_imp.imp,			
				psy_ui_point_make(
					psy_ui_value_make_px(translation.x + position.left),
					psy_ui_value_make_px(translation.y + position.top)),
				psy_ui_size_make(size.width, psy_ui_value_make_eh(10.0)));
	} else {
		self->win_combo_imp.imp.vtable->dev_setposition(&self->win_combo_imp.imp,
			psy_ui_point_make(
				psy_ui_value_make_px(position.left),
				psy_ui_value_make_px(position.top)),
			psy_ui_size_make(size.width, psy_ui_value_make_eh(10.0)));
	}
	SendMessage(self->win_combo_imp.hwnd, CB_SHOWDROPDOWN,
		(WPARAM)TRUE, (LPARAM)0);
}

psy_ui_RealPoint translatecoords(psy_ui_Component* src,
	psy_ui_Component* dst)
{
	psy_ui_RealPoint rv;
	psy_ui_Component* curr;
	psy_ui_RealRectangle r;

	curr = psy_ui_component_parent(src);
	psy_ui_realpoint_init(&rv);
	while (dst != curr && curr != NULL) {
		r = psy_ui_component_position(curr);
		psy_ui_realpoint_add(&rv, psy_ui_realrectangle_topleft(&r));
		curr = psy_ui_component_parent(curr);
	}
	return rv;
}

#endif /* PSYCLE_TK_WIN32 */
