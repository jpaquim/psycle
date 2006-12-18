/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "napp.h"
#include "natoms.h"
#include "nimage.h"
#include "nsplashscreen.h"
#include "nconfig.h"
#include "nwindow.h"
#include "ntimer.h"
#include "ndockpanel.h"
#include <algorithm>


NConfig* NApp::config_ = 0;

std::vector<NRuntime*> NApp::removePipe;

using namespace std;

map<WinHandle,NWindow*> NApp::winMap;
std::vector<NWindow*> NApp::repaintWin_;
std::vector<NWindow*> NApp::popups_;
std::map<NKeyAccelerator, NObject*> NApp::keyAccelerator_;
bool NApp::popupUnmapped_ = false;
bool NApp::in_thread_ = false;
std::vector<NImgFilter*> NApp::filter;

char NApp::buffer[40];
#ifdef __unix__
KeySym NApp::mykeysym;
XComposeStatus NApp::compose;
Time NApp::lastBtnPressTime;
#endif

NWindow* NApp::modalWin_ = false;
NWindow* NApp::lastOverWin_;
NWindow* NApp::mainWin_;
int NApp::modalExitLoop_ = 0;




NApp::NApp()
{
  mainWin_ = 0;
  #ifdef __unix__
  lastBtnPressTime = 0;
  #endif
  lastOverWin_ = 0;
  xpmFilter_ = new NXPMFilter();
  filter.push_back(xpmFilter_);
  splashScreen_ = 0;
  config_ = new NConfig();
}


NApp::~NApp()
{
  delete xpmFilter_;
  delete config_;
}

// here you find all public methods

void NApp::run( )
{
  if (splashScreen_!=0) splashScreen_->setVisible(false);
  eventLoop();
}

void NApp::setMainWindow( NWindow* window )
{
  mainWin_ = window; 
}

NSystem & NApp::system( )
{
  return *NSystem::Instance();
}

// private methods

void NApp::eventLoop( )
{

  modalWin_ = 0;
     
  if (mainWin_!=0) {
       mainWin_->setVisible(true);
  }
  #ifdef __unix__
  system().flush();
  WEvent event;
  int n = 0;
  int fd = ConnectionNumber(system().dpy());
  fd_set readfds;
  timeval timeout;
  int exitLoop = 0;

  int c = 0;

  while (!exitLoop) {
    timeout.tv_sec =  0;
    timeout.tv_usec = 10;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    n = select(fd + 1, &readfds, NULL, NULL, &timeout);

    while (in_thread_) usleep(100);

    if (n == 0) {
       // select timeout
       NTimer::selectTimeOut(timeout.tv_usec);

       if (c >= 200 && repaintWin_.size()!=0) {
          c = 0;
          for (std::vector<NWindow*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
             NWindow* win = *it;
             win->graphics()->resize(win->width(),win->height());
             win->repaint(win->pane(),NRect(0,0,win->width(),win->height()));
          }
          repaintWin_.clear();
       }
       c+=10;
    }

    while (XPending(system().dpy())) {
     	XNextEvent(system().dpy(), &event);
       system().setKeyState(event.xkey.state);
       std::map<Window,NWindow*>::iterator itr;
       int winId = event.xany.window;
       if ( (itr = winMap.find(winId)) == winMap.end() )
       {
         // not my windows
       } else {
        NWindow* window = itr->second;
//        if (window->visible()) {
        exitLoop = processEvent(window, & event);
        if (window->win() != mainWin_->win()) {
          if (exitLoop==nDestroyWindow) {
            delete window;
            exitLoop = 0;
            lastOverWin_ = 0;
          } else 
          if (exitLoop==nDockWindow) {
           window->dock()->onDockWindow();
           exitLoop=0;
           lastOverWin_ = 0;
          }
  //      }
       } else {
          if (exitLoop==nDestroyWindow) {
             break;
          }
       }
     }
    }
    callRemovePipe();
  }
  #else
  
  MSG Msg;
  // The Message Loop
  
  while ( GetMessage( & Msg, NULL, 0,0) > 0 )
  {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);      
    
    // select timeout
    if (repaintWin_.size()!=0) {
      for (std::vector<NWindow*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
        NWindow* win = *it;
        win->graphics()->resize(win->width(),win->height());
        win->repaint(win->pane(),NRect(0,0,win->width(),win->height()));
      }
      repaintWin_.clear();
    }
  }
  #endif
}

void NApp::modalEventLoop(NWindow* modalWin )
{
  #ifdef __unix__
  WEvent event;
  int n;
  int fd = ConnectionNumber(system().dpy());
  fd_set readfds;
  timeval timeout;
  int exitLoop = 0;

  while (!exitLoop) {
    timeout.tv_sec =  0;
    timeout.tv_usec = 100;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    n = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (n == 0) {
       // select timeout
       if (repaintWin_.size()!=0) {
          for (std::vector<NWindow*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
             NWindow* win = *it;
             win->graphics()->resize(win->width(),win->height());
             win->repaint(win->pane(),NRect(0,0,win->width(),win->height()));
          }
          repaintWin_.clear();
       }
    }
    while (XPending(system().dpy()) && exitLoop==0) {
     XNextEvent(system().dpy(), &event);
       system().setKeyState(event.xkey.state);
       std::map<Window,NWindow*>::iterator itr;
       int winId = event.xany.window;
       if ( (itr = winMap.find(winId)) == winMap.end() )
       {
         // not my windows
       } else {
        NWindow* window = itr->second;
        window->setExitLoop(0);
//        if (window->visible()) {
        if (window == modalWin || window->isChildOf(modalWin) ||
        event.type == Expose) {
          exitLoop = processEvent(window, & event);
          if (window->exitLoop() == 1) {
            // at Modal no delete here
            exitLoop = 1;
          } else exitLoop=0;
  //      }
        }
       }
     }
     NApp::callRemovePipe();
  }
  #else

  NWindow* oldModal = modalWin_;
  
  modalWin_ = modalWin;

//  static HWND hwndOwner;
//  hwndOwner = GetWindow( modalWin->win(), GW_OWNER);
//  EnableWindow( hwndOwner, FALSE );          
  
  MSG Msg;
  // The Message Loop  

  modalExitLoop_ = 0;

  while ( !modalExitLoop_ && GetMessage( & Msg, NULL, 0,0) > 0 )
  {                
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);    
    
    
    // select timeout
    if (repaintWin_.size()!=0) {
      for (std::vector<NWindow*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
        NWindow* win = *it;
        win->graphics()->resize(win->width(),win->height());
        win->repaint(win->pane(),NRect(0,0,win->width(),win->height()));       
      }
      repaintWin_.clear();
    }
    NApp::callRemovePipe();
  }  

  //  EnableWindow( hwndOwner, TRUE );
  //  SetFocus( hwndOwner );

  modalWin_ = oldModal;

  #endif

}

#ifdef __unix__
#else

LRESULT CALLBACK NApp::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

  std::map< WinHandle, NWindow* >::iterator itr;  

  if ( (itr = winMap.find( hwnd )) == winMap.end() )
  {
      std::cout << "not me" << std::endl;
    // not my windows
  } else {
                               
    NWindow* window = itr->second;
    window->setExitLoop(0);

    WEvent event;
    
    event.hwnd   = hwnd;
    event.msg    = msg;
    event.wParam = wParam; 
    event.lParam = lParam;
    
    int erg = 0;
    
    
     if (window == modalWin_ || window->isChildOf(modalWin_) ||
        event.msg == WM_PAINT ) {
        if ( event.msg == WM_CLOSE ) {
           modalExitLoop_ = 1;          
        }
        erg = processEvent(window, & event);
      }
    else
      erg = processEvent(window, & event);


   if ( window->exitLoop() == nDestroyWindow ) modalExitLoop_ = 1;
        
    return erg;
  }

  return DefWindowProc( hwnd, msg, wParam, lParam);
}

#endif


int NApp::processEvent( NWindow * win, WEvent * event )
{
  int exitloop = 0;
  #ifdef __unix__
  switch (event->type) {
    case Expose:
        if (win->doubleBuffered()) win->graphics()->swap(NRect(event->xexpose.x,event->xexpose.y,event->xexpose.width,event->xexpose.height)); else
        doRepaint(win);
    break;
    break;
	case MapNotify:
		doRepaint(win);
	break;
    case ConfigureNotify:{
			while (XCheckTypedWindowEvent( NApp::system().dpy(), win->win() , ConfigureNotify, event));
       if (event->xconfigure.width - 2*event->xconfigure.border_width != win->graphics()->dblWidth() || event->xconfigure.height != win->graphics()->dblHeight()  ) {
         doRepaint(win);
       }
    }
    break;
    case ButtonPress: 
         buttonPress( win, event, event->xbutton.button );
    break;
		case SelectionNotify:
			if( event->xselection.property != None)
        win->onSelection();
		break;
    case ButtonRelease:
        win->onMousePressed(event->xbutton.x,event->xbutton.y,event->xbutton.button);
    break;
    case MotionNotify:
           while (XCheckMaskEvent(NApp::system().dpy(), ButtonMotionMask, event));

           if (lastOverWin_!=0 && win!=lastOverWin_ ) lastOverWin_->onMouseExit();
           win->onMouseOver(event->xcrossing.x,event->xcrossing.y);
           lastOverWin_ = win;
         break;
    case KeyPress:
            {
             char keys[32];
             XQueryKeymap( NApp::system().dpy(), keys );
	     
	         int sState = nsNone;    
             long keycode= XKeysymToKeycode( NApp::system().dpy(), XK_Shift_L);
             if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsShift;
             keycode = XKeysymToKeycode( NApp::system().dpy(), XK_Control_L);
             if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsCtrl;
                 
             XLookupString(&event->xkey, buffer,15, &mykeysym, &compose);
             KeySym lower_return;
             KeySym upper_return;
             XConvertCase(mykeysym, &lower_return, &upper_return );
             mykeysym = upper_return;
             
             if (buffer!=NULL) {           
                  if (mykeysym<0xF000) {
                    NObject* acellNotify = findAcceleratorNotifier(NKeyAccelerator(NApp::system().keyState(),mykeysym));
                    if (acellNotify!=0) {
                        acellNotify->onKeyAcceleratorNotify(NKeyAccelerator(NApp::system().keyState(),mykeysym));
                    }
                  }
                  win->onKeyPress(NKeyEvent(0,buffer,mykeysym,sState));
              } else win->onKeyPress(NKeyEvent(0,buffer,mykeysym,sState));
            }
          break;
        case KeyRelease:
            {
              char keys[32];
              XQueryKeymap( NApp::system().dpy(), keys);            
              
             int sState = nsNone;    
             long keycode= XKeysymToKeycode( NApp::system().dpy(), XK_Shift_L);
             if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsShift;
             keycode = XKeysymToKeycode( NApp::system().dpy(), XK_Control_L);
             if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsCtrl;                           
                                                
              XLookupString(&event->xkey, buffer,15, &mykeysym, &compose);
              KeySym lower_return;
              KeySym upper_return;
              XConvertCase(mykeysym, &lower_return, &upper_return );
              mykeysym = upper_return;
              if (buffer!=NULL) {
                  if (mykeysym<0xF000) {
                  }
                  win->onKeyRelease(NKeyEvent(0,buffer,mykeysym, sState));
              } else win->onKeyRelease(NKeyEvent(0,buffer,mykeysym, sState));
            }
          break;

    case ClientMessage:
            if(event->xclient.data.l[0] == (int) NApp::system().atoms().wm_delete_window()) {
               exitloop = win->onClose();
            }
         break;

    default : ;
  }
  return exitloop;
  #else
//  system().setKeyState( NK_ShiftMask);
  HDC hdc;
  PAINTSTRUCT ps;
  switch (event->msg) {
    case WM_PAINT:
      hdc = BeginPaint( win->win(), &ps);     
        if (win->doubleBuffered()) win->graphics()->swap(NRect(0,0,win->width(),win->height())); else
        doRepaint(win);
//          win->graphics()->resize(win->width(),win->height());
//          win->repaint(win->pane(),NRect(0,0,win->width(),win->height()));    
      EndPaint( win->win(), &ps);
    break;
    case WM_SIZE:
      doRepaint(win);         
    break;
    case WM_CLOSE:
      exitloop = win->onClose();
      if ( exitloop != nDestroyWindow ) return 0; 
      DestroyWindow( event->hwnd );  
    break;
    case WM_DESTROY:   
      if ( win == mainWin_  )
        PostQuitMessage(0);
      else
      {
        if ( win->exitLoop() == nDestroyWindow ) {
            delete win;
            lastOverWin_ = 0;
        }                 
      }              
    break;
    case WM_MOUSEMOVE:
      if (lastOverWin_!=0 && win!=lastOverWin_ ) lastOverWin_->onMouseExit();
      win->onMouseOver( LOWORD( event->lParam ), HIWORD( event->lParam ) );
      lastOverWin_ = win;
    break;
    case WM_LBUTTONDOWN:
      buttonPress( win, event, 1 );         
    break;
    case WM_RBUTTONDOWN:
      buttonPress( win, event, 3 );                  
    break;
    case WM_LBUTTONDBLCLK:
      win->onMouseDoublePress( LOWORD( event->lParam ), HIWORD( event->lParam ), 1 );         
    break;     
    case WM_RBUTTONDBLCLK:
      win->onMouseDoublePress( LOWORD( event->lParam ), HIWORD( event->lParam ), 3 );   
    break;
    case WM_LBUTTONUP:
      win->onMousePressed( LOWORD( event->lParam ), HIWORD( event->lParam ), 1 );
    break;
    case WM_RBUTTONUP:
      win->onMousePressed( LOWORD( event->lParam ), HIWORD( event->lParam ), 3 );
    break;    
    case WM_KEYDOWN : {
         BYTE keyboardState[256];
         GetKeyboardState( keyboardState );
                  
         int sState = nsNone;  
         if ( ( keyboardState[ VK_SHIFT ] & 0x80 ) == 0x80 )   sState |= nsShift;
         if ( ( keyboardState[ VK_CONTROL ] & 0x80 ) == 0x80 ) sState |= nsCtrl;

         WPARAM vkey = event->wParam;
         if ( vkey != VK_SHIFT ) {
           WORD wordchar;
           int retv = ToAscii( vkey, MapVirtualKey( vkey, 0 ), keyboardState, & wordchar, 0 );
           char theChar = wordchar & 0xff;
           buffer[0] = wordchar & 0xff ;
           buffer[1] = 0;
         } else buffer[0] = '\0';

         win->onKeyPress( NKeyEvent( 0, buffer, event->wParam & 255, sState ));
    }
    break;
    case WM_SETCURSOR:
       if ( LOWORD( event->lParam ) == HTCLIENT ) {  
         NApp::system().setCursor(  NApp::system().cursor() , win );
         return TRUE;
       }
    break;
    case WM_KEYUP : {
       BYTE keyboardState[256];
       GetKeyboardState( keyboardState );
       
       int sState = nsNone;  
       if ( ( keyboardState[ VK_SHIFT ] & 0x80 ) == 0x80 )   sState |= nsShift;
       if ( ( keyboardState[ VK_CONTROL ] & 0x80 ) == 0x80 ) sState |= nsCtrl;

       int vkey = event->wParam;
       if ( vkey != VK_SHIFT ) {
       WORD wordchar;
         int retv = ToAscii( vkey, MapVirtualKey( vkey, 0 ), keyboardState, & wordchar, 0 );
         char theChar = wordchar & 0xff;
         buffer[0] = wordchar & 0xff ;
         buffer[1] = 0;
       } else buffer[0] = '\0';
       win->onKeyRelease(NKeyEvent(0,buffer, event->wParam & 255, sState ));         
    }   
    break;
    default:
      return DefWindowProc( event->hwnd, event->msg, event->wParam, event->lParam);
  }
  return DefWindowProc( event->hwnd, event->msg, event->wParam, event->lParam);
  #endif
}

void NApp::buttonPress( NWindow* win, WEvent* event, int button )
{
  bool autoUnmap_ = false;
  vector<NWindow*>::iterator itr;
  for (itr = popups_.begin(); itr < popups_.end(); itr++) {
    NWindow* popup = *itr;
    if (popup->visible() && (popup != win || ((popup == win && !mouseOverWindow()))) ) {
      popupUnmapped_ = true;
      NEvent ev(win,"ngrs_global_hide");
      popup->onMessage(&ev);
      autoUnmap_ = true;
    }
  }
  #ifdef __unix__
  Time time = event->xbutton.time;
  if ( time - lastBtnPressTime < 300 ) win->onMouseDoublePress(event->xbutton.x,event->xbutton.y,event->xbutton.button);
  win->onMousePress(event->xbutton.x,event->xbutton.y,event->xbutton.button);
  lastBtnPressTime = time;
  #else
  win->onMousePress( LOWORD( event->lParam ), HIWORD( event->lParam ), button);
  #endif
  if (autoUnmap_) {
    win->checkForRemove(0);
  }

  popupUnmapped_ = false;
}       

void NApp::addWindow( WinHandle handle, NWindow * window )
{
  winMap[handle]=window;
}

void NApp::removeWindow( WinHandle handle )
{
 std::map<WinHandle, NWindow*>::iterator itr;
 if ( (itr = winMap.find(handle)) == winMap.end())
 {
   // not my windows
 } else {
	 std::vector<NWindow*>::iterator it = find( repaintWin_.begin(), repaintWin_.end(), itr->second);
	 if ( it != repaintWin_.end() )repaintWin_.erase(it);
	 winMap.erase(itr);
 }
 
}

void NApp::doRepaint( NWindow * win )
{ 
  vector<NWindow*>::iterator it = find(repaintWin_.begin(), repaintWin_.end(), win);
  if (it == repaintWin_.end()) {
    repaintWin_.push_back(win);
  }
}

void NApp::addKeyAccelerator( const NKeyAccelerator & accelerator, NObject * notify )
{
  keyAccelerator_[accelerator] = notify;
}

NObject * NApp::findAcceleratorNotifier(const NKeyAccelerator & acc )
{
   std::map<NKeyAccelerator, NObject*>::iterator itr;
   if ( (itr = keyAccelerator_.find(acc)) == keyAccelerator_.end()) return 0; else
                                                             return itr->second;
}

NWindow * NApp::mouseOverWindow( )
{
  #ifdef __unix__
  Window root_win,child_win;
  int x_win; int y_win; unsigned int mask;
  if (  XQueryPointer(system().dpy(),system().rootWindow(),&root_win,&child_win,&x_win,&y_win,&x_win,&y_win,&mask)
  ) {
    std::map<Window,NWindow*>::iterator itr;
    if ( (itr = winMap.find(child_win)) == winMap.end() )
     return 0;
    else
     return itr->second;
  } else return 0;
  #else
  return 0;
  #endif
}

void NApp::registerPopupWindow( NWindow * win )
{
  popups_.push_back(win);
}

void NApp::unregisterPopupWindow( NWindow * win )
{
  std::vector<NWindow*>::iterator it = find(popups_.begin(), popups_.end(), win);
  if (it != popups_.end() ) {
    popups_.erase( it );
  }
}

void NApp::runModal(NWindow* modalWin )
{
  modalEventLoop(modalWin);
}

void NApp::unmapPopupWindows( )
{
  vector<NWindow*>::iterator itr;
  for (itr = popups_.begin(); itr < popups_.end(); itr++) {
    NWindow* popup = *itr;
    if (popup->visible()) {
      popupUnmapped_ = true;
      popup->setVisible(false);
			popup->checkForRemove(0);
    }
  }
}

void NApp::setSplashScreen(NSplashScreen * splashScreen )
{
  splashScreen_ = splashScreen;
  if (splashScreen_!=0) {
     splashScreen_->setVisible(true);
     NApp::system().mapWindow(splashScreen_->win());
     splashScreen_->graphics()->resize(splashScreen_->width(),splashScreen_->height());
     splashScreen_->repaint(splashScreen->pane(),NRect(0,0,splashScreen_->width(),splashScreen_->height()));
     splashScreen_->graphics()->swap(NRect(0,0,splashScreen_->width(),splashScreen_->height()));
  }
}

void NApp::flushEventQueue( )
{
  clearEventQueue();

  NApp::system().flush();
}

void NApp::clearEventQueue( )
{
  #ifdef __unix__
  WEvent event;
  int exitLoop;
  while (XPending(system().dpy())) {
     XNextEvent(system().dpy(), &event);
       system().setKeyState(event.xkey.state);
       std::map<Window,NWindow*>::iterator itr;
       int winId = event.xany.window;
       if ( (itr = winMap.find(winId)) == winMap.end() )
       {
         // not my windows
       } else {
        NWindow* window = itr->second;
//        if (window->visible()) {
        exitLoop = processEvent(window, & event);
        if (event.xany.window != mainWin_->win()) {
        if (exitLoop==nDestroyWindow) {
          delete window;
          exitLoop = 0;
        } else exitLoop=0;
  //      }
       }
     }
  }

  if (repaintWin_.size()!=0) {
          for (std::vector<NWindow*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
             NWindow* win = *it;
             win->graphics()->resize(win->width(),win->height());
             win->repaint(win->pane(),NRect(0,0,win->width(),win->height()));
          }
          repaintWin_.clear();
       }
  #endif
}


NConfig* NApp::config( )
{
  return config_;
}

void NApp::addRemovePipe( NRuntime * component )
{
  removePipe.push_back(component);
}

void NApp::callRemovePipe( )
{
 for (std::vector<NRuntime*>::iterator iter = removePipe.begin(); iter<removePipe.end(); iter++) {
  lastOverWin_ = 0;  
  NRuntime* component = *iter;
    delete component;
 } 
 removePipe.clear();
}

void NApp::enterThread( )
{
  in_thread_=true;
  clearEventQueue();
}

void NApp::leaveThread( )
{
  in_thread_=false;
}






