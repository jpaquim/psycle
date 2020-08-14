// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11app.h"
#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include "X11/keysym.h"

#include "uix11graphicsimp.h"
#include "uix11fontimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uix11componentimp.h"
#include <stdlib.h>
#include <stdio.h>

// int iDeltaPerLine = 120;
//extern psy_Table menumap;
extern psy_ui_App app;

//static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
//static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
//static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);

//LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	//WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	//WPARAM wParam, LPARAM lParam);
//static void psy_ui_winapp_registerclasses(psy_ui_WinApp*);

//static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
//{
	//return (psy_ui_win_ComponentImp*)self->imp->vtable->dev_platform(self->imp);
//}

//static int FilterException(const char* msg, int code, struct _EXCEPTION_POINTERS *ep) 
//{	
	//// char txt[512];				
	//MessageBox(0, msg, "Psycle Ui Exception", MB_OK | MB_ICONERROR);
	//return EXCEPTION_EXECUTE_HANDLER;
// }

static int shapeEventBase, shapeErrorBase;

// prototypes
static int handleevent(psy_ui_X11App*, XEvent*);
static void dispose_window(psy_ui_X11App*, Window);
static psy_ui_KeyEvent translate_keyevent(XKeyEvent*);
static void sendeventtoparent(psy_ui_X11App*, psy_ui_x11_ComponentImp*,
	int mask, XEvent*);
static int timertick(psy_ui_X11App*);
	
// implementation
void psy_ui_x11app_init(psy_ui_X11App* self, void* instance)
{
	static const char szAppClass[] = "PsycleApp";	
	static const char szComponentClass[] = "PsycleComponent";
	int argc = 0;
	bool shape_extension;
	
	printf("open display\n");	
	self->dpy = XOpenDisplay(NULL);
	self->appclass = szAppClass;
	self->componentclass = szComponentClass;	
	self->winid = 20000;	
	psy_table_init(&self->selfmap);
	psy_table_init(&self->winidmap);	
	self->wmDeleteMessage = XInternAtom(self->dpy, "WM_DELETE_WINDOW", False);
	self->running = FALSE;
	shape_extension = XShapeQueryExtension (self->dpy, &shapeEventBase,
		&shapeErrorBase);
	if (!shape_extension) {
		printf("XShapeQueryExtension error\n");
	}
	self->timers = 0;
}

void psy_ui_x11app_dispose(psy_ui_X11App* self)
{	
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);
	XCloseDisplay(self->dpy);
	psy_list_deallocate(&self->timers, NULL);	
//	DeleteObject(self->defaultbackgroundbrush);
}

//void psy_ui_winapp_registerclasses(psy_ui_WinApp* self)
//{
	//WNDCLASS     wndclass ;
	//INITCOMMONCONTROLSEX icex;
	//int succ;
		
	//wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    //wndclass.lpfnWndProc   = self->winproc;
    //wndclass.cbClsExtra    = 0;
    //wndclass.cbWndExtra    = 0;
    //wndclass.hInstance     = (HINSTANCE) self->instance;
    //wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    //wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    //wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH) ;
    //wndclass.lpszMenuName  = NULL ;
    //wndclass.lpszClassName = self->appclass;
	//if (!RegisterClass (&wndclass))
    //{
		//MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      //self->appclass, MB_ICONERROR) ;		
    //}
	
	//wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	//wndclass.lpfnWndProc   = self->winproc;
	//wndclass.cbClsExtra    = 0;
	//wndclass.cbWndExtra    = sizeof (long); 
	//wndclass.hInstance     = self->instance;
	//wndclass.hIcon         = NULL;
	//wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	//wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	//wndclass.lpszMenuName  = NULL;
	//wndclass.lpszClassName = self->componentclass;
     
	//RegisterClass (&wndclass) ;	
	//// Ensure that the common control DLL is loaded.     		
	//icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    //icex.dwICC = ICC_USEREX_CLASSES;
    //succ = InitCommonControlsEx(&icex);
//}

//LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	//WPARAM wParam, LPARAM lParam)
//{
	//psy_ui_win_ComponentImp* imp;
	//psy_ui_WinApp* winapp;
	//psy_ui_fp_winproc winproc;

	//winapp = (psy_ui_WinApp*) app.platform;
	//imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	//if (imp) {		
		//winproc = imp->wndproc;		
		//switch (message)
		//{
			//case WM_NCDESTROY:
				//// restore default winproc
				//if (imp->component && imp->component->signal_destroyed.slots) {
					//psy_signal_emit(&imp->component->signal_destroyed, imp->component,
						//0);
				//}
//#if defined(_WIN64)		
				//SetWindowLongPtr(imp->hwnd, GWLP_WNDPROC, (LONG_PTR)
					//imp->wndproc);
//#else	
				//SetWindowLong(imp->hwnd, GWL_WNDPROC, (LONG)imp->wndproc);
//#endif				
				//if (imp->component) {
					//psy_ui_component_dispose(imp->component);
				//}
				//psy_table_remove(&winapp->selfmap, (uintptr_t) hwnd);
			//break;
			//case WM_DESTROY:
				//if (imp->component && imp->component->signal_destroy.slots) {
					//psy_signal_emit(&imp->component->signal_destroy,
						//imp->component, 0);
				//}								
			//break;
			//case WM_TIMER:				
				//if (imp->component && imp->component->signal_timer.slots) {
					//psy_signal_emit(&imp->component->signal_timer, imp->component, 1,
						//(int) wParam);
				//}
			//break;
			//case WM_KEYDOWN:								
			//{
				//psy_ui_KeyEvent ev;
				
				//psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					//(lParam & 0x40000000) == 0x40000000);
				//imp->component->vtable->onkeydown(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_keydown, imp->component, 1,
					//&ev);
				//if (ev.bubble != FALSE &&
						//psy_table_at(&winapp->selfmap,
						//(uintptr_t) GetParent (hwnd))) {
					//SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				//}				
			//}
			//break;
			//case WM_KILLFOCUS:
				//if (imp->component && imp->component->signal_focuslost.slots) {
					//psy_signal_emit(&imp->component->signal_focuslost, imp->component, 0);
				//}
			//break;			
			//default:
			//break;
		//}
		//return CallWindowProc(winproc, hwnd, message, wParam, lParam);
	//}
	//return DefWindowProc(hwnd, message, wParam, lParam);
//}

//LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               //WPARAM wParam, LPARAM lParam)
//{	
    //PAINTSTRUCT  ps ;     
	//psy_ui_win_ComponentImp* imp;
	//psy_ui_Graphics	 g;
	//// HMENU		 hMenu;
	//// psy_ui_Menu* menu;
	//// int			 menu_id;
	//psy_ui_WinApp* winapp;

	//winapp = (psy_ui_WinApp*) app.platform;
	//imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
	//if (imp) {
		//switch (message)
		//{		
			//case WM_SHOWWINDOW:							
				//if (wParam == TRUE) {
					//psy_signal_emit(&imp->component->signal_show, imp->component, 0);
				//} else {
					//psy_signal_emit(&imp->component->signal_hide, imp->component, 0);
				//}
				//return 0 ;				
			//break;		
			//case WM_SIZE:			
				//{
					//psy_ui_Size size;
					
					//if (imp->component->alignchildren) {
						//psy_ui_component_align(imp->component);
					//}
					//size.width = LOWORD(lParam);
					//size.height = HIWORD(lParam);
					//imp->component->vtable->onsize(imp->component, &size);
					//psy_signal_emit(&imp->component->signal_size, imp->component, 1,
						//(void*)&size);

					//return 0 ;
				//}			
			//break;
			//case WM_TIMER:			
				//if (imp->component->signal_timer.slots) {
					//psy_signal_emit(&imp->component->signal_timer, imp->component, 1,
						//(int) wParam);				
					//return 0 ;
				//}
			//break;		
			//case WM_CTLCOLORLISTBOX:
			//case WM_CTLCOLORSTATIC:
			//case WM_CTLCOLOREDIT:
				//imp = psy_table_at(&winapp->selfmap, (uintptr_t) lParam);
				//if (imp && imp->component) {					
					//SetTextColor((HDC) wParam, imp->component->color);
					//SetBkColor((HDC) wParam, imp->component->backgroundcolor);
					//if ((imp->component->backgroundmode & BACKGROUND_SET) == BACKGROUND_SET) {
						//return (intptr_t) psy_ui_win_component_details(imp->component)->background;
					//} else {
						//return (intptr_t) GetStockObject(NULL_BRUSH);
					//}
				//} else {				
					//SetTextColor((HDC) wParam, psy_ui_defaults_color(&app.defaults));
					//SetBkColor((HDC) wParam,
						//psy_ui_defaults_backgroundcolor(&app.defaults));
					//return (intptr_t) winapp->defaultbackgroundbrush;
				//}
			//break;
			//case WM_ERASEBKGND:
				//return 1;
			//break;
			//case WM_COMMAND:
			  ///*hMenu = GetMenu (hwnd) ;
			  //menu_id = LOWORD (wParam);
			  //menu = psy_table_at(&menumap, (uintptr_t) menu_id);
			  //if (menu && menu->execute) {	
				//menu->execute(menu);
			  //}*/
			  //imp = psy_table_at(&winapp->winidmap, (uintptr_t) LOWORD(wParam));
			  //if (imp && imp->component && imp->component->signal_command.slots) {
					//psy_signal_emit(&imp->component->signal_command, imp->component, 2, 
						//wParam, lParam);
					//return 0;
			  //}
			  //if (imp && imp->imp.signal_command.slots) {
				  //psy_signal_emit(&imp->imp.signal_command, imp->component, 2,
					  //wParam, lParam);
				  //return 0;
			  //}
			  //return 0 ;  
			//break;          
			//case WM_CREATE:			
				//if (imp->component->signal_create.slots) {	
					//psy_signal_emit(&imp->component->signal_create, imp->component, 0);
				//}
				//return 0 ;
			//break;
			//case WM_PAINT :			
				//if (imp->component->vtable->ondraw || imp->component->signal_draw.slots ||
						//imp->component->backgroundmode != BACKGROUND_NONE) {
					//HDC bufferDC;
					//HBITMAP bufferBmp;
					//HBITMAP oldBmp;
					//HDC hdc;				
					//RECT rect;
					//HFONT hPrevFont = 0;
					//HFONT hfont = 0;
					//psy_ui_win_GraphicsImp* win_g = 0;

					//hdc = BeginPaint (hwnd, &ps);
					//GetClientRect(hwnd, &rect);
					//if (imp->component->doublebuffered) {					
						//bufferDC = CreateCompatibleDC(hdc);					
						//bufferBmp = CreateCompatibleBitmap(hdc, rect.right,
							//rect.bottom);
						//oldBmp = SelectObject(bufferDC, bufferBmp);					
						//psy_ui_graphics_init(&g, bufferDC);
						//win_g = (psy_ui_win_GraphicsImp*) g.imp;
					//} else {
						//psy_ui_graphics_init(&g, hdc);
						//win_g = (psy_ui_win_GraphicsImp*) g.imp;
					//}
					//psy_ui_setrectangle(&g.clip,
						//ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
						//ps.rcPaint.bottom - ps.rcPaint.top);				
					//if (imp->component->backgroundmode == BACKGROUND_SET) {
						//psy_ui_Rectangle r;

						//psy_ui_setrectangle(&r,
						//rect.left, rect.top, rect.right - rect.left,
						//rect.bottom - rect.top);
						//psy_ui_rectangle_union(&r, &g.clip);
						//psy_ui_drawsolidrectangle(&g, r, imp->component->backgroundcolor);
					//}					
					//hfont = ((psy_ui_win_FontImp*)
						//psy_ui_component_font(imp->component)->imp)->hfont;
					//hPrevFont = SelectObject(win_g->hdc, hfont);					
					//if (imp->component->vtable->ondraw) {
						//imp->component->vtable->ondraw(imp->component, &g);
					//}
					//psy_signal_emit(&imp->component->signal_draw, imp->component, 1, &g);
					//if (hPrevFont) {
						//SelectObject(win_g->hdc, hPrevFont);
					//}
					//if (imp->component->doublebuffered) {
						//win_g->hdc = hdc;
						//BitBlt(hdc, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
							//bufferDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);				
						//SelectObject(bufferDC, oldBmp);
						//DeleteObject(bufferBmp);
						//DeleteDC(bufferDC);					
					//}
					//psy_ui_graphics_dispose(&g);
					//EndPaint(hwnd, &ps);
					//return 0 ;
				//}
			//break;
			//case WM_NCDESTROY:
				//if (imp->component && imp->component->signal_destroyed.slots) {
					//psy_signal_emit(&imp->component->signal_destroyed, imp->component, 0);
				//}
				//psy_ui_component_dispose(imp->component);
				//psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
				//return 0;
			//break;
			//case WM_DESTROY:
				//if (imp->component && imp->component->signal_destroy.slots) {
					//psy_signal_emit(&imp->component->signal_destroy, imp->component, 0);
				//}
				//return 0;
			//break;
			//case WM_SYSKEYDOWN:
				//if (wParam >= VK_F10 && wParam <= VK_F12) {					
					//if (imp->component->signal_keydown.slots) {
						//psy_ui_KeyEvent ev;
						
						//psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
							//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
							//(lParam & 0x40000000) == 0x40000000);
						//psy_signal_emit(&imp->component->signal_keydown, imp->component, 1,
							//&ev);					
						//if (ev.bubble != FALSE &&
							//psy_table_at(&winapp->selfmap,
							//(uintptr_t) GetParent (hwnd))) {				
							//SendMessage (GetParent (hwnd), message, wParam, lParam) ;
						//}
					//}
					//return 0;
				//}
			//break;
			//case WM_KEYDOWN:
			//{
				//psy_ui_KeyEvent ev;
				
				//psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					//(lParam & 0x40000000) == 0x40000000);
				//imp->component->vtable->onkeydown(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_keydown, imp->component, 1,
					//&ev);
				//if (ev.bubble != FALSE &&
						//psy_table_at(&winapp->selfmap,
						//(uintptr_t) GetParent (hwnd))) {
					//SendMessage(GetParent (hwnd), message, wParam, lParam);
				//}				
				//return 0;
			//}
			//break;
			//case WM_KEYUP:
			//{
				//psy_ui_KeyEvent ev;
								
				//psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
					//(lParam & 0x40000000) == 0x40000000);
				//imp->component->vtable->onkeyup(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_keyup, imp->component, 1,
					//&ev);
				//if (ev.bubble != FALSE &&
						//psy_table_at(&winapp->selfmap,
							//(uintptr_t) GetParent (hwnd))) {
					//SendMessage(GetParent (hwnd), message, wParam, lParam);
				//}				
				//return 0;
			//}
			//break;
			//case WM_LBUTTONUP:
			//{
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_LBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmouseup(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					//&ev);
				//return 0;
			//}
			//break;
			//case WM_RBUTTONUP:							
			//{
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmouseup(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					//&ev);
				//return 0;
			//}
			//break;
			//case WM_MBUTTONUP:			
			//{			
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_MBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmouseup(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					//&ev);
				//return 0 ;
			//}
			//break;
			//case WM_LBUTTONDOWN:
			//{
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_LBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmousedown(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					//&ev);
				//return 0;
			//}
			//break;
			//case WM_RBUTTONDOWN:
			//{
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmousedown(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					//&ev);
				//return 0;
			//}
			//break;
			//case WM_MBUTTONDOWN:			
			//{		
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_MBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmousedown(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					//&ev);
				//return 0;
			//}
			//break;
			//case WM_LBUTTONDBLCLK:
			//{
				//psy_ui_MouseEvent ev;
				
				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam),
					//(SHORT)HIWORD (lParam), MK_LBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
					//&ev);				
				//if (ev.bubble != FALSE &&
						//psy_table_at(&winapp->selfmap,
						//(uintptr_t) GetParent (hwnd))) {
					//SendMessage (GetParent (hwnd), message, wParam, lParam) ;               
				//}				
				//return 0;
			//}
			//break;
			//case WM_MBUTTONDBLCLK:
			//{
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_MBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
					//&ev);
				//return 0 ;
			//}
			//break;		
			//case WM_RBUTTONDBLCLK:
			//{				
				//psy_ui_MouseEvent ev;

				//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
					//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				//imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				//psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
					//&ev);
				//return 0;
			//}
			//break;
			//case WM_MOUSEMOVE:
				//if (!imp->component->mousetracking) {
					//TRACKMOUSEEVENT tme;
					
					//imp->component->vtable->onmouseenter(imp->component);
					//psy_signal_emit(&imp->component->signal_mouseenter, imp->component, 0);					
					//tme.cbSize = sizeof(TRACKMOUSEEVENT);
					//tme.dwFlags = TME_LEAVE | TME_HOVER;
					//tme.dwHoverTime = 200;
					//tme.hwndTrack = hwnd;
					//if (_TrackMouseEvent(&tme)) {
						//imp->component->mousetracking = 1;
					//} 
					//return 0;
				//}								
				//{
					//psy_ui_MouseEvent ev;

					//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD (lParam), 
						//(SHORT)HIWORD (lParam), wParam, 0,
						//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
					//imp->component->vtable->onmousemove(imp->component, &ev);
					//psy_signal_emit(&imp->component->signal_mousemove, imp->component, 1,
						//&ev);
					//return 0 ;
				//}
			//break;			
			//case WM_SETTINGCHANGE:
			//{
				//static int ulScrollLines;

				//SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0) ;
      
			   //// ulScrollLines usually equals 3 or 0 (for no scrolling)
			   //// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
				//if (ulScrollLines)
					//iDeltaPerLine = WHEEL_DELTA / ulScrollLines ;
				//else
					//iDeltaPerLine = 0 ;
			//}
			//return 0 ;
			//break;          
			//case WM_MOUSEWHEEL:	
			//{
				//int preventdefault = 0;
				//if (imp->component->signal_mousewheel.slots) {
					//psy_ui_MouseEvent ev;

					//psy_ui_mouseevent_init(&ev, (SHORT)LOWORD(lParam),
						//(SHORT)HIWORD(lParam), LOWORD(wParam), (short)HIWORD(wParam),
						//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
					//psy_signal_emit(&imp->component->signal_mousewheel, imp->component, 1,
						//&ev);
					//preventdefault = ev.preventdefault;
				//}
				//if (!preventdefault && imp->component->wheelscroll > 0) {
					//if (iDeltaPerLine != 0) {
						//imp->component->accumwheeldelta += (short)HIWORD(wParam); // 120 or -120
						//while (imp->component->accumwheeldelta >= iDeltaPerLine)
						//{
							//int iPos;
							//int scrollmin;
							//int scrollmax;

							//psy_ui_component_verticalscrollrange(imp->component, &scrollmin,
								//&scrollmax);
							//iPos = psy_ui_component_verticalscrollposition(imp->component) -
								//imp->component->wheelscroll;
							//if (iPos < scrollmin) {
								//iPos = scrollmin;
							//}
							//SendMessage((HWND)psy_ui_win_component_details(imp->component)->hwnd,
								//WM_VSCROLL,
								//MAKELONG(SB_THUMBTRACK, iPos), 0);
							//imp->component->accumwheeldelta -= iDeltaPerLine;
						//}
						//while (imp->component->accumwheeldelta <= -iDeltaPerLine)
						//{
							//int iPos;
							//int scrollmin;
							//int scrollmax;

							//psy_ui_component_verticalscrollrange(imp->component, &scrollmin,
								//&scrollmax);
							//iPos = psy_ui_component_verticalscrollposition(imp->component) +
								//imp->component->wheelscroll;
							//if (iPos > scrollmax) {
								//iPos = scrollmax;
							//}
							//SendMessage((HWND)psy_ui_win_component_details(imp->component)->hwnd, WM_VSCROLL,
								//MAKELONG(SB_THUMBTRACK, iPos), 0);
							//imp->component->accumwheeldelta += iDeltaPerLine;
						//}
					//}
				//}
			//}
			//break;
			//case WM_MOUSEHOVER:			
				//if (imp->component->signal_mousehover.slots) {	                    
					//psy_signal_emit(&imp->component->signal_mousehover, imp->component, 0);
					//return 0;
				//}
			//break;
			//case WM_MOUSELEAVE:
			//{
				//imp->component->mousetracking = 0;
				//imp->component->vtable->onmouseleave(imp->component);
				//psy_signal_emit(&imp->component->signal_mouseleave, imp->component, 0);
				//return 0;
			//}				
			//break;
			//case WM_VSCROLL:
				//handle_vscroll(hwnd, wParam, lParam);
				//return 0;
			//break;
			//case WM_HSCROLL:				
				//handle_hscroll(hwnd, wParam, lParam);
				//return 0;
			//break;
			//case WM_KILLFOCUS:
				//if (imp->component->signal_focuslost.slots) {
					//psy_signal_emit(&imp->component->signal_focuslost, imp->component, 0);
					//return 0;
				//}
			//break;
			//default:			
			//break;
		//}	
	//}
	//return DefWindowProc (hwnd, message, wParam, lParam) ;
//}

//void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
//{
	//SCROLLINFO		si;	
    //int				iPos; //, iHorzPos;	
	//psy_ui_win_ComponentImp* imp;
     
	//si.cbSize = sizeof (si) ;
    //si.fMask  = SIF_ALL ;
    //GetScrollInfo (hwnd, SB_VERT, &si) ;	
	//// Save the position for comparison later on
	//iPos = si.nPos ;

	//handle_scrollparam(&si, wParam);	
	//// Set the position and then retrieve it.  Due to adjustments
	////   by Windows it may not be the same as the value set.
	//si.fMask = SIF_POS ;
	//SetScrollInfo (hwnd, SB_VERT, &si, TRUE) ;
	//GetScrollInfo (hwnd, SB_VERT, &si) ;
	//// If the position has changed, scroll the window and update it
	//if (si.nPos != iPos)
	//{
		//psy_ui_WinApp* winapp;

		//winapp = (psy_ui_WinApp*) app.platform;
		//imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		//if (imp->component && imp->component->signal_scroll.slots) {
			//psy_signal_emit(&imp->component->signal_scroll, imp->component, 2, 
				//0, (iPos - si.nPos));			
		//}
		//if (imp->component->handlevscroll) {
			//psy_ui_component_scrollstep(imp->component, 0, (iPos - si.nPos));
		//}
	//}
//}
//void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
//{
	//SCROLLINFO		si;	
    //int				iPos; 
	//psy_ui_win_ComponentImp* imp;
     
	//si.cbSize = sizeof (si) ;
    //si.fMask  = SIF_ALL ;
    //GetScrollInfo (hwnd, SB_HORZ, &si) ;	

	//// Save the position for comparison later on
	//iPos = si.nPos ;
	//handle_scrollparam(&si, wParam);
	//// Set the position and then retrieve it.  Due to adjustments
	//// by Windows it may not be the same as the value set.
	//si.fMask = SIF_POS ;
	//SetScrollInfo (hwnd, SB_HORZ, &si, TRUE) ;
	//GetScrollInfo (hwnd, SB_HORZ, &si) ;

	//// If the position has changed, scroll the window and update it

	//if (si.nPos != iPos)
	//{                    
		//psy_ui_WinApp* winapp;

		//winapp = (psy_ui_WinApp*) app.platform;
		//imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		//if (imp->component && imp->component->signal_scroll.slots) {
			//psy_signal_emit(&imp->component->signal_scroll, imp->component, 2, 
				//(iPos - si.nPos), 0);			
		//}
		//if (imp->component->handlehscroll) {
			//psy_ui_component_scrollstep(imp->component, (iPos - si.nPos), 0);
		//}
	//}
//}

//void handle_scrollparam(SCROLLINFO* si, WPARAM wParam)
//{
	//switch (LOWORD (wParam)) {
		//case SB_TOP:
		   //si->nPos = si->nMin ;
		//break ;
		//case SB_BOTTOM:
		   //si->nPos = si->nMax ;
		//break ;
		//case SB_LINEUP:
		   //si->nPos -= 1 ;
		//break ;
		//case SB_LINEDOWN:
		   //si->nPos += 1 ;
		//break ;
		//case SB_PAGEUP:
		   //si->nPos -= si->nPage ;
		//break ;
		//case SB_PAGEDOWN:
		   //si->nPos += si->nPage ;
		//break ;
		//case SB_THUMBTRACK:
		   //si->nPos = HIWORD(wParam);
		//break ;
		//default:
		//break ;         
	//}
//}

void widget_expose(Widget w, XtPointer clientdata,
  XmDrawingAreaCallbackStruct* cbs)
{
}

int psy_ui_x11app_run(psy_ui_X11App* self) 
{
	XEvent event;	
	int x11_fd;
	fd_set in_fds;
	struct timeval tv;
	
	x11_fd = ConnectionNumber(self->dpy);
	
	self->running = TRUE;
	while (self->running) {
		// create a file description set containing x11_fd
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        // set timer to 1 ms
        tv.tv_usec = 1000;
        tv.tv_sec = 0;
        
        // Wait for X Event or a Timer
        int num_ready_fds = select(x11_fd + 1, &in_fds, NULL, NULL, &tv);
        if (num_ready_fds > 0) {
            // printf("Event Received!\n");
		} else if (num_ready_fds == 0)
            // Handle timer here
            timertick(self);            
        else {
            printf("X11 select: An error occured!\n");
		}
	
		while(XPending(self->dpy)) {
            XNextEvent(self->dpy, &event);	      
			handleevent(self, &event);
		}
      //printf("%d\n", event.type);
      //printf("%d\n", StructureNotifyMask);
    }
    return 0;
}

void psy_ui_x11app_stop(psy_ui_X11App* self)
{
	self->running = FALSE;
//	PostQuitMessage(0);
}

int timertick(psy_ui_X11App* self)
{
	psy_List* p;
	
	for (p = self->timers; p != NULL; p = p->next) {
		psy_ui_X11TickCounter* counter;
		
		counter = (psy_ui_X11TickCounter*)psy_list_entry(p);
		if (counter->tick == 0) {
			psy_ui_x11_ComponentImp* imp;
	
			imp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap,
				(uintptr_t)counter->hwnd);
			if (imp && imp->component) {
				if (imp->component->signal_timer.slots) {
					psy_signal_emit(&imp->component->signal_timer,
						imp->component, 1, counter->id);
				}
			}
			counter->tick = counter->numticks;			
		}
		if (counter->tick > 0) {
			--counter->tick;
		}	
	}
}

void psy_ui_x11app_starttimer(psy_ui_X11App* self, uintptr_t hwnd, uintptr_t id,
	uintptr_t interval)
{
	psy_ui_X11TickCounter* counter;
		
	counter = (psy_ui_X11TickCounter*)malloc(sizeof(psy_ui_X11TickCounter));
	counter->hwnd = hwnd;
	counter->id = id;
	counter->numticks = interval;
	counter->tick = 0;
	psy_list_append(&self->timers, counter);
}

void psy_ui_x11app_stoptimer(psy_ui_X11App* self, uintptr_t hwnd, uintptr_t id)
{
	psy_List* p;
	
	for (p = self->timers; p != NULL; p = p->next) {
		psy_ui_X11TickCounter* counter;
		
		counter = (psy_ui_X11TickCounter*)psy_list_entry(p);
		if (counter->hwnd == hwnd && counter->id == id) {
			psy_list_remove(&self->timers, p);
			free(counter);
			break;
		}
	}
}

int handleevent(psy_ui_X11App* self, XEvent* event)
{	
	Window id;
	psy_ui_x11_ComponentImp* imp;
	psy_ui_x11_ComponentImp* rootimp;
	
	imp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap,
				(uintptr_t) event->xany.window);
	if (!imp) {
		return 0;
	}
	switch (event->type) {
		  case DestroyNotify:
			dispose_window(self, imp->hwnd);
		  break;
		  case Expose: {			
			if (event->xexpose.count != 0 ||
				!psy_ui_component_visible(imp->component)) {
				return 0;
			}			
			//GC gc;
			//PlatformXtGC xgc;
			//XGCValues gcv;
			//psy_ui_Graphics g;
			XColor dummy;
			XFontStruct *fontinfo;
			Window root;
			Window window;
			int     screen;			 
			unsigned int x = 0;
			unsigned int y = 0;
			unsigned int width = 0;
			unsigned int height = 0; 
			psy_ui_Rectangle r;				
			unsigned int temp = 0;
			XWindowAttributes win_attr;
			psy_ui_x11_GraphicsImp* gx11;
									
			window = event->xexpose.window;									
			//screen  = XDefaultScreen(self->dpy);			
            //gcv.function =   GXcopy;
            //gcv.plane_mask = AllPlanes;            
            //fontinfo = XLoadQueryFont(self->dpy,"6x10"); 
            //XSetFont(self->dpy,gc,fontinfo->fid); 
           // gc = XCreateGC(self->dpy, imp->hwnd ,
			//	GCFunction | GCPlaneMask, // | GCForeground | GCBackground,
			//	&gcv);
            //xgc.display =self->dpy;
			//xgc.window = imp->hwnd;	    
			//xgc.gc = gc;	
		//	XClearWindow(self->dpy,window);			 				
			    // XFlush(self->dpy);       
			//psy_ui_graphics_init(&g, &xgc);	
			//if (imp->component->backgroundmode ==
				//			psy_ui_BACKGROUND_SET) {
				// draw background
				
				//XGetGeometry(self->dpy, imp->hwnd, &root, &x, &y,
				//	&width, &height, &temp, &temp);					
				//	(int)event->xexpose.y,
				//	(int)(event->xexpose.y - y));
				//}
				
			XGetWindowAttributes(self->dpy, imp->hwnd, &win_attr);    
				psy_ui_setrectangle(&r,
					0,
					0,					
					win_attr.width,
					win_attr.height);
				psy_ui_drawsolidrectangle(&imp->g, r,
					psy_ui_component_backgroundcolor(imp->component));					
			//}
			// prepare a clip rect that can be used by a component to
			// optimize the draw amount
			psy_ui_setrectangle(&imp->g.clip,
				0,
				0,
				win_attr.width,
				win_attr.height);
			// prepare colors
			psy_ui_setcolor(&imp->g, psy_ui_component_color(imp->component));
			psy_ui_settextcolor(&imp->g, psy_ui_component_color(imp->component));
			psy_ui_setbackgroundmode(&imp->g, psy_ui_TRANSPARENT);
			// translate coordinates
			gx11 = (psy_ui_x11_GraphicsImp*)imp->g.imp;
			gx11->dx = -imp->component->scroll.x;
			gx11->dy = -imp->component->scroll.y;			
			if (imp->component->vtable->ondraw) {				
				imp->component->vtable->ondraw(imp->component, &imp->g);
			}
			psy_signal_emit(&imp->component->signal_draw, imp->component, 1,
				&imp->g);
			//XFlush(self->dpy);							
			//psy_ui_graphics_dispose(&g);		
			break; }
		case ConfigureNotify: {			
			XConfigureEvent xce = event->xconfigure;
			
			if (xce.width != imp->prev_w || xce.height != imp->prev_h) {
				psy_ui_Size size;
					
				imp->prev_w = xce.width;
				imp->prev_h = xce.height;
				if (imp->component->alignchildren) {
					psy_ui_component_align(imp->component);
				}
				size.width = psy_ui_value_makepx(xce.width);
				size.height = psy_ui_value_makepx(xce.height);						
							   
				imp->component->vtable->onsize(imp->component, &size);
				if (imp->component->overflow != psy_ui_OVERFLOW_HIDDEN) {
					psy_ui_component_updateoverflow(imp->component);						
				}
				psy_signal_emit(&imp->component->signal_size, imp->component, 1,
					(void*)&size);
			}
			return 0 ;			
		break; }
		case ClientMessage:
            if (event->xclient.data.l[0] == self->wmDeleteMessage) {
				XEvent e;
				uintptr_t hwnd;
                //bool close;

				//close = imp->component->vtable->onclose(imp->component);
				//if (imp->component->signal_close.slots) {
				//	psy_signal_emit(&imp->component->signal_close,
				//		imp->component, 1, (void*)&close);
				//}
				//if (!close) {
				//	return 0;
				//}
				hwnd = event->xclient.window;
				XDestroyWindow(self->dpy, event->xclient.window);
				while (TRUE) {
					XNextEvent(self->dpy, event);
					if (event->type ==  DestroyNotify) {
						dispose_window(self, event->xany.window);
						if (hwnd == event->xany.window) {
							printf("cleaned up\n");
							break;
						}
					}
				}
				self->running = FALSE;				
			}		
            break;
        case KeyPress: {
			psy_ui_KeyEvent ev;
			
			ev = translate_keyevent(&event->xkey);			
			imp->component->vtable->onkeydown(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_keydown, imp->component,
				1, &ev);
			if (ev.bubble != FALSE && imp->parent && imp->parent->hwnd) {
				XKeyEvent xkevent;

				xkevent = event->xkey;
				xkevent.window      = imp->parent->hwnd;				
				XSendEvent(self->dpy, imp->parent->hwnd, True, KeyPressMask,
					(XEvent*)&xkevent);
			}			
			return 0;
			break; }
        case KeyRelease: {
			psy_ui_KeyEvent ev;
							
			ev = translate_keyevent(&event->xkey);		
			//(int)wParam, lParam, 
			//	GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
			//	(lParam & 0x40000000) == 0x40000000);
			imp->component->vtable->onkeyup(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_keyup, imp->component,
				1, &ev);
			if (ev.bubble != FALSE && imp->parent && imp->parent->hwnd) {
				XKeyEvent xkevent;

				xkevent = event->xkey;
				xkevent.window      = imp->parent->hwnd;				
				XSendEvent(self->dpy, imp->parent->hwnd, True, KeyReleaseMask,
					(XEvent*)&xkevent);
			}
			return 0;
			break; }
        case ButtonPress: {
			psy_ui_MouseEvent ev;
			XButtonEvent xbe;
			
			xbe = event->xbutton;			
			psy_ui_mouseevent_init(&ev,
				xbe.x,
				xbe.y,
				xbe.button,
				0,
				0,
				0);
				//(SHORT)LOWORD (lParam), 
				//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);			
			imp->component->vtable->onmousedown(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1,
					&ev);
			return 0;
			break; }
		case ButtonRelease: {
			psy_ui_MouseEvent ev;
			XButtonEvent xbe;
			
			xbe = event->xbutton;			
			psy_ui_mouseevent_init(&ev,
				xbe.x,
				xbe.y,
				xbe.button,
				0,
				0,
				0);
				//(SHORT)LOWORD (lParam), 
				//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);			
			imp->component->vtable->onmouseup(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					&ev);
			return 0;
			break; }
		case MotionNotify: {
			psy_ui_MouseEvent ev;
			XMotionEvent xme;
			
			xme = event->xmotion;
			psy_ui_mouseevent_init(&ev,
				xme.x,
				xme.y,
				0, // button
				0,
				0,
				0);
				//(SHORT)LOWORD (lParam), 
				//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);			
			imp->component->vtable->onmousemove(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_mousemove, imp->component, 1,
					&ev);
			return 0;
			break; }				
		  default:
			break;
	  }
	return 0;
}

void dispose_window(psy_ui_X11App* self, Window window)
{
	psy_ui_x11_ComponentImp* imp;
	
	printf("dispose: %u\n", (unsigned int)window);	
	imp = (psy_ui_x11_ComponentImp*)psy_table_at(
		&self->selfmap, (uintptr_t) window);
	if (imp) {	
		if (imp->component) {
			psy_ui_component_dispose(imp->component);
		} else {
			imp->imp.vtable->dev_dispose(&imp->imp);
		}
		psy_table_remove(&self->selfmap, (uintptr_t)imp->hwnd);		
	}
}

psy_ui_KeyEvent translate_keyevent(XKeyEvent* event)
{
	psy_ui_KeyEvent rv;
	KeySym keysym = NoSymbol;
	int repeat = 0;					
	static unsigned char buf[64];
	static unsigned char bufnomod[2];
	int ret;
	XKeyEvent xkevent;
	bool shift;
	bool ctrl;
	
	xkevent = *event;	
	shift = (xkevent.state & ShiftMask) == ShiftMask;
	ctrl = (xkevent.state & ControlMask) == ControlMask;					
	ret = XLookupString(&xkevent, buf, sizeof buf, &keysym, 0);
	switch (keysym) {
		case XK_Home:
			keysym = psy_ui_KEY_HOME;
			break;
		case XK_Escape:
			keysym = psy_ui_KEY_ESCAPE;
			break;
		case XK_Return:
			keysym = psy_ui_KEY_RETURN;
			break;
		case XK_Tab:
			keysym = psy_ui_KEY_TAB;
			break;
		case XK_Prior:
			keysym = psy_ui_KEY_PRIOR;
			break;
		case XK_Next:
			keysym = psy_ui_KEY_NEXT;
			break;
		case XK_Left:
			keysym = psy_ui_KEY_LEFT;
			break;
		case XK_Up:
			keysym = psy_ui_KEY_UP;
			break;
		case XK_Right:
			keysym = psy_ui_KEY_RIGHT;
			break;
		case XK_Down:
			keysym = psy_ui_KEY_DOWN;
			break;
		case XK_Delete:
			keysym = psy_ui_KEY_DELETE;
			break;
		case XK_BackSpace:
			keysym = psy_ui_KEY_BACK;
			break;
		case XK_F1:
			keysym = psy_ui_KEY_F1;
			break;
		case XK_F2:
			keysym = psy_ui_KEY_F2;
			break;
		case XK_F3:
			keysym = psy_ui_KEY_F3;
			break;
		case XK_F4:
			keysym = psy_ui_KEY_F4;
			break;
		case XK_F5:
			keysym = psy_ui_KEY_F5;
			break;
		case XK_F6:
			keysym = psy_ui_KEY_F6;
			break;
		case XK_F7:
			keysym = psy_ui_KEY_F7;
			break;
		case XK_F8:
			keysym = psy_ui_KEY_F8;
			break;
		case XK_F9:
			keysym = psy_ui_KEY_F9;
			break;
		case XK_F10:
			keysym = psy_ui_KEY_F10;
			break;
		case XK_F11:
			keysym = psy_ui_KEY_F11;
			break;
		case XK_F12:
			keysym = psy_ui_KEY_F12;
			break;
		default:
			if (ret && buf[0] != '\0') {
				if (buf[0] >= 'A' && buf[0] <= 'Z') {					
					keysym = psy_ui_KEY_A +
						buf[0] - 'A';
				} else if (buf[0] >= 'a' && buf[0] <= 'z') {
					keysym = psy_ui_KEY_A +
						buf[0] - 'a';
				} else if (buf[0] >= '0' && buf[0] <= '9') {
					keysym = psy_ui_KEY_DIGIT0 +
						buf[0] - '0';
				} else {
					keysym = psy_ui_KEY_A; //buf[0];
				}
			}
			break;		
	}
	// if (ret && buf[0] != '\0') {
	// 	keysym = buf[0];
	// 	printf("%d,%d\n", ret, (int)buf[0]);
	// } else {
	// 	printf("no lookup %d\n", keysym);
	// }
	psy_ui_keyevent_init(&rv,
		keysym,
		0,
		shift,
		ctrl,
		repeat);
	return rv;
}

void sendeventtoparent(psy_ui_X11App* self, psy_ui_x11_ComponentImp* imp,
	int mask,
	XEvent* xev)
{	
	if (xev && psy_table_at(&self->selfmap,
			(uintptr_t)imp->parent->hwnd)) {
		//XEvent event;
		
		//event = *xev;
		//event.xany.window = imp->parent->hwnd;
		// psy_list_append(&winapp->targetids, imp->hwnd);
		// winapp->eventretarget = imp->component;
		//xev->xany.window = imp->parent->hwnd;
		XSendEvent(self->dpy, imp->parent->hwnd, True, mask,
			xev);
	} else {
		//psy_list_free(winapp->targetids);
		//winapp->targetids = NULL;
	}
	//winapp->eventretarget = 0;
}

#endif
