// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwincomponentimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwinimpfactory.h"
#include "uicomponent.h"
#include "uiwinfontimp.h"
#include "uiwinbitmapimp.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;
static BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam);

static int windowstyle(psy_ui_win_ComponentImp*);
static int windowexstyle(psy_ui_win_ComponentImp*);

static void psy_ui_win_component_create_window(psy_ui_win_ComponentImp* self,
	psy_ui_win_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);
static HINSTANCE psy_ui_win_component_instance(psy_ui_win_ComponentImp*
	parent);
static void psy_ui_win_component_init_wndproc(psy_ui_win_ComponentImp* self,
	LPCSTR classname);

// VTable Prototypes
static void dev_dispose(psy_ui_win_ComponentImp*);
static void dev_destroy(psy_ui_win_ComponentImp*);
static void dev_show(psy_ui_win_ComponentImp*);
static void dev_showstate(psy_ui_win_ComponentImp*, int state);
static void dev_hide(psy_ui_win_ComponentImp*);
static int dev_visible(psy_ui_win_ComponentImp*);
static void dev_move(psy_ui_win_ComponentImp*, int left, int top);
static void dev_resize(psy_ui_win_ComponentImp*, psy_ui_Size);
static void dev_clientresize(psy_ui_win_ComponentImp*, int width, int height);
static psy_ui_Rectangle dev_position(psy_ui_win_ComponentImp*);
static void dev_setposition(psy_ui_win_ComponentImp*, psy_ui_Point topleft, psy_ui_Size);
static psy_ui_Size dev_size(psy_ui_win_ComponentImp*);
static psy_ui_Size dev_framesize(psy_ui_win_ComponentImp*);
static void dev_scrollto(psy_ui_win_ComponentImp*, intptr_t dx, intptr_t dy);
static psy_ui_Component* dev_parent(psy_ui_win_ComponentImp*);
static void dev_setparent(psy_ui_win_ComponentImp* self, psy_ui_Component* parent);
static void dev_insert(psy_ui_win_ComponentImp* self, psy_ui_win_ComponentImp* child, psy_ui_win_ComponentImp* insertafter);
static void dev_capture(psy_ui_win_ComponentImp*);
static void dev_releasecapture(psy_ui_win_ComponentImp*);
static void dev_invalidate(psy_ui_win_ComponentImp*);
static void dev_invalidaterect(psy_ui_win_ComponentImp*, const psy_ui_Rectangle*);
static void dev_update(psy_ui_win_ComponentImp*);
static void dev_setfont(psy_ui_win_ComponentImp*, psy_ui_Font*);
static void dev_showhorizontalscrollbar(psy_ui_win_ComponentImp*);
static void dev_hidehorizontalscrollbar(psy_ui_win_ComponentImp*);
static void dev_sethorizontalscrollrange(psy_ui_win_ComponentImp*, int min, int max);
static void dev_horizontalscrollrange(psy_ui_win_ComponentImp*, int* scrollmin,
	int* scrollmax);
static int dev_horizontalscrollposition(psy_ui_win_ComponentImp* self);
static void dev_sethorizontalscrollposition(psy_ui_win_ComponentImp* self, int position);
static void dev_showverticalscrollbar(psy_ui_win_ComponentImp*);
static void dev_hideverticalscrollbar(psy_ui_win_ComponentImp*);
static void dev_setverticalscrollrange(psy_ui_win_ComponentImp*, int min, int max);
static void dev_verticalscrollrange(psy_ui_win_ComponentImp*, int* scrollmin,
	int* scrollmax);
static int dev_verticalscrollposition(psy_ui_win_ComponentImp*);
static void dev_setverticalscrollposition(psy_ui_win_ComponentImp*, int position);
static psy_List* dev_children(psy_ui_win_ComponentImp*, int recursive);
static void dev_enableinput(psy_ui_win_ComponentImp*);
static void dev_preventinput(psy_ui_win_ComponentImp*);
static void dev_setcursor(psy_ui_win_ComponentImp*, psy_ui_CursorStyle);
static void dev_starttimer(psy_ui_win_ComponentImp*, uintptr_t id, uintptr_t interval);
static void dev_stoptimer(psy_ui_win_ComponentImp*, uintptr_t id);
static void dev_seticonressource(psy_ui_win_ComponentImp*, int ressourceid);
static psy_ui_TextMetric dev_textmetric(psy_ui_win_ComponentImp*, psy_ui_Font*);
static psy_ui_Size dev_textsize(psy_ui_win_ComponentImp*, const char* text, psy_ui_Font*);
static void dev_setbackgroundcolor(psy_ui_win_ComponentImp*, uint32_t color);
static void dev_settitle(psy_ui_win_ComponentImp*, const char* title);
static void dev_setfocus(psy_ui_win_ComponentImp*);
static int dev_hasfocus(psy_ui_win_ComponentImp*);

// VTable init
static psy_ui_ComponentImpVTable vtable;
static int vtable_initialized = 0;

static void win_imp_vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
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
		vtable.dev_setparent = (psy_ui_fp_componentimp_dev_setparent) dev_setparent;
		vtable.dev_insert = (psy_ui_fp_componentimp_dev_insert) dev_insert;
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
		vtable_initialized = 1;
	}
}

void psy_ui_win_componentimp_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{	
	psy_ui_win_ComponentImp* parent_imp;	

	psy_ui_componentimp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &vtable;
	self->component = component;
	self->background = 0;
	self->winid = -1;
	self->hwnd = 0;
	self->wndproc = 0;
	self->vscrollmin = 0;
	self->vscrollmax = 100;
	self->hscrollmin = 0;
	self->hscrollmax = 100;
	self->preventwmchar = 0;
	self->sizecachevalid = FALSE;
	parent_imp = parent ? (psy_ui_win_ComponentImp*)parent : 0;	
	psy_ui_win_component_create_window(self, parent_imp, classname, x, y, width, height,
		dwStyle, usecommand);
	if (self->hwnd) {
		psy_ui_win_component_init_wndproc(self, classname);
	}
}

void psy_ui_win_component_create_window(psy_ui_win_ComponentImp* self,
	psy_ui_win_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_WinApp* winapp;
	HINSTANCE instance;
	int err = 0;

	winapp = (psy_ui_WinApp*)app.platform;
	instance = psy_ui_win_component_instance(parent);
	self->hwnd = CreateWindow(
		classname,
		NULL,
		dwStyle,
		x, y, width, height,
		parent ? parent->hwnd : (HWND)NULL,
		usecommand ? (HMENU) winapp->winid : NULL,
		instance,
		NULL);
	if (self->hwnd == NULL) {
		char text[256];
		unsigned long err;

		err = GetLastError();
		psy_snprintf(text, 256, "Failed To Create Component (Error %u)", err);
		MessageBox(NULL, text, "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		psy_table_insert(&winapp->selfmap, (uintptr_t) self->hwnd, self);
	}
	if (err == 0 && usecommand) {
		psy_table_insert(&winapp->winidmap, winapp->winid, self);
		++winapp->winid;
	}
}

HINSTANCE psy_ui_win_component_instance(psy_ui_win_ComponentImp* parent)
{
	HINSTANCE rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	if (parent) {
#if defined(_WIN64)		
		rv = (HINSTANCE)GetWindowLongPtr(parent->hwnd, GWLP_HINSTANCE);
#else
		rv = (HINSTANCE)GetWindowLong(parent->hwnd, GWL_HINSTANCE);
#endif
	} else {
		rv = winapp->instance;
	}
	return rv;
}

void psy_ui_win_component_init_wndproc(psy_ui_win_ComponentImp* self,
	LPCSTR classname)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;	
#if defined(_WIN64)		
	self->wndproc = (winproc) GetWindowLongPtr(self->hwnd, GWLP_WNDPROC);
#else		
	self->wndproc = (winproc) GetWindowLong(self->hwnd, GWL_WNDPROC);
#endif
	if (classname != winapp->componentclass && classname != winapp->appclass) {
#if defined(_WIN64)		
		SetWindowLongPtr(self->hwnd, GWLP_WNDPROC, (LONG_PTR)winapp->comwinproc);
#else	
		SetWindowLong(self->hwnd, GWL_WNDPROC, (LONG)winapp->comwinproc);
#endif
	}
}

// win32 implementation method for psy_ui_Component
void dev_dispose(psy_ui_win_ComponentImp* self)
{
	if (self->background) {
		DeleteObject(self->background);
	}
	psy_ui_componentimp_dispose(&self->imp);
}

psy_ui_win_ComponentImp* psy_ui_win_componentimp_alloc(void)
{
	return (psy_ui_win_ComponentImp*) malloc(sizeof(psy_ui_win_ComponentImp));
}

psy_ui_win_ComponentImp* psy_ui_win_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_componentimp_alloc();
	if (rv) {
		psy_ui_win_componentimp_init(rv,
			component,
			parent,
			classname,
			x, y, width, height,
			dwStyle,
			usecommand
		);
	}	
	return rv;
}

void dev_destroy(psy_ui_win_ComponentImp* self)
{	
	DestroyWindow(self->hwnd);
}

void dev_show(psy_ui_win_ComponentImp* self)
{
	ShowWindow(self->hwnd, SW_SHOW);
	UpdateWindow(self->hwnd);
}

void dev_showstate(psy_ui_win_ComponentImp* self, int state)
{
	ShowWindow(self->hwnd, state);
	UpdateWindow(self->hwnd);
}

void dev_hide(psy_ui_win_ComponentImp* self)
{
	ShowWindow(self->hwnd, SW_HIDE);
	UpdateWindow(self->hwnd);
}

int dev_visible(psy_ui_win_ComponentImp* self)
{
	return IsWindowVisible(self->hwnd);
}

void dev_move(psy_ui_win_ComponentImp* self, int left, int top)
{
	SetWindowPos(self->hwnd, NULL,
		left, top,
		0, 0,
		SWP_NOZORDER | SWP_NOSIZE);
}

void dev_resize(psy_ui_win_ComponentImp* self, psy_ui_Size size)
{
	psy_ui_TextMetric tm;

	tm = dev_textmetric(self, self->component ? psy_ui_component_font(self->component) : NULL);
	self->sizecachevalid = FALSE;
	SetWindowPos(self->hwnd, NULL,
		0, 0,
		psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm),
		SWP_NOZORDER | SWP_NOMOVE);
	
	self->sizecache = size;
	self->sizecachevalid = TRUE;
}

void dev_clientresize(psy_ui_win_ComponentImp* self, int width, int height)
{
	RECT rc;
	
	rc.left = 0;
	rc.top = 0;
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRectEx(&rc, windowstyle(self),
		GetMenu(self->hwnd) != NULL,
		windowexstyle(self));
	dev_resize(self,
		psy_ui_size_make(
			psy_ui_value_makepx(rc.right - rc.left),
			psy_ui_value_makepx(rc.bottom - rc.top)));
}


psy_ui_Rectangle dev_position(psy_ui_win_ComponentImp* self)
{
	psy_ui_Rectangle rv;
	RECT rc;
	POINT pt;
	int width;
	int height;

	GetWindowRect(self->hwnd, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(self->hwnd), &pt);
	rv.left = pt.x;
	rv.top = pt.y;
	rv.right = pt.x + width;
	rv.bottom = pt.y + height;
	return rv;
}

void dev_setposition(psy_ui_win_ComponentImp* self, psy_ui_Point topleft, psy_ui_Size size)
{
	psy_ui_TextMetric tm;

	tm = dev_textmetric(self, self->component ? psy_ui_component_font(self->component) : NULL);
	self->sizecachevalid = FALSE;
	SetWindowPos(self->hwnd, 0,
		psy_ui_value_px(&topleft.x, &tm),
		psy_ui_value_px(&topleft.y, &tm),
		psy_ui_value_px(&size.width, &tm), psy_ui_value_px(&size.height, &tm),
		SWP_NOZORDER);	
	self->sizecachevalid = TRUE;
	self->sizecache = size;
}

psy_ui_Size dev_size(psy_ui_win_ComponentImp* self)
{
	if (self->sizecachevalid) {
		return self->sizecache;
	} else {
		psy_ui_Size rv;
		RECT rect;

		GetClientRect(self->hwnd, &rect);
		rv.width = psy_ui_value_makepx(rect.right);
		rv.height = psy_ui_value_makepx(rect.bottom);
		return rv;
	}
}

psy_ui_Size dev_framesize(psy_ui_win_ComponentImp* self)
{
	psy_ui_Size rv;
	RECT rect;

	GetWindowRect(self->hwnd, &rect);
	rv.width = psy_ui_value_makepx(rect.right);
	rv.height = psy_ui_value_makepx(rect.bottom);
	return rv;
}

void dev_scrollto(psy_ui_win_ComponentImp* self, intptr_t dx, intptr_t dy)
{
	ScrollWindow(self->hwnd, dx, dy, NULL, NULL);
	UpdateWindow(self->hwnd);
}

psy_ui_Component* dev_parent(psy_ui_win_ComponentImp* self)
{
	psy_ui_WinApp* winapp;
	psy_ui_win_ComponentImp* imp;

	winapp = (psy_ui_WinApp*) app.platform;
	imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap,
		(uintptr_t)GetParent(self->hwnd));
	return imp ? imp->component : 0;
}

void dev_setparent(psy_ui_win_ComponentImp* self, psy_ui_Component* parent)
{
	psy_ui_WinApp* winapp;
	psy_ui_win_ComponentImp* parentimp;

	winapp = (psy_ui_WinApp*)app.platform;
	parentimp = (psy_ui_win_ComponentImp*) parent->imp;
	if (parentimp) {
		SetParent(self->hwnd, parentimp->hwnd);
	}
}

void dev_insert(psy_ui_win_ComponentImp* self, psy_ui_win_ComponentImp* child, psy_ui_win_ComponentImp* insertafter)
{
	SetParent(child->hwnd, self->hwnd);
	SetWindowPos(
		child->hwnd,
		insertafter->hwnd,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE
	);
}

void dev_capture(psy_ui_win_ComponentImp* self)
{
	SetCapture(self->hwnd);
}

void dev_releasecapture(psy_ui_win_ComponentImp* self)
{
	ReleaseCapture();
}

void dev_invalidate(psy_ui_win_ComponentImp* self)
{
	InvalidateRect(self->hwnd, NULL, FALSE);
}

void dev_invalidaterect(psy_ui_win_ComponentImp* self, const psy_ui_Rectangle* r)
{
	RECT rc;

	rc.left = r->left - self->component->scroll.x;
	rc.top = r->top - self->component->scroll.y;
	rc.right = r->right - self->component->scroll.x;
	rc.bottom = r->bottom - self->component->scroll.y;
	InvalidateRect(self->hwnd, &rc, FALSE);
}

void dev_update(psy_ui_win_ComponentImp* self)
{
	UpdateWindow(self->hwnd);
}

void dev_setfont(psy_ui_win_ComponentImp* self, psy_ui_Font* source)
{
	if (source) {
		HFONT hfont;
		
		hfont = ((psy_ui_win_FontImp*)(source->imp))->hfont;
		SendMessage(self->hwnd, WM_SETFONT, (WPARAM)hfont, 0);
	}	
}

void dev_showhorizontalscrollbar(psy_ui_win_ComponentImp* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE,
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) | WS_HSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE,
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_HSCROLL);
#endif
}

void dev_hidehorizontalscrollbar(psy_ui_win_ComponentImp* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE,
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE,
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#endif
}

void dev_sethorizontalscrollrange(psy_ui_win_ComponentImp* self, int scrollmin, int scrollmax)
{	
	if (self->hscrollmin != scrollmin || self->hscrollmax != scrollmax) {
		SCROLLINFO si;

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE;
		GetScrollInfo(self->hwnd, SB_HORZ, &si);
		si.nMin = max(0, scrollmin);
		si.nMax = max(0, scrollmax);
		self->hscrollmin = si.nMin;
		self->hscrollmax = si.nMax;
		SetScrollInfo(self->hwnd, SB_HORZ, &si, TRUE);
	}
}

void dev_horizontalscrollrange(psy_ui_win_ComponentImp* self, int* scrollmin,
	int* scrollmax)
{
	*scrollmin = self->hscrollmin;
	*scrollmax = self->hscrollmax;
}

void dev_showverticalscrollbar(psy_ui_win_ComponentImp* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE,
		GetWindowLongPtr(self->hwnd, GWL_STYLE) | WS_VSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE,
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_VSCROLL);
#endif
}

void dev_hideverticalscrollbar(psy_ui_win_ComponentImp* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE,
		GetWindowLongPtr(self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE,
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#endif
}

void dev_setverticalscrollrange(psy_ui_win_ComponentImp* self, int scrollmin, int scrollmax)
{		
	if (self->vscrollmin != scrollmin || self->vscrollmax != scrollmax) {
		SCROLLINFO si;

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE;
		si.nMin = max(0, scrollmin);
		si.nMax = max(0, scrollmax);
		self->vscrollmin = si.nMin;
		self->vscrollmax = si.nMax;
		si.nPage = 0;
		SetScrollInfo(self->hwnd, SB_VERT, &si, TRUE);		
	}
}

int dev_verticalscrollposition(psy_ui_win_ComponentImp* self)
{
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(self->hwnd, SB_VERT, &si);
	return si.nPos;
}

void dev_setverticalscrollposition(psy_ui_win_ComponentImp* self, int position)
{
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;
	GetScrollInfo(self->hwnd, SB_VERT, &si);
	if (position < si.nMax) {
		si.nPos = (int)position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;
	SetScrollInfo(self->hwnd, SB_VERT, &si, TRUE);
}

int dev_horizontalscrollposition(psy_ui_win_ComponentImp* self)
{
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(self->hwnd, SB_HORZ, &si);
	return si.nPos;
}

void dev_sethorizontalscrollposition(psy_ui_win_ComponentImp* self, int position)
{
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;
	GetScrollInfo(self->hwnd, SB_HORZ, &si);
	if (position < si.nMax) {
		si.nPos = (int)position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;
	SetScrollInfo(self->hwnd, SB_HORZ, &si, TRUE);
}

void dev_verticalscrollrange(psy_ui_win_ComponentImp* self, int* scrollmin,
	int* scrollmax)
{
	*scrollmin = self->vscrollmin;
	*scrollmax = self->vscrollmax;
}

psy_List* dev_children(psy_ui_win_ComponentImp* self, int recursive)
{	
	psy_List* children = 0;
	if (recursive == 1) {
		EnumChildWindows(self->hwnd, allchildenumproc, (LPARAM) &children);
	}
	else {
		uintptr_t hwnd = (uintptr_t)GetWindow(self->hwnd, GW_CHILD);
		if (hwnd) {
			psy_ui_WinApp* winapp;
			psy_ui_win_ComponentImp* imp;
			psy_ui_Component* child;

			winapp = (psy_ui_WinApp*)app.platform;
			imp = psy_table_at(&winapp->selfmap, hwnd);
			child = imp ? imp->component : 0;
			if (child) {
				children = psy_list_create(child);
			}
		}
		while (hwnd) {
			hwnd = (uintptr_t) GetNextWindow((HWND)hwnd, GW_HWNDNEXT);
			if (hwnd) {
				psy_ui_WinApp* winapp;
				psy_ui_win_ComponentImp* imp;
				psy_ui_Component* child;

				winapp = (psy_ui_WinApp*)app.platform;
				imp = psy_table_at(&winapp->selfmap, hwnd);
				child = imp ? imp->component : 0;
				if (child) {
					psy_list_append(&children, child);
				}
			}
		}
	}
	return children;	
}

void dev_enableinput(psy_ui_win_ComponentImp* self)
{
	EnableWindow(self->hwnd, 1);
}

void dev_preventinput(psy_ui_win_ComponentImp* self)
{
	EnableWindow(self->hwnd, 0);
}

psy_ui_TextMetric dev_textmetric(psy_ui_win_ComponentImp* self, psy_ui_Font* font)
{
	psy_ui_TextMetric rv;
	TEXTMETRIC tm;
	HDC hdc;
	HFONT hPrevFont = 0;
	HFONT hfont = 0;

	hdc = GetDC(self->hwnd);
	SaveDC(hdc);
	if (font) {
		hfont = ((psy_ui_win_FontImp*)font->imp)->hfont;
		if (hfont) {
			hPrevFont = SelectObject(hdc, hfont);
		}
	}
	GetTextMetrics(hdc, &tm);
	if (font) {
		if (hPrevFont) {
			SelectObject(hdc, hPrevFont);
		}
	}
	RestoreDC(hdc, -1);
	ReleaseDC(self->hwnd, hdc);
	rv.tmHeight = tm.tmHeight;
	rv.tmAscent = tm.tmAscent;
	rv.tmDescent = tm.tmDescent;
	rv.tmInternalLeading = tm.tmInternalLeading;
	rv.tmExternalLeading = tm.tmExternalLeading;
	rv.tmAveCharWidth = tm.tmAveCharWidth;
	rv.tmMaxCharWidth = tm.tmMaxCharWidth;
	rv.tmWeight = tm.tmWeight;
	rv.tmOverhang = tm.tmOverhang;
	rv.tmDigitizedAspectX = tm.tmDigitizedAspectX;
	rv.tmDigitizedAspectY = tm.tmDigitizedAspectY;
	rv.tmFirstChar = tm.tmFirstChar;
	rv.tmLastChar = tm.tmLastChar;
	rv.tmDefaultChar = tm.tmDefaultChar;
	rv.tmBreakChar = tm.tmBreakChar;
	rv.tmItalic = tm.tmItalic;
	rv.tmUnderlined = tm.tmUnderlined;
	rv.tmStruckOut = tm.tmStruckOut;
	rv.tmPitchAndFamily = tm.tmPitchAndFamily;
	rv.tmCharSet = tm.tmCharSet;
	return rv;
}

void dev_setcursor(psy_ui_win_ComponentImp* self, psy_ui_CursorStyle cursorstyle)
{
	HCURSOR hc;

	switch (cursorstyle) {
	case psy_ui_CURSORSTYLE_AUTO:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_MOVE:
		hc = LoadCursor(NULL, IDC_SIZEALL);
		break;
	case psy_ui_CURSORSTYLE_NODROP:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_COL_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_ALL_SCROLL:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_POINTER:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_NOT_ALLOWED:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_ROW_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZENS);
		break;
	case psy_ui_CURSORSTYLE_CROSSHAIR:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_PROGRESS:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_E_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_NE_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_DEFAULT_TEXT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_N_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_NW_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_HELP:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_VERTICAL_TEXT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_S_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_SE_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_INHERIT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_WAIT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_W_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_SW_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	default:
		hc = 0;
		break;
	}
	if (hc) {
		SetCursor(hc);
	}
}

void dev_starttimer(psy_ui_win_ComponentImp* self, uintptr_t id,
	uintptr_t interval)
{
	SetTimer(self->hwnd, id, interval, 0);
}

void dev_stoptimer(psy_ui_win_ComponentImp* self, unsigned int id)
{
	KillTimer(self->hwnd, id);
}

void dev_seticonressource(psy_ui_win_ComponentImp* self, int ressourceid)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
#if defined(_WIN64)	
	SetClassLongPtr(self->hwnd, GCLP_HICON,
		(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
#else	
	SetClassLong(self->hwnd, GCL_HICON,
		(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
#endif
}

psy_ui_Size dev_textsize(psy_ui_win_ComponentImp* self, const char* text, psy_ui_Font* font)
{
	psy_ui_Size rv;
	psy_ui_Graphics g;
	HDC hdc;

	hdc = GetDC(self->hwnd);
	SaveDC(hdc);
	psy_ui_graphics_init(&g, hdc);
	psy_ui_setfont(&g, font);
	rv = psy_ui_textsize(&g, text);
	psy_ui_graphics_dispose(&g);
	RestoreDC(hdc, -1);
	ReleaseDC(self->hwnd, hdc);
	return rv;
}

void dev_setbackgroundcolor(psy_ui_win_ComponentImp* self, uint32_t color)
{	
	if (self->background) {
		DeleteObject(self->background);
	}
	self->background = CreateSolidBrush(color);
}

void dev_settitle(psy_ui_win_ComponentImp* self, const char* title)
{
	SetWindowText(self->hwnd, title);
}

void dev_setfocus(psy_ui_win_ComponentImp* self)
{
	SetFocus(self->hwnd);
	psy_signal_emit(&self->component->signal_focus, self, 0);
}

int dev_hasfocus(psy_ui_win_ComponentImp* self)
{
	return self->hwnd == GetFocus();
}

BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam)
{
	psy_List** pChildren = (psy_List**)lParam;
	psy_ui_WinApp* winapp;
	psy_ui_win_ComponentImp* imp;
	psy_ui_Component* child;

	winapp = (psy_ui_WinApp*)app.platform;
	imp = psy_table_at(&winapp->selfmap, (uintptr_t)hwnd);
	child = imp ? imp->component : 0;
	if (child) {
		psy_list_append(pChildren, child);
	}
	return TRUE;
}

int windowstyle(psy_ui_win_ComponentImp* self)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(self->hwnd, GWLP_STYLE);
#else
	rv = (int)GetWindowLong(self->hwnd, GWL_STYLE);
#endif
	return rv;
}

int windowexstyle(psy_ui_win_ComponentImp* self)
{
	int rv;
#if defined(_WIN64)		
	rv = (int) GetWindowLongPtr(self->hwnd, GWLP_EXSTYLE);
#else
	rv = (int)GetWindowLong(self->hwnd, GWL_EXSTYLE);
#endif
	return rv;
}

#endif
