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
#include "nsystem.h"
#include <stdexcept>


using namespace std;

NSystem::NSystem()
{
  initX();
}


NSystem::~NSystem()
{
}

// here you will find all public methods

Display* NSystem::dpy( ) const
{
   return dpy_;
}

int NSystem::depth( ) const
{
  return depth_;
}

int NSystem::screen( ) const
{
  return screen_;
}

Window NSystem::rootWindow( ) const
{
    return rootWindow_;
}

Visual * NSystem::visual( ) const
{
  return visual_;
}

Colormap NSystem::colormap( ) const
{
  return colormap_;
}

int NSystem::keyState( ) const
{
  return keyState_;
}

void NSystem::setKeyState( int keyState )
{
  keyState_ = keyState;
}

Window NSystem::registerWindow(Window parent )
{
  Window win_ = 0;

  XSetWindowAttributes  attr;
  attr.win_gravity = StaticGravity;
  attr.bit_gravity = NorthWestGravity;
  XColor near_color, true_color;
  XAllocNamedColor( dpy(), colormap(), "lightgray", &near_color, &true_color ); 
  attr.background_pixel = BlackPixel(dpy(), 0)^near_color.pixel;
  attr.colormap = colormap();
  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                    ButtonPressMask| ButtonReleaseMask | PointerMotionMask
                    |PropertyChangeMask | NoEventMask | StructureNotifyMask |  GravityNotify;
  win_ = XCreateWindow(dpy(), parent,
                        0, 0, 400, 400,
                        0, depth(), InputOutput, visual(),
                        CWBackPixel | CWColormap | CWEventMask  | CWWinGravity | CWBitGravity ,
                        &attr) ;
  XSetWMProtocols(dpy(), win_, &wm_delete_window, 1);

  return win_;
}

// and here are the private methods

void NSystem::initX( )
{
  XInitThreads();

  dpy_        = XOpenDisplay(NULL);
  if(!dpy_)
  {
    std::ostringstream s;
    s << "ngrs: failed to open default display. is the DISPLAY environment variable set to some available display?";
    std::cerr << s << std::endl;
    throw std::runtime_error(s.str().c_str());
  }
  screen_     = DefaultScreen(dpy_);
  rootWindow_ = RootWindow(dpy_, screen_);
  colormap_   = DefaultColormap(dpy_, screen_);
  visual_     = DefaultVisual(dpy_, screen_);
  depth_      = DefaultDepth(dpy_, screen_);

  matchVisual();

  wm_delete_window = XInternAtom(dpy_, "WM_DELETE_WINDOW", False);
  keyState_ = 0;
}

void NSystem::flush( )
{
  XFlush(dpy());
}

NFontStructure NSystem::getXFontValues( const NFont & nFnt )
{
 NFontStructure fnt;
 NFontStructure cacheFnt;
 std::map<NFont, NFontStructure>::iterator itr;

 if (nFnt.antialias() ) {

   if ( (itr = xftfntCache.find(nFnt)) == xftfntCache.end())
   {
     string xname = getFontPattern(nFnt);
     if (xname=="") xname="8x13";
     cacheFnt.xftFnt = XftFontOpenXlfd(dpy(),screen(),xname.c_str());
     if (cacheFnt.xftFnt==NULL) {
        cacheFnt.antialias = false;
        cacheFnt.xFnt = XLoadQueryFont(dpy(),xname.c_str());
        xftfntCache[nFnt]=cacheFnt;
        fnt = cacheFnt;
     } else {
       cacheFnt.antialias = true;
       xftfntCache[nFnt]=fnt=cacheFnt;
     }
   } else {
     fnt =  itr->second;
   }
 } else

 if (!nFnt.antialias()) {
   std::map<NFont, NFontStructure>::iterator itr;
   if ( (itr = xfntCache.find(nFnt)) == xfntCache.end())
   {
     string xname = getFontPattern(nFnt);
     if (xname=="") xname="8x13";
     cacheFnt.antialias = false;
     cacheFnt.xFnt = XLoadQueryFont(dpy(),xname.c_str());
     xfntCache[nFnt]=cacheFnt;
     fnt = cacheFnt;
   } else {
     fnt =  itr->second;
   }
 }
 fnt.textColor = nFnt.textColor();
 return fnt;
}

string NSystem::getFontPattern( const NFont & font )
{
  string xfntname = "";
  if (isWellFormedFont(font.fontString())) return font.fontString();
  else {
    string pattern = fontPattern(font);
    int count = 0;
    char** myFonts = getFontList(dpy(), pattern, &count);    
    if (count!=0) {
      xfntname = getFontPatternWithSizeStyle(dpy(),0,myFonts[0],font.size()*10);
    }
    XFreeFontNames(myFonts);
    if (count==0) {
      // nothing found 
       // try get a non-scalable font
       char sz[20]; sprintf(sz,"%d",font.size());
       string xfntname = "-*-"+std::string(font.name())+"-"+"*"+"-"+"*"+"-*-*-"+string(sz) +"-*-*-*-*-*-*-*";
       char** myFonts = getFontList(dpy(),xfntname.c_str(),&count); 
       if (count==0) {
          return "";
       }
       XFreeFontNames(myFonts);
    }
  }
  return xfntname;
}

bool NSystem::isWellFormedFont(string name)
{
 string::size_type pos = 0;
 int field = 0;
 while ( (pos = name.find('-', pos)) != string::npos) field++;
 return (field == 14) ? 1 : 0;
}

bool NSystem::isScalableFont(string name)
{
 const char* name_ = name.c_str();
    int i, field;
    if ((name_ == NULL) || (name_[0] != '-')) return false;
    for(i = field = 0; name_[i] != '\0'; i++) {
        if (name_[i] == '-') {
            field++;
            if ((field == 7) || (field == 8) || (field == 12))
                if ((name_[i+1] != '0') || (name_[i+2] != '-'))
                    return 0;
        }
    }
    if (field != 14) return false;
    else return true;
}

string NSystem::fontPattern(const NFont & font) {

  string styleString  = "*";
  string italicString = "r";

  if (font.style() & nBold)    styleString  = "bold";  
  if (font.style() & nMedium)  styleString  = "medium";
  if (font.style() & nItalic)  italicString = "i";

  return string("-*-")+font.name()+string("-")+styleString+string("-")+italicString+string("-*-*-0-0-*-*-*-0-*-*");

}

string NSystem::getFontPatternWithSizeStyle(Display* dpy, int screen, const char* name, int size)
{
    int i,j, field;
    char newname[500];    /* big enough for a long font name */
    int res_x, res_y;     /* resolution values for this screen */
    /* catch obvious errors */
    if ((name == NULL) || (name[0] != '-')) return NULL;
    /* calculate our screen resolution in dots per inch. 25.4mm = 1 inch */
    res_x = (int) (DisplayWidth(dpy, screen)/(DisplayWidthMM(dpy, screen)/25.4));
    res_y = (int) (DisplayHeight(dpy, screen)/(DisplayHeightMM(dpy, screen)/25.4));
    /* copy the font name, changing the scalable fields as we do so */
    for(i = j = field = 0; name[i] != '\0' && field <= 14; i++) {
        newname[j++] = name[i];
        if (name[i] == '-') {
            field++;
            switch(field) {
            case 7:  /* pixel size */
            case 12: /* average width */
                /* change from "-0-" to "-*-" */
                newname[j] = '*';
                j++;
                if (name[i+1] != '\0') i++;
                break;
            case 8:  /* point size */
                /* change from "-0-" to "-<size>-" */
                sprintf(&newname[j], "%d", size);
                while (newname[j] != '\0') j++;
                if (name[i+1] != '\0') i++;
                break;
            case 9:  /* x-resolution */
            case 10: /* y-resolution */
                /* change from an unspecified resolution to res_x or res_y */
                sprintf(&newname[j], "%d", (field == 9) ? res_x : res_y);
                while(newname[j] != '\0') j++;
                while((name[i+1] != '-') && (name[i+1] != '\0')) i++;
                break;
            }
        }
    }
    newname[j] = '\0';
    /* if there aren't 14 hyphens, it isn't a well formed name */    
    if (field != 14) return NULL;
    return string(newname);
}

char ** NSystem::getFontList(Display* dpy, string pattern, int* count)
{
   return XListFonts(dpy, pattern.c_str(), 32767, count);
}

void NSystem::setWindowPosition(Window win, int left, int top, int width, int height )
{
  if (width  <= 0) width  = 1;
  if (height <= 0) height = 1;

  XWindowAttributes attrib;
  XGetWindowAttributes( dpy(), win, &attrib );

  if (width!=attrib.width || height!=attrib.height) {
    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PPosition ;
    size_hints->x=left;
    size_hints->y=top;
    size_hints->min_width = 10;
    size_hints->min_height = 10;
    XSetNormalHints(dpy(),win,size_hints);
    XFree(size_hints);

    XWindowChanges wc;
    wc.x=left;
    wc.y=top;
    wc.width=width;
    wc.height=height;	
    unsigned int value = CWX | CWY | CWWidth | CWHeight ;
    XConfigureWindow(dpy(),win,value,&wc);
  } else {
      XWindowChanges wc;
    wc.x=left;
    wc.y=top;
    wc.width=width;
    wc.height=height;
    unsigned int value = CWX | CWY;
    XConfigureWindow(dpy(),win,value,&wc);
  }
}

void NSystem::destroyWindow( Window win )
{
  XDestroyWindow(dpy(), win);
}

int NSystem::pixelsize( ) const
{
 int pixelsize;
 switch(depth())
      {
      case 8:
        pixelsize=1;
        break;
      case 16:
        pixelsize=2;
        break;
      case 24:
        pixelsize=4;
        break;
      default:
        pixelsize=1;
      }
  return pixelsize;
}

void NSystem::setWindowDecoration( Window win, bool on )
{
  XSetWindowAttributes attribs;
  int vmask = CWOverrideRedirect;
  attribs.override_redirect = !on;
  XChangeWindowAttributes(dpy(), win, vmask, &attribs);
  
}

void NSystem::unmapWindow( Window  win )
{
  if (isWindowMapped(win)) {
  // XEvent ev;

   XUnmapWindow(dpy(), win);//, DefaultScreen(dpy()));

  }
}

void NSystem::mapWindow( Window  win )
{
  XMapWindow(dpy(), win);
}

bool NSystem::isWindowMapped( Window win )
{
  XWindowAttributes attr;
  XGetWindowAttributes( dpy(), win, &attr );
  return !(attr.map_state == IsUnmapped);
}

int NSystem::windowLeft( Window win )
{
  XWindowAttributes actual;
  XGetWindowAttributes(dpy(), win, &actual);
  Window cr; 
  int X,Y; 
  XTranslateCoordinates(dpy(), win, actual.root,0, 0, &X, &Y, &cr);
  return X;
}

int NSystem::windowTop( Window win )
{
  XWindowAttributes actual;
  XGetWindowAttributes(dpy(), win, &actual);
  Window cr;
  int X,Y;
  XTranslateCoordinates(dpy(), win, actual.root,0, 0, &X, &Y, &cr);
  return Y;
}

void NSystem::setWindowGrab( Window win, bool on )
{
  if (win!=0) {
    if (on) {
     XGrabPointer(dpy(), win, true,
                     ButtonPressMask | ButtonReleaseMask,
                     GrabModeAsync, GrabModeAsync,
                     None, None , CurrentTime);
     XGrabKeyboard(dpy(), win, false, GrabModeAsync, GrabModeAsync, CurrentTime);
   } else {
      XUngrabPointer( dpy(), CurrentTime);
      XUngrabKeyboard(dpy(), CurrentTime);
      XFlush(dpy());
   }
  }
}

int NSystem::screenWidth( ) const
{
  return DisplayWidth(dpy(),screen());
}

int NSystem::screenHeight( ) const
{
  return DisplayHeight(dpy(),screen());
}

void NSystem::setStayAbove( Window win )
{
  Atom atom = XInternAtom(dpy(),"ATOM",0);
  Atom _net_wm_state_above = XInternAtom(dpy(), "_NET_WM_STATE_ABOVE", 0);
  Atom data[1];
  data[0]=_net_wm_state_above;
  XChangeProperty(dpy(), win, _net_wm_state_above,atom, 32,PropModeReplace,
                   (const unsigned char*) &data, 1);
  XSync(dpy(),true);
}

unsigned long NSystem::getXColorValue(int r, int g, int b )
{
 if (isTrueColor()) {

   unsigned long value = ((r << 16) & red_mask)  | ((g << 8) & green_mask) | (b & blue_mask);
   return value;
 }

 unsigned long key = (r << 16) | (g << 8) | (b);
 unsigned long value = 0;
 std::map<unsigned long, unsigned long>::iterator itr;
 if ( (itr = colorCache.find(key)) == colorCache.end()) {
  char rs[5]; sprintf(rs,"%.2x",r);
  char gs[5]; sprintf(gs,"%.2x",g);
  char bs[5]; sprintf(bs,"%.2x",b);
  std::string colorName = string("rgb:")+string(rs)+string("/")+string(gs)+string("/")+string(bs);
  XColor near_color, true_color;
  XAllocNamedColor( dpy(), colormap(), colorName.c_str(), &near_color, &true_color );
  colorCache[key]= value = BlackPixel(dpy(), 0)^near_color.pixel;
 } else value = itr->second;
 return value;
}

void NSystem::matchVisual( )
{
  isTrueColor_ = false;

  int visualsMatched;       // Number of visuals that match
  XVisualInfo vTemplate;    // Template of the visual we want
  XVisualInfo *visualList;  // List of XVisualInfo structs that match


  vTemplate.depth   = depth();
  vTemplate.screen  = screen();
  vTemplate.c_class = TrueColor;

  visualList = XGetVisualInfo( dpy(), VisualDepthMask | VisualScreenMask | VisualClassMask, &vTemplate, &visualsMatched);

  if (visualsMatched != 0) {
    // we found at least one TrueColor visual on the screen, with the required depth.
    std::cout << "ngrs: visual class: true-color, bypassing color map" << std::endl;
    visual_ = visualList[0].visual;
    isTrueColor_ = true;
    colormap_ = XCreateColormap (dpy(), rootWindow(),visualList[0].visual, AllocNone);
  }
  else
  {
    // use colormap
    std::cout << "ngrs: visual class: not true-color, using color map" << std::endl;
  }

  red_mask   = visual_->red_mask;
  green_mask = visual_->green_mask;
  blue_mask  = visual_->blue_mask;

  XFree(visualList);
}

bool NSystem::isTrueColor()
{
  return isTrueColor_;
}
