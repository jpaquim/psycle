// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11comboboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uix11componentimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uix11app.h"
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;

// WinComponentImp VTable Delegation
static void dev_dispose(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_dispose(&self->x11_component_imp.imp); }
static void dev_destroy(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_destroy(&self->x11_component_imp.imp); }
static void dev_show(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_show(&self->x11_component_imp.imp); }
static void dev_showstate(psy_ui_x11_ComboBoxImp* self, int state) { self->x11_component_imp.imp.vtable->dev_showstate(&self->x11_component_imp.imp, state); }
static void dev_hide(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_hide(&self->x11_component_imp.imp); }
static int dev_visible(psy_ui_x11_ComboBoxImp* self) { return self->x11_component_imp.imp.vtable->dev_visible(&self->x11_component_imp.imp); }
static void dev_move(psy_ui_x11_ComboBoxImp* self, psy_ui_Point origin) { self->x11_component_imp.imp.vtable->dev_move(&self->x11_component_imp.imp, origin); }
static void dev_resize(psy_ui_x11_ComboBoxImp* self, psy_ui_Size size) { self->x11_component_imp.imp.vtable->dev_resize(&self->x11_component_imp.imp, size); }
static void dev_clientresize(psy_ui_x11_ComboBoxImp* self, int width, int height) { self->x11_component_imp.imp.vtable->dev_clientresize(&self->x11_component_imp.imp, width, height); }
static psy_ui_RealRectangle dev_position(psy_ui_x11_ComboBoxImp* self) { return self->x11_component_imp.imp.vtable->dev_position(&self->x11_component_imp.imp); }
static void dev_setposition(psy_ui_x11_ComboBoxImp* self, psy_ui_Point topleft, psy_ui_Size size) { self->x11_component_imp.imp.vtable->dev_setposition(&self->x11_component_imp.imp, topleft, size); }
static psy_ui_Size dev_size(const psy_ui_x11_ComboBoxImp* self) { return self->x11_component_imp.imp.vtable->dev_size(&self->x11_component_imp.imp); }
static psy_ui_Size dev_framesize(psy_ui_x11_ComboBoxImp* self) { return self->x11_component_imp.imp.vtable->dev_framesize(&self->x11_component_imp.imp); }
static void dev_scrollto(psy_ui_x11_ComboBoxImp* self, intptr_t dx, intptr_t dy) { self->x11_component_imp.imp.vtable->dev_scrollto(&self->x11_component_imp.imp, dx, dy); }
static psy_ui_Component* dev_parent(psy_ui_x11_ComboBoxImp* self) { return self->x11_component_imp.imp.vtable->dev_parent(&self->x11_component_imp.imp); }
static void dev_capture(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_capture(&self->x11_component_imp.imp); }
static void dev_releasecapture(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_releasecapture(&self->x11_component_imp.imp); }
static void dev_invalidate(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_invalidate(&self->x11_component_imp.imp); }
static void dev_invalidaterect(psy_ui_x11_ComboBoxImp* self, const psy_ui_RealRectangle* r) { self->x11_component_imp.imp.vtable->dev_invalidaterect(&self->x11_component_imp.imp, r); }
static void dev_update(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_update(&self->x11_component_imp.imp); }
static void dev_setfont(psy_ui_x11_ComboBoxImp* self, psy_ui_Font* font) { self->x11_component_imp.imp.vtable->dev_setfont(&self->x11_component_imp.imp, font); }
static psy_List* dev_children(psy_ui_x11_ComboBoxImp* self, int recursive) { return self->x11_component_imp.imp.vtable->dev_children(&self->x11_component_imp.imp, recursive); }
static void dev_enableinput(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_enableinput(&self->x11_component_imp.imp); }
static void dev_preventinput(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_preventinput(&self->x11_component_imp.imp); }
static void dev_setcursor(psy_ui_x11_ComboBoxImp* self, psy_ui_CursorStyle style) { self->x11_component_imp.imp.vtable->dev_setcursor(&self->x11_component_imp.imp, style); }
static void dev_starttimer(psy_ui_x11_ComboBoxImp* self, uintptr_t id, uintptr_t interval) { self->x11_component_imp.imp.vtable->dev_starttimer(&self->x11_component_imp.imp, id, interval); }
static void dev_stoptimer(psy_ui_x11_ComboBoxImp* self, uintptr_t id) { self->x11_component_imp.imp.vtable->dev_stoptimer(&self->x11_component_imp.imp, id); }
static void dev_seticonressource(psy_ui_x11_ComboBoxImp* self, int ressourceid) { self->x11_component_imp.imp.vtable->dev_seticonressource(&self->x11_component_imp.imp, ressourceid); }
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_x11_ComboBoxImp* self, psy_ui_Font* font) { return self->x11_component_imp.imp.vtable->dev_textmetric(&self->x11_component_imp.imp); }
static psy_ui_Size dev_textsize(psy_ui_x11_ComboBoxImp* self, const char* text, psy_ui_Font* font) { return self->x11_component_imp.imp.vtable->dev_textsize(&self->x11_component_imp.imp, text, font); }
static void dev_setbackgroundcolour(psy_ui_x11_ComboBoxImp* self, psy_ui_Colour colour) { self->x11_component_imp.imp.vtable->dev_setbackgroundcolour(&self->x11_component_imp.imp, colour); }
static void dev_settitle(psy_ui_x11_ComboBoxImp* self, const char* title) { self->x11_component_imp.imp.vtable->dev_settitle(&self->x11_component_imp.imp, title); }
static void dev_setfocus(psy_ui_x11_ComboBoxImp* self) { self->x11_component_imp.imp.vtable->dev_setfocus(&self->x11_component_imp.imp); }
static int dev_hasfocus(psy_ui_x11_ComboBoxImp* self) { return self->x11_component_imp.imp.vtable->dev_hasfocus(&self->x11_component_imp.imp); }
static void* dev_platform(psy_ui_x11_ComboBoxImp* self) { return (void*)&self->x11_component_imp; }

// VTable init
static psy_ui_ComponentImpVTable vtable;
static int vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dev_dispose = (psy_ui_fp_componentimp_dev_dispose)dev_dispose;
		vtable.dev_destroy = (psy_ui_fp_componentimp_dev_destroy)dev_destroy;
		vtable.dev_show = (psy_ui_fp_componentimp_dev_show)dev_show;
		vtable.dev_showstate = (psy_ui_fp_componentimp_dev_showstate)dev_showstate;
		vtable.dev_hide = (psy_ui_fp_componentimp_dev_hide)dev_hide;
		vtable.dev_visible = (psy_ui_fp_componentimp_dev_visible)dev_visible;
		vtable.dev_move = (psy_ui_fp_componentimp_dev_move)dev_move;
		vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize)dev_resize;
		vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize)dev_clientresize;
		vtable.dev_position = (psy_ui_fp_componentimp_dev_position)dev_position;
		vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition)dev_setposition;
		vtable.dev_size = (psy_ui_fp_componentimp_dev_size)dev_size;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize)dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto)dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent)dev_parent;
		vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture)dev_capture;
		vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture)dev_releasecapture;
		vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate)dev_invalidate;
		vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect)dev_invalidaterect;
		vtable.dev_update = (psy_ui_fp_componentimp_dev_update)dev_update;
		vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont)dev_setfont;
		vtable.dev_children = (psy_ui_fp_componentimp_dev_children)dev_children;
		vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput)dev_enableinput;
		vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput)dev_preventinput;
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor)dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer)dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer)dev_stoptimer;
		vtable.dev_seticonressource = (psy_ui_fp_componentimp_dev_seticonressource)dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric)dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize)dev_textsize;
		vtable.dev_setbackgroundcolour = (psy_ui_fp_componentimp_dev_setbackgroundcolour)dev_setbackgroundcolour;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle)dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus)dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus)dev_hasfocus;
		vtable.dev_platform = (psy_ui_fp_componentimp_dev_platform)dev_platform;
		vtable_initialized = 1;
	}
}

// ComboBoxImp VTable
static int dev_addtext(psy_ui_x11_ComboBoxImp*, const char* text);
static void dev_settext(psy_ui_x11_ComboBoxImp*, const char* text,
	intptr_t index);
static void dev_text(psy_ui_x11_ComboBoxImp*, char* text);
static void dev_setstyle(psy_ui_x11_ComboBoxImp*, int style);
static void dev_clear(psy_ui_x11_ComboBoxImp*);
static void dev_setcursel(psy_ui_x11_ComboBoxImp*, intptr_t index);
static intptr_t dev_cursel(psy_ui_x11_ComboBoxImp*);
static void dev_selitems(psy_ui_x11_ComboBoxImp*, int* items, int maxitems);
static intptr_t dev_selcount(psy_ui_x11_ComboBoxImp*);
static intptr_t dev_count(psy_ui_x11_ComboBoxImp*);
static void dev_showdropdown(psy_ui_x11_ComboBoxImp*);

static void onlistbox_selected(psy_ui_x11_ComboBoxImp*, psy_ui_ListBox* sender,
	intptr_t index);

static psy_ui_ComboBoxImpVTable comboboximp_vtable;
static int comboboximp_vtable_initialized = 0;

static void comboboximp_imp_vtable_init(psy_ui_x11_ComboBoxImp* self)
{
	if (!comboboximp_vtable_initialized) {
		comboboximp_vtable.dev_addtext =
			(psy_ui_fp_comboboximp_dev_addtext)dev_addtext;
		comboboximp_vtable.dev_settext =
			(psy_ui_fp_comboboximp_dev_settext)dev_settext;
		comboboximp_vtable.dev_text =
			(psy_ui_fp_comboboximp_dev_text)dev_text;
		comboboximp_vtable.dev_setstyle =
			(psy_ui_fp_comboboximp_dev_setstyle)dev_setstyle;
		comboboximp_vtable.dev_clear =
			(psy_ui_fp_comboboximp_dev_clear)dev_clear;
		comboboximp_vtable.dev_setcursel =
			(psy_ui_fp_comboboximp_dev_setcursel)dev_setcursel;
		comboboximp_vtable.dev_cursel =
			(psy_ui_fp_comboboximp_dev_cursel)dev_cursel;
		comboboximp_vtable.dev_count =
			(psy_ui_fp_comboboximp_dev_count)dev_count;
		comboboximp_vtable.dev_selitems =
			(psy_ui_fp_comboboximp_dev_selitems)dev_selitems;
		comboboximp_vtable.dev_selcount =
			(psy_ui_fp_comboboximp_dev_selcount)dev_selcount;
		comboboximp_vtable.dev_showdropdown =
			(psy_ui_fp_comboboximp_dev_showdropdown)dev_showdropdown;
		comboboximp_vtable_initialized = 1;
	}
}

void psy_ui_x11_comboboximp_init(psy_ui_x11_ComboBoxImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{	
	psy_ui_X11App* x11app;
	XSetWindowAttributes xattr;
	psy_ui_x11_ComponentImp* x11_combo_imp;
	unsigned long xattrmask = CWOverrideRedirect;

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	psy_ui_x11_componentimp_init(&self->x11_component_imp,
		component,
		parent,
		x11app->componentclass,
		0, 0, 90, 90,
        0,
		0);
    imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_listbox_init(&self->x11_combo, 0);
	psy_signal_connect(&self->x11_combo.signal_selchanged, self,
		onlistbox_selected);	
	x11_combo_imp = (psy_ui_x11_ComponentImp*)self->x11_combo.component.imp;
	xattr.override_redirect = True;
	XChangeWindowAttributes(x11app->dpy, x11_combo_imp->hwnd, xattrmask,
		&xattr);
	psy_ui_comboboximp_init(&self->imp);	
	self->component = component;
	comboboximp_imp_vtable_init(self);
	self->imp.vtable = &comboboximp_vtable;
}

psy_ui_x11_ComboBoxImp* psy_ui_x11_comboboximp_alloc(void)
{
	return (psy_ui_x11_ComboBoxImp*)malloc(sizeof(psy_ui_x11_ComboBoxImp));
}

psy_ui_x11_ComboBoxImp* psy_ui_x11_comboboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_x11_ComboBoxImp* rv;

	rv = psy_ui_x11_comboboximp_alloc();
	if (rv) {
		psy_ui_x11_comboboximp_init(rv, component, parent);
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

void dev_setstyle(psy_ui_x11_ComboBoxImp* self, int style)
{
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
	dev_invalidate(self);
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
	psy_ui_Rectangle r;
	int x, y;
	Window child_return;
	XWindowAttributes xwa;
	psy_ui_X11App* x11app;
	psy_ui_x11_ComponentImp* x11_combo_imp;

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	x11_combo_imp = (psy_ui_x11_ComponentImp*)self->x11_combo.component.imp;
	XTranslateCoordinates(x11app->dpy, self->x11_component_imp.hwnd,
		XRootWindow(x11app->dpy, DefaultScreen(x11app->dpy)),
		0, 0, &x, &y, &child_return );
	XGetWindowAttributes(x11app->dpy, self->x11_component_imp.hwnd, &xwa);	
	psy_ui_component_setposition(&self->x11_combo.component,
		psy_ui_rectangle_make(
		psy_ui_point_make(
			psy_ui_value_make_px(x), psy_ui_value_make_px(y + xwa.height)),
		psy_ui_size_make(
			psy_ui_value_make_px(xwa.width),
			psy_ui_value_make_eh(10))));	
	psy_ui_x11app_startgrab(x11app, x11_combo_imp->hwnd);
	psy_ui_component_show(&self->x11_combo.component);	
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
