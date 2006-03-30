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
#include "nimage.h"
#include "nsplashscreen.h"
#include "nconfig.h"
#include "nwindow.h"
#include "ntimer.h"


NSystem* NApp::system_ = 0;
NConfig* NApp::config_ = 0;

std::vector<NRuntime*> NApp::removePipe;

using namespace std;

map<Window,NWindow*> NApp::winMap;
std::vector<NWindow*> NApp::repaintWin_;
std::vector<NWindow*> NApp::popups_;
std::map<NKeyAccelerator, NObject*> NApp::keyAccelerator_;
bool NApp::popupUnmapped_ = false;
bool NApp::in_thread_ = false;
std::vector<NImgFilter*> NApp::filter;

char NApp::buffer[40];
KeySym NApp::mykeysym;
XComposeStatus NApp::compose;
Time NApp::lastBtnPressTime;

NWindow* NApp::lastOverWin_;
NWindow* NApp::mainWin_;




NApp::NApp()
{
  mainWin_ = 0;
  lastBtnPressTime = 0;
  lastOverWin_ = 0;
  xpmFilter_ = new NXPMFilter();
  filter.push_back(xpmFilter_);
  splashScreen_ = 0;
  config_ = new NConfig();
}


NApp::~NApp()
{
  delete xpmFilter_;
}

// here you find all public methods

void NApp::run( )
{
  if (splashScreen_!=0) system().unmapWindow(splashScreen_->win());
  eventLoop();
}

void NApp::setMainWindow( NWindow* window )
{
  mainWin_ = window; 
}

NSystem & NApp::system( )
{
  if (!system_) system_ = new NSystem(); 
  return *system_;
}

// private methods

void NApp::eventLoop( )
{
  if (mainWin_!=0) {
       mainWin_->setVisible(true);
  }
  system().flush();
  XEvent event;
  int n;
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
             win->repaint(0,0,win->width(),win->height());
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
        if (event.xany.window != mainWin_->win()) {
        if (exitLoop==nDestroyWindow) {
          delete window;
          exitLoop = 0;
        } else exitLoop=0;
  //      }
       }
     }
       //doRemove();
    }
  }
}

void NApp::modalEventLoop( )
{
  XEvent event;
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
             win->repaint(0,0,win->width(),win->height());
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
        exitLoop = processEvent(window, & event);
        if (window->exitLoop() == 1) {
          // at Modal no delete here
          exitLoop = 1;
        } else exitLoop=0;
  //      }
       }
     }
       //doRemove();
  }
}



int NApp::processEvent( NWindow * win, XEvent * event )
{
  int exitloop = 0;
  switch (event->type) {
    case Expose:
        if (win->doubleBuffered()) win->graphics()->swap(NRect(event->xexpose.x,event->xexpose.y,event->xexpose.width,event->xexpose.height)); else
        doRepaint(win);
    break;
    case ConfigureNotify:{
      while (XCheckMaskEvent(NApp::system().dpy(),StructureNotifyMask , event));
       if (event->xconfigure.width - 2*event->xconfigure.border_width != win->graphics()->dblWidth() || event->xconfigure.height != win->graphics()->dblHeight()  ) {
         doRepaint(win);
       }
    }
    break;
    case ButtonPress: {
        vector<NWindow*>::iterator itr;
        for (itr = popups_.begin(); itr < popups_.end(); itr++) {
           NWindow* popup = *itr;
           if (popup->visible() && win!=popup) {
               popupUnmapped_ = true;
               popup->setVisible(false);
           }
        }
        Time time = event->xbutton.time;
        if ( time - lastBtnPressTime < 300 ) win->onMouseDoublePress(event->xbutton.x,event->xbutton.y,event->xbutton.button);
        win->onMousePress(event->xbutton.x,event->xbutton.y,event->xbutton.button);
        lastBtnPressTime = time;
        popupUnmapped_ = false;
      }
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
              int count = XLookupString(&event->xkey, buffer,15, &mykeysym, &compose);
              if (buffer!=NULL) {
                  if (mykeysym<0xF000) {
                    NObject* acellNotify = findAcceleratorNotifier(NKeyAccelerator(NApp::system().keyState(),mykeysym));
                    if (acellNotify!=0) {
                        acellNotify->onKeyAcceleratorNotify(NKeyAccelerator(NApp::system().keyState(),mykeysym));
                    }
                  }
                  win->onKeyPress(NKeyEvent(buffer,mykeysym));
              }
            }
          break;
    case ClientMessage:
            if(event->xclient.data.l[0] == NApp::system().wm_delete_window) exitloop = win->onClose();
         break;

    default : ;
  }
  return exitloop;
}

void NApp::addWindow( Window handle, NWindow * window )
{
  winMap[handle]=window;
}

void NApp::removeWindow( Window handle )
{
 std::map<Window,NWindow*>::iterator itr;
 if ( (itr = winMap.find(handle)) == winMap.end())
 {
   // not my windows
 } else {
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
}

void NApp::addPopupWindow( NWindow * win )
{
  popups_.push_back(win);
}

void NApp::runModal( )
{
  modalEventLoop();
}

void NApp::unmapPopupWindows( )
{
  vector<NWindow*>::iterator itr;
  for (itr = popups_.begin(); itr < popups_.end(); itr++) {
    NWindow* popup = *itr;
    if (popup->visible()) {
      popupUnmapped_ = true;
      popup->setVisible(false);
    }
  }
}

void NApp::setSplashScreen(NSplashScreen * splashScreen )
{
  splashScreen_ = splashScreen;
  if (splashScreen_!=0) {
     NApp::system().mapWindow(splashScreen_->win());
     splashScreen_->graphics()->resize(splashScreen_->width(),splashScreen_->height());
     splashScreen_->repaint(0,0,splashScreen_->width(),splashScreen_->height());
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
  XEvent event;
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
             win->repaint(0,0,win->width(),win->height());
          }
          repaintWin_.clear();
       }
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
  NRuntime* component = *iter;
    delete component;
 } 
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


