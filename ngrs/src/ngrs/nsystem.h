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
#ifndef NSYSTEM_H
#define NSYSTEM_H

#include <string>
#include "nfont.h"
#include "nfontstructure.h"
#include "color_converter.hpp"
#include "nclipboard.h"
#include "mwm.h"
#include "ncrdefine.h"

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

/**
@author Stefan
*/


class NAtoms;
class NWindow;

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

    Display* dpy() const;
    int depth() const;
    int pixelSize( int depth ) const;
    int screen() const;
    Window rootWindow() const;
    Visual *visual() const;
    Colormap colormap() const;
    int screenWidth() const;
    int screenHeight() const;

    int keyState() const;
    void setKeyState(int keyState);

    Window registerWindow(Window parent);
    void destroyWindow(Window win);
    void setWindowPosition(Window win, int left, int top, int width, int height);
    void setWindowSize(Window win, int width, int height);
    void setWindowMinimumSize(Window win, int minWidth, int minHeight);
    void setWindowGrab(Window win,bool on);
    void unmapWindow(Window win);
    void mapWindow(Window win);
    void flush();
    void setWindowDecoration(Window win, bool on);
    void setStayAbove(Window win);
    bool isWindowMapped(Window win);
    NFontStructure getXFontValues(const NFont & nFnt);
    int windowLeft(Window win);
    int windowTop(Window win);
    unsigned long getXColorValue(int r, int g, int b);

    bool isTrueColor();
    bool propertysActive();

    MWMHints getMotifHints(Window win) const;
    void setMotifModalMode(Window win);
    void setMotifHints( Window win , MWMHints hints);
    void setModalMode(Window win);
    void setFocus( NWindow* window );

    const NAtoms & atoms() const;

    NClipBoard & clipBoard();
    const NClipBoard & clipBoard() const;

		void setCursor( int crIdentifier, NWindow* win );
    int cursor() const;

private:

    NAtoms* atoms_;

    bool isTrueColor_;

    int depth_;
    int screen_;
    int keyState_;
    int cursorId_;

    

    typedef ngrs::color_converter<8, unsigned long int> color_converter;
    color_converter color_converter_;

    Display* dpy_;
    Window rootWindow_;
    Visual *visual_;
    Colormap colormap_;

    std::map<NFont,NFontStructure>  xfntCache;
    std::map<NFont,NFontStructure>  xftfntCache;
    std::map<unsigned long,unsigned long> colorCache;
    std::map<int, Cursor> cursorMap;

    void initX();
    void matchVisual();
    std::string getFontPattern(const NFont & font);
    static bool isWellFormedFont(std::string name);
    static bool isScalableFont(std::string name);
    static std::string fontPattern(const NFont & font);
    static char ** getFontList(Display* dpy, std::string pattern, int* count);
    static std::string getFontPatternWithSizeStyle(Display* dpy, int screen, const char* name, int size);

    void initCursorMap();

    NClipBoard clipBoard_;
};

#endif
