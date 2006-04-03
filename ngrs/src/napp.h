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
#include <sys/select.h>


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
   static void runModal();
   void setMainWindow(NWindow* window);
   static void addWindow(Window handle, NWindow* window);
   static void addKeyAccelerator(const NKeyAccelerator & accelerator, NObject* notify);
   static void removeWindow( Window handle );
   static NSystem & system();
   static void doRepaint(NWindow* win);

   static NWindow* mouseOverWindow();
   static void addPopupWindow(NWindow* win);
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

private:

   static NConfig* config_;

   NSplashScreen* splashScreen_;
   static char buffer[40];
   static KeySym mykeysym;
   static XComposeStatus compose;

   static Time lastBtnPressTime;

   static std::map<Window,NWindow*> winMap;
   static NWindow* mainWin_;

   static NSystem* system_;

   void eventLoop();
   static void modalEventLoop();

   static int processEvent(NWindow* win, XEvent* event);

   static std::vector<NWindow*> repaintWin_;
   static std::vector<NVisualComponent*> scrollControl_;

   static std::map<NKeyAccelerator, NObject*> keyAccelerator_;

   static NObject* findAcceleratorNotifier(const NKeyAccelerator & acc);
   static std::vector<NWindow*> popups_;

   static NWindow* lastOverWin_;

   static std::vector<NRuntime*> removePipe;
   static void callRemovePipe( );

   NXPMFilter* xpmFilter_;

   pthread_mutex_t m_Mutex;
   static bool in_thread_;
};

#endif
