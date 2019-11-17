// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponent.h"
#include "uimenu.h"
#include "hashtbl.h"
#include <memory.h>
#include <commctrl.h>   // includes the common control header
#include <stdio.h>
#include <shlobj.h>
#include <portable.h>

HINSTANCE appInstance = 0;
HWND appMainComponentHandle = 0;

TCHAR szAppClass[] = TEXT("PsycleApp");
static TCHAR szComponentClass[] = TEXT("PsycleComponent");
static uintptr_t winid = 20000;
static Table selfmap;
static Table winidmap;
static ui_font defaultfont;
static int defaultbackgroundcolor = 0x00232323;
static int defaultcolor = 0x00D1C5B6;
static HBRUSH defaultbackgroundbrush;
static int tracking = 0;

extern Table menumap;

static LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam);
BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);
static void enableinput(ui_component* self, int enable, int recursive);
static void onpreferredsize(ui_component*, ui_component* sender, 
	ui_size* limit, ui_size* rv);

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
		List* q;

		if (main->font.hfont == defaultfont.hfont) {
			ui_component_setfont(main, font);
		}
		for (p = q = ui_component_children(main, 1); p != 0; p = p->next) {
			ui_component* child;

			child = (ui_component*)p->entry;
			if (child->font.hfont == defaultfont.hfont) {
				ui_component_setfont(child, font);
				ui_component_align(child);
			}		
		}		
		list_free(q);
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
		hInstance = (HINSTANCE) GetWindowLongPtr(
			(HWND)parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong(
			(HWND)parent->hwnd, GWL_HINSTANCE);
#endif
	} else {
		hInstance = appInstance;
	}
	self->hwnd = (uintptr_t) CreateWindow(
		classname,
		NULL,		
		dwStyle,
		x, y, width, height,
		parent ? (HWND) parent->hwnd : NULL,
		usecommand ? (HMENU)winid : NULL,
		hInstance,
		NULL);	
	if ((HWND) self->hwnd == NULL) {
        MessageBox(NULL, "Failed To Create Component", "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		table_insert(&selfmap, (uintptr_t) self->hwnd, self);
	}
	if (err == 0 && usecommand) {
		table_insert(&winidmap, winid, self);
		winid++;		
	}
	ui_component_init_base(self);		
#if defined(_WIN64)		
		self->wndproc = (winproc)GetWindowLongPtr((HWND)self->hwnd, GWLP_WNDPROC);
#else		
		self->wndproc = (winproc)GetWindowLong((HWND)self->hwnd, GWL_WNDPROC);
#endif
	if (classname != szComponentClass && classname != szAppClass) {
#if defined(_WIN64)		
		SetWindowLongPtr((HWND)self->hwnd, GWLP_WNDPROC, (LONG_PTR) ui_com_winproc);
#else	
		SetWindowLong((HWND)self->hwnd, GWL_WNDPROC, (LONG)ui_com_winproc);
#endif
	}
	if (!parent) {
		appMainComponentHandle = (HWND) self->hwnd;
	}
	return err;
}

void ui_component_init(ui_component* component, ui_component* parent)
{		
	ui_win32_component_init(component, parent, szComponentClass,
		0, 0, 90, 90, WS_CHILDWINDOW | WS_VISIBLE, 0);
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
	signal_init(&component->signal_focuslost);
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
	self->backgroundmode = BACKGROUND_SET;
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
	signal_dispose(&component->signal_focuslost);
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

void ui_component_destroy(ui_component* self)
{
	DestroyWindow((HWND)self->hwnd);
}

LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	ui_component*   component;	

	component = table_at(&selfmap, (uintptr_t) hwnd);
	if (component) {		
		switch (message)
		{
			case WM_DESTROY:
				if (component->signal_destroy.slots) {
					signal_emit(&component->signal_destroy, component, 0);
				}
				ui_component_dispose(component);
			break;
			case WM_TIMER:				
				if (component && component->signal_timer.slots) {
					signal_emit(&component->signal_timer, component, 1, (int) wParam);					
				}
			case WM_KEYDOWN:				
				if (component->signal_keydown.slots) {
					KeyEvent keyevent;

					keyevent_init(&keyevent, (int)wParam, lParam);
					signal_emit(&component->signal_keydown, component, 1, &keyevent);
				}				
			break;
			case WM_KILLFOCUS:
				if (component->signal_focuslost.slots) {
					signal_emit(&component->signal_focuslost, component, 0);					
				}
			break;
		break;		
			default:
			break;
		}
		return CallWindowProc(component->wndproc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
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

	component = table_at(&selfmap, (uintptr_t) hwnd);	
	if (component && component->signal_windowproc.slots) {				
		signal_emit(&component->signal_windowproc, component, 3, (LONG)message, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam));
		if (component->preventdefault) {					
			return 0;
		} else {
			return DefWindowProc (hwnd, message, wParam, lParam);
		}
	}

	if (component) {
		switch (message)
		{		
			case WM_SHOWWINDOW:							
				if (wParam == TRUE) {
					signal_emit(&component->signal_show, component, 0);
				} else {
					signal_emit(&component->signal_hide, component, 0);
				}
				return 0 ;				
			break;		
			case WM_SIZE:			
				{
					ui_size size;
					if (component->alignchildren == 1) {
						ui_component_align(component);
					}
					size.width = LOWORD(lParam);
					size.height = HIWORD(lParam);
					signal_emit(&component->signal_size, component, 1, (void*)&size);
					return 0 ;
				}			
			break;
			case WM_TIMER:			
				if (component->signal_timer.slots) {
					signal_emit(&component->signal_timer, component, 1, (int) wParam);				
					return 0 ;
				}
			break;		
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORSTATIC:	
				component = table_at(&selfmap, (uintptr_t) lParam);
				if (component) {					
					SetTextColor((HDC) wParam, component->color);
					SetBkColor((HDC) wParam, component->backgroundcolor);
					if ((component->backgroundmode & BACKGROUND_SET) == BACKGROUND_SET) {
						return (intptr_t) component->background;
					} else {
						return (intptr_t) GetStockObject(NULL_BRUSH);
					}
				} else {				
					SetTextColor((HDC) wParam, defaultcolor);
					SetBkColor((HDC) wParam, defaultbackgroundcolor);
					return (intptr_t) defaultbackgroundbrush;
				}
			break;
			case WM_ERASEBKGND:
				return 1;
			break;
			case WM_COMMAND:
			  hMenu = GetMenu (hwnd) ;
			  menu_id = LOWORD (wParam);
			  menu = table_at(&menumap, (uintptr_t) menu_id);
			  if (menu && menu->execute) {	
				menu->execute(menu);
			  }
			  component = table_at(&winidmap, (uintptr_t) LOWORD(wParam));
			  if (component && component->signal_command.slots) {
					signal_emit(&component->signal_command, component, 2, wParam, lParam);				
					return 0;
				}
			  return 0 ;  
			break;          
			case WM_CREATE:			
				if (component->signal_create.slots) {	
					signal_emit(&component->signal_create, component, 0);
				}
				return 0 ;
			break;
			case WM_PAINT :			
				if (component->signal_draw.slots ||
						component->backgroundmode != BACKGROUND_NONE) {
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
					signal_emit(&component->signal_draw, component, 1, &g);
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
				if (component->signal_destroy.slots) {
					signal_emit(&component->signal_destroy, component, 0);
				}
				ui_component_dispose(component);							
				return 0;
			break;
			case WM_KEYDOWN:							
				component->propagateevent = component->defaultpropagation;
				if (component->signal_keydown.slots) {
					KeyEvent keyevent;

					keyevent_init(&keyevent, (int)wParam, lParam);
					signal_emit(&component->signal_keydown, component, 1, &keyevent);
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				}				
				component->propagateevent = component->defaultpropagation;
				return 0;				
			break;
			case WM_KEYUP:			
				if (component->signal_keyup.slots) {
					KeyEvent keyevent;

					keyevent_init(&keyevent, (int)wParam, lParam);
					signal_emit(&component->signal_keyup, component, 1, &keyevent);
				}
				return 0;
			break;
			case WM_LBUTTONUP:			
				if (component->signal_mouseup.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_LBUTTON);
					signal_emit(&component->signal_mouseup, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_RBUTTONUP:			
				if (component->signal_mouseup.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_RBUTTON);
					signal_emit(&component->signal_mouseup, component, 1,
						&mouseevent);
					return 0 ;
				}			
			break;
			case WM_MBUTTONUP:			
				if (component->signal_mouseup.slots) {			
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_MBUTTON);
					signal_emit(&component->signal_mouseup, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_LBUTTONDOWN:			
				if (component->signal_mousedown.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_LBUTTON);
					signal_emit(&component->signal_mousedown, component, 1,
						&mouseevent);
					return 0 ;
				}			
			break;
			case WM_RBUTTONDOWN:			
				if (component->signal_mousedown.slots) {		
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_RBUTTON);
					signal_emit(&component->signal_mousedown, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_MBUTTONDOWN:			
				if (component->signal_mousedown.slots) {		
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_MBUTTON);
					signal_emit(&component->signal_mousedown, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_LBUTTONDBLCLK:							
				component->propagateevent = component->defaultpropagation;
				if (component->signal_mousedoubleclick.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_LBUTTON);					
					signal_emit(&component->signal_mousedoubleclick, component, 1,
						&mouseevent);
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;               
				}				
				component->propagateevent = component->defaultpropagation;
				return 0;
			break;
			case WM_MBUTTONDBLCLK:			
				if (component->signal_mousedoubleclick.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_MBUTTON);
					signal_emit(&component->signal_mousedoubleclick, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;		
			case WM_RBUTTONDBLCLK:			
				if (component->signal_mousedoubleclick.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_RBUTTON);
					signal_emit(&component->signal_mousedoubleclick, component, 1,
						&mouseevent);
					return 0;
				}
			break;
			case WM_MOUSEMOVE:							
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
				if (component->signal_mousemove.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), wParam);					
					signal_emit(&component->signal_mousemove, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_MOUSEHOVER:			
				if (component->signal_mousehover.slots) {				                    
					signal_emit(&component->signal_mousehover, component, 0);
					return 0;
				}
			break;
			case WM_MOUSELEAVE:	
				tracking = 0;			
				if (component->signal_mouseleave.slots) {				                    
					signal_emit(&component->signal_mouseleave, component, 0);
					return 0;
				}			
			break; 
			case WM_VSCROLL:
				handle_vscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_HSCROLL:
				component = table_at(&selfmap, (uintptr_t) (int) lParam);
				if (component && component->signal_windowproc.slots) {				                    
					signal_emit(&component->signal_windowproc, component, 3, message, wParam, lParam);
					return DefWindowProc (hwnd, message, wParam, lParam);
				}
				handle_hscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_KILLFOCUS:
				if (component->signal_focuslost.slots) {
					signal_emit(&component->signal_focuslost, component, 0);
					return 0;
				}
			break;
			default:			
			break;
		}	
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
		component = table_at(&selfmap, (uintptr_t) hwnd);
		if (component && component->signal_scroll.slots) {
			signal_emit(&component->signal_scroll, component, 2, 
				0, component->scrollstepy * (iPos - si.nPos));			
		}
		ScrollWindow (hwnd, 0, component->scrollstepy * (iPos - si.nPos), 
                                   NULL, NULL) ;
		UpdateWindow (hwnd);
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
		component = table_at(&selfmap, (uintptr_t) hwnd);
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
	    
    GetClientRect((HWND)self->hwnd, &rect);
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

ui_rectangle ui_component_position(ui_component* self)
{   
	ui_rectangle rv;
	RECT rc;
	RECT prc;	
	HWND pHwnd;
	    	
    GetWindowRect((HWND)self->hwnd, &rc);
	pHwnd = GetParent((HWND)self->hwnd);
	GetWindowRect(pHwnd, &prc);	
	rv.left = rc.left - prc.left;
	rv.top = rc.top - prc.top;
	rv.right =  rv.left + (rc.right - rc.left);
	rv.bottom = rv.top + (rc.bottom - rc.top);
	return rv;
}

ui_size ui_component_frame_size(ui_component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetWindowRect((HWND)self->hwnd, &rect) ;
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

void ui_component_show_state(ui_component* self, int cmd)
{
	ShowWindow((HWND)self->hwnd, cmd);
	UpdateWindow((HWND)self->hwnd) ;
}

void ui_component_show(ui_component* self)
{
	self->visible = 1;
	ShowWindow((HWND)self->hwnd, SW_SHOW);
	UpdateWindow((HWND)self->hwnd) ;
}

void ui_component_hide(ui_component* self)
{
	self->visible = 0;
	ShowWindow((HWND)self->hwnd, SW_HIDE);
	UpdateWindow((HWND)self->hwnd) ;
}

void ui_component_showhorizontalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) | WS_HSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) | WS_HSCROLL);
#endif
}


void ui_component_hidehorizontalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#endif
}

void ui_component_sethorizontalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo((HWND)self->hwnd, SB_HORZ, &si, TRUE);
}

void ui_component_showverticalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) | WS_VSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) | WS_VSCROLL);
#endif
}

void ui_component_hideverticalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#endif
}

void ui_component_setverticalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = max(0, min);
	si.nMax = max(si.nMin, max);
	si.fMask = SIF_RANGE;	
	SetScrollInfo((HWND)self->hwnd, SB_VERT, &si, TRUE);
}

void ui_component_setverticalscrollposition(ui_component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo((HWND)self->hwnd, SB_VERT, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo((HWND)self->hwnd, SB_VERT, &si, TRUE);
}

void ui_component_sethorizontalscrollposition(ui_component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo((HWND)self->hwnd, SB_HORZ, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo((HWND)self->hwnd, SB_HORZ, &si, TRUE);
}

void ui_component_verticalscrollrange(ui_component* self, int* scrollmin,
	int* scrollmax)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo((HWND)self->hwnd, SB_VERT, &si);
	*scrollmin = si.nMin;
	*scrollmax = si.nMax;
}

void ui_component_move(ui_component* self, int left, int top)
{
	SetWindowPos((HWND)self->hwnd, NULL, 
	   left,
	   top,
	   0, 0, SWP_NOZORDER | SWP_NOSIZE) ;	
}

void ui_component_resize(ui_component* self, int width, int height)
{	
	SetWindowPos((HWND)self->hwnd, NULL, 
	   0,
	   0,
	   width, height, SWP_NOZORDER | SWP_NOMOVE);	
}

void ui_component_setposition(ui_component* self, int x, int y, int width, int height)
{	
	SetWindowPos((HWND)self->hwnd, 0, x, y, width, height, SWP_NOZORDER);	
}

void ui_component_setmenu(ui_component* self, ui_menu* menu)
{
	SetMenu((HWND)self->hwnd, menu->hmenu);
}

void ui_component_settitle(ui_component* self, const char* title)
{
	SetWindowText((HWND)self->hwnd, title);
}

void ui_component_enumerate_children(ui_component* self, void* context, 
	int (*childenum)(void*, void*))
{	
	EnumCallback callback;
	
	callback.context = context;
	callback.childenum = childenum;
	EnumChildWindows((HWND)self->hwnd, ChildEnumProc, (LPARAM) &callback);
}

BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam)
{
	EnumCallback* callback = (EnumCallback*) lParam;
	ui_component* child = table_at(&selfmap, (uintptr_t) hwnd);
	if (child &&  callback->childenum) {
		return callback->childenum(callback->context, child);		  
	}     
    return FALSE ;
}

List* ui_component_children(ui_component* self, int recursive)
{	
	List* children = 0;	
	if (recursive == 1) {
		EnumChildWindows ((HWND)self->hwnd, AllChildEnumProc, (LPARAM) &children);
	} else {
		uintptr_t hwnd = (uintptr_t)GetWindow((HWND)self->hwnd, GW_CHILD);
		if (hwnd) {
			ui_component* child = table_at(&selfmap, hwnd);
			if (child) {				
				children = list_create(child);				
			}
		}
		while (hwnd) {
			hwnd = (uintptr_t) GetNextWindow((HWND)hwnd, GW_HWNDNEXT);
			if (hwnd) {
				ui_component* child = table_at(&selfmap, hwnd);
				if (child) {					
					list_append(&children, child);							
				}
			}
		}
	}
	return children;
}

BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam)
{
	List** pChildren = (List**) lParam;
	ui_component* child = table_at(&selfmap, (uintptr_t) hwnd);
	if (child) {		
		list_append(pChildren, child);				
	}     
    return TRUE;
}

void ui_component_capture(ui_component* self)
{
	SetCapture((HWND)self->hwnd);
}

void ui_component_releasecapture()
{
	ReleaseCapture();
}

void ui_component_invalidate(ui_component* self)
{
	InvalidateRect((HWND)self->hwnd, NULL, FALSE);
}

void ui_component_invalidaterect(ui_component* self, const ui_rectangle* r)
{
	RECT rc;

	rc.left = r->left;
	rc.top = r->top;
	rc.right = r->right;
	rc.bottom = r->bottom;
	InvalidateRect((HWND)self->hwnd, &rc, FALSE);
}

void ui_component_update(ui_component* self)
{
	UpdateWindow((HWND)self->hwnd);
}

void ui_component_setfocus(ui_component* self)
{
	SetFocus((HWND)self->hwnd);
}

int ui_component_hasfocus(ui_component* self)
{
	return (HWND) self->hwnd == GetFocus();
}

void ui_component_setfont(ui_component* self, ui_font* font)
{
	if (font && font->hfont) {
		SendMessage((HWND)self->hwnd, WM_SETFONT, (WPARAM) font->hfont, 0);
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
	return IsWindowVisible((HWND) self->hwnd);
}

void ui_component_align(ui_component* self)
{	
	int cpx = 0;
	int cpx_r;
	int cpy = 0;
	int cpy_b = 0;
	int cpymax = 0;
	List* p;
	List* q;
	List* wrap = 0;
	ui_size size;
	ui_component* client = 0;
		
	size = ui_component_size(self);
	cpx_r = size.width;
	cpy_b = size.height;
	for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
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
					cpx_r - component->margin.left - component->margin.right,
					componentsize.height);
				cpy += component->margin.bottom;
				cpy += componentsize.height;
			} else
			if (component->align == UI_ALIGN_BOTTOM) {
				cpy_b -= component->margin.bottom;
				ui_component_setposition(component, 
					component->margin.left, 
					cpy_b -= componentsize.height,
					cpx_r - component->margin.left - component->margin.right,
					componentsize.height);
				cpy_b -= component->margin.top;
				cpy_b -= componentsize.height;

			} else
			if (component->align == UI_ALIGN_RIGHT) {
				int requiredcomponentwidth;

				requiredcomponentwidth = componentsize.width + component->margin.left +
					component->margin.right;
				cpx_r -= requiredcomponentwidth;
				ui_component_setposition(component,
					cpx_r + component->margin.left,
					cpy + component->margin.top,
					componentsize.width,										
					size.height - component->margin.top - component->margin.bottom);
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
					list_append(&wrap, component);					
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
					cpx + client->margin.left,
					cpy + client->margin.top,
					size.width - cpx - client->margin.left - client->margin.right - (size.width - cpx_r),
					size.height - cpy - (size.height - cpy_b) - client->margin.top - client->margin.bottom);
	}
	list_free(q);
	list_free(wrap);
	signal_emit(&self->signal_align, self, 0);
}

void onpreferredsize(ui_component* self, ui_component* sender,
	ui_size* limit, ui_size* rv)
{			
	if (rv) {
		int cpxmax = 0;	
		int cpymax = 0;		
		ui_size size;

		size = ui_component_size(self);
		if (self->alignchildren) {
			List* p;
			List* q;
			int cpx = 0;
			int cpy = 0;

			if ((self->alignexpandmode & UI_HORIZONTALEXPAND) == UI_HORIZONTALEXPAND) {
				size.width = 0;		
			} else {
				size.width = limit->width;
			}	
			for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
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
					if (component->align == UI_ALIGN_BOTTOM) {
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
			list_free(q);
			rv->width = cpxmax;
			rv->height = cpymax;
		} else {
			*rv = size;
		}
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
	EnableWindow((HWND) self->hwnd, enable);
	if (recursive) {
		List* p;
		List* q;
		
		for (p = q = ui_component_children(self, recursive); p != 0; p = p->next) {
			EnableWindow((HWND)((ui_component*)(p->entry))->hwnd, enable);
		}
		list_free(q);
	}
}

int ui_openfile(ui_component* self, char* szTitle, char* szFilter,
	char* szDefExtension, char* szOpenName)
{
	int rv;
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	*szOpenName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= (HWND) self->hwnd; 
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
	rv = GetOpenFileName(&ofn);
	InvalidateRect(appMainComponentHandle, 0, FALSE);
	UpdateWindow(appMainComponentHandle);
	return rv;
}

int ui_savefile(ui_component* self, char* szTitle, char* szFilter,
	char* szDefExtension, char* szFileName)
{
	int rv;
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	*szFileName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= (HWND) self->hwnd; 
	ofn.lpstrFilter= szFilter;	
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szFileName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szTitle; 
	ofn.nMaxFileTitle= _MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR)NULL; 
	ofn.Flags= OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension;
	InvalidateRect(appMainComponentHandle, 0, FALSE);
	UpdateWindow(appMainComponentHandle);
	rv = GetSaveFileName(&ofn);
	return rv;
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
	
	hdc = GetDC((HWND)self->hwnd);	
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
		(uintptr_t) GetParent((HWND)self->hwnd));
}

List* ui_components_setalign(List* list, UiAlignType align, const ui_margin* margin)
{
	List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setalign((ui_component*)p->entry, align);
		if (margin) {
			ui_component_setmargin((ui_component*)p->entry, margin);
		}
	}
	return list;
}

List* ui_components_setmargin(List* list, const ui_margin* margin)
{
	List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setmargin((ui_component*)p->entry, margin);		
	}
	return list;
}

ui_size ui_component_preferredsize(ui_component* self, ui_size* limit)
{
	ui_size rv;	
	signal_emit(&self->signal_preferredsize, self, 2, limit, &rv);	
	return rv;	
}

TEXTMETRIC ui_component_textmetric(ui_component* self)
{			
	TEXTMETRIC tm;
	HDC hdc;		
	HFONT hPrevFont = 0;	
	
	hdc = GetDC((HWND)self->hwnd);	
    SaveDC(hdc) ;          	
	if (self->font.hfont) {
		hPrevFont = SelectObject(hdc, self->font.hfont);
	}
	GetTextMetrics (hdc, &tm);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}	
	RestoreDC(hdc, -1);	
	ReleaseDC(NULL, hdc);
	return tm;
}


void ui_component_seticonressource(ui_component* self, int ressourceid)
{
#if defined(_WIN64)	
	SetClassLongPtr((HWND)self->hwnd, GCLP_HICON, 
		(intptr_t)LoadIcon(appInstance, MAKEINTRESOURCE(ressourceid)));
#else	
	SetClassLong((HWND)self->hwnd, GCL_HICON, 
		(intptr_t)LoadIcon(appInstance, MAKEINTRESOURCE(ressourceid)));
#endif
}

ui_component* ui_maincomponent(void)
{
	return table_at(&selfmap, (uintptr_t) appMainComponentHandle);
}

void ui_component_starttimer(ui_component* self, unsigned int id,
	unsigned int interval)
{
	SetTimer((HWND)self->hwnd, id, interval, 0);
}

void ui_component_stoptimer(ui_component* self, unsigned int id)
{
	KillTimer((HWND)self->hwnd, id);
}

int ui_browsefolder(ui_component* self, const char* title, char* path)
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
		bi.hwndOwner = (HWND) self->hwnd;
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
