// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// target win98 or nt 4 or later systems
#define _WIN32_WINNT 0x400

#include "uiwinapp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include <windows.h>
#include <excpt.h>
#include <stdlib.h>
#include <commctrl.h> // common control header

int iDeltaPerLine = 120;
static int mousetracking = 0;
extern psy_Table menumap;
extern psy_ui_App app;

static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);

LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void psy_ui_winapp_registerclasses(psy_ui_WinApp*);

static int FilterException(const char* msg, int code, struct _EXCEPTION_POINTERS *ep) 
{	
	// char txt[512];				
	MessageBox(0, msg, "Psycle Ui Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

void psy_ui_winapp_init(psy_ui_WinApp* self, HINSTANCE instance)
{
	static TCHAR szAppClass[] = TEXT("PsycleApp");	
	static TCHAR szComponentClass[] = TEXT("PsycleComponent");

	self->instance = instance;
	self->appclass = szAppClass;
	self->componentclass = szComponentClass;
	self->winproc = ui_winproc;
	self->comwinproc = ui_com_winproc;
	self->winid = 20000;
	psy_ui_winapp_registerclasses(self);
	psy_table_init(&self->selfmap);
	psy_table_init(&self->winidmap);
	self->defaultbackgroundbrush = CreateSolidBrush(
		app.defaults.defaultbackgroundcolor);
}

void psy_ui_winapp_dispose(psy_ui_WinApp* self)
{
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);
	DeleteObject(self->defaultbackgroundbrush);
}

void psy_ui_winapp_registerclasses(psy_ui_WinApp* self)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	int succ;
		
	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = self->winproc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = (HINSTANCE) self->instance;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = self->appclass;
	if (!RegisterClass (&wndclass))
    {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      self->appclass, MB_ICONERROR) ;		
    }
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = self->winproc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (long); 
	wndclass.hInstance     = self->instance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = self->componentclass;
     
	RegisterClass (&wndclass) ;	
	// Ensure that the common control DLL is loaded.     		
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);
}

LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	psy_ui_Component*   component;
	psy_ui_WinApp* winapp;
	psy_ui_fp_winproc winproc;

	winapp = (psy_ui_WinApp*) app.platform;
	component = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);

	if (component) {		
		winproc = component->platform->wndproc;		
		switch (message)
		{
			case WM_NCDESTROY:
				// restore default winproc
				if (component->signal_destroyed.slots) {
					psy_signal_emit(&component->signal_destroyed, component,
						0);
				}
				#if defined(_WIN64)		
					SetWindowLongPtr((HWND)component->platform->hwnd, GWLP_WNDPROC,
						(LONG_PTR) component->platform->wndproc);
				#else	
					SetWindowLong((HWND)component->platform->hwnd, GWL_WNDPROC,
						(LONG)component->platform->wndproc);
				#endif				
				psy_ui_component_dispose(component);
			break;
			case WM_DESTROY:
				if (component->signal_destroy.slots) {
					psy_signal_emit(&component->signal_destroy, component,
						0);
				}								
			break;
			case WM_TIMER:				
				if (component && component->signal_timer.slots) {
					psy_signal_emit(&component->signal_timer, component, 1,
						(int) wParam);
				}
			break;
			case WM_KEYDOWN:								
			{
				psy_ui_KeyEvent ev;
				
				psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					(lParam & 0x40000000) == 0x40000000);
				component->vtable->onkeydown(component, &ev);
				psy_signal_emit(&component->signal_keydown, component, 1,
					&ev);
				if (ev.bubble != FALSE &&
						psy_table_at(&winapp->selfmap,
						(uintptr_t) GetParent (hwnd))) {
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				}				
			}
			break;
			case WM_KILLFOCUS:
				if (component->signal_focuslost.slots) {
					psy_signal_emit(&component->signal_focuslost, component, 0);
				}
			break;			
			default:
			break;
		}
		return CallWindowProc(winproc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{	
    PAINTSTRUCT  ps ;     
	psy_ui_Component*   component;
	psy_ui_Graphics	 g;
	HMENU		 hMenu;
	psy_ui_Menu* menu;
	int			 menu_id;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	component = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	if (component) {
		switch (message)
		{		
			case WM_SHOWWINDOW:							
				if (wParam == TRUE) {
					psy_signal_emit(&component->signal_show, component, 0);
				} else {
					psy_signal_emit(&component->signal_hide, component, 0);
				}
				return 0 ;				
			break;		
			case WM_SIZE:			
				{
					psy_ui_Size size;
					if (component->alignchildren) {
						psy_ui_component_align(component);
					}
					size.width = LOWORD(lParam);
					size.height = HIWORD(lParam);
					component->vtable->onsize(component, &size);
					psy_signal_emit(&component->signal_size, component, 1,
						(void*)&size);

					return 0 ;
				}			
			break;
			case WM_TIMER:			
				if (component->signal_timer.slots) {
					psy_signal_emit(&component->signal_timer, component, 1,
						(int) wParam);				
					return 0 ;
				}
			break;		
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORSTATIC:
			case WM_CTLCOLOREDIT:
				component = psy_table_at(&winapp->selfmap, (uintptr_t) lParam);
				if (component) {					
					SetTextColor((HDC) wParam, component->color);
					SetBkColor((HDC) wParam, component->backgroundcolor);
					if ((component->backgroundmode & BACKGROUND_SET) == BACKGROUND_SET) {
						return (intptr_t) component->platform->background;
					} else {
						return (intptr_t) GetStockObject(NULL_BRUSH);
					}
				} else {				
					SetTextColor((HDC) wParam, psy_ui_defaults_color(&app.defaults));
					SetBkColor((HDC) wParam,
						psy_ui_defaults_backgroundcolor(&app.defaults));
					return (intptr_t) winapp->defaultbackgroundbrush;
				}
			break;
			case WM_ERASEBKGND:
				return 1;
			break;
			case WM_COMMAND:
			  hMenu = GetMenu (hwnd) ;
			  menu_id = LOWORD (wParam);
			  menu = psy_table_at(&menumap, (uintptr_t) menu_id);
			  if (menu && menu->execute) {	
				menu->execute(menu);
			  }
			  component = psy_table_at(&winapp->winidmap, (uintptr_t) LOWORD(wParam));
			  if (component && component->signal_command.slots) {
					psy_signal_emit(&component->signal_command, component, 2, 
						wParam, lParam);
					return 0;
				}
			  return 0 ;  
			break;          
			case WM_CREATE:			
				if (component->signal_create.slots) {	
					psy_signal_emit(&component->signal_create, component, 0);
				}
				return 0 ;
			break;
			case WM_PAINT :			
				if (component->vtable->ondraw || component->signal_draw.slots ||
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
						psy_ui_graphics_init(&g, bufferDC);
					} else {
						psy_ui_graphics_init(&g, hdc);
					}
					psy_ui_setrectangle(&g.clip,
						ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top);				
					if (component->backgroundmode == BACKGROUND_SET) {
						psy_ui_Rectangle r;

						psy_ui_setrectangle(&r,
						rect.left, rect.top, rect.right - rect.left,
						rect.bottom - rect.top);				
						psy_ui_drawsolidrectangle(&g, r, component->backgroundcolor);
					}
					if (component->font.hfont) {
						hPrevFont = SelectObject(g.hdc, component->font.hfont);
					} else {
						hPrevFont = SelectObject(g.hdc,
							app.defaults.defaultfont.hfont);
					}
					if (component->vtable->ondraw) {
						component->vtable->ondraw(component, &g);
					}
					psy_signal_emit(&component->signal_draw, component, 1, &g);
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
					psy_ui_graphics_dispose(&g);
					EndPaint (hwnd, &ps) ;
					return 0 ;
				}
			break;
			case WM_NCDESTROY:
				if (component->signal_destroyed.slots) {
					psy_signal_emit(&component->signal_destroyed, component, 0);
				}
				psy_ui_component_dispose(component);
				return 0;
			break;
			case WM_DESTROY:
				if (component->signal_destroy.slots) {
					psy_signal_emit(&component->signal_destroy, component, 0);
				}
				return 0;
			break;
			case WM_SYSKEYDOWN:
				if (wParam >= VK_F10 && wParam <= VK_F12) {					
					if (component->signal_keydown.slots) {
						psy_ui_KeyEvent ev;
						
						psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
							GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
							(lParam & 0x40000000) == 0x40000000);
						psy_signal_emit(&component->signal_keydown, component, 1,
							&ev);					
						if (ev.bubble != FALSE &&
							psy_table_at(&winapp->selfmap,
							(uintptr_t) GetParent (hwnd))) {				
							SendMessage (GetParent (hwnd), message, wParam, lParam) ;
						}
					}
					return 0;
				}
			break;
			case WM_KEYDOWN:
			{
				psy_ui_KeyEvent ev;
				
				psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					(lParam & 0x40000000) == 0x40000000);
				component->vtable->onkeydown(component, &ev);
				psy_signal_emit(&component->signal_keydown, component, 1,
					&ev);
				if (ev.bubble != FALSE &&
						psy_table_at(&winapp->selfmap,
						(uintptr_t) GetParent (hwnd))) {
					SendMessage(GetParent (hwnd), message, wParam, lParam);
				}				
				return 0;
			}
			break;
			case WM_KEYUP:
			{
				psy_ui_KeyEvent ev;
								
				psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					(lParam & 0x40000000) == 0x40000000);
				component->vtable->onkeyup(component, &ev);
				psy_signal_emit(&component->signal_keyup, component, 1,
					&ev);
				if (ev.bubble != FALSE &&
						psy_table_at(&winapp->selfmap,
						(uintptr_t) GetParent (hwnd))) {
					SendMessage(GetParent (hwnd), message, wParam, lParam);
				}				
				return 0;
			}
			break;
			case WM_LBUTTONUP:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_LBUTTON, 0);
				component->vtable->onmouseup(component, &ev);
				psy_signal_emit(&component->signal_mouseup, component, 1,
					&ev);
				return 0;
			}
			break;
			case WM_RBUTTONUP:							
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_RBUTTON, 0);
				component->vtable->onmouseup(component, &ev);
				psy_signal_emit(&component->signal_mouseup, component, 1,
					&ev);
				return 0;
			}
			break;
			case WM_MBUTTONUP:			
			{			
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_MBUTTON, 0);
				component->vtable->onmouseup(component, &ev);
				psy_signal_emit(&component->signal_mouseup, component, 1,
					&ev);
				return 0 ;
			}
			break;
			case WM_LBUTTONDOWN:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_LBUTTON, 0);
				component->vtable->onmousedown(component, &ev);
				psy_signal_emit(&component->signal_mousedown, component, 1,
					&ev);
				return 0;
			}
			break;
			case WM_RBUTTONDOWN:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_RBUTTON, 0);
				component->vtable->onmousedown(component, &ev);
				psy_signal_emit(&component->signal_mousedown, component, 1,
					&ev);
				return 0;
			}
			break;
			case WM_MBUTTONDOWN:			
			{		
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_MBUTTON, 0);
				component->vtable->onmousedown(component, &ev);
				psy_signal_emit(&component->signal_mousedown, component, 1,
					&ev);
				return 0;
			}
			break;
			case WM_LBUTTONDBLCLK:
			{
				psy_ui_MouseEvent ev;
				
				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam),
					(SHORT)HIWORD (lParam), MK_LBUTTON, 0);				
				component->vtable->onmousedoubleclick(component, &ev);
				psy_signal_emit(&component->signal_mousedoubleclick, component, 1,
					&ev);				
				if (ev.bubble != FALSE &&
						psy_table_at(&winapp->selfmap,
						(uintptr_t) GetParent (hwnd))) {
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;               
				}				
				return 0;
			}
			break;
			case WM_MBUTTONDBLCLK:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_MBUTTON, 0);
				component->vtable->onmousedoubleclick(component, &ev);
				psy_signal_emit(&component->signal_mousedoubleclick, component, 1,
					&ev);
				return 0 ;
			}
			break;		
			case WM_RBUTTONDBLCLK:
			{				
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_RBUTTON, 0);
				component->vtable->onmousedoubleclick(component, &ev);
				psy_signal_emit(&component->signal_mousedoubleclick, component, 1,
					&ev);
				return 0;
			}
			break;
			case WM_MOUSEMOVE:
				if (!mousetracking) {
					TRACKMOUSEEVENT tme;
					
					component->vtable->onmouseenter(component);
					psy_signal_emit(&component->signal_mouseenter, component, 0);					
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 200;
					tme.hwndTrack = hwnd;
					if (_TrackMouseEvent(&tme)) {
						mousetracking = 1;
					} 
					return 0;
				}								
				{
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), wParam, 0);
					component->vtable->onmousemove(component, &ev);
					psy_signal_emit(&component->signal_mousemove, component, 1,
						&ev);
					return 0 ;
				}
			break;			
			case WM_SETTINGCHANGE:
			{
				static int ulScrollLines;

				SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0) ;
      
			   // ulScrollLines usually equals 3 or 0 (for no scrolling)
			   // WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
				if (ulScrollLines)
					iDeltaPerLine = WHEEL_DELTA / ulScrollLines ;
				else
					iDeltaPerLine = 0 ;
			}
			return 0 ;
			break;          
			case WM_MOUSEWHEEL:
				if (component->signal_mousewheel.slots) {
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), LOWORD(wParam), HIWORD(wParam));
					psy_signal_emit(&component->signal_mousewheel, component, 1,
						&ev);
				} else
				if (component->wheelscroll > 0) {
					if (iDeltaPerLine != 0) {
						component->accumwheeldelta += (short) HIWORD (wParam); // 120 or -120
						while (component->accumwheeldelta >= iDeltaPerLine)
						{           
							int iPos;
							int scrollmin;
							int scrollmax;

							psy_ui_component_verticalscrollrange(component, &scrollmin,
								&scrollmax);							
							iPos = psy_ui_component_verticalscrollposition(component) - 
								component->wheelscroll;
							if (iPos < scrollmin) {
								iPos = scrollmin;
							}
							SendMessage((HWND) component->platform->hwnd, 
								WM_VSCROLL,
								MAKELONG(SB_THUMBTRACK, iPos), 0);
							component->accumwheeldelta -= iDeltaPerLine ;							
						}				
						while (component->accumwheeldelta <= -iDeltaPerLine)
						{
							int iPos;
							int scrollmin;
							int scrollmax;

							psy_ui_component_verticalscrollrange(component, &scrollmin,
								&scrollmax);
							iPos = psy_ui_component_verticalscrollposition(component) + 
								component->wheelscroll;
							if (iPos > scrollmax) {
								iPos = scrollmax;
							}
							SendMessage((HWND) component->platform->hwnd, WM_VSCROLL,
								MAKELONG(SB_THUMBTRACK, iPos), 0);							
							component->accumwheeldelta += iDeltaPerLine;							
						}
					}
				}
			break;
			case WM_MOUSEHOVER:			
				if (component->signal_mousehover.slots) {	                    
					psy_signal_emit(&component->signal_mousehover, component, 0);
					return 0;
				}
			break;
			case WM_MOUSELEAVE:
			{
				mousetracking = 0;
				component->vtable->onmouseleave(component);
				psy_signal_emit(&component->signal_mouseleave, component, 0);
				return 0;
			}				
			break;
			case WM_VSCROLL:
				handle_vscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_HSCROLL:				
				handle_hscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_KILLFOCUS:
				if (component->signal_focuslost.slots) {
					psy_signal_emit(&component->signal_focuslost, component, 0);
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
	psy_ui_Component*   component;
     
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
		psy_ui_WinApp* winapp;

		winapp = (psy_ui_WinApp*) app.platform;
		component = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		if (component && component->signal_scroll.slots) {
			psy_signal_emit(&component->signal_scroll, component, 2, 
				0, (iPos - si.nPos));			
		}
		if (component->handlevscroll) {
			psy_ui_component_scrollstep(component, 0, (iPos - si.nPos));
		}
	}
}
void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; 
	psy_ui_Component*   component;
     
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
		psy_ui_WinApp* winapp;

		winapp = (psy_ui_WinApp*) app.platform;
		component = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		if (component && component->signal_scroll.slots) {
			psy_signal_emit(&component->signal_scroll, component, 2, 
				(iPos - si.nPos), 0);			
		}
		if (component->handlehscroll) {
			psy_ui_component_scrollstep(component, (iPos - si.nPos), 0);
		}
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
		   si->nPos = HIWORD(wParam);
		break ;
		default:
		break ;         
	}
}

int psy_ui_winapp_run(psy_ui_WinApp* self) 
{
	MSG msg; 

	// __try
	// {
		while (GetMessage (&msg, NULL, 0, 0))
		{
			  TranslateMessage (&msg) ;
			  DispatchMessage (&msg) ;
		}
	// }
	// __except(FilterException("app loop", GetExceptionCode(), GetExceptionInformation())) {			
	// }
    return (int) msg.wParam ;
}

void psy_ui_winapp_stop(psy_ui_WinApp* self)
{
	PostQuitMessage(0);
}
