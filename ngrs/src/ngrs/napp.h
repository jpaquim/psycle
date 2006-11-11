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
#ifndef NAPP_H
#define NAPP_H

#include <map>
#include "nkeyaccelerator.h"
#include "nxpmfilter.h"
#include <sys/time.h>
#ifdef __unix__
#include <sys/select.h>
#endif
#include <unistd.h>

#ifdef __unix__
typedef XEvent WEvent;
#else
typedef int WEvent;
#endif

class NSystem;
class NWindow;
class NConfig;
class NSplashScreen;
class NVisualComponent;

/**
beinhaltet u.a die Eventloop 

@author Stefan
*/
class NApp{
public:
    NApp();

    ~NApp();

   void run();
   static void runModal(NWindow* modalWin);
   void setMainWindow(NWindow* window);
   static void addWindow(WinHandle handle, NWindow* window);
   static void addKeyAccelerator(const NKeyAccelerator & accelerator, NObject* notify);
   static void removeWindow( WinHandle handle );
   static NSystem & system();
   static void doRepaint(NWindow* win);

   static NWindow* mouseOverWindow();
   static void registerPopupWindow(NWindow* win);
   static void unregisterPopupWindow(NWindow* win);
   static void unmapPopupWindows();

   static bool popupUnmapped_;

   static std::vector<NImgFilter*> filter;

   void setSplashScreen(NSplashScreen * splashScreen);

   static void clearEventQueue();
   static void flushEventQueue();

   static NConfig * config();

   static void addRemovePipe(NRuntime* component);

   static void enterThread();
   static void leaveThread();

   static NWindow* lastOverWin_;
   
   #ifdef __unix__
   #else
   static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   #endif

private:

   static NConfig* config_;

   NSplashScreen* splashScreen_;
   static char buffer[40];
   #ifdef __unix__
   static KeySym mykeysym;
   static XComposeStatus compose;
   static Time lastBtnPressTime;
   #endif

   static std::map<WinHandle,NWindow*> winMap;
   static NWindow* mainWin_;

   void eventLoop();
   static void modalEventLoop(NWindow* modalWin);

   static int processEvent(NWindow* win, WEvent* event);

   static std::vector<NWindow*> repaintWin_;
   static std::vector<NVisualComponent*> scrollControl_;

   static std::map<NKeyAccelerator, NObject*> keyAccelerator_;

   static NObject* findAcceleratorNotifier(const NKeyAccelerator & acc);
   static std::vector<NWindow*> popups_;

   static std::vector<NRuntime*> removePipe;
   static void callRemovePipe( );

   NXPMFilter* xpmFilter_;
   #ifdef __unix__
   pthread_mutex_t m_Mutex;
   #endif
   static bool in_thread_;

};

#endif
