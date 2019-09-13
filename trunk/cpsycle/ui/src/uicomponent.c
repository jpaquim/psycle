// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uicomponent.h"
#include "uimenu.h"
#include "hashtbl.h"
#include <memory.h>
#include <commctrl.h>   // includes the common control header
#include <stdio.h>


static TCHAR szAppClass[] = TEXT ("PsycleApp") ;
static TCHAR szComponentClass[] = TEXT ("PsycleComponent") ;
IntHashTable selfmap;
IntHashTable winidmap;
int winid = 20000;
extern IntHashTable menumap;

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                           WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam);
BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);
static void enableinput(ui_component* self, int enable, int recursive);

HINSTANCE appInstance = 0;

void ui_init(HINSTANCE hInstance)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	int succ;
	
	InitIntHashTable(&selfmap, 100);
	InitIntHashTable(&winidmap, 100);	

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = ui_winproc ;
    wndclass.cbClsExtra    = 0 ;
    wndclass.cbWndExtra    = 0 ;
    wndclass.hInstance     = hInstance ;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = szAppClass ;	
	if (!RegisterClass (&wndclass))
    {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      szAppClass, MB_ICONERROR) ;		
    }
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = ui_winproc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (long); 
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szComponentClass;
     

	RegisterClass (&wndclass) ;

	ui_menu_setup();


	// Ensure that the common control DLL is loaded.     		
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);        

	appInstance = hInstance;
}

void ui_dispose()
{
	DisposeIntHashTable(&selfmap);
	DisposeIntHashTable(&winidmap);
}


LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{	
    PAINTSTRUCT  ps ;     
	ui_component*   component;
	ui_graphics	 g;
	HMENU		 hMenu;
	ui_menu*	 menu;
	int			 menu_id;		


	component = SearchIntHashTable(&selfmap, (int) hwnd);
	if (component && component->signal_windowproc.slots) {				
		signal_emit(&component->signal_windowproc, component, 3, (LONG)message, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam));
		if (component->preventdefault) {					
			return 0;
		} else {
			return DefWindowProc (hwnd, message, wParam, lParam);
		}
	}

	switch (message)
    {		
		case WM_SHOWWINDOW:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component) {				                    
				if (wParam == TRUE) {
					signal_emit(&component->signal_show, component, 0);
				} else {
					signal_emit(&component->signal_hide, component, 0);
				}
				return 0 ;
			}
		break;		
		case WM_SIZE:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component) {				                    
				if (component->alignchildren == 1) {
					ui_component_align(component);
				}
				signal_emit(&component->signal_size, component, 2, LOWORD (lParam), HIWORD (lParam));
				return 0 ;
			}			
		break;
		case WM_TIMER:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_timer.slots) {
				signal_emit(&component->signal_timer, component, 1, (int) wParam);				
				return 0 ;
			}
		break;
		case WM_COMMAND:
          hMenu = GetMenu (hwnd) ;
		  menu_id = LOWORD (wParam);
          menu = SearchIntHashTable(&menumap, menu_id);		  
          if (menu && menu->execute) {	
			menu->execute(menu);
		  }
		  component = SearchIntHashTable(&winidmap, LOWORD(wParam));
			if (component && component->events.command) {				                    
				component->events.command(component->events.cmdtarget, wParam, lParam);
				return 0;
			}
		  return 0 ;  
		break;          
		case WM_CREATE:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_create.slots) {	
				signal_emit(&component->signal_create, component, 0);
			}
			return 0 ;
		break;
		case WM_PAINT :
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && (component->signal_draw.slots ||
					component->backgroundmode != BACKGROUND_NONE)) {		
				HDC bufferDC;
				HBITMAP bufferBmp;
				HBITMAP oldBmp;
				HDC hdc;				
				RECT rect;
				HFONT hPrevFont = 0;

				ui_graphics_init(&g, BeginPaint (hwnd, &ps));				
				if (component->doublebuffered) {
					hdc = g.hdc;
					bufferDC = CreateCompatibleDC(hdc);
					GetClientRect(hwnd, &rect);
					bufferBmp = CreateCompatibleBitmap(hdc, rect.right,
						rect.bottom);
					oldBmp = SelectObject(bufferDC, bufferBmp);
					g.hdc = bufferDC;
				}									
				ui_setrectangle(&g.clip,
					ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
					ps.rcPaint.bottom - ps.rcPaint.top);				
				if (component->backgroundmode == BACKGROUND_SET) {
					ui_drawsolidrectangle(&g, g.clip, component->backgroundcolor);
				}
				if (component->font.hfont) {
					hPrevFont = SelectObject(g.hdc, component->font.hfont);
				}
				signal_emit(&component->signal_draw, component, 1, (int)&g);
				if (hPrevFont) {
					SelectObject(g.hdc, hPrevFont);
				}
				if (component->doublebuffered) {
					g.hdc = hdc;
					BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom,
						bufferDC,0,0,SRCCOPY);				
					SelectObject(bufferDC, oldBmp);
					DeleteObject(bufferBmp);
					DeleteDC(bufferDC);
				}
				ui_graphics_dispose(&g);
				EndPaint (hwnd, &ps) ;
				return 0 ;
			}
		break;
		case WM_DESTROY:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_destroy.slots) {
				signal_emit(&component->signal_destroy, component, 0);
				ui_component_dispose(component);
			}			
			return 0;
		break;
		case WM_KEYDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component) {			
				component->propagateevent = component->defaultpropagation;
				if (component->signal_keydown.slots) {
					signal_emit(&component->signal_keydown, component, 2, (int)wParam, lParam);
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				}				
				component->propagateevent = component->defaultpropagation;
				return 0 ;								
			}
		break;
		case WM_KEYUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_keyup.slots) {
				signal_emit(&component->signal_keyup, component, 2, (int)wParam, lParam);			
			}
			return 0;
		break;
		case WM_LBUTTONUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mouseup.slots) {
				signal_emit(&component->signal_mouseup, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);				
				return 0 ;
			}
		break;
		case WM_RBUTTONUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mouseup.slots) {			
				signal_emit(&component->signal_mouseup, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);				
				return 0 ;
			}			
		break;
		case WM_MBUTTONUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mouseup.slots) {			
				signal_emit(&component->signal_mouseup, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);				
				return 0 ;
			}
		break;
		case WM_LBUTTONDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mousedown.slots) {
				signal_emit(&component->signal_mousedown, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);				
				return 0 ;
			}			
		break;
		case WM_RBUTTONDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mousedown.slots) {		
				signal_emit(&component->signal_mousedown, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);
				return 0 ;
			}
		break;
		case WM_MBUTTONDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mousedown.slots) {		
				signal_emit(&component->signal_mousedown, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);				
				return 0 ;
			}
		break;
		case WM_LBUTTONDBLCLK:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component) {			
				component->propagateevent = component->defaultpropagation;
				if (component->signal_mousedoubleclick.slots) {					
					signal_emit(&component->signal_mousedoubleclick, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);					
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;               
				}				
				component->propagateevent = component->defaultpropagation;
				return 0 ;								
			}
		break;
		case WM_MBUTTONDBLCLK:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mousedoubleclick.slots) {
				signal_emit(&component->signal_mousedoubleclick, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);
				return 0 ;
			}
		break;
		case WM_RBUTTONDBLCLK:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mousedoubleclick.slots) {
				signal_emit(&component->signal_mousedoubleclick, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);
				return 0 ;
			}
		break;
		case WM_MOUSEMOVE:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->signal_mousemove.slots) {
				signal_emit(&component->signal_mousemove, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);
				return 0 ;
			}
		break;
		case WM_VSCROLL:
			handle_vscroll(hwnd, wParam, lParam);
			return 0;
		break;
		case WM_HSCROLL:
			component = SearchIntHashTable(&selfmap, (int) lParam);
			if (component && component->signal_windowproc.slots) {				                    
				signal_emit(&component->signal_windowproc, component, 3, message, wParam, lParam);
				return DefWindowProc (hwnd, message, wParam, lParam);
			}
			handle_hscroll(hwnd, wParam, lParam);
			return 0;
		break;
		default:			
		break;
	}	
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}


void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; //, iHorzPos;	
	ui_component*   component;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_VERT, &si) ;	
	// Save the position for comparison later on
	iPos = si.nPos ;

	handle_scrollparam(&si, wParam);	
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_VERT, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_VERT, &si) ;
	// If the position has changed, scroll the window and update it
	if (si.nPos != iPos)
	{                    
		component = SearchIntHashTable(&selfmap, (int) hwnd);
		if (component && component->signal_scroll.slots) {
			signal_emit(&component->signal_scroll, component, 2, 
				0, component->scrollstepy * (iPos - si.nPos));			
		}
		ScrollWindow (hwnd, 0, component->scrollstepy * (iPos - si.nPos), 
                                   NULL, NULL) ;
		UpdateWindow (hwnd) ;
	}
}

void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; 
	ui_component*   component;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_HORZ, &si) ;	

	// Save the position for comparison later on
	iPos = si.nPos ;
	handle_scrollparam(&si, wParam);
	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_HORZ, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_HORZ, &si) ;

	// If the position has changed, scroll the window and update it

	if (si.nPos != iPos)
	{                    
		component = SearchIntHashTable(&selfmap, (int) hwnd);
		if (component && component->signal_scroll.slots) {
			signal_emit(&component->signal_scroll, component, 2, 
				component->scrollstepx * (iPos - si.nPos), 0);			
		}
		ScrollWindow (hwnd, component->scrollstepx * (iPos - si.nPos), 0, 
                                   NULL, NULL) ;
		UpdateWindow (hwnd) ;
	}
}

void handle_scrollparam(SCROLLINFO* si, WPARAM wParam)
{
	switch (LOWORD (wParam)) {
		case SB_TOP:
		   si->nPos = si->nMin ;
		break ;
		case SB_BOTTOM:
		   si->nPos = si->nMax ;
		break ;
		case SB_LINEUP:
		   si->nPos -= 1 ;
		break ;
		case SB_LINEDOWN:
		   si->nPos += 1 ;
		break ;
		case SB_PAGEUP:
		   si->nPos -= si->nPage ;
		break ;
		case SB_PAGEDOWN:
		   si->nPos += si->nPage ;
		break ;
		case SB_THUMBTRACK:
		   si->nPos = si->nTrackPos ;
		break ;
		default:
		break ;         
	}
}

void ui_frame_init(ui_component* frame, ui_component* parent)
{		
	HWND hWndParent = 0;
	int style = 0;

	if (parent) {
	  hWndParent = parent->hwnd;
	  // style  |= WS_CHILD;
	}
	memset(&frame->events, 0, sizeof(ui_events));	
	ui_component_init_signals(frame);
	frame->doublebuffered = 0;
	frame->hwnd = CreateWindow (szAppClass, TEXT ("Psycle"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT | style,
                          hWndParent, NULL, appInstance, NULL);     	
	InsertIntHashTable(&selfmap, (int)frame->hwnd, frame);	
	frame->events.target = frame;
	frame->events.cmdtarget = frame;	
}

void ui_component_init(ui_component* component, ui_component* parent)
{		
	memset(&component->events, 0, sizeof(ui_events));
	ui_component_init_signals(component);	
	component->doublebuffered = 0;
	component->hwnd = CreateWindow (szComponentClass, NULL,
		WS_CHILDWINDOW | WS_VISIBLE,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)component->hwnd, component);
	component->events.target = component;			
	component->scrollstepx = 100;
	component->scrollstepy = 12;
	component->backgroundcolor = 0xFFFFFFFF;
	component->backgroundmode = BACKGROUND_NONE;
}

void ui_component_init_signals(ui_component* component)
{
	signal_init(&component->signal_size);
	signal_init(&component->signal_draw);
	signal_init(&component->signal_timer);
	signal_init(&component->signal_keydown);
	signal_init(&component->signal_keyup);
	signal_init(&component->signal_mousedown);
	signal_init(&component->signal_mouseup);
	signal_init(&component->signal_mousemove);
	signal_init(&component->signal_mousedoubleclick);
	signal_init(&component->signal_scroll);
	signal_init(&component->signal_create);
	signal_init(&component->signal_destroy);
	signal_init(&component->signal_show);
	signal_init(&component->signal_hide);
	signal_init(&component->signal_windowproc);
	component->propagateevent = 0;
	component->preventdefault = 0;
	component->align = UI_ALIGN_NONE;
	component->alignchildren = 0;
	memset(&component->margin, 0, sizeof(ui_margin));
	component->debugflag = 0;
	component->defaultpropagation = 0;
	component->font.stock = 0;
	component->font.hfont = 0;
}

void ui_component_dispose(ui_component* component)
{	
	signal_dispose(&component->signal_size);
	signal_dispose(&component->signal_draw);
	signal_dispose(&component->signal_timer);
	signal_dispose(&component->signal_keydown);
	signal_dispose(&component->signal_keyup);
	signal_dispose(&component->signal_mousedown);
	signal_dispose(&component->signal_mouseup);
	signal_dispose(&component->signal_mousemove);
	signal_dispose(&component->signal_mousedoubleclick);
	signal_dispose(&component->signal_scroll);
	signal_dispose(&component->signal_create);
	signal_dispose(&component->signal_destroy);
	signal_dispose(&component->signal_show);
	signal_dispose(&component->signal_hide);
	signal_dispose(&component->signal_windowproc);
	if (component->font.hfont) {
		ui_font_dispose(&component->font);
	}
}

void ui_classcomponent_init(ui_component* component, ui_component* parent, const char* classname)
{
	memset(&component->events, 0, sizeof(ui_events));	
	ui_component_init_signals(component);
	component->doublebuffered = 0;
	component->hwnd = CreateWindow (classname, NULL,
		WS_CHILDWINDOW | WS_VISIBLE,
		0, 0, 100, 100,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)component->hwnd, component);	
	component->events.target = component;
	component->align = 0;
}

ui_size ui_component_size(ui_component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetClientRect (self->hwnd, &rect) ;
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

ui_size ui_component_frame_size(ui_component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetWindowRect (self->hwnd, &rect) ;
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

void ui_component_show_state(ui_component* self, int cmd)
{
	ShowWindow(self->hwnd, cmd);
	UpdateWindow(self->hwnd) ;
}

void ui_component_show(ui_component* self)
{
	ShowWindow(self->hwnd, SW_SHOW);
	UpdateWindow(self->hwnd) ;
}

void ui_component_hide(ui_component* self)
{
	ShowWindow(self->hwnd, SW_HIDE);
	UpdateWindow(self->hwnd) ;
}

void ui_component_showhorizontalscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_HSCROLL);
}

void ui_component_hidehorizontalscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
}

void ui_component_sethorizontalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo(self->hwnd, SB_HORZ, &si, TRUE);
}

void ui_component_showverticalscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_VSCROLL);
}

void ui_component_hideverticalscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
}

void ui_component_setverticalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo(self->hwnd, SB_VERT, &si, TRUE);
}

void ui_component_move(ui_component* self, int left, int top)
{
	SetWindowPos (self->hwnd, NULL, 
	   left,
	   top,
	   0, 0, SWP_NOZORDER | SWP_NOSIZE) ;	
}

void ui_component_resize(ui_component* self, int width, int height)
{	
	ui_size size = ui_component_size(self);
	SetWindowPos (self->hwnd, NULL, 
	   0,
	   0,
	   width, height, SWP_NOZORDER | SWP_NOMOVE);
	if (size.width == width && size.height == height) {
		SendMessage(self->hwnd, WM_SIZE, SIZE_RESTORED, MAKELONG(width, height));
	}
}

void ui_component_setposition(ui_component* component, int x, int y, int width, int height)
{
	ui_component_move(component, x, y);
	ui_component_resize(component, width, height);
}

void ui_component_setmenu(ui_component* self, ui_menu* menu)
{
	SetMenu(self->hwnd, menu->hmenu);
}

void ui_component_settitle(ui_component* self, const char* title)
{
	SetWindowText(self->hwnd, title);
}

void ui_component_enumerate_children(ui_component* self, void* context, int (*childenum)(void*, void*))
{
	self->events.childenum = childenum;
	self->events.target = context;
	EnumChildWindows (self->hwnd, ChildEnumProc, (LPARAM) self);    
}

BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam)
{
	ui_component* self = (ui_component*) lParam;
	ui_component* child = SearchIntHashTable(&selfmap, (int)hwnd);
	if (child &&  self->events.childenum) {
		return self->events.childenum(self->events.target, child);		  
	}     
    return FALSE ;
}

List* ui_component_children(ui_component* self, int recursive)
{	
	List* children = 0;	
	if (recursive == 1) {
		EnumChildWindows (self->hwnd, AllChildEnumProc, (LPARAM) &children);
	} else {
		HWND hwnd = GetWindow(self->hwnd, GW_CHILD);
		if (hwnd) {
			ui_component* child = SearchIntHashTable(&selfmap, (int)hwnd);
			if (child) {				
				children = list_create(child);				
			}
		}
		while (hwnd) {
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
			if (hwnd) {
				ui_component* child = SearchIntHashTable(&selfmap, (int)hwnd);
				if (child) {
					if (children == 0) {
						children = list_create(child);
					} else {
						list_append(children, child);
					}		
				}
			}
		}
	}
	return children;
}

BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam)
{
	List** pChildren = (List**) lParam;
	ui_component* child = SearchIntHashTable(&selfmap, (int)hwnd);
	if (child) {
		if (*pChildren == 0) {
			*pChildren = list_create(child);
		} else {
			list_append(*pChildren, child);
		}		
	}     
    return TRUE;
}

void ui_component_capture(ui_component* self)
{
	SetCapture(self->hwnd);
}

void ui_component_releasecapture()
{
	ReleaseCapture();
}

void ui_invalidate(ui_component* self)
{
	InvalidateRect(self->hwnd, NULL, TRUE);
}

void ui_component_setfocus(ui_component* self)
{
	SetFocus(self->hwnd);
}

void ui_component_setfont(ui_component* self, ui_font* font)
{
	if (font && font->hfont) {
		SendMessage (self->hwnd, WM_SETFONT, (WPARAM) font->hfont, 0);
	}
	if (font) {
		self->font = *font;
	}
}

void ui_component_propagateevent(ui_component* self)
{
	self->propagateevent = 1;
}

void ui_component_preventdefault(ui_component* self)
{
	self->preventdefault = 1;
}

int ui_component_visible(ui_component* self)
{
	return IsWindowVisible(self->hwnd);
}

void ui_component_align(ui_component* self)
{	
	int cpy = 0;
	List* p;
	ui_size size;

	size = ui_component_size(self);	
	for (p = ui_component_children(self, 0); p != 0; p = p->next) {
		ui_component* component = (ui_component*)p->entry;
		if (component->debugflag == 1) {
			cpy = cpy;
		}
		if (IsWindowVisible(component->hwnd)) {
			if (component->align == UI_ALIGN_FILL) {
				ui_component_setposition(component,
					component->margin.left,
					component->margin.top,				
					size.width - component->margin.left - component->margin.right,
					size.height - component->margin.top - component->margin.bottom);
			} else
			if (component->align == UI_ALIGN_TOP) {			
				ui_size componentsize = ui_component_size(component);		
				cpy += component->margin.top;
				ui_component_setposition(component, component->margin.left, cpy,
					size.width - component->margin.left - component->margin.right,
					componentsize.height);
				cpy += component->margin.bottom;
				cpy += componentsize.height;
			}		
		}
	}
	list_free(p);
}

void ui_component_setmargin(ui_component* self, const ui_margin* margin)
{
	if (margin) {
		self->margin = *margin;
	} else {
		memset(&self->margin, 0, sizeof(ui_margin));
	}
}

void ui_component_setalign(ui_component* self, UiAlignType align)
{
	self->align = align;
}

void ui_component_enablealign(ui_component* self)
{
	self->alignchildren = 1;
}

void ui_component_preventalign(ui_component* self)
{
	self->alignchildren = 0;
}

void ui_component_enableinput(ui_component* self, int recursive)
{
	enableinput(self, TRUE, recursive);
}

void ui_component_preventinput(ui_component* self, int recursive)
{
	enableinput(self, FALSE, recursive);
}

void enableinput(ui_component* self, int enable, int recursive)
{	
	EnableWindow(self->hwnd, enable);
	if (recursive) {
		List* p;
		
		for (p = ui_component_children(self, recursive); p != 0; p = p->next) {
			EnableWindow(((ui_component*)(p->entry))->hwnd, enable);
		}
		list_free(p);
	}
}

int ui_openfile(ui_component* self, char* szTitle, char* szFilter,
	char* szDefExtension, char* szOpenName)
{
	OPENFILENAME ofn;

	*szOpenName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= self->hwnd; 
	ofn.lpstrFilter= szFilter; 
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szOpenName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szTitle; 
	ofn.nMaxFileTitle= MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR)NULL; 
	ofn.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST; 
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension; 

	return GetOpenFileName(&ofn);
}

void ui_component_setbackgroundmode(ui_component* self, BackgroundMode mode)
{
	self->backgroundmode = mode;
}

void ui_component_setbackgroundcolor(ui_component* self, unsigned int color)
{
	self->backgroundcolor = color;
}

ui_size ui_component_textsize(ui_component* self, const char* text)
{
	ui_size rv;
	ui_graphics g;
	HFONT hPrevFont = 0;
	HDC hdc;
	
	hdc = GetDC (self->hwnd);	
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
	ReleaseDC(NULL, hdc);
	return rv;
}