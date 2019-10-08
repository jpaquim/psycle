// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uicomponent.h"
#include "uimenu.h"
#include "hashtbl.h"
#include <memory.h>
#include <commctrl.h>   // includes the common control header
#include <stdio.h>

HINSTANCE appInstance = 0;

TCHAR szAppClass[] = TEXT("PsycleApp");
static TCHAR szComponentClass[] = TEXT("PsycleComponent");
static winid_t winid = 20000;
Table selfmap;
static Table winidmap;
static ui_font defaultfont;
static int defaultbackgroundcolor = 0x00232323;
static int defaultcolor = 0x00D1C5B6;
static HBRUSH defaultbackgroundbrush;
static int tracking = 0;

extern Table menumap;

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);
BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam);
BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);
static void enableinput(ui_component* self, int enable, int recursive);
static void onpreferredsize(ui_component*, ui_component* sender, 
	ui_size* limit, int* width, int* height);

void ui_init(HINSTANCE hInstance)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	ui_fontinfo fontinfo;
	int succ;
	
	table_init(&selfmap);
	table_init(&winidmap);

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
	
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&defaultfont, &fontinfo);	
	defaultbackgroundbrush = CreateSolidBrush(0x00232323);
}

void ui_dispose()
{
	table_dispose(&selfmap);
	table_dispose(&winidmap);
	DeleteObject(defaultfont.hfont);
	DeleteObject(defaultbackgroundbrush);
}

void ui_replacedefaultfont(ui_component* main, ui_font* font)
{		
	if (font && main) {
		List* p;

		if (main->font.hfont == defaultfont.hfont) {
			ui_component_setfont(main, font);
		}
		for (p = ui_component_children(main, 1); p != 0; p = p->next) {
			ui_component* child;

			child = (ui_component*)p->entry;
			if (child->font.hfont == defaultfont.hfont) {
				ui_component_setfont(child, font);
				ui_component_align(child);
			}		
		}		
		ui_font_dispose(&defaultfont);
		defaultfont = *font;
		ui_component_align(main);
	}
}

int ui_win32_component_init(ui_component* self, ui_component* parent,
		LPCTSTR classname, 
		int x, int y, int width, int height,
		DWORD dwStyle,
		int usecommand)
{
	int err = 0;
	HINSTANCE hInstance;

	ui_component_init_signals(self);
	if (parent) {
#if defined(_WIN64)		
		hInstance = (HINSTANCE) GetWindowLongPtr (parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE);
#endif
	} else {
		hInstance = appInstance;
	}
	self->hwnd = CreateWindow(
		classname,
		NULL,		
		dwStyle,
		x, y, width, height,
		parent ? parent->hwnd : NULL,
		usecommand ? (HMENU)winid : NULL,
		hInstance,
		NULL);	
	if (!self->hwnd) {
        MessageBox(NULL, "Failed To Create Component", "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		table_insert(&selfmap, (int)self->hwnd, self);
	}
	if (err == 0 && usecommand) {
		table_insert(&winidmap, (int)winid, self);
		winid++;		
	}
	ui_component_init_base(self);
	return err;
}

void ui_component_init(ui_component* component, ui_component* parent)
{
	HINSTANCE hInstance;
    
#if defined(_WIN64)
		hInstance = (HINSTANCE) GetWindowLongPtr (parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE);
#endif
	ui_component_init_signals(component);	
	component->doublebuffered = 0;
	component->hwnd = CreateWindow (szComponentClass, NULL,
		WS_CHILDWINDOW | WS_VISIBLE,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		hInstance,
		NULL);		
	table_insert(&selfmap, (int)component->hwnd, component);
	ui_component_init_base(component);
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
	signal_init(&component->signal_mouseenter);
	signal_init(&component->signal_mousehover);
	signal_init(&component->signal_mouseleave);
	signal_init(&component->signal_scroll);
	signal_init(&component->signal_create);
	signal_init(&component->signal_destroy);
	signal_init(&component->signal_show);
	signal_init(&component->signal_hide);
	signal_init(&component->signal_align);
	signal_init(&component->signal_preferredsize);
	signal_init(&component->signal_windowproc);
	signal_init(&component->signal_command);
}

void ui_component_init_base(ui_component* self) {
	self->scrollstepx = 100;
	self->scrollstepy = 12;
	self->propagateevent = 0;
	self->preventdefault = 0;
	self->align = UI_ALIGN_NONE;
	self->justify = UI_JUSTIFY_EXPAND;
	self->alignchildren = 0;
	self->alignexpandmode = UI_NOEXPAND;
	memset(&self->margin, 0, sizeof(ui_margin));
	self->debugflag = 0;
	self->defaultpropagation = 0;	
	self->visible = 1;
	self->doublebuffered = 0;
	self->backgroundmode = BACKGROUND_NONE;
	self->backgroundcolor = defaultbackgroundcolor;
	self->background = 0;
	self->color = defaultcolor;
	ui_component_setfont(self, &defaultfont);
	ui_component_setbackgroundcolor(self, self->backgroundcolor);
	signal_connect(&self->signal_preferredsize, self, onpreferredsize);
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
	signal_dispose(&component->signal_mouseenter);
	signal_dispose(&component->signal_mousehover);	
	signal_dispose(&component->signal_mouseleave);
	signal_dispose(&component->signal_scroll);
	signal_dispose(&component->signal_create);
	signal_dispose(&component->signal_destroy);
	signal_dispose(&component->signal_show);
	signal_dispose(&component->signal_hide);
	signal_dispose(&component->signal_align);
	signal_dispose(&component->signal_preferredsize);
	signal_dispose(&component->signal_windowproc);
	signal_dispose(&component->signal_command);
	if (component->font.hfont && component->font.hfont != defaultfont.hfont) {
		ui_font_dispose(&component->font);
	}
	if (component->background) {
		DeleteObject(component->background);
	}
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

	component = table_at(&selfmap, (int) hwnd);	
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
			component = table_at(&selfmap, (int) hwnd);
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
			component = table_at(&selfmap, (int) hwnd);
			if (component) {				                    
				if (component->alignchildren == 1) {
					ui_component_align(component);
				}
				signal_emit(&component->signal_size, component, 2, LOWORD (lParam), HIWORD (lParam));
				return 0 ;
			}			
		break;
		case WM_TIMER:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_timer.slots) {
				signal_emit(&component->signal_timer, component, 1, (int) wParam);				
				return 0 ;
			}
		break;		
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSTATIC:			
			component = table_at(&selfmap, lParam);			
			if (component) {					
				SetTextColor((HDC) wParam, component->color);
				SetBkColor((HDC) wParam, component->backgroundcolor);
				if ((component->backgroundmode & BACKGROUND_SET) == BACKGROUND_SET) {
					return (long) component->background;
				} else {
					return (long) GetStockObject(NULL_BRUSH);
				}
			} else {
				SetTextColor((HDC) wParam, defaultcolor);
				SetBkColor((HDC) wParam, defaultbackgroundcolor);
				return (long) defaultbackgroundbrush;
			}
		break;
		case WM_ERASEBKGND:
			return 1;
		break;
		case WM_COMMAND:
          hMenu = GetMenu (hwnd) ;
		  menu_id = LOWORD (wParam);
          menu = table_at(&menumap, menu_id);		  
          if (menu && menu->execute) {	
			menu->execute(menu);
		  }
		  component = table_at(&winidmap, LOWORD(wParam));
		  if (component && component->signal_command.slots) {
				signal_emit(&component->signal_command, component, 2, wParam, lParam);				
				return 0;
			}
		  return 0 ;  
		break;          
		case WM_CREATE:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_create.slots) {	
				signal_emit(&component->signal_create, component, 0);
			}
			return 0 ;
		break;
		case WM_PAINT :
			component = table_at(&selfmap, (int) hwnd);
			if (component && (component->signal_draw.slots ||
					component->backgroundmode != BACKGROUND_NONE)) {		
				HDC bufferDC;
				HBITMAP bufferBmp;
				HBITMAP oldBmp;
				HDC hdc;				
				RECT rect;
				HFONT hPrevFont = 0;

				hdc = BeginPaint (hwnd, &ps);
				GetClientRect(hwnd, &rect);
				if (component->doublebuffered) {					
					bufferDC = CreateCompatibleDC(hdc);					
					bufferBmp = CreateCompatibleBitmap(hdc, rect.right,
						rect.bottom);
					oldBmp = SelectObject(bufferDC, bufferBmp);					
					ui_graphics_init(&g, bufferDC);
				} else {
					ui_graphics_init(&g, hdc);
				}
				ui_setrectangle(&g.clip,
					ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
					ps.rcPaint.bottom - ps.rcPaint.top);				
				if (component->backgroundmode == BACKGROUND_SET) {
					ui_rectangle r;
					ui_setrectangle(&r,
					rect.left, rect.top, rect.right - rect.left,
					rect.bottom - rect.top);				
					ui_drawsolidrectangle(&g, r, component->backgroundcolor);
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
					BitBlt(hdc, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
						bufferDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);				
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
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_destroy.slots) {
				signal_emit(&component->signal_destroy, component, 0);
				ui_component_dispose(component);
			}			
			return 0;
		break;
		case WM_KEYDOWN:
			component = table_at(&selfmap, (int) hwnd);
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
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_keyup.slots) {
				signal_emit(&component->signal_keyup, component, 2, (int)wParam, lParam);			
			}
			return 0;
		break;
		case WM_LBUTTONUP:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mouseup.slots) {
				signal_emit(&component->signal_mouseup, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);				
				return 0 ;
			}
		break;
		case WM_RBUTTONUP:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mouseup.slots) {			
				signal_emit(&component->signal_mouseup, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);				
				return 0 ;
			}			
		break;
		case WM_MBUTTONUP:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mouseup.slots) {			
				signal_emit(&component->signal_mouseup, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);				
				return 0 ;
			}
		break;
		case WM_LBUTTONDOWN:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mousedown.slots) {
				signal_emit(&component->signal_mousedown, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);				
				return 0 ;
			}			
		break;
		case WM_RBUTTONDOWN:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mousedown.slots) {		
				signal_emit(&component->signal_mousedown, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);
				return 0 ;
			}
		break;
		case WM_MBUTTONDOWN:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mousedown.slots) {		
				signal_emit(&component->signal_mousedown, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);				
				return 0 ;
			}
		break;
		case WM_LBUTTONDBLCLK:
			component = table_at(&selfmap, (int) hwnd);
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
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mousedoubleclick.slots) {
				signal_emit(&component->signal_mousedoubleclick, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);
				return 0 ;
			}
		break;		
		case WM_RBUTTONDBLCLK:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mousedoubleclick.slots) {
				signal_emit(&component->signal_mousedoubleclick, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);
				return 0 ;
			}
		break;
		case WM_MOUSEMOVE:
			component = table_at(&selfmap, (int) hwnd);
			if (component) {
				if (!tracking) {
					TRACKMOUSEEVENT tme;

					if (component && component->signal_mouseenter.slots) {	
						signal_emit(&component->signal_mouseenter, component, 0);
					}
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 200;
					tme.hwndTrack = hwnd;
					if (_TrackMouseEvent(&tme)) {
						tracking = 1;
					} 
					return 0;
				}				
			}
			if (component && component->signal_mousemove.slots) {				
				signal_emit(&component->signal_mousemove, component, 3, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);
				return 0 ;
			}
		break;
		case WM_MOUSEHOVER:
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mousehover.slots) {				                    
				signal_emit(&component->signal_mousehover, component, 0);
				return 0;
			}
		break;
		case WM_MOUSELEAVE:	
			tracking = 0;
			component = table_at(&selfmap, (int) hwnd);
			if (component && component->signal_mouseleave.slots) {				                    
				signal_emit(&component->signal_mouseleave, component, 0);
				return 0;
			}			
		break; 
		case WM_VSCROLL:
			handle_vscroll(hwnd, wParam, lParam);
			return 0;
		break;
		case WM_HSCROLL:
			component = table_at(&selfmap, (int) lParam);
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
		component = table_at(&selfmap, (int) hwnd);
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
		component = table_at(&selfmap, (int) hwnd);
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
	self->visible = 1;
	ShowWindow(self->hwnd, SW_SHOW);
	UpdateWindow(self->hwnd) ;
}

void ui_component_hide(ui_component* self)
{
	self->visible = 0;
	ShowWindow(self->hwnd, SW_HIDE);
	UpdateWindow(self->hwnd) ;
}

void ui_component_showhorizontalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE, 
		GetWindowLongPtr(self->hwnd, GWL_STYLE) | WS_HSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_HSCROLL);
#endif
}


void ui_component_hidehorizontalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE, 
		GetWindowLongPtr(self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#endif
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
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE, 
		GetWindowLongPtr(self->hwnd, GWL_STYLE) | WS_VSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_VSCROLL);
#endif
}

void ui_component_hideverticalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->hwnd, GWL_STYLE, 
		GetWindowLongPtr(self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#else
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#endif
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

void ui_component_setverticalscrollposition(ui_component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(self->hwnd, SB_VERT, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo(self->hwnd, SB_VERT, &si, TRUE);
}

void ui_component_verticalscrollrange(ui_component* self, int* scrollmin,
	int* scrollmax)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo(self->hwnd, SB_VERT, &si);
	*scrollmin = si.nMin;
	*scrollmax = si.nMax;
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
	SetWindowPos (self->hwnd, NULL, 
	   0,
	   0,
	   width, height, SWP_NOZORDER | SWP_NOMOVE);	
}

void ui_component_setposition(ui_component* self, int x, int y, int width, int height)
{	
	SetWindowPos(self->hwnd, 0, x, y, width, height, SWP_NOZORDER);	
}

void ui_component_setmenu(ui_component* self, ui_menu* menu)
{
	SetMenu(self->hwnd, menu->hmenu);
}

void ui_component_settitle(ui_component* self, const char* title)
{
	SetWindowText(self->hwnd, title);
}

void ui_component_enumerate_children(ui_component* self, void* context, 
	int (*childenum)(void*, void*))
{	
	EnumCallback callback;
	
	callback.context = context;
	callback.childenum = childenum;
	EnumChildWindows (self->hwnd, ChildEnumProc, (LPARAM) &callback);
}

BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam)
{
	EnumCallback* callback = (EnumCallback*) lParam;
	ui_component* child = table_at(&selfmap, (int)hwnd);
	if (child &&  callback->childenum) {
		return callback->childenum(callback->context, child);		  
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
			ui_component* child = table_at(&selfmap, (int)hwnd);
			if (child) {				
				children = list_create(child);				
			}
		}
		while (hwnd) {
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
			if (hwnd) {
				ui_component* child = table_at(&selfmap, (int)hwnd);
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
	ui_component* child = table_at(&selfmap, (int)hwnd);
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
	int cpx = 0;
	int cpy = 0;
	int cpymax = 0;
	List* p;
	List* wrap = 0;
	ui_size size;
	ui_component* client = 0;

	size = ui_component_size(self);	
	for (p = ui_component_children(self, 0); p != 0; p = p->next) {		
		ui_component* component;
			
		component = (ui_component*)p->entry;		
		if (component->visible) {
			ui_size componentsize;			
			componentsize = ui_component_preferredsize(component, &size);
			if (component->align == UI_ALIGN_CLIENT) {
				client = component;
			} 
			if (component->align == UI_ALIGN_FILL) {
				ui_component_setposition(component,
					component->margin.left,
					component->margin.top,				
					size.width - component->margin.left - component->margin.right,
					size.height - component->margin.top - component->margin.bottom);
			} else
			if (component->align == UI_ALIGN_TOP) {				
				cpy += component->margin.top;
				ui_component_setposition(component, 
					component->margin.left, 
					cpy,
					size.width - component->margin.left - component->margin.right,
					componentsize.height);
				cpy += component->margin.bottom;
				cpy += componentsize.height;
			} else
			if (component->align == UI_ALIGN_LEFT) {
				if ((self->alignexpandmode & UI_HORIZONTALEXPAND) == UI_HORIZONTALEXPAND) {
				} else {
					int requiredcomponentwidth;

					requiredcomponentwidth = componentsize.width + component->margin.left +
						component->margin.right;				
					if (cpx + requiredcomponentwidth > size.width) {
						List* w;						
						cpx = 0;
						for (w = wrap; w != 0; w = w->next) {
							ui_component* c;
							c = (ui_component*)w->entry;
							ui_component_resize(c, ui_component_size(c).width,
								cpymax - cpy - component->margin.top -
									component->margin.bottom);
						}
						cpy = cpymax;
						list_free(wrap);						
						wrap = 0;
					}
					if (wrap) {
						list_append(wrap, component);
					} else {
						wrap = list_create(component);
					}
				}
				cpx += component->margin.left;
				ui_component_setposition(component,
					cpx,
					cpy + component->margin.top,
					componentsize.width,
					component->justify == UI_JUSTIFY_EXPAND 
					? size.height - cpy - component->margin.top - component->margin.bottom
					: componentsize.height);
				cpx += component->margin.right;
				cpx += componentsize.width;				
				if (cpymax < cpy + componentsize.height + component->margin.top + component->margin.bottom) {
					cpymax = cpy + componentsize.height + component->margin.top + component->margin.bottom;
				}
			}				
		}
	}
	if (client) {
		ui_component_setposition(client,
					client->margin.left,
					cpy + client->margin.top,
					size.width - client->margin.left - client->margin.right,
					size.height - cpy - client->margin.top - client->margin.bottom);
	}
	list_free(p);
	list_free(wrap);
	signal_emit(&self->signal_align, self, 0);
}

void onpreferredsize(ui_component* self, ui_component* sender,
	ui_size* limit, int* width, int* height)
{			
	int cpxmax = 0;	
	int cpymax = 0;		
	ui_size size;

	size = ui_component_size(self);
	if (self->alignchildren) {
		List* p;
		int cpx = 0;
		int cpy = 0;

		if ((self->alignexpandmode & UI_HORIZONTALEXPAND) == UI_HORIZONTALEXPAND) {
			size.width = 0;		
		} else {
			size.width = limit->width;
		}	
		for (p = ui_component_children(self, 0); p != 0; p = p->next) {		
			ui_component* component;
				
			component = (ui_component*)p->entry;		
			if (component->visible) {
				ui_size componentsize;			
				
				componentsize = ui_component_preferredsize(component, &size);				
				if (component->align == UI_ALIGN_TOP) {
					cpy += component->margin.top;										
					cpy += componentsize.height;
					cpy += component->margin.bottom;
					if (cpymax < cpy) {
						cpymax = cpy;
					}
					if (cpxmax < componentsize.width + component->margin.left + component->margin.right) {
						cpxmax = componentsize.width + component->margin.left + component->margin.right;
					}
				} else
				if (component->align == UI_ALIGN_LEFT) {					
					if (size.width != 0) {
						int requiredcomponentwidth;

						requiredcomponentwidth = componentsize.width + component->margin.left +
							component->margin.right;				
						if (cpx + requiredcomponentwidth > size.width) {
							cpy = cpymax;
							cpx = 0;							
						}						
					}
					cpx += component->margin.left;				
					cpx += component->margin.right;
					cpx += componentsize.width;
					if (cpxmax < cpx) {
						cpxmax = cpx;
					}
					if (cpymax < cpy + componentsize.height + component->margin.top + component->margin.bottom) {
						cpymax = cpy + componentsize.height + component->margin.top + component->margin.bottom;
					}
				}				
			}
		}
		list_free(p);
		*width = cpxmax;
		*height = cpymax;
	} else {
		*width = size.width;
		*height = size.height;
	}
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

void ui_component_setalignexpand(ui_component* self, UiExpandMode mode)
{
	self->alignexpandmode = mode;
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
	if (self->background) {
		DeleteObject(self->background);
	}
	self->background = CreateSolidBrush(color);
}

void ui_component_setcolor(ui_component* self, unsigned int color)
{
	self->color = color;
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

ui_component* ui_component_parent(ui_component* self)
{			
	return (ui_component*) table_at(&selfmap,
		(int) GetParent(self->hwnd));
}

void ui_components_setalign(List* list, UiAlignType align, const ui_margin* margin)
{
	List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setalign((ui_component*)p->entry, align);
		if (margin) {
			ui_component_setmargin((ui_component*)p->entry, margin);
		}
	}
}

void ui_components_setmargin(List* list, const ui_margin* margin)
{
	List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setmargin((ui_component*)p->entry, margin);		
	}
}

ui_size ui_component_preferredsize(ui_component* self, ui_size* limit)
{
	ui_size rv;
	
	signal_emit(&self->signal_preferredsize, self, 3, limit, &rv.width,
		&rv.height);
	return rv;	
}

TEXTMETRIC ui_component_textmetric(ui_component* self)
{			
	TEXTMETRIC tm;
	HDC hdc;		
	HFONT hPrevFont = 0;	
	
	hdc = GetDC (self->hwnd);	
    SaveDC (hdc) ;          	
	if (self->font.hfont) {
		hPrevFont = SelectObject(hdc, self->font.hfont);
	}
	GetTextMetrics (hdc, &tm);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}	
	RestoreDC (hdc, -1);	
	ReleaseDC(NULL, hdc);
	return tm;
}
