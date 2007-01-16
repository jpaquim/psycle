/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "app.h"
#include "atoms.h"
#include "system.h"
#include "image.h"
#include "splashscreen.h"
#include "config.h"
#include "window.h"
#include "timer.h"
#include "dockpanel.h"
#include <algorithm>

namespace ngrs {

  Config* App::config_ = 0;

  std::vector<Runtime*> App::removePipe;

  using namespace std;

  map<WinHandle,Window*> App::winMap;
  std::vector<Window*> App::repaintWin_;
  std::vector<Window*> App::popups_;
  std::map<KeyAccelerator, Object*> App::keyAccelerator_;
  bool App::popupUnmapped_ = false;
  bool App::in_thread_ = false;
  std::vector<ImgFilter*> App::filter;

  char App::buffer[40];
#ifdef __unix__
  KeySym App::mykeysym;
  XComposeStatus App::compose;
  Time App::lastBtnPressTime;
#endif

  Window* App::modalWin_ = false;
  Window* App::lastOverWin_;
  Window* App::mainWin_;
  int App::modalExitLoop_ = 0;




  App::App()
  {
    mainWin_ = 0;
#ifdef __unix__
    lastBtnPressTime = 0;
#endif
    lastOverWin_ = 0;
    xpmFilter_ = new NXPMFilter();
    filter.push_back(xpmFilter_);
    splashScreen_ = 0;
    config_ = new Config();
  }


  App::~App()
  {
    delete xpmFilter_;
    delete config_;
  }

  // here you find all public methods

  void App::run( )
  {
    if (splashScreen_!=0) splashScreen_->setVisible(false);
    eventLoop();
  }

  void App::setMainWindow( Window* window )
  {
    mainWin_ = window; 
  }

  Window* App::mainWindow() {
    return mainWin_;
  }

  NSystem & App::system( )
  {
    return *NSystem::Instance();
  }

  // private methods

  void App::eventLoop( )
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
        Timer::selectTimeOut(timeout.tv_usec);

        if (c >= 200 && repaintWin_.size()!=0) {
          c = 0;
          for (std::vector<Window*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
            Window* win = *it;
            win->graphics().resize(win->width(),win->height());
            win->repaint(win->pane(),Rect(0,0,win->width(),win->height()));
          }
          repaintWin_.clear();
        }
        c+=10;
      }

      while (XPending(system().dpy())) {
        XNextEvent(system().dpy(), &event);
        system().setKeyState(event.xkey.state);
        std::map<Window,Window*>::iterator itr;
        int winId = event.xany.window;
        if ( (itr = winMap.find(winId)) == winMap.end() )
        {
          // not my windows
        } else {
          Window* window = itr->second;
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
        for (std::vector<Window*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
          Window* win = *it;
          win->graphics().resize(win->width(),win->height());
          win->repaint( win->pane(), Rect(0,0,win->width(),win->height()) );
        }
        repaintWin_.clear();
      }	
      callRemovePipe( );
    }
#endif
  }

  void App::modalEventLoop(Window* modalWin )
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
          for (std::vector<Window*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
            Window* win = *it;
            win->graphics().resize(win->width(),win->height());
            win->repaint(win->pane(),Rect(0,0,win->width(),win->height()));
          }
          repaintWin_.clear();
        }
      }
      while (XPending(system().dpy()) && exitLoop==0) {
        XNextEvent(system().dpy(), &event);
        system().setKeyState(event.xkey.state);
        std::map<Window,Window*>::iterator itr;
        int winId = event.xany.window;
        if ( (itr = winMap.find(winId)) == winMap.end() )
        {
          // not my windows
        } else {
          Window* window = itr->second;
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
      App::callRemovePipe();
    }
#else

    Window* oldModal = modalWin_;

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
        for (std::vector<Window*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
          Window* win = *it;
          win->graphics().resize( win->width(), win->height() );
          win->repaint( win->pane(), Rect( 0, 0, win->width(), win->height() ) );
        }
        repaintWin_.clear();
      }
      App::callRemovePipe( );
    }  

    //  EnableWindow( hwndOwner, TRUE );
    //  SetFocus( hwndOwner );

    modalWin_ = oldModal;

#endif

  }

#ifdef __unix__
#else

  LRESULT CALLBACK App::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    std::map< WinHandle, Window* >::iterator itr;  
    if ( (itr = winMap.find( hwnd )) == winMap.end() )
    {
      // not my windows
      return DefWindowProc( hwnd, msg, wParam, lParam);
    } else {    

      Window* window = itr->second;
      window->setExitLoop(0);

      WEvent event;

      event.hwnd   = hwnd;
      event.msg    = msg;
      event.wParam = wParam; 
      event.lParam = lParam;

      LRESULT erg = 0;


      if (window == modalWin_ || window->isChildOf(modalWin_) ||
        event.msg == WM_PAINT ) {
          if ( event.msg == WM_CLOSE ) {
            modalExitLoop_ = 1;          
          }
          erg = processEvent(window, & event);
      } else erg = processEvent( window, & event );
      if ( window->exitLoop() == nDestroyWindow ) modalExitLoop_ = 1;        
      return erg;
    }

    return DefWindowProc( hwnd, msg, wParam, lParam);
  }

#endif


#ifdef __unix__
  unsigned int App::processEvent( Window * win, WEvent * event )
#else
  LRESULT App::processEvent( Window * win, WEvent * event )
#endif
  {
    int exitloop = 0;
#ifdef __unix__
    switch (event->type) {
    case Expose:
      if (win->doubleBuffered()) 
        win->graphics().swap(Rect(event->xexpose.x,event->xexpose.y,event->xexpose.width,event->xexpose.height)); 
      else
        doRepaint(win);
      break;
    case MapNotify:
      doRepaint(win);
      break;
    case ConfigureNotify:{
      while (XCheckTypedWindowEvent( App::system().dpy(), win->win() , ConfigureNotify, event));
      if (event->xconfigure.width - 2*event->xconfigure.border_width != win->graphics().dblWidth() || event->xconfigure.height != win->graphics().dblHeight()  ) {
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
      while (XCheckMaskEvent(App::system().dpy(), ButtonMotionMask, event));

      if (lastOverWin_!=0 && win!=lastOverWin_ ) lastOverWin_->onMouseExit();
      win->onMouseOver(event->xcrossing.x,event->xcrossing.y);
      lastOverWin_ = win;
      break;
    case KeyPress:
      {
        char keys[32];
        XQueryKeymap( App::system().dpy(), keys );

        int sState = nsNone;    
        long keycode= XKeysymToKeycode( App::system().dpy(), XK_Shift_L);
        if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsShift;
        keycode = XKeysymToKeycode( App::system().dpy(), XK_Control_L);
        if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsCtrl;

        XLookupString(&event->xkey, buffer,15, &mykeysym, &compose);
        KeySym lower_return;
        KeySym upper_return;
        XConvertCase(mykeysym, &lower_return, &upper_return );
        mykeysym = upper_return;

        if (buffer!=NULL) {           
          if (mykeysym<0xF000) {
            Object* acellNotify = findAcceleratorNotifier(KeyAccelerator(App::system().keyState(),mykeysym));
            if (acellNotify!=0) {
              acellNotify->onKeyAcceleratorNotify(KeyAccelerator(App::system().keyState(),mykeysym));
            }
          }
          win->onKeyPress(KeyEvent(0,buffer,mykeysym,sState));
        } else win->onKeyPress(KeyEvent(0,buffer,mykeysym,sState));
      }
      break;
    case KeyRelease:
      {
        char keys[32];
        XQueryKeymap( App::system().dpy(), keys);            

        int sState = nsNone;    
        long keycode= XKeysymToKeycode( App::system().dpy(), XK_Shift_L);
        if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsShift;
        keycode = XKeysymToKeycode( App::system().dpy(), XK_Control_L);
        if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsCtrl;                           

        XLookupString(&event->xkey, buffer,15, &mykeysym, &compose);
        KeySym lower_return;
        KeySym upper_return;
        XConvertCase(mykeysym, &lower_return, &upper_return );
        mykeysym = upper_return;
        if (buffer!=NULL) {
          if (mykeysym<0xF000) {
          }
          win->onKeyRelease(KeyEvent(0,buffer,mykeysym, sState));
        } else win->onKeyRelease(KeyEvent(0,buffer,mykeysym, sState));
      }
      break;

    case ClientMessage:
      if(event->xclient.data.l[0] == (int) App::system().atoms().wm_delete_window()) {
        exitloop = win->onClose();
      }
      break;

    default : ;
    }
    return exitloop;
#else
    HDC hdc;
    PAINTSTRUCT ps;
    switch (event->msg) {
    case WM_PAINT:
      hdc = BeginPaint( win->win(), &ps);      
        if ( win->doubleBuffered() ) 
        win->graphics().swap( system().convertPlatformRect( ps.rcPaint ));
      else
        doRepaint(win);      
      EndPaint( win->win(), &ps);
      break;
    case WM_SIZE:
      doRepaint( win );
      break;
    case WM_ERASEBKGND:
      return 1;
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
      {           
        if (lastOverWin_!=0 && win!=lastOverWin_ ) lastOverWin_->onMouseExit();
        win->onMouseOver( static_cast<SHORT>( LOWORD( event->lParam ) ), static_cast<SHORT>( HIWORD( event->lParam ) ) );
        lastOverWin_ = win;
      }
      break;
    case WM_LBUTTONDOWN:
      SetCapture( win->win() );
      buttonPress( win, event, 1 );         
      break;
    case WM_RBUTTONDOWN:
      SetCapture( win->win() );
      buttonPress( win, event, 3 );                  
      break;
    case WM_LBUTTONDBLCLK:
      win->onMouseDoublePress( static_cast<SHORT>( LOWORD( event->lParam ) ), static_cast<SHORT>( HIWORD( event->lParam ) ), 1 );         
      break;     
    case WM_RBUTTONDBLCLK:
      win->onMouseDoublePress( LOWORD( event->lParam ), HIWORD( event->lParam ), 3 );   
      break;
    case WM_LBUTTONUP:	  
      ReleaseCapture();
      win->onMousePressed( LOWORD( event->lParam ), HIWORD( event->lParam ), 1 );     
      break;
    case WM_RBUTTONUP:
      ReleaseCapture();
      win->onMousePressed( LOWORD( event->lParam ), HIWORD( event->lParam ), 3 );      
      break;    
    case WM_KEYDOWN : {
      BYTE keyboardState[256];
      GetKeyboardState( keyboardState );

      int sState = nsNone;  
      if ( ( keyboardState[ VK_SHIFT ] & 0x80 ) == 0x80 )   sState |= nsShift;
      if ( ( keyboardState[ VK_CONTROL ] & 0x80 ) == 0x80 ) sState |= nsCtrl;

      UINT vkey = static_cast<UINT>( event->wParam );
      if ( vkey != VK_SHIFT ) {
        WORD wordchar;
        int retv = ToAscii( vkey, MapVirtualKey( vkey, 0 ), keyboardState, & wordchar, 0 );
        char theChar = wordchar & 0xff;
        buffer[0] = wordchar & 0xff ;
        buffer[1] = 0;
      } else buffer[0] = '\0';

      win->onKeyPress( KeyEvent( 0, buffer, static_cast<int>( event->wParam & 255 ), sState ));
                      }
                      break;
    case WM_SETCURSOR:
      if ( LOWORD( event->lParam ) == HTCLIENT ) {  
        App::system().setCursor(  App::system().cursor() , win );
        return TRUE;
      }
      break;
    case WM_KEYUP : {
      BYTE keyboardState[256];
      GetKeyboardState( keyboardState );

      int sState = nsNone;  
      if ( ( keyboardState[ VK_SHIFT ] & 0x80 ) == 0x80 )   sState |= nsShift;
      if ( ( keyboardState[ VK_CONTROL ] & 0x80 ) == 0x80 ) sState |= nsCtrl;

      UINT vkey = static_cast<UINT>( event->wParam );
      if ( vkey != VK_SHIFT ) {
        WORD wordchar;
        int retv = ToAscii( vkey, MapVirtualKey( vkey, 0 ), keyboardState, & wordchar, 0 );
        char theChar = wordchar & 0xff;
        buffer[0] = wordchar & 0xff ;
        buffer[1] = 0;
      } else buffer[0] = '\0';
      win->onKeyRelease(KeyEvent(0,buffer, static_cast<UINT>( event->wParam & 255 ), sState ));         
                    }   
                    break;
    default:
      return DefWindowProc( event->hwnd, event->msg, event->wParam, event->lParam);
    }
    return DefWindowProc( event->hwnd, event->msg, event->wParam, event->lParam);
#endif
  }

  void App::buttonPress( Window* win, WEvent* event, int button )
  {
    bool autoUnmap_ = false;
    vector<Window*>::iterator itr;
    for (itr = popups_.begin(); itr < popups_.end(); itr++) {
      Window* popup = *itr;
      if (popup->visible() && (popup != win || ((popup == win && !mouseOverWindow()))) ) {
        popupUnmapped_ = true;
        Event ev(win,"ngrs_global_hide");
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
    win->onMousePress( static_cast<SHORT>( LOWORD( event->lParam ) ), static_cast<SHORT>( HIWORD( event->lParam ) ), button);
#endif
    if (autoUnmap_) {
      win->checkForRemove(0);
    }

    popupUnmapped_ = false;
  }       

  void App::addWindow( WinHandle handle, Window * window )
  {
    winMap[handle]=window;
  }

  void App::removeWindow( WinHandle handle )
  {
    std::map<WinHandle, Window*>::iterator itr;
    if ( (itr = winMap.find(handle)) == winMap.end())
    {
      // not my windows
    } else {
      std::vector<Window*>::iterator it = find( repaintWin_.begin(), repaintWin_.end(), itr->second);
      if ( it != repaintWin_.end() )repaintWin_.erase(it);
      winMap.erase(itr);
    }

  }

  void App::doRepaint( Window * win )
  { 
    vector<Window*>::iterator it = find(repaintWin_.begin(), repaintWin_.end(), win);
    if (it == repaintWin_.end()) {
      repaintWin_.push_back(win);
    }
  }

  void App::addKeyAccelerator( const KeyAccelerator & accelerator, Object * notify )
  {
    keyAccelerator_[accelerator] = notify;
  }

  Object * App::findAcceleratorNotifier(const KeyAccelerator & acc )
  {
    std::map<KeyAccelerator, Object*>::iterator itr;
    if ( (itr = keyAccelerator_.find(acc)) == keyAccelerator_.end()) return 0; else
      return itr->second;
  }

  Window * App::mouseOverWindow( )
  {
#ifdef __unix__
    Window root_win,child_win;
    int x_win; int y_win; unsigned int mask;
    if (  XQueryPointer(system().dpy(),system().rootWindow(),&root_win,&child_win,&x_win,&y_win,&x_win,&y_win,&mask)
      ) {
        std::map<Window,Window*>::iterator itr;
        if ( (itr = winMap.find(child_win)) == winMap.end() )
          return 0;
        else
          return itr->second;
    } else return 0;
#else
    POINT point;
    GetCursorPos( &point );
    WinHandle childWin = WindowFromPoint(point);
    if ( childWin ) {
      std::map< WinHandle, Window* >::iterator itr;
      if ( (itr = winMap.find( childWin ) ) == winMap.end( ) )
        return 0;
      else
        return itr->second;
    }    
#endif
    return 0;
  }

  void App::registerPopupWindow( Window * win )
  {
    popups_.push_back(win);
  }

  void App::unregisterPopupWindow( Window * win )
  {
    std::vector<Window*>::iterator it = find(popups_.begin(), popups_.end(), win);
    if (it != popups_.end() ) {
      popups_.erase( it );
    }
  }

  void App::runModal(Window* modalWin )
  {
    modalEventLoop(modalWin);
  }

  void App::unmapPopupWindows( )
  {
    vector<Window*>::iterator itr;
    for (itr = popups_.begin(); itr < popups_.end(); itr++) {
      Window* popup = *itr;
      if (popup->visible()) {
        popupUnmapped_ = true;
        popup->setVisible(false);
        popup->checkForRemove(0);
      }
    }
  }

  void App::setSplashScreen(SplashScreen * splashScreen )
  {
    splashScreen_ = splashScreen;
    if (splashScreen_!=0) {
      splashScreen_->setVisible(true);
      App::system().mapWindow(splashScreen_->win());
      splashScreen_->graphics().resize(splashScreen_->width(),splashScreen_->height());
      splashScreen_->repaint(splashScreen->pane(),Rect(0,0,splashScreen_->width(),splashScreen_->height()));
      splashScreen_->graphics().swap(Rect(0,0,splashScreen_->width(),splashScreen_->height()));
    }
  }

  void App::flushEventQueue( )
  {
    clearEventQueue();

    App::system().flush();
  }

  void App::clearEventQueue( )
  {
#ifdef __unix__
    WEvent event;
    int exitLoop;
    while (XPending(system().dpy())) {
      XNextEvent(system().dpy(), &event);
      system().setKeyState(event.xkey.state);
      std::map<Window,Window*>::iterator itr;
      int winId = event.xany.window;
      if ( (itr = winMap.find(winId)) == winMap.end() )
      {
        // not my windows
      } else {
        Window* window = itr->second;
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
      for (std::vector<Window*>::iterator it = repaintWin_.begin(); it < repaintWin_.end(); it++) {
        Window* win = *it;
        win->graphics().resize(win->width(),win->height());
        win->repaint(win->pane(),Rect(0,0,win->width(),win->height()));
      }
      repaintWin_.clear();
    }
#endif
  }


  Config* App::config( )
  {
    return config_;
  }

  void App::addRemovePipe( Runtime * component )
  {
     removePipe.push_back( component );
  }

  void App::callRemovePipe(  )
  {
    for (std::vector<Runtime*>::iterator iter = removePipe.begin(); iter<removePipe.end(); iter++) {   
      lastOverWin_ = 0;  
      Runtime* component = *iter;
      delete component;
    } 
    removePipe.clear();
  }

  void App::enterThread( )
  {
    in_thread_=true;
    clearEventQueue();
  }

  void App::leaveThread( )
  {
    in_thread_=false;
  }

}
