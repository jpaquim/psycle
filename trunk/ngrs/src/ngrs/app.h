/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef APP_H
#define APP_H

#include <map>
#include "keyaccelerator.h"
#include "xpmfilter.h"

#ifdef __unix__
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif

namespace ngrs {

#ifdef __unix__
  typedef XEvent WEvent;
#else
  struct WEvent {
    HWND hwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;            
  };
#endif

  class System;
  class Window;
  class Config;
  class SplashScreen;
  class VisualComponent;

  /**
  beinhaltet u.a die Eventloop 

  @author  Stefan
  */
  class App{
  public:
    App();

    ~App();

    void run();
    static void runModal(Window* modalWin);
    void setMainWindow(Window* window);
    static Window* mainWindow();
    static void addWindow(WinHandle handle, Window* window);
    static void addKeyAccelerator(const KeyAccelerator & accelerator, Object* notify);
    static void removeWindow( WinHandle handle );
    static System & system();
    static void doRepaint(Window* win);

    static Window* mouseOverWindow();
    static void registerPopupWindow(Window* win);
    static void unregisterPopupWindow(Window* win);
    static void unmapPopupWindows();

    static bool popupUnmapped_;

    static std::vector<ImgFilter*> filter;

    void setSplashScreen(SplashScreen * splashScreen);

    static void clearEventQueue();
    static void flushEventQueue();

    static Config * config();

    static void addRemovePipe(Runtime* component);

    static void enterThread();
    static void leaveThread();

    static void doDrag( bool on, ngrs::Window* win );
    static bool drag();

    static Window* lastOverWin_;

#ifdef __unix__
#else
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

  private:

    static Config* config_;

    SplashScreen* splashScreen_;
    static char buffer[40];
#ifdef __unix__
    static KeySym mykeysym;
    static XComposeStatus compose;
    static Time lastBtnPressTime;
#endif
    static Window* modalWin_;

    static std::map<WinHandle,Window*> winMap;
    static Window* mainWin_;

    void eventLoop();
    static void modalEventLoop(Window* modalWin);

#ifdef __unix__
    static unsigned int processEvent( Window* win, WEvent* event);
#else
    static LRESULT processEvent( Window* win, WEvent* event);
#endif

    static std::vector<Window*> repaintWin_;
    static std::vector<VisualComponent*> scrollControl_;

    static std::map<KeyAccelerator, Object*> keyAccelerator_;

    static Object* findAcceleratorNotifier(const KeyAccelerator & acc);
    static std::vector<Window*> popups_;

    static std::vector<Runtime*> removePipe;
    static void callRemovePipe( );

    NXPMFilter* xpmFilter_;
#ifdef __unix__
    pthread_mutex_t m_Mutex;
#endif
    static bool in_thread_;

    static void buttonPress( Window* win, WEvent* event, int button );

    static int modalExitLoop_;

    static bool doDrag_;

  };

}

#endif
