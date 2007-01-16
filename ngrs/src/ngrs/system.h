/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NSYSTEM_H
#define NSYSTEM_H

#include <string>
#include "font.h"
#include "fontstructure.h"
#include "color_converter.hpp"
#include "clipboard.h"
#include "mwm.h"
#include "crdefine.h"
#include "keydefine.h"
#include "rect.h"

#ifdef __unix__
  #include <X11/Xlib.h>
  #include <X11/Xft/Xft.h>
  #include <X11/cursorfont.h>
  #include <X11/Xutil.h>
  #include <X11/Xatom.h>
  #include <X11/cursorfont.h>
#else
  #include <windows.h>
#endif

/**
@author Stefan
*/

namespace ngrs {

#ifdef __unix__
  typedef Window WinHandle;
#else
  typedef HWND WinHandle;
#endif

#ifdef __unix__
 typedef XRect PlatformRect;
#else
 typedef RECT PlatformRect;
#endif

  class Atoms;
  class Window;

  class NSystem{
    // Singleton Pattern
  private:
    NSystem();
    ~NSystem();
    NSystem( NSystem const & );
    NSystem& operator=(NSystem const&);

  public:
    static NSystem* Instance() {
      //use only single threaded!
      static NSystem s;
      return &s; 
    }
    // Singleton pattern end

    WinHandle rootWindow() const;

#ifdef __unix__
    Display* dpy() const;
    Visual *visual() const;
    Colormap colormap() const;
#else
    HINSTANCE hInst() const;
#endif
    int depth() const;
    int pixelSize( int depth ) const;
    int screen() const;

    int screenWidth() const;
    int screenHeight() const;  

    int shiftState() const;
    int keyState( int vkey ) const; // 0 up : 1 down 2 : toggled
    std::map<int,int> keyboardState() const;

    Rect convertPlatformRect( PlatformRect & rect ) const;

    WinHandle registerWindow(WinHandle parent);
    void destroyWindow(WinHandle win);
    void setWindowPosition(WinHandle win, int left, int top, int width, int height);
    void setWindowSize(WinHandle win, int width, int height);
    void setWindowMinimumSize(WinHandle win, int minWidth, int minHeight);
    void setWindowGrab(WinHandle win,bool on);
    void unmapWindow(WinHandle win);
    void mapWindow(WinHandle win);
    void flush();
    void setWindowDecoration(WinHandle win, bool on);
    void setStayAbove(WinHandle win);
    bool isWindowMapped(WinHandle win);
    FontStructure getFontValues(const NFont & nFnt);
    int windowLeft(WinHandle win);
    int windowTop(WinHandle win);
    unsigned long getXColorValue(int r, int g, int b);

    bool isTrueColor();
    bool propertysActive();

#ifdef __unix__
    MWMHints getMotifHints(WinHandle win) const;
    void setMotifModalMode(WinHandle win);
    void setMotifHints( WinHandle win , MWMHints hints);
#endif

    void setModalMode(WinHandle win);
    void setFocus( Window* window );

    const Atoms & atoms() const;

    ClipBoard & clipBoard();
    const ClipBoard & clipBoard() const;

    void setCursor( int crIdentifier, Window* win );
    int cursor() const;

  private:

    Atoms* atoms_;

    bool isTrueColor_;

    int depth_;
    int screen_;
    int cursorId_;

    typedef color_converter<8, unsigned long int> color_converter;
    color_converter color_converter_;

    WinHandle rootWindow_;
#ifdef __unix__
    Display* dpy_;
    Visual *visual_;
    Colormap colormap_;
#else
    WNDCLASSEX wc;
#endif

    std::map<NFont,FontStructure>  xfntCache;
    std::map<NFont,FontStructure>  xftfntCache;
    std::map<unsigned long,unsigned long> colorCache;
#ifdef __unix__
    std::map<int, Cursor> cursorMap;
#else
    std::map<int, HCURSOR> cursorMap;    
#endif

    void initX();
    void matchVisual();
    std::string getFontPattern(const NFont & font);
#ifdef __unix__
    static bool isWellFormedFont(std::string name);
    static bool isScalableFont(std::string name);
    static std::string fontPattern(const NFont & font);
    static char ** getFontList(Display* dpy, std::string pattern, int* count);
    static std::string getFontPatternWithSizeStyle(Display* dpy, int screen, const char* name, int size);
#endif

    void initCursorMap();

    ClipBoard clipBoard_;
  };

}

#endif
