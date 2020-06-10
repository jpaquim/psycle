// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uixtlistboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uixtcomponentimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uixtapp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

// WinComponentImp VTable Delegation
static void dev_dispose(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_dispose(&self->xt_component_imp.imp); }
static void dev_destroy(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_destroy(&self->xt_component_imp.imp); }
static void dev_show(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_show(&self->xt_component_imp.imp); }
static void dev_showstate(psy_ui_xt_ListBoxImp* self, int state) { self->xt_component_imp.imp.vtable->dev_showstate(&self->xt_component_imp.imp, state); }
static void dev_hide(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_hide(&self->xt_component_imp.imp); }
static int dev_visible(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_visible(&self->xt_component_imp.imp); }
static void dev_move(psy_ui_xt_ListBoxImp* self, int left, int top) { self->xt_component_imp.imp.vtable->dev_move(&self->xt_component_imp.imp, left, top); }
static void dev_resize(psy_ui_xt_ListBoxImp* self, psy_ui_Size size) { self->xt_component_imp.imp.vtable->dev_resize(&self->xt_component_imp.imp, size); }
static void dev_clientresize(psy_ui_xt_ListBoxImp* self, int width, int height) { self->xt_component_imp.imp.vtable->dev_clientresize(&self->xt_component_imp.imp, width, height); }
static psy_ui_Rectangle dev_position(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_position(&self->xt_component_imp.imp); }
static void dev_setposition(psy_ui_xt_ListBoxImp* self, psy_ui_Point topleft, psy_ui_Size size) { self->xt_component_imp.imp.vtable->dev_setposition(&self->xt_component_imp.imp, topleft, size); }
static psy_ui_Size dev_size(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_size(&self->xt_component_imp.imp); }
static psy_ui_Size dev_framesize(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_framesize(&self->xt_component_imp.imp); }
static void dev_scrollto(psy_ui_xt_ListBoxImp* self, intptr_t dx, intptr_t dy) { self->xt_component_imp.imp.vtable->dev_scrollto(&self->xt_component_imp.imp, dx, dy); }
static psy_ui_Component* dev_parent(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_parent(&self->xt_component_imp.imp); }
static void dev_capture(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_capture(&self->xt_component_imp.imp); }
static void dev_releasecapture(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_releasecapture(&self->xt_component_imp.imp); }
static void dev_invalidate(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_invalidate(&self->xt_component_imp.imp); }
static void dev_invalidaterect(psy_ui_xt_ListBoxImp* self, const psy_ui_Rectangle* r) { self->xt_component_imp.imp.vtable->dev_invalidaterect(&self->xt_component_imp.imp, r); }
static void dev_update(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_update(&self->xt_component_imp.imp); }
static void dev_setfont(psy_ui_xt_ListBoxImp* self, psy_ui_Font* font) { self->xt_component_imp.imp.vtable->dev_setfont(&self->xt_component_imp.imp, font); }
static void dev_showhorizontalscrollbar(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_showhorizontalscrollbar(&self->xt_component_imp.imp); }
static void dev_hidehorizontalscrollbar(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_hidehorizontalscrollbar(&self->xt_component_imp.imp); }
static void dev_sethorizontalscrollrange(psy_ui_xt_ListBoxImp* self, int min, int max) { self->xt_component_imp.imp.vtable->dev_sethorizontalscrollrange(&self->xt_component_imp.imp, min, max); }
static void dev_horizontalscrollrange(psy_ui_xt_ListBoxImp* self, int* scrollmin, int* scrollmax) { self->xt_component_imp.imp.vtable->dev_horizontalscrollrange(&self->xt_component_imp.imp, scrollmin, scrollmax); }
static int dev_horizontalscrollposition(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_horizontalscrollposition(&self->xt_component_imp.imp); }
static void dev_sethorizontalscrollposition(psy_ui_xt_ListBoxImp* self, int position) { self->xt_component_imp.imp.vtable->dev_sethorizontalscrollposition(&self->xt_component_imp.imp, position); }
static void dev_showverticalscrollbar(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_showverticalscrollbar(&self->xt_component_imp.imp); }
static void dev_hideverticalscrollbar(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_hideverticalscrollbar(&self->xt_component_imp.imp); }
static void dev_setverticalscrollrange(psy_ui_xt_ListBoxImp* self, int min, int max) { self->xt_component_imp.imp.vtable->dev_setverticalscrollrange(&self->xt_component_imp.imp, min, max); }
static void dev_verticalscrollrange(psy_ui_xt_ListBoxImp* self, int* scrollmin, int* scrollmax) { self->xt_component_imp.imp.vtable->dev_verticalscrollrange(&self->xt_component_imp.imp, scrollmin, scrollmax); }
static int dev_verticalscrollposition(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_verticalscrollposition(&self->xt_component_imp.imp); }
static void dev_setverticalscrollposition(psy_ui_xt_ListBoxImp* self, int position) { self->xt_component_imp.imp.vtable->dev_setverticalscrollposition(&self->xt_component_imp.imp, position); }
static psy_List* dev_children(psy_ui_xt_ListBoxImp* self, int recursive) { return self->xt_component_imp.imp.vtable->dev_children(&self->xt_component_imp.imp, recursive); }
static void dev_enableinput(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_enableinput(&self->xt_component_imp.imp); }
static void dev_preventinput(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_preventinput(&self->xt_component_imp.imp); }
static void dev_setcursor(psy_ui_xt_ListBoxImp* self, psy_ui_CursorStyle style) { self->xt_component_imp.imp.vtable->dev_setcursor(&self->xt_component_imp.imp, style); }
static void dev_starttimer(psy_ui_xt_ListBoxImp* self, unsigned int id, unsigned int interval) { self->xt_component_imp.imp.vtable->dev_starttimer(&self->xt_component_imp.imp, id, interval); }
static void dev_stoptimer(psy_ui_xt_ListBoxImp* self, unsigned int id) { self->xt_component_imp.imp.vtable->dev_stoptimer(&self->xt_component_imp.imp, id); }
static void dev_seticonressource(psy_ui_xt_ListBoxImp* self, int ressourceid) { self->xt_component_imp.imp.vtable->dev_seticonressource(&self->xt_component_imp.imp, ressourceid); }
static psy_ui_TextMetric dev_textmetric(psy_ui_xt_ListBoxImp* self, psy_ui_Font* font) { return self->xt_component_imp.imp.vtable->dev_textmetric(&self->xt_component_imp.imp, font); }
static psy_ui_Size dev_textsize(psy_ui_xt_ListBoxImp* self, const char* text, psy_ui_Font* font) { return self->xt_component_imp.imp.vtable->dev_textsize(&self->xt_component_imp.imp, text, font); }
static void dev_setbackgroundcolor(psy_ui_xt_ListBoxImp* self, uint32_t color) { self->xt_component_imp.imp.vtable->dev_setbackgroundcolor(&self->xt_component_imp.imp, color); }
static void dev_settitle(psy_ui_xt_ListBoxImp* self, const char* title) { self->xt_component_imp.imp.vtable->dev_settitle(&self->xt_component_imp.imp, title); }
static void dev_setfocus(psy_ui_xt_ListBoxImp* self) { self->xt_component_imp.imp.vtable->dev_setfocus(&self->xt_component_imp.imp); }
static int dev_hasfocus(psy_ui_xt_ListBoxImp* self) { return self->xt_component_imp.imp.vtable->dev_hasfocus(&self->xt_component_imp.imp); }
static void* dev_platform(psy_ui_xt_ListBoxImp* self) { return (void*) &self->xt_component_imp; }

// VTable init
static psy_ui_ComponentImpVTable vtable;
static int vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!vtable_initialized) {		
		vtable.dev_dispose = (psy_ui_fp_componentimp_dev_dispose) dev_dispose;
		vtable.dev_destroy = (psy_ui_fp_componentimp_dev_destroy) dev_destroy;
		vtable.dev_show = (psy_ui_fp_componentimp_dev_show) dev_show;
		vtable.dev_showstate = (psy_ui_fp_componentimp_dev_showstate) dev_showstate;
		vtable.dev_hide = (psy_ui_fp_componentimp_dev_hide) dev_hide;
		vtable.dev_visible = (psy_ui_fp_componentimp_dev_visible) dev_visible;
		vtable.dev_move = (psy_ui_fp_componentimp_dev_move) dev_move;
		vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize) dev_resize;
		vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize) dev_clientresize;
		vtable.dev_position = (psy_ui_fp_componentimp_dev_position) dev_position;
		vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition) dev_setposition;
		vtable.dev_size = (psy_ui_fp_componentimp_dev_size) dev_size;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize) dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto) dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent) dev_parent;
		vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture) dev_capture;
		vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture) dev_releasecapture;
		vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate) dev_invalidate;
		vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect) dev_invalidaterect;
		vtable.dev_update = (psy_ui_fp_componentimp_dev_update) dev_update;
		vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont) dev_setfont;
		vtable.dev_showhorizontalscrollbar = (psy_ui_fp_componentimp_dev_showhorizontalscrollbar) dev_showhorizontalscrollbar;
		vtable.dev_hidehorizontalscrollbar = (psy_ui_fp_componentimp_dev_hidehorizontalscrollbar) dev_hidehorizontalscrollbar;
		vtable.dev_sethorizontalscrollrange = (psy_ui_fp_componentimp_dev_sethorizontalscrollrange) dev_sethorizontalscrollrange;
		vtable.dev_horizontalscrollrange = (psy_ui_fp_componentimp_dev_horizontalscrollrange) dev_horizontalscrollrange;
		vtable.dev_horizontalscrollposition = (psy_ui_fp_componentimp_dev_horizontalscrollposition) dev_horizontalscrollposition;
		vtable.dev_sethorizontalscrollposition = (psy_ui_fp_componentimp_dev_sethorizontalscrollposition) dev_sethorizontalscrollposition;
		vtable.dev_showverticalscrollbar = (psy_ui_fp_componentimp_dev_showverticalscrollbar) dev_showverticalscrollbar;
		vtable.dev_hideverticalscrollbar = (psy_ui_fp_componentimp_dev_hideverticalscrollbar) dev_hideverticalscrollbar;
		vtable.dev_setverticalscrollrange = (psy_ui_fp_componentimp_dev_setverticalscrollrange) dev_setverticalscrollrange;
		vtable.dev_verticalscrollrange = (psy_ui_fp_componentimp_dev_verticalscrollrange) dev_verticalscrollrange;
		vtable.dev_verticalscrollposition = (psy_ui_fp_componentimp_dev_verticalscrollposition) dev_verticalscrollposition;
		vtable.dev_setverticalscrollposition = (psy_ui_fp_componentimp_dev_setverticalscrollposition) dev_setverticalscrollposition;
		vtable.dev_children = (psy_ui_fp_componentimp_dev_children) dev_children;
		vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput) dev_enableinput;
		vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput) dev_preventinput;
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor) dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer) dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer) dev_stoptimer;
		vtable.dev_seticonressource = (psy_ui_fp_componentimp_dev_seticonressource) dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric) dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize) dev_textsize;
		vtable.dev_setbackgroundcolor = (psy_ui_fp_componentimp_dev_setbackgroundcolor) dev_setbackgroundcolor;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle) dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus) dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus) dev_hasfocus;
		vtable.dev_platform = (psy_ui_fp_componentimp_dev_platform) dev_platform;		
		vtable_initialized = 1;
	}
}

//static void oncommand(psy_ui_ListBox*, psy_ui_Component* sender, WPARAM wParam,
//	LPARAM lParam);

// ListBoxImp VTable

static int dev_addtext(psy_ui_xt_ListBoxImp*, const char* text);
static void dev_settext(psy_ui_xt_ListBoxImp*, const char* text, intptr_t index);
static void dev_text(psy_ui_xt_ListBoxImp*, char* text, intptr_t index);
static void dev_setstyle(psy_ui_xt_ListBoxImp*, int style);
static void dev_clear(psy_ui_xt_ListBoxImp*);
static void dev_setcursel(psy_ui_xt_ListBoxImp*, intptr_t index);
static intptr_t dev_cursel(psy_ui_xt_ListBoxImp*);
static void dev_selitems(psy_ui_xt_ListBoxImp*, int* items, int maxitems);
static intptr_t dev_selcount(psy_ui_xt_ListBoxImp*);

static psy_ui_ListBoxImpVTable listboximp_vtable;
static int listboximp_vtable_initialized = 0;

static void listboximp_imp_vtable_init(psy_ui_xt_ListBoxImp* self)
{
	if (!listboximp_vtable_initialized) {
		listboximp_vtable.dev_addtext = (psy_ui_fp_listboximp_dev_addtext) dev_addtext;
		listboximp_vtable.dev_settext = (psy_ui_fp_listboximp_dev_settext) dev_settext;
		listboximp_vtable.dev_text = (psy_ui_fp_listboximp_dev_text) dev_text;
		listboximp_vtable.dev_setstyle = (psy_ui_fp_listboximp_dev_setstyle) dev_setstyle;
		listboximp_vtable.dev_clear = (psy_ui_fp_listboximp_dev_clear) dev_clear;
		listboximp_vtable.dev_setcursel = (psy_ui_fp_listboximp_dev_setcursel) dev_setcursel;
		listboximp_vtable.dev_cursel = (psy_ui_fp_listboximp_dev_cursel) dev_cursel;
		listboximp_vtable.dev_selitems = (psy_ui_fp_listboximp_dev_selitems) dev_selitems;
		listboximp_vtable.dev_selcount = (psy_ui_fp_listboximp_dev_selcount) dev_selcount;
		listboximp_vtable_initialized = 1;
	}
}

void psy_ui_xt_listboximp_init(psy_ui_xt_ListBoxImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{	
	psy_ui_xt_componentimp_init(&self->xt_component_imp,
		component,
		parent,
		"LISTBOX",
		0, 0, 100, 20,
		0,
		// (WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY) & ~WS_BORDER,
		1);
	imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_listboximp_init(&self->imp);
	listboximp_imp_vtable_init(self);
	self->imp.vtable = &listboximp_vtable;
//	psy_signal_connect(&self->xt_component_imp.imp.signal_command, component, oncommand);
}

void psy_ui_xt_listboximp_multiselect_init(psy_ui_xt_ListBoxImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_xt_componentimp_init(&self->xt_component_imp,
		component,
		parent,
		"LISTBOX",
		0, 0, 100, 20,
		0,
		//(WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL | LBS_NOTIFY)
		//& ~WS_BORDER,
		1);
	imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_listboximp_init(&self->imp);
	listboximp_imp_vtable_init(self);
	self->imp.vtable = &listboximp_vtable;
}

psy_ui_xt_ListBoxImp* psy_ui_xt_listboximp_alloc(void)
{
	return (psy_ui_xt_ListBoxImp*) malloc(sizeof(psy_ui_xt_ListBoxImp));
}

psy_ui_xt_ListBoxImp* psy_ui_xt_listboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_xt_ListBoxImp* rv;

	rv = psy_ui_xt_listboximp_alloc();
	if (rv) {
		psy_ui_xt_listboximp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_xt_ListBoxImp* psy_ui_xt_listboximp_multiselect_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_xt_ListBoxImp* rv;

	rv = psy_ui_xt_listboximp_alloc();
	if (rv) {
		psy_ui_xt_listboximp_multiselect_init(rv, component, parent);
	}
	return rv;
}

int dev_addtext(psy_ui_xt_ListBoxImp* self, const char* text)
{
	//return SendMessage(self->xt_component_imp.hwnd, LB_ADDSTRING, 0, (LPARAM) text);
	return 0;
}

void dev_settext(psy_ui_xt_ListBoxImp* self, const char* text, intptr_t index)
{
	//intptr_t sel;
	
	//sel = SendMessage(self->xt_component_imp.hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	//if (sel != -1) {
		//SendMessage(self->xt_component_imp.hwnd, LB_DELETESTRING, (WPARAM)index, (LPARAM)text);
		//SendMessage(self->xt_component_imp.hwnd, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
		//SendMessage(self->xt_component_imp.hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);
		//SetWindowText(self->xt_component_imp.hwnd, text);
	//}
}

void dev_setstyle(psy_ui_xt_ListBoxImp* self, int style)
{
//#if defined(_WIN64)
	//SetWindowLongPtr(self->xt_component_imp.hwnd, GWL_STYLE, style);
//#else
	//SetWindowLong(self->xt_component_imp.hwnd, GWL_STYLE, style);
//#endif
}

void dev_text(psy_ui_xt_ListBoxImp* self, char* text, intptr_t index)
{	
//	SendMessage(self->xt_component_imp.hwnd, LB_GETTEXT, (WPARAM)index, (LPARAM)text);
}

void dev_clear(psy_ui_xt_ListBoxImp* self)
{
//	SendMessage(self->xt_component_imp.hwnd, LB_RESETCONTENT, 0, (LPARAM)0);
}

void dev_setcursel(psy_ui_xt_ListBoxImp* self, intptr_t index)
{
//	SendMessage(self->xt_component_imp.hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);
}

intptr_t dev_cursel(psy_ui_xt_ListBoxImp* self)
{
//	return SendMessage(self->xt_component_imp.hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}

void dev_selitems(psy_ui_xt_ListBoxImp* self, int* items, int maxitems)
{
//	SendMessage(self->xt_component_imp.hwnd, LB_GETSELITEMS, (WPARAM)maxitems,
//		(LPARAM)items);
}

intptr_t dev_selcount(psy_ui_xt_ListBoxImp* self)
{
//	return SendMessage(self->xt_component_imp.hwnd, LB_GETSELCOUNT,
//		(WPARAM)0, (LPARAM)0);
	return 0;
}

//void oncommand(psy_ui_ListBox* self, psy_ui_Component* sender, WPARAM wParam,
	//LPARAM lParam) {
	//switch (HIWORD(wParam))
	//{
	//case LBN_SELCHANGE:
	//{
		//if (self->signal_selchanged.slots) {
			//intptr_t sel = psy_ui_listbox_cursel(self);
			//psy_signal_emit(&self->signal_selchanged, self, 1, sel);
		//}
	//}
	//break;
	//default:
		//break;
	//}
//}

#endif
