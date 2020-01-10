// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponent.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include "uimenu.h"
#include "hashtbl.h"
#include <memory.h>
#include <commctrl.h>	// includes the common control header
#include <stdio.h>
#include <shlobj.h>
#include <portable.h>
#include "uiwincomponent.h"


BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam);
BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam);

static int GetWindowStyle(HWND hwnd);
static int GetWindowExStyle(HWND hwnd);

static void enableinput(psy_ui_Component*, int enable, int recursive);
// vtable defaults
static void preferredsize(psy_ui_Component*, ui_size* limit, ui_size* rv);



void ui_replacedefaultfont(psy_ui_Component* main, ui_font* font)
{		
	if (font && main) {
		psy_List* p;
		psy_List* q;

		if (main->font.hfont == app.defaults.defaultfont.hfont) {
			main->font.hfont = font->hfont;
			ui_win_component_sendmessage(main, WM_SETFONT,
				(WPARAM) font->hfont, 0);
		}
		for (p = q = ui_component_children(main, 1); p != 0; p = p->next) {
			psy_ui_Component* child;

			child = (psy_ui_Component*)p->entry;
			if (child->font.hfont == app.defaults.defaultfont.hfont) {
				child->font.hfont = font->hfont;	
				ui_win_component_sendmessage(child, WM_SETFONT,
					(WPARAM) font->hfont, 0);
				ui_component_align(child);
			}		
		}
		for (p = q; p != 0; p = p->next) {
			psy_ui_Component* child;

			child = (psy_ui_Component*)p->entry;
			if (child->font.hfont == font->hfont) {
				child = (psy_ui_Component*)p->entry;
				ui_component_align(child);
			}
		}
		psy_list_free(q);
		DeleteObject(app.defaults.defaultfont.hfont);
		app.defaults.defaultfont = *font;
		ui_component_align(main);
	}
}

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.draw = 0;
		vtable.preferredsize = preferredsize;
	}
}

int ui_win32_component_init(psy_ui_Component* self, psy_ui_Component* parent,
		LPCTSTR classname, 
		int x, int y, int width, int height,
		DWORD dwStyle,
		int usecommand)
{
	int err = 0;
	HINSTANCE hInstance;	

	psy_ui_WinApp* winapp;
	psy_ui_WinComponent* platform;

	platform = malloc(sizeof(psy_ui_WinComponent));
	memset(platform, 0, sizeof(psy_ui_WinComponent));
	self->platform = platform;
	winapp = (psy_ui_WinApp*) app.platform;	
	ui_component_init_signals(self);
	if (parent) {
#if defined(_WIN64)		
		hInstance = (HINSTANCE) GetWindowLongPtr(
			ui_win_component_hwnd(parent), GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong(
			ui_win_component_hwnd(parent), GWL_HINSTANCE);
#endif
	} else {
		hInstance = winapp->instance;
	}
	ui_win_component(self)->hwnd = CreateWindow(
		classname,
		NULL,		
		dwStyle,
		x, y, width, height,
		parent ? ui_win_component_hwnd(parent) : NULL,
		usecommand ? (HMENU)winapp->winid : NULL,
		hInstance,
		NULL);	
	if (ui_win_component_hwnd(self) == NULL) {
		char text[256];
		unsigned long err;

		err = GetLastError();
		psy_snprintf(text, 256, "Failed To Create Component (Error %u)", err);
        MessageBox(NULL, text, "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		psy_table_insert(&winapp->selfmap, (uintptr_t)
			ui_win_component_hwnd(self), self);
	}
	if (err == 0 && usecommand) {
		psy_table_insert(&winapp->winidmap, winapp->winid, self);
		++winapp->winid;
	}
	ui_component_init_base(self);		
#if defined(_WIN64)		
		ui_win_component(self)->wndproc = (winproc) GetWindowLongPtr(
			ui_win_component_hwnd(self), GWLP_WNDPROC);
#else		
		ui_win_component(self)->wndproc = (winproc) GetWindowLong(
			ui_win_component_hwnd(self), GWL_WNDPROC);
#endif
	if (classname != winapp->componentclass && classname != winapp->appclass) {
#if defined(_WIN64)		
		SetWindowLongPtr(ui_win_component_hwnd(self), GWLP_WNDPROC,
			(LONG_PTR) winapp->comwinproc);
#else	
		SetWindowLong(ui_win_component_hwnd(self), GWL_WNDPROC,
			(LONG)winapp->comwinproc);
#endif
	}
	if (!parent) {
		app.main = self;
	}
	return err;
}

void* ui_component_platform(psy_ui_Component* self)
{
	return self->platform;
}

void ui_component_init(psy_ui_Component* component, psy_ui_Component* parent)
{		
	psy_ui_WinApp* winapp;	

	winapp = (psy_ui_WinApp*) app.platform;
	ui_win32_component_init(component, parent, winapp->componentclass,
		0, 0, 90, 90, WS_CHILDWINDOW | WS_VISIBLE, 0);
}

void ui_component_init_signals(psy_ui_Component* component)
{
	psy_signal_init(&component->signal_size);
	psy_signal_init(&component->signal_draw);
	psy_signal_init(&component->signal_timer);
	psy_signal_init(&component->signal_keydown);
	psy_signal_init(&component->signal_keyup);
	psy_signal_init(&component->signal_mousedown);
	psy_signal_init(&component->signal_mouseup);
	psy_signal_init(&component->signal_mousemove);
	psy_signal_init(&component->signal_mousewheel);
	psy_signal_init(&component->signal_mousedoubleclick);
	psy_signal_init(&component->signal_mouseenter);
	psy_signal_init(&component->signal_mousehover);
	psy_signal_init(&component->signal_mouseleave);
	psy_signal_init(&component->signal_scroll);
	psy_signal_init(&component->signal_create);
	psy_signal_init(&component->signal_destroy);
	psy_signal_init(&component->signal_show);
	psy_signal_init(&component->signal_hide);
	psy_signal_init(&component->signal_focus);
	psy_signal_init(&component->signal_focuslost);
	psy_signal_init(&component->signal_align);
	psy_signal_init(&component->signal_preferredsize);
	psy_signal_init(&component->signal_windowproc);
	psy_signal_init(&component->signal_command);
}

void ui_component_init_base(psy_ui_Component* self) {
	self->scrollstepx = 100;
	self->scrollstepy = 12;
	self->propagateevent = 0;
	self->preventdefault = 0;
	self->preventpreferredsize = 0;
	self->align = UI_ALIGN_NONE;
	self->justify = UI_JUSTIFY_EXPAND;
	self->alignchildren = 0;
	self->alignexpandmode = UI_NOEXPAND;	
	ui_margin_init(&self->margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_margin_init(&self->spacing, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
	self->debugflag = 0;
	self->defaultpropagation = 0;	
	self->visible = 1;
	self->doublebuffered = 0;
	self->wheelscroll = 0;
	self->accumwheeldelta = 0;
	self->handlevscroll = 1;
	self->handlehscroll = 1;
	self->backgroundmode = BACKGROUND_SET;
	self->backgroundcolor = ui_defaults_backgroundcolor(&app.defaults);	
	self->color = ui_defaults_color(&app.defaults);
	self->cursor = UI_CURSOR_DEFAULT;
	ui_font_init(&self->font, 0);
	ui_component_setfont(self, &app.defaults.defaultfont);
	ui_component_setbackgroundcolor(self, self->backgroundcolor);
	vtable_init();
	self->vtable = &vtable;	
}

void ui_component_dispose(psy_ui_Component* self)
{	
	psy_signal_dispose(&self->signal_size);
	psy_signal_dispose(&self->signal_draw);
	psy_signal_dispose(&self->signal_timer);
	psy_signal_dispose(&self->signal_keydown);
	psy_signal_dispose(&self->signal_keyup);
	psy_signal_dispose(&self->signal_mousedown);
	psy_signal_dispose(&self->signal_mouseup);
	psy_signal_dispose(&self->signal_mousemove);
	psy_signal_dispose(&self->signal_mousewheel);
	psy_signal_dispose(&self->signal_mousedoubleclick);
	psy_signal_dispose(&self->signal_mouseenter);
	psy_signal_dispose(&self->signal_mousehover);
	psy_signal_dispose(&self->signal_mouseleave);
	psy_signal_dispose(&self->signal_scroll);
	psy_signal_dispose(&self->signal_create);
	psy_signal_dispose(&self->signal_destroy);
	psy_signal_dispose(&self->signal_show);
	psy_signal_dispose(&self->signal_hide);
	psy_signal_dispose(&self->signal_focus);
	psy_signal_dispose(&self->signal_focuslost);
	psy_signal_dispose(&self->signal_align);
	psy_signal_dispose(&self->signal_preferredsize);
	psy_signal_dispose(&self->signal_windowproc);
	psy_signal_dispose(&self->signal_command);
	if (self->font.hfont && self->font.hfont !=
			app.defaults.defaultfont.hfont) {
		ui_font_dispose(&self->font);
	}
	if (self->platform && ui_win_component(self)->background) {
		DeleteObject(ui_win_component(self)->background);
	}
	free(self->platform);
	self->platform = 0;
}

void ui_component_destroy(psy_ui_Component* self)
{
	if (self->platform) {
		DestroyWindow(ui_win_component_hwnd(self));
	}
}


void ui_component_scrollstep(psy_ui_Component* self, intptr_t stepx,
	intptr_t stepy)
{
	ScrollWindow(ui_win_component_hwnd(self),
		self->scrollstepx * stepx,
		self->scrollstepy * stepy, 
		NULL, NULL) ;
	UpdateWindow(ui_win_component_hwnd(self));
}


ui_size ui_component_size(psy_ui_Component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetClientRect(ui_win_component_hwnd(self), &rect);
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

ui_rectangle ui_component_position(psy_ui_Component* self)
{   
	ui_rectangle rv;
	RECT rc;
	POINT pt;	
	int width;
	int height;	
	    	
    GetWindowRect(ui_win_component_hwnd(self), &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(ui_win_component_hwnd(self)), &pt);
	rv.left = pt.x;
	rv.top = pt.y;
	rv.right =  pt.x + width;
	rv.bottom = pt.y + height;
	return rv;
}

ui_size ui_component_frame_size(psy_ui_Component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetWindowRect(ui_win_component_hwnd(self), &rect) ;
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

void ui_component_show_state(psy_ui_Component* self, int cmd)
{
	ShowWindow(ui_win_component_hwnd(self), cmd);
	UpdateWindow(ui_win_component_hwnd(self)) ;
}

void ui_component_show(psy_ui_Component* self)
{
	self->visible = 1;
	ShowWindow(ui_win_component_hwnd(self), SW_SHOW);
	UpdateWindow(ui_win_component_hwnd(self)) ;
}

void ui_component_hide(psy_ui_Component* self)
{
	self->visible = 0;
	ShowWindow(ui_win_component_hwnd(self), SW_HIDE);
	UpdateWindow(ui_win_component_hwnd(self)) ;
}

void ui_component_showhorizontalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE) | WS_HSCROLL);
#else
	SetWindowLong(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLong(ui_win_component_hwnd(self), GWL_STYLE) | WS_HSCROLL);
#endif
}

void ui_component_hidehorizontalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE) & ~WS_HSCROLL);
#else
	SetWindowLong(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLong(ui_win_component_hwnd(self), GWL_STYLE) & ~WS_HSCROLL);
#endif
}

void ui_component_sethorizontalscrollrange(psy_ui_Component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo(ui_win_component_hwnd(self), SB_HORZ, &si, TRUE);
}

void ui_component_showverticalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE) | WS_VSCROLL);
#else
	SetWindowLong(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLong(ui_win_component_hwnd(self), GWL_STYLE) | WS_VSCROLL);
#endif
}

void ui_component_hideverticalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLongPtr(ui_win_component_hwnd(self), GWL_STYLE) & ~WS_VSCROLL);
#else
	SetWindowLong(ui_win_component_hwnd(self), GWL_STYLE, 
		GetWindowLong(ui_win_component_hwnd(self), GWL_STYLE) & ~WS_VSCROLL);
#endif
}

void ui_component_setverticalscrollrange(psy_ui_Component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = max(0, min);
	si.nMax = max(si.nMin, max);
	si.fMask = SIF_RANGE;	
	SetScrollInfo(ui_win_component_hwnd(self), SB_VERT, &si, TRUE);
}

int ui_component_verticalscrollposition(psy_ui_Component* self)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;	
	GetScrollInfo(ui_win_component_hwnd(self), SB_VERT, &si);	
	return si.nPos;
}

void ui_component_setverticalscrollposition(psy_ui_Component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(ui_win_component_hwnd(self), SB_VERT, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo(ui_win_component_hwnd(self), SB_VERT, &si, TRUE);
}

int ui_component_horizontalscrollposition(psy_ui_Component* self)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;	
	GetScrollInfo(ui_win_component_hwnd(self), SB_HORZ, &si);	
	return si.nPos;
}

void ui_component_sethorizontalscrollposition(psy_ui_Component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(ui_win_component_hwnd(self), SB_HORZ, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo(ui_win_component_hwnd(self), SB_HORZ, &si, TRUE);
}

void ui_component_verticalscrollrange(psy_ui_Component* self, int* scrollmin,
	int* scrollmax)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(ui_win_component_hwnd(self), SB_VERT, &si);
	*scrollmin = si.nMin;
	*scrollmax = si.nMax;
}

void ui_component_move(psy_ui_Component* self, int left, int top)
{
	SetWindowPos(ui_win_component_hwnd(self), NULL, 
	   left, top,
	   0, 0,
	   SWP_NOZORDER | SWP_NOSIZE) ;	
}

void ui_component_resize(psy_ui_Component* self, int width, int height)
{	
	SetWindowPos(ui_win_component_hwnd(self), NULL, 
	   0, 0,
	   width, height,
	   SWP_NOZORDER | SWP_NOMOVE);	
}

void ui_component_setposition(psy_ui_Component* self, int x, int y, int width, int height)
{	
	SetWindowPos(ui_win_component_hwnd(self), 0, x, y, width, height, SWP_NOZORDER);	
}

void ui_component_setmenu(psy_ui_Component* self, ui_menu* menu)
{
	SetMenu(ui_win_component_hwnd(self), menu->hmenu);
}

void ui_component_settitle(psy_ui_Component* self, const char* title)
{
	SetWindowText(ui_win_component_hwnd(self), title);
}

void ui_component_enumerate_children(psy_ui_Component* self, void* context, 
	int (*childenum)(void*, void*))
{	
	EnumCallback callback;
	
	callback.context = context;
	callback.childenum = childenum;
	EnumChildWindows(ui_win_component_hwnd(self), ChildEnumProc, (LPARAM) &callback);
}

BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam)
{
	EnumCallback* callback = (EnumCallback*) lParam;
	psy_ui_WinApp* winapp;
	psy_ui_Component* child;

	winapp = (psy_ui_WinApp*) app.platform;
	child = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	if (child &&  callback->childenum) {
		return callback->childenum(callback->context, child);		  
	}     
    return FALSE ;
}

psy_List* ui_component_children(psy_ui_Component* self, int recursive)
{	
	psy_List* children = 0;	
	if (recursive == 1) {
		EnumChildWindows (ui_win_component_hwnd(self), AllChildEnumProc, (LPARAM) &children);
	} else {
		uintptr_t hwnd = (uintptr_t)GetWindow(ui_win_component_hwnd(self), GW_CHILD);
		if (hwnd) {
			psy_ui_WinApp* winapp;							
			psy_ui_Component* child;

			winapp = (psy_ui_WinApp*) app.platform;			
			child = psy_table_at(&winapp->selfmap, hwnd);
			if (child) {				
				children = psy_list_create(child);				
			}
		}
		while (hwnd) {
			hwnd = (uintptr_t) GetNextWindow((HWND)hwnd, GW_HWNDNEXT);
			if (hwnd) {
				psy_ui_WinApp* winapp;							
				psy_ui_Component* child;

				winapp = (psy_ui_WinApp*) app.platform;			
				child = psy_table_at(&winapp->selfmap, hwnd);
				if (child) {					
					psy_list_append(&children, child);							
				}
			}
		}
	}
	return children;
}

BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam)
{
	psy_List** pChildren = (psy_List**) lParam;
	psy_ui_WinApp* winapp;							
	psy_ui_Component* child;

	winapp = (psy_ui_WinApp*) app.platform;			
	child = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	if (child) {		
		psy_list_append(pChildren, child);				
	}     
    return TRUE;
}

void ui_component_capture(psy_ui_Component* self)
{
	SetCapture(ui_win_component_hwnd(self));
}

void ui_component_releasecapture()
{
	ReleaseCapture();
}

void ui_component_invalidate(psy_ui_Component* self)
{
	InvalidateRect(ui_win_component_hwnd(self), NULL, FALSE);
}

void ui_component_invalidaterect(psy_ui_Component* self, const ui_rectangle* r)
{
	RECT rc;

	rc.left = r->left;
	rc.top = r->top;
	rc.right = r->right;
	rc.bottom = r->bottom;
	InvalidateRect(ui_win_component_hwnd(self), &rc, FALSE);
}

void ui_component_update(psy_ui_Component* self)
{
	UpdateWindow(ui_win_component_hwnd(self));
}

void ui_component_setfocus(psy_ui_Component* self)
{
	SetFocus(ui_win_component_hwnd(self));
	psy_signal_emit(&self->signal_focus, self, 0);
}

int ui_component_hasfocus(psy_ui_Component* self)
{
	return ui_win_component_hwnd(self) == GetFocus();
}

void ui_component_setfont(psy_ui_Component* self, ui_font* source)
{
	ui_font font;

	font.hfont = 0;
	font.stock = 0;
	if (source && source->hfont && source->hfont !=
			app.defaults.defaultfont.hfont) {
		ui_font_init(&font, 0);
		ui_font_copy(&font, source);
	} else {
		font.hfont = app.defaults.defaultfont.hfont;
	}	
	SendMessage(ui_win_component_hwnd(self), WM_SETFONT, (WPARAM) font.hfont, 0);	
	if (self->font.hfont && self->font.hfont != 
			app.defaults.defaultfont.hfont) {
		ui_font_dispose(&self->font);		
	}
	self->font = font;
}

void ui_component_propagateevent(psy_ui_Component* self)
{
	self->propagateevent = 1;
}

void ui_component_preventdefault(psy_ui_Component* self)
{
	self->preventdefault = 1;
}

int ui_component_visible(psy_ui_Component* self)
{
	return IsWindowVisible(ui_win_component_hwnd(self));
}

void ui_component_align(psy_ui_Component* self)
{	
	ui_size size;
	ui_textmetric tm;
	ui_point cp_topleft = { 0, 0 };
	ui_point cp_bottomright = { 0, 0 };	
	int cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	psy_ui_Component* client = 0;
		
	size = ui_component_size(self);
	tm = ui_component_textmetric(self);
	cp_bottomright.x = size.width;
	cp_bottomright.y = size.height;
	for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)p->entry;		
		if (component->visible) {
			ui_size componentsize;
			ui_size limit;

			limit.width = cp_bottomright.x - cp_topleft.x;
			limit.height = cp_bottomright.y - cp_topleft.y;
			componentsize = ui_component_preferredsize(component, &limit);
			if (component->align == UI_ALIGN_CLIENT) {
				client = component;
			} 
			if (component->align == UI_ALIGN_FILL) {
				ui_component_setposition(component,
					ui_value_px(&component->margin.left, &tm),
					ui_value_px(&component->margin.top, &tm),				
					size.width - ui_value_px(&component->margin.left, &tm) -
						ui_value_px(&component->margin.right, &tm),
					size.height -
						ui_margin_height_px(&component->margin, &tm));
			} else
			if (component->align == UI_ALIGN_TOP) {
				cp_topleft.y += ui_value_px(&component->margin.top, &tm);
				ui_component_setposition(component, 
					cp_topleft.x + ui_value_px(&component->margin.left, &tm), 
					cp_topleft.y,
					cp_bottomright.x - cp_topleft.x -
						ui_margin_width_px(&component->margin, &tm),
					componentsize.height);
				cp_topleft.y += ui_value_px(&component->margin.bottom, &tm);
				cp_topleft.y += componentsize.height;
			} else
			if (component->align == UI_ALIGN_BOTTOM) {
				cp_bottomright.y -=
					ui_value_px(&component->margin.bottom, &tm);
				ui_component_setposition(component, 
					cp_topleft.x + ui_value_px(&component->margin.left, &tm), 
					cp_bottomright.y - componentsize.height,
					cp_bottomright.x - cp_topleft.x -
						ui_margin_width_px(&component->margin, &tm),						
					componentsize.height);
				cp_bottomright.y -= ui_value_px(&component->margin.top, &tm);
				cp_bottomright.y -= componentsize.height;
			} else
			if (component->align == UI_ALIGN_RIGHT) {
				int requiredcomponentwidth;

				requiredcomponentwidth = componentsize.width +
					ui_margin_width_px(&component->margin, &tm);
				cp_bottomright.x -= requiredcomponentwidth;
				ui_component_setposition(component,
					cp_bottomright.x + ui_value_px(&component->margin.left, &tm),
					cp_topleft.y +
						ui_value_px(&component->margin.top, &tm),
					componentsize.width,										
					cp_bottomright.y - cp_topleft.y - 
						ui_margin_height_px(&component->margin, &tm));
			} else
			if (component->align == UI_ALIGN_LEFT) {
				if ((self->alignexpandmode & UI_HORIZONTALEXPAND)
						== UI_HORIZONTALEXPAND) {
				} else {
					int requiredcomponentwidth;

					requiredcomponentwidth = componentsize.width +
						ui_margin_width_px(&component->margin, &tm);
					if (cp_topleft.x + requiredcomponentwidth > size.width) {
						psy_List* w;						
						cp_topleft.x = 0;
						for (w = wrap; w != 0; w = w->next) {
							psy_ui_Component* c;
							c = (psy_ui_Component*)w->entry;
							ui_component_resize(c, ui_component_size(c).width,
								cpymax - cp_topleft.y -
								ui_margin_height_px(&component->margin, &tm));
						}
						cp_topleft.y = cpymax;
						psy_list_free(wrap);						
						wrap = 0;
					}					
					psy_list_append(&wrap, component);					
				}
				cp_topleft.x += ui_value_px(&component->margin.left, &tm);
				ui_component_setposition(component,
					cp_topleft.x,
					cp_topleft.y + ui_value_px(&component->margin.top, &tm),
					componentsize.width,
					component->justify == UI_JUSTIFY_EXPAND 
					? cp_bottomright.y - cp_topleft.y - 
						ui_margin_height_px(&component->margin, &tm)
					: componentsize.height);
				cp_topleft.x += ui_value_px(&component->margin.right, &tm);
				cp_topleft.x += componentsize.width;				
				if (cpymax < cp_topleft.y + componentsize.height +
						ui_margin_height_px(&component->margin, &tm)) {
					cpymax = cp_topleft.y + componentsize.height +
						ui_margin_height_px(&component->margin, &tm);
				}
			}				
		}
	}
	if (client) {		
		ui_component_setposition(client,
			cp_topleft.x + ui_value_px(&client->margin.left, &tm),
			cp_topleft.y + ui_value_px(&client->margin.top, &tm),
			cp_bottomright.x - cp_topleft.x -
				ui_margin_width_px(&client->margin, &tm),
			cp_bottomright.y - cp_topleft.y -
				ui_margin_height_px(&client->margin, &tm));
	}
	psy_list_free(q);
	psy_list_free(wrap);
	psy_signal_emit(&self->signal_align, self, 0);
}

/*void preferredsize_new(psy_ui_Component* self, ui_size* limit, ui_size* rv)
{
	ui_size size;
	ui_textmetric tm;
	ui_point cp_topleft = { 0, 0 };
	ui_point cp_bottomright = { 0, 0 };	
	int cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	psy_ui_Component* client = 0;

	size = ui_component_size(self);
	tm = ui_component_textmetric(self);	
	if (self->alignchildren) {
		size.width = (self->alignexpandmode & UI_HORIZONTALEXPAND) ==
				UI_HORIZONTALEXPAND 
				? 0
				: limit->width;	
	}
	cp_bottomright.x = size.width;
	cp_bottomright.y = size.height;
	for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)p->entry;		
		if (component->visible) {
			ui_size componentsize;
		}
	}
}*/

void preferredsize(psy_ui_Component* self, ui_size* limit, ui_size* rv)
{			
	if (rv) {
		ui_size size;
		ui_textmetric tm;

		size = ui_component_size(self);
		tm = ui_component_textmetric(self);
		if (self->alignchildren && !self->preventpreferredsize) {
			psy_List* p;
			psy_List* q;
			ui_point cp = { 0, 0 };
			ui_size maxsize = { 0, 0 };
			
			
			size.width = (self->alignexpandmode & UI_HORIZONTALEXPAND) ==
				UI_HORIZONTALEXPAND 
				? 0
				: limit->width;
			for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
				psy_ui_Component* component;
					
				component = (psy_ui_Component*)p->entry;		
				if (component->visible) {
					ui_size componentsize;			
					
					componentsize = ui_component_preferredsize(component, &size);
					if (component->align == UI_ALIGN_TOP ||
							component->align == UI_ALIGN_BOTTOM) {
						cp.y += componentsize.height +
							ui_margin_height_px(&component->margin, &tm);
						if (maxsize.height < cp.y) {
							maxsize.height = cp.y;
						}
						if (maxsize.width < componentsize.width +
								ui_margin_width_px(&component->margin, &tm)) {
							maxsize.width = componentsize.width +
								ui_margin_width_px(&component->margin, &tm);
						}
					} else					
					if (component->align == UI_ALIGN_LEFT) {
						if (size.width != 0) {
							int requiredcomponentwidth;

							requiredcomponentwidth = componentsize.width +
								ui_margin_width_px(&component->margin, &tm);
							if (cp.x + requiredcomponentwidth > size.width) {
								cp.y = maxsize.height;
								cp.x = 0;							
							}						
						}
						cp.x += componentsize.width +
							ui_margin_width_px(&component->margin, &tm);						
						if (maxsize.width < cp.x) {
							maxsize.width = cp.x;
						}
						if (maxsize.height < cp.y + componentsize.height +
								ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = cp.y + componentsize.height +
								ui_margin_height_px(&component->margin, &tm);
						}
					}				
				}
			}
			psy_list_free(q);
			*rv = maxsize;
			rv->width += ui_margin_width_px(&self->spacing, &tm);
			rv->height += ui_margin_height_px(&self->spacing, &tm);
		} else {
			*rv = size;
			rv->width += ui_margin_width_px(&self->spacing, &tm);
			rv->height += ui_margin_height_px(&self->spacing, &tm);
		}
	}	
}

void ui_component_setmargin(psy_ui_Component* self, const ui_margin* margin)
{	
	if (margin) {
		self->margin = *margin;		
	} else {
		memset(&self->margin, 0, sizeof(ui_margin));
	}
}

void ui_component_setspacing(psy_ui_Component* self, const ui_margin* spacing)
{	
	if (spacing) {
		self->spacing = *spacing;
	} else {
		memset(&self->spacing, 0, sizeof(ui_margin));
	}
}

void ui_component_setalign(psy_ui_Component* self, UiAlignType align)
{
	self->align = align;
}

void ui_component_enablealign(psy_ui_Component* self)
{
	self->alignchildren = 1;	
}

void ui_component_setalignexpand(psy_ui_Component* self, UiExpandMode mode)
{
	self->alignexpandmode = mode;
}

void ui_component_preventalign(psy_ui_Component* self)
{
	self->alignchildren = 0;
}

void ui_component_enableinput(psy_ui_Component* self, int recursive)
{
	enableinput(self, TRUE, recursive);
}

void ui_component_preventinput(psy_ui_Component* self, int recursive)
{
	enableinput(self, FALSE, recursive);
}

void enableinput(psy_ui_Component* self, int enable, int recursive)
{	
	EnableWindow(ui_win_component_hwnd(self), enable);
	if (recursive) {
		psy_List* p;
		psy_List* q;
		
		for (p = q = ui_component_children(self, recursive); p != 0;
				p = p->next) {
			EnableWindow(ui_win_component_hwnd((psy_ui_Component*)p->entry),
				enable);
		}
		psy_list_free(q);
	}
}

int ui_openfile(psy_ui_Component* self, char* szTitle, char* szFilter,
	char* szDefExtension, const char* szInitialDir, char* szOpenName)
{
	int rv;
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	*szOpenName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner = ui_win_component_hwnd(self); 
	ofn.lpstrFilter= szFilter;	
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szOpenName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szTitle; 
	ofn.nMaxFileTitle= MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR) szInitialDir; 
	ofn.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST; 
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension;
	rv = GetOpenFileName(&ofn);
	if (app.main) {
		InvalidateRect(ui_win_component_hwnd(app.main), 0, FALSE);
		UpdateWindow(ui_win_component_hwnd(app.main));
	}
	return rv;
}

int ui_savefile(psy_ui_Component* self, char* szTitle, char* szFilter,
	char* szDefExtension, const char* szInitialDir, char* szFileName)
{
	int rv;
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	*szFileName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner = ui_win_component_hwnd(self); 
	ofn.lpstrFilter= szFilter;	
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szFileName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szTitle; 
	ofn.nMaxFileTitle= _MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR) szInitialDir;
	ofn.Flags= OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension;
	if (app.main) {
		InvalidateRect(ui_win_component_hwnd(app.main), 0, FALSE);
		UpdateWindow(ui_win_component_hwnd(app.main));
	}
	rv = GetSaveFileName(&ofn);
	return rv;
}
	
void ui_component_setbackgroundmode(psy_ui_Component* self, BackgroundMode mode)
{
	self->backgroundmode = mode;	
}

void ui_component_setbackgroundcolor(psy_ui_Component* self, unsigned int color)
{
	self->backgroundcolor = color;
	if (ui_win_component(self)->background) {
		DeleteObject(ui_win_component(self)->background);
	}
	ui_win_component(self)->background = CreateSolidBrush(color);
}

void ui_component_setcolor(psy_ui_Component* self, unsigned int color)
{
	self->color = color;
}

ui_size ui_component_textsize(psy_ui_Component* self, const char* text)
{
	ui_size rv;
	psy_ui_Graphics g;
	HFONT hPrevFont = 0;
	HDC hdc;
	
	hdc = GetDC(ui_win_component_hwnd(self));	
    SaveDC (hdc) ;          
	ui_graphics_init(&g, hdc);
	if (self->font.hfont) {
		hPrevFont = SelectObject(hdc, self->font.hfont);
	}
	rv = ui_textsize(&g, text);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}
	ui_graphics_dispose(&g);
	RestoreDC (hdc, -1);	
	ReleaseDC(ui_win_component_hwnd(self), hdc);
	return rv;
}

psy_ui_Component* ui_component_parent(psy_ui_Component* self)
{		
	psy_ui_WinApp* winapp;							

	winapp = (psy_ui_WinApp*) app.platform;
	return (psy_ui_Component*) psy_table_at(&winapp->selfmap, 
		(uintptr_t) GetParent(ui_win_component_hwnd(self)));
}

psy_List* ui_components_setalign(psy_List* list, UiAlignType align,
	const ui_margin* margin)
{
	psy_List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setalign((psy_ui_Component*)p->entry, align);
		if (margin) {
			ui_component_setmargin((psy_ui_Component*)p->entry, margin);
		}
	}
	return list;
}

psy_List* ui_components_setmargin(psy_List* list, const ui_margin* margin)
{
	psy_List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setmargin((psy_ui_Component*)p->entry, margin);		
	}
	return list;
}

ui_size ui_component_preferredsize(psy_ui_Component* self, ui_size* limit)
{
	ui_size rv;
	self->vtable->preferredsize(self, limit, &rv);
	if (self->signal_preferredsize.slots) {
		psy_signal_emit(&self->signal_preferredsize, self, 2, limit, &rv);
	}
	return rv;	
}

ui_textmetric ui_component_textmetric(psy_ui_Component* self)
{			
	TEXTMETRIC tm;
	HDC hdc;		
	HFONT hPrevFont = 0;	
	
	hdc = GetDC(ui_win_component_hwnd(self));	
    SaveDC(hdc) ;          	
	if (self->font.hfont) {
		hPrevFont = SelectObject(hdc, self->font.hfont);
	}
	GetTextMetrics (hdc, &tm);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}	
	RestoreDC(hdc, -1);	
	ReleaseDC(ui_win_component_hwnd(self), hdc);
	return tm;
}

void ui_component_seticonressource(psy_ui_Component* self, int ressourceid)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
#if defined(_WIN64)	
	SetClassLongPtr(ui_win_component_hwnd(self), GCLP_HICON, 
		(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
#else	
	SetClassLong(ui_win_component_hwnd(self), GCL_HICON, 
		(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
#endif
}

void ui_component_starttimer(psy_ui_Component* self, unsigned int id,
	unsigned int interval)
{
	SetTimer(ui_win_component_hwnd(self), id, interval, 0);
}

void ui_component_stoptimer(psy_ui_Component* self, unsigned int id)
{
	KillTimer(ui_win_component_hwnd(self), id);
}

int ui_browsefolder(psy_ui_Component* self, const char* title, char* path)
{

	///\todo: alternate browser window for Vista/7: http://msdn.microsoft.com/en-us/library/bb775966%28v=VS.85%29.aspx
	// SHCreateItemFromParsingName(
	int val= 0;
	
	LPMALLOC pMalloc;
	// Gets the Shell's default allocator
	//
	path[0] = '\0';
	if (SHGetMalloc(&pMalloc) == NOERROR)
	{
		char pszBuffer[MAX_PATH];		
		BROWSEINFO bi;
		LPITEMIDLIST pidl;

		pszBuffer[0]='\0';
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		//
		bi.hwndOwner = ui_win_component_hwnd(self);
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = title;
#if defined _MSC_VER > 1200
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
#else
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
#endif
		bi.lpfn = NULL;
		bi.lParam = 0;
		// This next call issues the dialog box.
		//
		if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
			if (SHGetPathFromIDList(pidl, pszBuffer)) {
				// At this point pszBuffer contains the selected path
				//
				val = 1;
				psy_snprintf(path, MAX_PATH, "%s", pszBuffer);
				path[MAX_PATH - 1] = '\0';				
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			//
			pMalloc->lpVtbl->Free(pMalloc, pidl);
		}
		// Release the shell's allocator.
		//
		pMalloc->lpVtbl->Release(pMalloc);
	}
	return val;
}

void ui_component_clientresize(psy_ui_Component* self, int width, int height)
{
	RECT rc;

	rc.left = 0;
	rc.top = 0;
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRectEx(&rc, GetWindowStyle(ui_win_component_hwnd(self)),
		GetMenu(ui_win_component_hwnd(self)) != NULL,
		GetWindowExStyle(ui_win_component_hwnd(self)));
	ui_component_resize(self, rc.right - rc.left,
		rc.bottom - rc.top);
}

int GetWindowStyle(HWND hwnd)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(hwnd, GWLP_STYLE);
#else
	rv = (int)GetWindowLong(hwnd, GWL_STYLE);
#endif
	return rv;
}

int GetWindowExStyle(HWND hwnd)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(hwnd, GWLP_EXSTYLE);
#else
	rv = (int)GetWindowLong(hwnd, GWL_EXSTYLE);
#endif
	return rv;
}

uintptr_t ui_component_platformhandle(psy_ui_Component* self)
{
	assert(self->platform);
	return (uintptr_t) (self->platform ? ui_win_component_hwnd(self) : 0);
}
