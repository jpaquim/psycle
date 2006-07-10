/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
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


#include "mainwindow.h"
#include "song.h"
#include "plugin.h"
#include <ngrs/nsplashscreen.h>
#include <ngrs/napp.h>
#include <ngrs/nfile.h>
#include <ngrs/nproperty.h>
#include <iostream>
#include <cstdlib>

using namespace std;

using namespace psycle::host;

/*inline int f2i(double d) { 
    // 2^51 + 2^
    asm volatile("fldcw %0"::"m"(0x137f));
    const double magic = 6755399441055744.0; // 2^51 + 2^52
    double tmp = (d-0.5) + magic; 
    return 2047 &(int)tmp;
}*/

int f2i(double q) { return ((int)q)&2047; }

int main(int argc, char *argv[])
{
    NApp app;

    Global gl;
/* at an not guilty path here can be an segfault so outcomment so far
    NSplashScreen* splash = new NSplashScreen();
    splash->loadImageFromFile(Global::pConfig()->iconPath + "/splash.xpm");
    app.setSplashScreen(splash);*/

    NWindow* mainWin = new MainWindow();
    app.setMainWindow(mainWin);
    app.run();

    return EXIT_SUCCESS;
}
