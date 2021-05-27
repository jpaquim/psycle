// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"


#include "uix11comboboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

#include "uix11componentimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uix11app.h"
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;

// ComboBoxImp VTable
static int dev_addtext(psy_ui_x11_ComboBoxImp*, const char* text);
static void dev_settext(psy_ui_x11_ComboBoxImp*, const char* text,
	intptr_t index);
static void dev_text(psy_ui_x11_ComboBoxImp*, char* text);
static void dev_clear(psy_ui_x11_ComboBoxImp*);
static void dev_setcursel(psy_ui_x11_ComboBoxImp*, intptr_t index);
static intptr_t dev_cursel(psy_ui_x11_ComboBoxImp*);
static void dev_selitems(psy_ui_x11_ComboBoxImp*, int* items, int maxitems);
static intptr_t dev_selcount(psy_ui_x11_ComboBoxImp*);
static intptr_t dev_count(psy_ui_x11_ComboBoxImp*);
static void dev_showdropdown(psy_ui_x11_ComboBoxImp*);
static psy_ui_RealPoint translatecoords(psy_ui_Component* src,
	psy_ui_Component* dst);

static void onlistbox_selected(psy_ui_x11_ComboBoxImp*, psy_ui_ListBox* sender,
	intptr_t index);

static psy_ui_ComboboxImpVTable comboboximp_vtable;
static int comboboximp_vtable_initialized = 0;

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

void psy_ui_x11_comboboximp_init(psy_ui_x11_ComboBoxImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view)
{	
	psy_ui_X11App* x11app;
	psy_ui_x11_ComponentImp* x11_combo_imp;	
	XSetWindowAttributes xattr;	
	unsigned long xattrmask = CWOverrideRedirect;
	
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	self->component = component;
	self->view = view;
	psy_ui_listbox_init(&self->x11_combo, NULL);	
	psy_signal_connect(&self->x11_combo.signal_selchanged, self,
		onlistbox_selected);	
	x11_combo_imp = (psy_ui_x11_ComponentImp*)self->x11_combo.component.imp;
	xattr.override_redirect = True;
	XChangeWindowAttributes(x11app->dpy, x11_combo_imp->hwnd, xattrmask,
		&xattr);	
	psy_ui_comboboximp_extend(&self->imp);	
	self->imp.vtable = x11_combo_imp->imp.vtable;	
	self->imp.extended_imp = self;	
	comboboximp_imp_vtable_init(&self->imp);	
}

psy_ui_x11_ComboBoxImp* psy_ui_x11_comboboximp_alloc(void)
{
	return (psy_ui_x11_ComboBoxImp*)malloc(sizeof(psy_ui_x11_ComboBoxImp));
}

psy_ui_x11_ComboBoxImp* psy_ui_x11_comboboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_x11_ComboBoxImp* rv;

	rv = psy_ui_x11_comboboximp_alloc();
	if (rv) {
		psy_ui_x11_comboboximp_init(rv, component, parent, view);
	}
	return rv;
}

int dev_addtext(psy_ui_x11_ComboBoxImp* self, const char* text)
{	
	psy_ui_listbox_addtext(&self->x11_combo, text);
}

void dev_settext(psy_ui_x11_ComboBoxImp* self, const char* text, intptr_t index)
{
	psy_ui_listbox_settext(&self->x11_combo, text, index);	
}

void dev_text(psy_ui_x11_ComboBoxImp* self, char* text)
{
	psy_ui_listbox_text(&self->x11_combo, text,
	 	psy_ui_listbox_cursel(&self->x11_combo));	
}

void dev_clear(psy_ui_x11_ComboBoxImp* self)
{
	psy_ui_listbox_clear(&self->x11_combo);
}

void dev_setcursel(psy_ui_x11_ComboBoxImp* self, intptr_t index)
{	
	psy_ui_listbox_setcursel(&self->x11_combo, index);
	psy_ui_component_invalidate(self->component);
}

intptr_t dev_cursel(psy_ui_x11_ComboBoxImp* self)
{
	return psy_ui_listbox_cursel(&self->x11_combo);	
}

intptr_t dev_count(psy_ui_x11_ComboBoxImp* self)
{
	return 0;
}

void dev_selitems(psy_ui_x11_ComboBoxImp* self, int* items, int maxitems)
{
	//SendMessage(self->xt_combo_imp.hwnd, CB_GETSELITEMS, (WPARAM)maxitems,
	//	(LPARAM)items);
}

intptr_t dev_selcount(psy_ui_x11_ComboBoxImp* self)
{
	return 0;
}

void dev_showdropdown(psy_ui_x11_ComboBoxImp* self)
{		
	psy_ui_X11App* x11app;
	psy_ui_x11_ComponentImp* x11_combo_imp;
	psy_ui_Size size;	
	psy_ui_RealRectangle position;

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	size = psy_ui_component_scrollsize(self->component);
	position = psy_ui_component_screenposition(self->component);
	x11_combo_imp = (psy_ui_x11_ComponentImp*)self->x11_combo.component.imp;
	if (self->view) {
		psy_ui_RealPoint translation;
		
		translation = translatecoords(self->component, self->view);		
		x11_combo_imp->imp.vtable->dev_setposition(&x11_combo_imp->imp,			
				psy_ui_point_make(
					psy_ui_value_make_px(translation.x + position.left),
					psy_ui_value_make_px(translation.y + position.top)),
				psy_ui_size_make(size.width, psy_ui_value_make_eh(10.0)));
	} else {
		x11_combo_imp->imp.vtable->dev_setposition(&x11_combo_imp->imp,
			psy_ui_point_make(
				psy_ui_value_make_px(position.left),
				psy_ui_value_make_px(position.top)),
			psy_ui_size_make(size.width, psy_ui_value_make_eh(10.0)));
	}
	psy_ui_component_show(&self->x11_combo.component);	
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


void onlistbox_selected(psy_ui_x11_ComboBoxImp* self, psy_ui_ListBox* sender,
	intptr_t index)
{	
	psy_ui_component_releasecapture(&self->x11_combo.component);
	psy_ui_component_hide(&self->x11_combo.component);
	psy_ui_component_invalidate(self->component);
	psy_signal_emit(&((psy_ui_ComboBox*)self->component)->signal_selchanged,
		self->component, 1, index);	
}

	
	
	

#endif
