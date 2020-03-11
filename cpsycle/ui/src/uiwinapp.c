// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinapp.h"
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32


#include "uiwingraphicsimp.h"
#include "uiwinfontimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uiwincomponentimp.h"
#include <excpt.h>
#include <stdlib.h>
#include <commctrl.h> // common control header

int iDeltaPerLine = 120;
extern psy_Table menumap;
extern psy_ui_App app;

static void sendmessagetoparent(psy_ui_win_ComponentImp* imp, uintptr_t message, WPARAM wparam, LPARAM lparam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);

LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void psy_ui_winapp_registerclasses(psy_ui_WinApp*);

static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp->vtable->dev_platform(self->imp);
}

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
	self->eventretarget = 0;
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
	psy_ui_win_ComponentImp* imp;
	psy_ui_WinApp* winapp;
	psy_ui_fp_winproc winproc;

	winapp = (psy_ui_WinApp*) app.platform;
	imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	if (imp) {		
		winproc = imp->wndproc;		
		switch (message)
		{
			case WM_NCDESTROY:
				// restore default winproc
				if (imp->component && imp->component->signal_destroyed.slots) {
					psy_signal_emit(&imp->component->signal_destroyed, imp->component,
						0);
				}
#if defined(_WIN64)		
				SetWindowLongPtr(imp->hwnd, GWLP_WNDPROC, (LONG_PTR)
					imp->wndproc);
#else	
				SetWindowLong(imp->hwnd, GWL_WNDPROC, (LONG)imp->wndproc);
#endif				
				if (imp->component) {
					psy_ui_component_dispose(imp->component);
				}
				psy_table_remove(&winapp->selfmap, (uintptr_t) hwnd);
			break;
			case WM_DESTROY:
				if (imp->component && imp->component->signal_destroy.slots) {
					psy_signal_emit(&imp->component->signal_destroy,
						imp->component, 0);
				}								
			break;
			case WM_TIMER:								
				imp->component->vtable->ontimer(imp->component, (int) wParam);
				if (imp->component->signal_timer.slots) {
					psy_signal_emit(&imp->component->signal_timer, imp->component, 1,
						(int)wParam);
				}
			break;
			case WM_KEYDOWN:								
			{
				psy_ui_KeyEvent ev;
				
				psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					(lParam & 0x40000000) == 0x40000000);
				imp->component->vtable->onkeydown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_keydown, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE &&
						psy_table_at(&winapp->selfmap,
						(uintptr_t) GetParent (hwnd))) {
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				}				
			}
			break;
			case WM_KILLFOCUS:
				if (imp->component && imp->component->signal_focuslost.slots) {
					psy_signal_emit(&imp->component->signal_focuslost, imp->component, 0);
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
	psy_ui_win_ComponentImp* imp;
	psy_ui_Graphics	 g;
	// HMENU		 hMenu;
	// psy_ui_Menu* menu;
	// int			 menu_id;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	if (imp) {
		switch (message)
		{		
			case WM_SHOWWINDOW:							
				if (wParam == TRUE) {
					psy_signal_emit(&imp->component->signal_show, imp->component, 0);
				} else {
					psy_signal_emit(&imp->component->signal_hide, imp->component, 0);
				}
				return 0 ;				
			break;		
			case WM_SIZE:			
				{
					psy_ui_Size size;
					
					if (imp->component->alignchildren) {
						psy_ui_component_align(imp->component);
					}
					size.width = LOWORD(lParam);
					size.height = HIWORD(lParam);
					imp->component->vtable->onsize(imp->component, &size);
					psy_signal_emit(&imp->component->signal_size, imp->component, 1,
						(void*)&size);

					return 0 ;
				}			
			break;
			case WM_TIMER:				
				imp->component->vtable->ontimer(imp->component, (int) wParam);
				if (imp->component->signal_timer.slots) {
					psy_signal_emit(&imp->component->signal_timer, imp->component, 1,
						(int)wParam);
				}
				return 0;
			break;		
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORSTATIC:
			case WM_CTLCOLOREDIT:
				imp = psy_table_at(&winapp->selfmap, (uintptr_t) lParam);
				if (imp && imp->component) {					
					SetTextColor((HDC) wParam, imp->component->color);
					SetBkColor((HDC) wParam, imp->component->backgroundcolor);
					if ((imp->component->backgroundmode & BACKGROUND_SET) == BACKGROUND_SET) {
						return (intptr_t) psy_ui_win_component_details(imp->component)->background;
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
			  /*hMenu = GetMenu (hwnd) ;
			  menu_id = LOWORD (wParam);
			  menu = psy_table_at(&menumap, (uintptr_t) menu_id);
			  if (menu && menu->execute) {	
				menu->execute(menu);
			  }*/
			  imp = psy_table_at(&winapp->winidmap, (uintptr_t) LOWORD(wParam));
			  if (imp && imp->component && imp->component->signal_command.slots) {
					psy_signal_emit(&imp->component->signal_command, imp->component, 2, 
						wParam, lParam);
					return 0;
			  }
			  if (imp && imp->imp.signal_command.slots) {
				  psy_signal_emit(&imp->imp.signal_command, imp->component, 2,
					  wParam, lParam);
				  return 0;
			  }
			  return 0 ;  
			break;          
			case WM_CREATE:			
				if (imp->component->signal_create.slots) {	
					psy_signal_emit(&imp->component->signal_create, imp->component, 0);
				}
				return 0 ;
			break;
			case WM_PAINT :			
				if (imp->component->vtable->ondraw || imp->component->signal_draw.slots ||
						imp->component->backgroundmode != BACKGROUND_NONE) {
					HDC bufferDC;
					HBITMAP bufferBmp;
					HBITMAP oldBmp;
					HDC hdc;				
					RECT rect;
					HFONT hPrevFont = 0;
					HFONT hfont = 0;
					psy_ui_win_GraphicsImp* win_g = 0;

					hdc = BeginPaint (hwnd, &ps);
					GetClientRect(hwnd, &rect);
					if (imp->component->doublebuffered) {					
						bufferDC = CreateCompatibleDC(hdc);					
						bufferBmp = CreateCompatibleBitmap(hdc, rect.right,
							rect.bottom);
						oldBmp = SelectObject(bufferDC, bufferBmp);					
						psy_ui_graphics_init(&g, bufferDC);
						win_g = (psy_ui_win_GraphicsImp*) g.imp;
					} else {
						psy_ui_graphics_init(&g, hdc);
						win_g = (psy_ui_win_GraphicsImp*) g.imp;
					}
					psy_ui_setrectangle(&g.clip,
						ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top);				
					if (imp->component->backgroundmode == BACKGROUND_SET) {
						psy_ui_Rectangle r;

						psy_ui_setrectangle(&r,
						rect.left, rect.top, rect.right - rect.left,
						rect.bottom - rect.top);
						psy_ui_rectangle_union(&r, &g.clip);
						psy_ui_drawsolidrectangle(&g, r, imp->component->backgroundcolor);
					}					
					hfont = ((psy_ui_win_FontImp*)
						psy_ui_component_font(imp->component)->imp)->hfont;
					hPrevFont = SelectObject(win_g->hdc, hfont);					
					if (imp->component->vtable->ondraw) {
						imp->component->vtable->ondraw(imp->component, &g);
					}
					psy_signal_emit(&imp->component->signal_draw, imp->component, 1, &g);
					if (hPrevFont) {
						SelectObject(win_g->hdc, hPrevFont);
					}
					if (imp->component->doublebuffered) {
						win_g->hdc = hdc;
						BitBlt(hdc, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
							bufferDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);				
						SelectObject(bufferDC, oldBmp);
						DeleteObject(bufferBmp);
						DeleteDC(bufferDC);					
					}
					psy_ui_graphics_dispose(&g);
					EndPaint(hwnd, &ps);
					return 0 ;
				}
			break;
			case WM_NCDESTROY:
				if (imp->component && imp->component->signal_destroyed.slots) {
					psy_signal_emit(&imp->component->signal_destroyed, imp->component, 0);
				}
				psy_ui_component_dispose(imp->component);
				psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
				return 0;
			break;
			case WM_DESTROY:
				if (imp->component && imp->component->signal_destroy.slots) {
					psy_signal_emit(&imp->component->signal_destroy, imp->component, 0);
				}
				return 0;
			break;
			case WM_SYSKEYDOWN:
				if (wParam >= VK_F10 && wParam <= VK_F12) {					
					if (imp->component->signal_keydown.slots) {
						psy_ui_KeyEvent ev;
						
						psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
							GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
							(lParam & 0x40000000) == 0x40000000);
						psy_signal_emit(&imp->component->signal_keydown, imp->component, 1,
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
				imp->component->vtable->onkeydown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_keydown, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
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
				imp->component->vtable->onkeyup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_keyup, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_LBUTTONUP:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_LBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmouseup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_RBUTTONUP:							
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmouseup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_MBUTTONUP:			
			{			
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_MBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmouseup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0 ;
			}
			break;
			case WM_LBUTTONDOWN:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_LBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmousedown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_RBUTTONDOWN:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				psy_ui_mouseevent_settarget(&ev, winapp->eventretarget);
				imp->component->vtable->onmousedown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_MBUTTONDOWN:			
			{		
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_MBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmousedown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_LBUTTONDBLCLK:
			{
				psy_ui_MouseEvent ev;
				
				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam),
					(SHORT)HIWORD (lParam), MK_LBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
					&ev);				
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_MBUTTONDBLCLK:
			{
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_MBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0 ;
			}
			break;		
			case WM_RBUTTONDBLCLK:
			{				
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
					&ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
			}
			break;
			case WM_MOUSEMOVE:
				if (!imp->component->mousetracking) {
					TRACKMOUSEEVENT tme;
					
					imp->component->vtable->onmouseenter(imp->component);
					psy_signal_emit(&imp->component->signal_mouseenter, imp->component, 0);					
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 200;
					tme.hwndTrack = hwnd;
					if (_TrackMouseEvent(&tme)) {
						imp->component->mousetracking = 1;
					} 
					return 0;
				}								
				{
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), wParam, 0,
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
					imp->component->vtable->onmousemove(imp->component, &ev);
					psy_signal_emit(&imp->component->signal_mousemove, imp->component, 1,
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
			{
				int preventdefault = 0;
				if (imp->component->signal_mousewheel.slots) {
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init(&ev, (SHORT)LOWORD(lParam),
						(SHORT)HIWORD(lParam), LOWORD(wParam), (short)HIWORD(wParam),
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
					psy_signal_emit(&imp->component->signal_mousewheel, imp->component, 1,
						&ev);
					preventdefault = ev.preventdefault;
				}
				if (!preventdefault && imp->component->wheelscroll > 0) {
					if (iDeltaPerLine != 0) {
						imp->component->accumwheeldelta += (short)HIWORD(wParam); // 120 or -120
						while (imp->component->accumwheeldelta >= iDeltaPerLine)
						{
							int iPos;
							int scrollmin;
							int scrollmax;

							psy_ui_component_verticalscrollrange(imp->component, &scrollmin,
								&scrollmax);
							iPos = psy_ui_component_verticalscrollposition(imp->component) -
								imp->component->wheelscroll;
							if (iPos < scrollmin) {
								iPos = scrollmin;
							}
							SendMessage((HWND)psy_ui_win_component_details(imp->component)->hwnd,
								WM_VSCROLL,
								MAKELONG(SB_THUMBTRACK, iPos), 0);
							imp->component->accumwheeldelta -= iDeltaPerLine;
						}
						while (imp->component->accumwheeldelta <= -iDeltaPerLine)
						{
							int iPos;
							int scrollmin;
							int scrollmax;

							psy_ui_component_verticalscrollrange(imp->component, &scrollmin,
								&scrollmax);
							iPos = psy_ui_component_verticalscrollposition(imp->component) +
								imp->component->wheelscroll;
							if (iPos > scrollmax) {
								iPos = scrollmax;
							}
							SendMessage((HWND)psy_ui_win_component_details(imp->component)->hwnd, WM_VSCROLL,
								MAKELONG(SB_THUMBTRACK, iPos), 0);
							imp->component->accumwheeldelta += iDeltaPerLine;
						}
					}
				}
			}
			break;
			case WM_MOUSEHOVER:			
				if (imp->component->signal_mousehover.slots) {	                    
					psy_signal_emit(&imp->component->signal_mousehover, imp->component, 0);
					return 0;
				}
			break;
			case WM_MOUSELEAVE:
			{
				imp->component->mousetracking = 0;
				imp->component->vtable->onmouseleave(imp->component);
				psy_signal_emit(&imp->component->signal_mouseleave, imp->component, 0);
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
				if (imp->component->signal_focuslost.slots) {
					psy_signal_emit(&imp->component->signal_focuslost, imp->component, 0);
					return 0;
				}
			break;
			default:			
			break;
		}	
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void sendmessagetoparent(psy_ui_win_ComponentImp* imp, uintptr_t message, WPARAM wparam, LPARAM lparam)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;	
	if (psy_table_at(&winapp->selfmap,
			(uintptr_t)GetParent(imp->hwnd))) {
		winapp->eventretarget = imp->component;
		SendMessage(GetParent(imp->hwnd), message, wparam, lparam);
	}
	winapp->eventretarget = 0;
}

void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; //, iHorzPos;	
	psy_ui_win_ComponentImp* imp;
     
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
		imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		if (imp->component && imp->component->signal_scroll.slots) {
			psy_signal_emit(&imp->component->signal_scroll, imp->component, 2, 
				0, (iPos - si.nPos));			
		}
		if (imp->component->handlevscroll) {
			psy_ui_component_scrollstep(imp->component, 0, (iPos - si.nPos));
		}
	}
}
void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; 
	psy_ui_win_ComponentImp* imp;
     
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
		imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		if (imp->component && imp->component->signal_scroll.slots) {
			psy_signal_emit(&imp->component->signal_scroll, imp->component, 2, 
				(iPos - si.nPos), 0);			
		}
		if (imp->component->handlehscroll) {
			psy_ui_component_scrollstep(imp->component, (iPos - si.nPos), 0);
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

#endif
