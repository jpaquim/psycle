// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponent.h"
#include "uialigner.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include "uimenu.h"
#include "uiwincompdetail.h"
#include <stdio.h>
#include <shlobj.h>
#include "uiwinfontimp.h"
#include "../../detail/portable.h"

typedef struct {		               
   int (*childenum)(void*, void*);   
   void* context;
} EnumCallback;

BOOL CALLBACK childenumproc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam);

static int windowstyle(psy_ui_Component*);
static int windowexstyle(psy_ui_Component*);
static void enableinput(psy_ui_Component*, int enable, int recursive);
static void psy_ui_updatesyles(psy_ui_Component* main);
static void psy_ui_component_updatefont(psy_ui_Component*);

void psy_ui_updatesyles(psy_ui_Component* main)
{
	if (main) {
		psy_List* p;
		psy_List* q;
		
		// merge
		psy_ui_component_updatefont(main);
		for (p = q = psy_ui_component_children(main, 1); p != 0; p = p->next) {
			psy_ui_Component* child;
			
			child = (psy_ui_Component*)p->entry;
			psy_ui_component_updatefont(child);
		}
		// align
		for (p = q; p != 0; p = p->next) {
			psy_ui_Component* child;

			child = (psy_ui_Component*)p->entry;							
			psy_ui_component_align(child);
		}
		psy_list_free(q);
		psy_ui_component_align(main);
	}
}

void psy_ui_component_updatefont(psy_ui_Component* self)
{	
	HFONT hfont;
	
	hfont = ((psy_ui_win_FontImp*) psy_ui_component_font(self)->imp)->hfont;	
	SendMessage(psy_ui_win_component_details(self)->hwnd, WM_SETFONT,
		(WPARAM) hfont, 0);
}

psy_ui_Font* psy_ui_component_font(psy_ui_Component* self)
{
	psy_ui_Font* rv;
	psy_ui_Style* common;	

	common = &app.defaults.style_common;
	if (self->style.use_font) {
		rv = &self->style.font;
	} else {
		rv = &app.defaults.style_common.font;
	}
	return rv;
}

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font* font)
{		
	if (main) {
		psy_ui_Style* common;
		psy_ui_Font old_default_font;

		common = &app.defaults.style_common;
		old_default_font = common->font;
		psy_ui_font_init(&common->font, 0);
		psy_ui_font_copy(&common->font, font);
		psy_ui_updatesyles(main);
		psy_ui_font_dispose(&old_default_font);
	}
}

// vtable
static void onpreferredsize(psy_ui_Component*, psy_ui_Size* limit, psy_ui_Size* rv);
static void onsize(psy_ui_Component* self, const psy_ui_Size* size) { }
static void onmousedown(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmousemove(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmouseup(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmousedoubleclick(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmouseenter(psy_ui_Component* self) { }
static void onmouseleave(psy_ui_Component* self) { }
static void onkeydown(psy_ui_Component* self, psy_ui_KeyEvent* ev) { }
static void onkeyup(psy_ui_Component* self, psy_ui_KeyEvent* ev) { }

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.ondraw = 0;
		vtable.onpreferredsize = onpreferredsize;
		vtable.onsize = onsize;
		vtable.onmousedown = onmousedown;
		vtable.onmousemove = onmousemove;
		vtable.onmouseup = onmouseup;
		vtable.onmousedoubleclick = onmousedoubleclick;
		vtable.onmouseenter = onmouseenter;
		vtable.onmouseleave = onmouseleave;
		vtable.onkeyup = onkeyup;
		vtable.onkeydown = onkeydown;
		vtable.onkeydown = onkeyup;
		vtable_initialized = 1;
	}
}

int psy_ui_win32_component_init(psy_ui_Component* self, psy_ui_Component* parent,
		LPCTSTR classname, 
		int x, int y, int width, int height,
		DWORD dwStyle,
		int usecommand)
{
	int err = 0;
	HINSTANCE hInstance;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;	
	psy_ui_component_init_signals(self);
	self->platform = malloc(sizeof(psy_ui_win_ComponentDetails));
	if (parent) {
#if defined(_WIN64)		
		hInstance = (HINSTANCE) GetWindowLongPtr(
			(HWND)psy_ui_win_component_details(parent)->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong(
			psy_ui_win_component_details(parent)->hwnd, GWL_HINSTANCE);
#endif
	} else {
		hInstance = winapp->instance;
	}
	psy_ui_win_component_details(self)->hwnd = CreateWindow(
		classname,
		NULL,		
		dwStyle,
		x, y, width, height,
		parent ? psy_ui_win_component_details(parent)->hwnd : (HWND) NULL,
		usecommand ? (HMENU)winapp->winid : NULL,
		hInstance,
		NULL);	
	if (psy_ui_win_component_details(self)->hwnd == NULL) {
		char text[256];
		unsigned long err;

		err = GetLastError();
		psy_snprintf(text, 256, "Failed To Create Component (Error %u)", err);
        MessageBox(NULL, text, "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		psy_table_insert(&winapp->selfmap, (uintptr_t) psy_ui_win_component_details(self)->hwnd, self);
	}
	if (err == 0 && usecommand) {
		psy_table_insert(&winapp->winidmap, winapp->winid, self);
		++winapp->winid;
	}
	psy_ui_component_init_base(self);		
#if defined(_WIN64)		
		psy_ui_win_component_details(self)->wndproc = (winproc)GetWindowLongPtr(psy_ui_win_component_details(self)->hwnd, GWLP_WNDPROC);
#else		
		psy_ui_win_component_details(self)->wndproc = (winproc)GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_WNDPROC);
#endif
	if (classname != winapp->componentclass && classname != winapp->appclass) {
#if defined(_WIN64)		
		SetWindowLongPtr(psy_ui_win_component_details(self)->hwnd, GWLP_WNDPROC, (LONG_PTR) winapp->comwinproc);
#else	
		SetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_WNDPROC, (LONG)winapp->comwinproc);
#endif
	}
	if (!parent) {
		app.main = self;
	}
	return err;
}

void psy_ui_component_init(psy_ui_Component* component, psy_ui_Component* parent)
{		
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	psy_ui_win32_component_init(component, parent, winapp->componentclass,
		0, 0, 90, 90, WS_CHILDWINDOW | WS_VISIBLE, 0);
}

void psy_ui_component_init_signals(psy_ui_Component* component)
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
	psy_signal_init(&component->signal_destroyed);
	psy_signal_init(&component->signal_show);
	psy_signal_init(&component->signal_hide);
	psy_signal_init(&component->signal_focus);
	psy_signal_init(&component->signal_focuslost);
	psy_signal_init(&component->signal_align);
//	psy_signal_init(&component->signal_preferredsize);	
	psy_signal_init(&component->signal_command);
}

void psy_ui_component_init_base(psy_ui_Component* self) {	
	self->scrollstepx = 100;
	self->scrollstepy = 12;	
	self->preventdefault = 0;
	self->preventpreferredsize = 0;
	self->align = psy_ui_ALIGN_NONE;
	self->justify = psy_ui_JUSTIFY_EXPAND;
	self->alignchildren = 0;
	self->alignexpandmode = psy_ui_NOEXPAND;
	psy_ui_style_init(&self->style);
	psy_ui_margin_init(&self->margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_margin_init(&self->spacing, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	self->debugflag = 0;	
	self->visible = 1;
	self->doublebuffered = 0;
	self->wheelscroll = 0;
	self->accumwheeldelta = 0;
	self->handlevscroll = 1;
	self->handlehscroll = 1;
	self->backgroundmode = BACKGROUND_SET;
	self->backgroundcolor = psy_ui_defaults_backgroundcolor(&app.defaults);
	psy_ui_win_component_details(self)->background = 0;
	self->color = psy_ui_defaults_color(&app.defaults);
	self->cursor = psy_ui_CURSOR_DEFAULT;	
	psy_ui_component_updatefont(self);
	psy_ui_component_setbackgroundcolor(self, self->backgroundcolor);
	vtable_init();
	self->vtable = &vtable;	
}

void psy_ui_component_dispose(psy_ui_Component* self)
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
	psy_signal_dispose(&self->signal_destroyed);
	psy_signal_dispose(&self->signal_show);
	psy_signal_dispose(&self->signal_hide);
	psy_signal_dispose(&self->signal_focus);
	psy_signal_dispose(&self->signal_focuslost);
	psy_signal_dispose(&self->signal_align);
	// psy_signal_dispose(self->signal_preferredsize);	
	psy_signal_dispose(&self->signal_command);	
	if (psy_ui_win_component_details(self)->background) {
		DeleteObject(psy_ui_win_component_details(self)->background);
	}
	psy_ui_style_dispose(&self->style);
	free(self->platform);
	self->platform = 0;
}

void psy_ui_component_destroy(psy_ui_Component* self)
{
	DestroyWindow(psy_ui_win_component_details(self)->hwnd);
}


void psy_ui_component_scrollstep(psy_ui_Component* self, intptr_t stepx, intptr_t stepy)
{
	ScrollWindow(psy_ui_win_component_details(self)->hwnd,
		self->scrollstepx * stepx,
		self->scrollstepy * stepy, 
		NULL, NULL) ;
	UpdateWindow(psy_ui_win_component_details(self)->hwnd);
}


psy_ui_Size psy_ui_component_size(psy_ui_Component* self)
{   
	psy_ui_Size rv;
	RECT rect ;
	    
    GetClientRect(psy_ui_win_component_details(self)->hwnd, &rect);
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

psy_ui_Rectangle psy_ui_component_position(psy_ui_Component* self)
{   
	psy_ui_Rectangle rv;
	RECT rc;
	POINT pt;	
	int width;
	int height;	
	    	
    GetWindowRect(psy_ui_win_component_details(self)->hwnd, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(psy_ui_win_component_details(self)->hwnd), &pt);
	rv.left = pt.x;
	rv.top = pt.y;
	rv.right =  pt.x + width;
	rv.bottom = pt.y + height;
	return rv;
}

psy_ui_Size psy_ui_component_frame_size(psy_ui_Component* self)
{   
	psy_ui_Size rv;
	RECT rect ;
	    
    GetWindowRect(psy_ui_win_component_details(self)->hwnd, &rect);
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

void psy_ui_component_show_state(psy_ui_Component* self, int cmd)
{
	ShowWindow(psy_ui_win_component_details(self)->hwnd, cmd);
	UpdateWindow(psy_ui_win_component_details(self)->hwnd) ;
}

void psy_ui_component_show(psy_ui_Component* self)
{
	self->visible = 1;
	ShowWindow(psy_ui_win_component_details(self)->hwnd, SW_SHOW);
	UpdateWindow(psy_ui_win_component_details(self)->hwnd) ;
}

void psy_ui_component_hide(psy_ui_Component* self)
{
	self->visible = 0;
	ShowWindow(psy_ui_win_component_details(self)->hwnd, SW_HIDE);
	UpdateWindow(psy_ui_win_component_details(self)->hwnd) ;
}

void psy_ui_component_showhorizontalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)psy_ui_win_component_details(self)->hwnd, GWL_STYLE) | WS_HSCROLL);
#else
	SetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE) | WS_HSCROLL);
#endif
}

void psy_ui_component_hidehorizontalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)psy_ui_win_component_details(self)->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#else
	SetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#endif
}

void psy_ui_component_sethorizontalscrollrange(psy_ui_Component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_HORZ, &si, TRUE);
}

void psy_ui_component_showverticalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLongPtr(psy_ui_win_component_details(self)->hwnd, GWL_STYLE) | WS_VSCROLL);
#else
	SetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE) | WS_VSCROLL);
#endif
}

void psy_ui_component_hideverticalscrollbar(psy_ui_Component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLongPtr(psy_ui_win_component_details(self)->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#else
	SetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE, 
		GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#endif
}

void psy_ui_component_setverticalscrollrange(psy_ui_Component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = max(0, min);
	si.nMax = max(si.nMin, max);
	si.fMask = SIF_RANGE;	
	SetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_VERT, &si, TRUE);
}

int psy_ui_component_verticalscrollposition(psy_ui_Component* self)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;	
	GetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_VERT, &si);	
	return si.nPos;
}

void psy_ui_component_setverticalscrollposition(psy_ui_Component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_VERT, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_VERT, &si, TRUE);
}

int psy_ui_component_horizontalscrollposition(psy_ui_Component* self)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;	
	GetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_HORZ, &si);	
	return si.nPos;
}

void psy_ui_component_sethorizontalscrollposition(psy_ui_Component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_HORZ, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_HORZ, &si, TRUE);
}

void psy_ui_component_verticalscrollrange(psy_ui_Component* self, int* scrollmin,
	int* scrollmax)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(psy_ui_win_component_details(self)->hwnd, SB_VERT, &si);
	*scrollmin = si.nMin;
	*scrollmax = si.nMax;
}

void psy_ui_component_move(psy_ui_Component* self, int left, int top)
{
	SetWindowPos(psy_ui_win_component_details(self)->hwnd, NULL, 
	   left, top,
	   0, 0,
	   SWP_NOZORDER | SWP_NOSIZE) ;	
}

void psy_ui_component_resize(psy_ui_Component* self, int width, int height)
{	
	SetWindowPos(psy_ui_win_component_details(self)->hwnd, NULL, 
	   0, 0,
	   width, height,
	   SWP_NOZORDER | SWP_NOMOVE);	
}

void psy_ui_component_setposition(psy_ui_Component* self, int x, int y, int width, int height)
{	
	SetWindowPos(psy_ui_win_component_details(self)->hwnd, 0, x, y, width, height, SWP_NOZORDER);	
}

void psy_ui_component_setmenu(psy_ui_Component* self, psy_ui_Menu* menu)
{
	SetMenu(psy_ui_win_component_details(self)->hwnd, menu->hmenu);
}

void psy_ui_component_settitle(psy_ui_Component* self, const char* title)
{
	SetWindowText(psy_ui_win_component_details(self)->hwnd, title);
}

void psy_ui_component_enumerate_children(psy_ui_Component* self, void* context, 
	int (*childenum)(void*, void*))
{	
	EnumCallback callback;
	
	callback.context = context;
	callback.childenum = childenum;
	EnumChildWindows(psy_ui_win_component_details(self)->hwnd, childenumproc, (LPARAM) &callback);
}

BOOL CALLBACK childenumproc(HWND hwnd, LPARAM lParam)
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

psy_List* psy_ui_component_children(psy_ui_Component* self, int recursive)
{	
	psy_List* children = 0;	
	if (recursive == 1) {
		EnumChildWindows((HWND)psy_ui_win_component_details(self)->hwnd, allchildenumproc, (LPARAM) &children);
	} else {
		uintptr_t hwnd = (uintptr_t)GetWindow((HWND)psy_ui_win_component_details(self)->hwnd, GW_CHILD);
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

BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam)
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

void psy_ui_component_capture(psy_ui_Component* self)
{
	SetCapture((HWND)psy_ui_win_component_details(self)->hwnd);
}

void psy_ui_component_releasecapture()
{
	ReleaseCapture();
}

void psy_ui_component_invalidate(psy_ui_Component* self)
{
	InvalidateRect(psy_ui_win_component_details(self)->hwnd, NULL, FALSE);
}

void psy_ui_component_invalidaterect(psy_ui_Component* self, const psy_ui_Rectangle* r)
{
	RECT rc;

	rc.left = r->left;
	rc.top = r->top;
	rc.right = r->right;
	rc.bottom = r->bottom;
	InvalidateRect(psy_ui_win_component_details(self)->hwnd, &rc, FALSE);
}

void psy_ui_component_update(psy_ui_Component* self)
{
	UpdateWindow(psy_ui_win_component_details(self)->hwnd);
}

void psy_ui_component_setfocus(psy_ui_Component* self)
{
	SetFocus(psy_ui_win_component_details(self)->hwnd);
	psy_signal_emit(&self->signal_focus, self, 0);
}

int psy_ui_component_hasfocus(psy_ui_Component* self)
{
	return psy_ui_win_component_details(self)->hwnd == GetFocus();
}

void psy_ui_component_setfont(psy_ui_Component* self, psy_ui_Font* source)
{	
	if (source) {
		HFONT hfont;
		int dispose;

		dispose = self->style.use_font;
		hfont = ((psy_ui_win_FontImp*)(source))->hfont;
		SendMessage(psy_ui_win_component_details(self)->hwnd, WM_SETFONT,
			(WPARAM) hfont, 0);
		if (dispose) {
			psy_ui_font_dispose(&self->style.font);
		}
		psy_ui_font_init(&self->style.font, 0);
		psy_ui_font_copy(&self->style.font, source);
		self->style.use_font = 1;
	} else {		
		int dispose;

		dispose = self->style.use_font;
		self->style.use_font = 0;
		psy_ui_component_updatefont(self);
		if (dispose) {
			psy_ui_font_dispose(&self->style.font);
		}
	}	
}

void psy_ui_component_preventdefault(psy_ui_Component* self)
{
	self->preventdefault = 1;
}

int psy_ui_component_visible(psy_ui_Component* self)
{
	return IsWindowVisible(psy_ui_win_component_details(self)->hwnd);
}

void psy_ui_component_align(psy_ui_Component* self)
{	
	psy_ui_Aligner aligner;

	psy_ui_aligner_init(&aligner, self);
	psy_ui_aligner_align(&aligner);	
	psy_signal_emit(&self->signal_align, self, 0);
}

void onpreferredsize(psy_ui_Component* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{			
	psy_ui_Aligner aligner;

	psy_ui_aligner_init(&aligner, self);
	psy_ui_aligner_preferredsize(&aligner, limit, rv);
}

void psy_ui_component_doublebuffer(psy_ui_Component* self)
{
	self->doublebuffered = 1;
}

void psy_ui_component_setmargin(psy_ui_Component* self,
	const psy_ui_Margin* margin)
{	
	if (margin) {
		self->margin = *margin;		
	} else {
		memset(&self->margin, 0, sizeof(psy_ui_Margin));
	}
}

void psy_ui_component_setspacing(psy_ui_Component* self,
	const psy_ui_Margin* spacing)
{	
	if (spacing) {
		self->spacing = *spacing;
	} else {
		memset(&self->spacing, 0, sizeof(psy_ui_Margin));
	}
}

void psy_ui_component_setalign(psy_ui_Component* self, psy_ui_AlignType align)
{
	self->align = align;
}

void psy_ui_component_enablealign(psy_ui_Component* self)
{
	self->alignchildren = 1;	
}

void psy_ui_component_setalignexpand(psy_ui_Component* self, psy_ui_ExpandMode mode)
{
	self->alignexpandmode = mode;
}

void psy_ui_component_preventalign(psy_ui_Component* self)
{
	self->alignchildren = 0;
}

void psy_ui_component_enableinput(psy_ui_Component* self, int recursive)
{
	enableinput(self, TRUE, recursive);
}

void psy_ui_component_preventinput(psy_ui_Component* self, int recursive)
{
	enableinput(self, FALSE, recursive);
}

void enableinput(psy_ui_Component* self, int enable, int recursive)
{	
	EnableWindow((HWND) psy_ui_win_component_details(self)->hwnd, enable);
	if (recursive) {
		psy_List* p;
		psy_List* q;
		
		for (p = q = psy_ui_component_children(self, recursive); p != 0;
				p = p->next) {					
			EnableWindow((HWND)psy_ui_win_component_details((psy_ui_Component*)(p->entry))->hwnd,
				enable);
		}
		psy_list_free(q);
	}
}

void psy_ui_component_setbackgroundmode(psy_ui_Component* self, BackgroundMode mode)
{
	self->backgroundmode = mode;	
}

void psy_ui_component_setbackgroundcolor(psy_ui_Component* self, unsigned int color)
{
	self->backgroundcolor = color;
	if (psy_ui_win_component_details(self)->background) {
		DeleteObject(psy_ui_win_component_details(self)->background);
	}
	psy_ui_win_component_details(self)->background = CreateSolidBrush(color);
}

void psy_ui_component_setcolor(psy_ui_Component* self, unsigned int color)
{
	self->color = color;
}

psy_ui_Size psy_ui_component_textsize(psy_ui_Component* self, const char* text)
{
	psy_ui_Size rv;
	psy_ui_Graphics g;	
	HDC hdc;
	
	hdc = GetDC(psy_ui_win_component_details(self)->hwnd);
    SaveDC (hdc) ;          
	psy_ui_graphics_init(&g, hdc);
	psy_ui_setfont(&g, psy_ui_component_font(self));	
	rv = psy_ui_textsize(&g, text);	
	psy_ui_graphics_dispose(&g);
	RestoreDC(hdc, -1);	
	ReleaseDC(psy_ui_win_component_details(self)->hwnd, hdc);
	return rv;
}

psy_ui_Component* psy_ui_component_parent(psy_ui_Component* self)
{		
	psy_ui_WinApp* winapp;							

	winapp = (psy_ui_WinApp*) app.platform;
	return (psy_ui_Component*) psy_table_at(&winapp->selfmap, 
		(uintptr_t) GetParent((HWND)psy_ui_win_component_details(self)->hwnd));
}

psy_List* psy_ui_components_setalign(psy_List* list, psy_ui_AlignType align,
	const psy_ui_Margin* margin)
{
	psy_List* p;

	for (p = list; p != 0; p = p->next) {
		psy_ui_component_setalign((psy_ui_Component*) p->entry, align);
		if (margin) {
			psy_ui_component_setmargin((psy_ui_Component*) p->entry, margin);
		}
	}
	return list;
}

psy_List* psy_ui_components_setmargin(psy_List* list, const psy_ui_Margin* margin)
{
	psy_List* p;

	for (p = list; p != 0; p = p->next) {
		psy_ui_component_setmargin((psy_ui_Component*) p->entry, margin);
	}
	return list;
}

psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component* self,
	psy_ui_Size* limit)
{
	psy_ui_Size rv;	
	self->vtable->onpreferredsize(self, limit, &rv);
	// psy_signal_emit(&self->signal_preferredsize, self, 2, limit, &rv);	
	return rv;	
}

psy_ui_TextMetric psy_ui_component_textmetric(psy_ui_Component* self)
{			
	psy_ui_TextMetric rv;
	TEXTMETRIC tm;
	HDC hdc;		
	HFONT hPrevFont = 0;
	HFONT hfont = 0;
	
	hdc = GetDC((HWND)psy_ui_win_component_details(self)->hwnd);
    SaveDC(hdc);
	hfont = ((psy_ui_win_FontImp*)psy_ui_component_font(self)->imp)->hfont;
	if (hfont) {
		hPrevFont = SelectObject(hdc, hfont);
	}
	GetTextMetrics (hdc, &tm);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}	
	RestoreDC(hdc, -1);	
	ReleaseDC(psy_ui_win_component_details(self)->hwnd, hdc);
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

void psy_ui_component_seticonressource(psy_ui_Component* self, int ressourceid)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
#if defined(_WIN64)	
	SetClassLongPtr((HWND)psy_ui_win_component_details(self)->hwnd, GCLP_HICON, 
		(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
#else	
	SetClassLong((HWND)psy_ui_win_component_details(self)->hwnd, GCL_HICON, 
		(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
#endif
}

void psy_ui_component_starttimer(psy_ui_Component* self, unsigned int id,
	unsigned int interval)
{
	SetTimer(psy_ui_win_component_details(self)->hwnd, id, interval, 0);
}

void psy_ui_component_stoptimer(psy_ui_Component* self, unsigned int id)
{
	KillTimer(psy_ui_win_component_details(self)->hwnd, id);
}

int psy_ui_browsefolder(psy_ui_Component* self, const char* title, char* path)
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
		bi.hwndOwner = (HWND) psy_ui_win_component_details(self)->hwnd;
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

void psy_ui_component_clientresize(psy_ui_Component* self, int width, int height)
{
	RECT rc;

	rc.left = 0;
	rc.top = 0;
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRectEx(&rc, windowstyle(self),
		GetMenu(psy_ui_win_component_details(self)->hwnd) != NULL,
		windowexstyle(self));
	psy_ui_component_resize(self, rc.right - rc.left,
		rc.bottom - rc.top);
}

int windowstyle(psy_ui_Component* self)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(psy_ui_win_component_details(self)->hwnd,
		GWLP_STYLE);
#else
	rv = (int)GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_STYLE);
#endif
	return rv;
}

int windowexstyle(psy_ui_Component* self)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(psy_ui_win_component_details(self)->hwnd,
		GWLP_EXSTYLE);
#else
	rv = (int)GetWindowLong(psy_ui_win_component_details(self)->hwnd, GWL_EXSTYLE);
#endif
	return rv;
}
