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
#include "natoms.h"
#include "nwindow.h"
#include "napp.h"
#include <stdexcept>


using namespace std;

NSystem::NSystem()
{
  initX();
  #ifdef __unix__
  atoms_ = new NAtoms( dpy() );
  #else
  // Step 1 : registering the Window Class    
  wc.cbSize        = sizeof(WNDCLASSEX);
  wc.style         = 0;
  wc.lpfnWndProc   = NApp::WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInst();
  wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = "myWindowClass";
  wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  if(!RegisterClassEx(&wc))
  {
    MessageBox(NULL, "Window Registration Failed!", "Error!",
    MB_ICONEXCLAMATION | MB_OK);
  }
  #endif
  
  cursorId_ = nCrDefault;
  initCursorMap();
}


NSystem::~NSystem()
{
  #ifdef __unix__
  delete atoms_;
  std::cout << "deleting cursor" << std::endl;
  std::map<int,Cursor>::iterator it = cursorMap.begin();
  for ( ; it != cursorMap.end(); it++ ) {
    {
	    if (it->second != None )
      XFreeCursor( dpy(), it->second );  
    }
  }
  #else
  std::map<NFont,NFontStructure>::iterator it = xftfntCache.begin();
  for ( ; it != xftfntCache.end(); it++ ) {
      NFontStructure fnt = it->second;
      DeleteObject( fnt.hFnt );
  }
  #endif
}

#ifdef __unix__
Display* NSystem::dpy( ) const
{
   return dpy_;
}
#else
HINSTANCE NSystem::hInst() const {
 return GetModuleHandle( 0 );         
}

#endif

int NSystem::depth( ) const
{
  return depth_;
}

int NSystem::screen( ) const
{
  return screen_;
}

WinHandle NSystem::rootWindow( ) const
{
    return rootWindow_;
}

#ifdef __unix__
Visual * NSystem::visual( ) const
{
  return visual_;
}

Colormap NSystem::colormap( ) const
{
  return colormap_;
}

#endif

int NSystem::keyState( ) const
{
  return keyState_;
}


int NSystem::shiftState() const {
  int sState = nsNone;  
  #ifdef __unix__
  char keys[32];
  XQueryKeymap( dpy(), keys );
	     
  long keycode= XKeysymToKeycode( NApp::system().dpy(), XK_Shift_L);
  if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsShift;
  keycode = XKeysymToKeycode( NApp::system().dpy(), XK_Control_L);
  if ( (keys[keycode/8] & 1<<(keycode%8) ) )   sState |= nsCtrl;
  #else  
  BYTE keyboardState[256];
  GetKeyboardState( keyboardState );

  if ( ( keyboardState[ VK_SHIFT ] & 0x80 ) == 0x80 )   sState |= nsShift;
  if ( ( keyboardState[ VK_CONTROL ] & 0x80 ) == 0x80 ) sState |= nsCtrl;  
  #endif  
  return sState;   
}    

int NSystem::keyState( int vkey ) const {
  // 0 up : 1 down 2 : toggled
  #ifdef __unix__
    char keys[32];
    XQueryKeymap( NApp::system().dpy(), keys );	     

    long keycode = XKeysymToKeycode( dpy(), vkey );
    if ( (keys[keycode/8] & 1<<(keycode%8) ) ) 
       return 1;
    else
       return 0;   
  #else
     return GetKeyState( vkey ) & 0x80;
  #endif
}

std::map<int,int> NSystem::keyboardState() const {
  std::map<int,int> states;
  // 0 up : 1 down 2 : toggled
  #ifdef __unix__
  char keys[32];
  XQueryKeymap( NApp::system().dpy(), keys );	     
    
  for (int keycode = 0; keycode < 256; keycode++ ) {
    states[ XKeycodeToKeysym( dpy(), keycode, 0 ) ] = (keys[keycode/8] & 1<<(keycode%8) ) ;
  }
  #else
  BYTE keyboardState[256];
  GetKeyboardState( keyboardState );
  
  for ( int keycode = 0 ; keycode < 255; keycode++ ) {
    if ( keyboardState[ keycode ] & 0x80 )
      states[ keycode ] = 1;
    else  
      states[ keycode ] = 0;
  }
  #endif                  
  return states;
}

                  
void NSystem::setKeyState( int keyState )
{
  keyState_ = keyState;
}

void NSystem::setCursor( int crIdentifier , NWindow* win) {  
  if ( crIdentifier != cursorId_ && win && win->win() ) {
    #ifdef __unix__   
    std::map<int,Cursor>::iterator it = cursorMap.find( crIdentifier );
    if ( it != cursorMap.end() ) {
      XDefineCursor( dpy(), win->win(), it->second);
      XFlush( dpy() );
    }
    #endif
    cursorId_ = crIdentifier;
  }
  
}

int NSystem::cursor() const {
  return cursorId_;
}

void NSystem::initCursorMap() {
 #ifdef __unix__
  cursorMap[ nCrDefault   ] = XCreateFontCursor( dpy(), XC_top_left_arrow );
  cursorMap[ nCrNone      ] = None;
  cursorMap[ nCrArrow     ] = XCreateFontCursor( dpy(), XC_top_left_arrow );
  cursorMap[ nCrCross     ] = XCreateFontCursor( dpy(), XC_cross );
  cursorMap[ nCrIBeam     ] = XCreateFontCursor( dpy(), XC_xterm );
  cursorMap[ nCrSize      ] = XCreateFontCursor( dpy(), XC_fleur );
  cursorMap[ nCrSizeNESW  ] = XCreateFontCursor( dpy(), XC_top_right_corner );
  cursorMap[ nCrSizeNS    ] = XCreateFontCursor( dpy(), XC_double_arrow );
  cursorMap[ nCrSizeNWSE  ] = XCreateFontCursor( dpy(), XC_bottom_right_corner );
  cursorMap[ nCrSizeWE    ] = XCreateFontCursor( dpy(), XC_sb_up_arrow);
  cursorMap[ nCrUpArrow   ] = XCreateFontCursor( dpy(), XC_sb_up_arrow);
  cursorMap[ nCrHourGlass ] = XCreateFontCursor( dpy(), XC_watch );
  cursorMap[ nCrDrag      ] = XCreateFontCursor( dpy(), XC_X_cursor );  
  cursorMap[ nCrNoDrop    ] = XCreateFontCursor( dpy(), XC_X_cursor);
  cursorMap[ nCrHSplit    ] = XCreateFontCursor( dpy(), XC_sb_v_double_arrow );
  cursorMap[ nCrVSplit    ] = XCreateFontCursor( dpy(), XC_sb_h_double_arrow );
  cursorMap[ nCrMultiDrag ] = XCreateFontCursor( dpy(), XC_hand1 );
  cursorMap[ nCrNo        ] = XCreateFontCursor( dpy(), XC_X_cursor );
  cursorMap[ nCrAppStart  ] = XCreateFontCursor( dpy(), XC_watch );
  cursorMap[ nCrHandPoint ] = XCreateFontCursor( dpy(), XC_hand1 );
  #endif
}

WinHandle NSystem::registerWindow(WinHandle parent )
{
  WinHandle win_ = 0;
  #ifdef __unix__

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
                        0, 0, 10, 10,
                        0, depth(), InputOutput, visual(),
                        CWBackPixel | CWColormap | CWEventMask  | CWWinGravity | CWBitGravity ,
                        &attr) ;
  Atom windowClose = atoms().wm_delete_window();
  XSetWMProtocols(dpy(), win_, &windowClose , 1);

  #else


    // Step2 : Creating the Window
    win_ = CreateWindowEx(
         0,
         "myWindowClass",
         "title of window",
         WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
         NULL, NULL, hInst(), NULL);
    
    if ( win_ == 0 )
    {
      MessageBox( NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK );     
    }

  #endif

  return win_;
}

// and here are the private methods

void NSystem::initX( )
{
  #ifdef __unix__
  XInitThreads();

  dpy_        = XOpenDisplay(NULL);
  if(!dpy_)
  {
    std::ostringstream s;
    s << "ngrs: failed to open default display. is the DISPLAY environment variable set to some available display?";
    std::cerr << s.str() << std::endl;
    exit(0);
  }
  screen_     = DefaultScreen(dpy_);
  rootWindow_ = RootWindow(dpy_, screen_);
  colormap_   = DefaultColormap(dpy_, screen_);
  visual_     = DefaultVisual(dpy_, screen_);
  depth_      = DefaultDepth(dpy_, screen_);

  matchVisual();
  #endif

  keyState_ = 0;
}

void NSystem::flush( )
{
  #ifdef __unix__
  XFlush(dpy());
  #endif
}

NFontStructure NSystem::getFontValues( const NFont & nFnt )
{
 NFontStructure fnt;
 NFontStructure cacheFnt;
 std::map<NFont, NFontStructure>::iterator itr;

 #ifdef __unix__
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
 #else

   if ( (itr = xftfntCache.find(nFnt)) == xftfntCache.end())
   {
    HDC hdc = GetDC(NULL);
    long lfHeight = -MulDiv( nFnt.size(), GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);   
        
    int fnWeight = 0;
    if ( nFnt.style() & nMedium ) fnWeight = FW_MEDIUM;
      else    
    if ( nFnt.style() & nBold ) fnWeight = FW_BOLD; 
        
    cacheFnt.hFnt = CreateFont(
         lfHeight, 0, 0, 0, fnWeight,
         nFnt.style() & nItalic , 0, 0, 0, 0, 0, 0, 0, 
         nFnt.name().c_str()
    );
          
     if ( !cacheFnt.hFnt ) {
        HFONT hGuiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        cacheFnt.hFnt = hGuiFont;
        fnt = cacheFnt;
     } else {
       xftfntCache[nFnt]=fnt=cacheFnt;
     }
   } else {
     fnt =  itr->second;
   } 
 #endif

 fnt.textColor = nFnt.textColor();
 return fnt;
}

std::string NSystem::getFontPattern( const NFont & font )
{
  string xfntname = "";
  #ifdef __unix__
  if (isWellFormedFont(font.fontString())) return font.fontString();
  else {
		// first search alias
		int count = 0;
		char** myFonts = getFontList(dpy(), font.name(), &count);
		if ( count!=0) {
			xfntname = myFonts[0];
			XFreeFontNames(myFonts);
			return xfntname;
		}
		
    string pattern = fontPattern(font);
    count = 0;
     myFonts = getFontList(dpy(), pattern, &count);
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
  #endif
  return xfntname;
}

#ifdef __unix__
bool NSystem::isWellFormedFont(std::string name)
{
 string::size_type pos = 0;
 int field = 0;
 while ( (pos = name.find('-', pos)) != string::npos) field++;
 return (field == 14) ? 1 : 0;
}

bool NSystem::isScalableFont(std::string name)
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

std::string NSystem::fontPattern(const NFont & font) {

  string styleString  = "*";
  string italicString = "r";

  if (font.style() & nBold)    styleString  = "bold";  
  if (font.style() & nMedium)  styleString  = "medium";
  if (font.style() & nItalic)  italicString = "i";

  return string("-*-")+font.name()+string("-")+styleString+string("-")+italicString+string("-*-*-0-0-*-*-*-0-*-*");

}


std::string NSystem::getFontPatternWithSizeStyle(Display* dpy, int screen, const char* name, int size)
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

#endif

void NSystem::setWindowPosition(WinHandle win, int left, int top, int width, int height )
{
  #ifdef __unix__
  if (width  <= 0) width  = 1;
  if (height <= 0) height = 1;

  XWindowAttributes attrib;
  XGetWindowAttributes( dpy(), win, &attrib );

  if (width!=attrib.width || height!=attrib.height) {
    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PPosition | PBaseSize;
    size_hints->x=left;
    size_hints->y=top;
    size_hints->min_width = 10;
    size_hints->min_height = 10;
    size_hints->base_width = width;
    size_hints->base_height = height;
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

    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PPosition | PBaseSize;
    size_hints->x=left;
    size_hints->y=top;
    size_hints->min_width = 10;
    size_hints->min_height = 10;
    size_hints->base_width = width;
    size_hints->base_height = height;
    XSetNormalHints(dpy(),win,size_hints);
    XFree(size_hints);

    XWindowChanges wc;
    wc.x=left;
    wc.y=top;
    wc.width=width;
    wc.height=height;
    unsigned int value = CWX | CWY | CWWidth | CWHeight;
    XConfigureWindow(dpy(),win,value,&wc);
  }
  XSync(dpy(),false);
  #else
  MoveWindow( win, left, top , width, height, true );
  #endif
}

void NSystem::setWindowSize( WinHandle win, int width, int height )
{
  #ifdef __unix__
  if (width  <= 0) width  = 1;
  if (height <= 0) height = 1;

  XWindowAttributes attrib;
  XGetWindowAttributes( dpy(), win, &attrib );

  if (width!=attrib.width || height!=attrib.height) {
    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PPosition ;
    size_hints->min_width = 10;
    size_hints->min_height = 10;
    XSetNormalHints(dpy(),win,size_hints);
    XFree(size_hints);

    XWindowChanges wc;
    wc.width=width;
    wc.height=height;	
    unsigned int value = CWWidth | CWHeight ;
    XConfigureWindow(dpy(),win,value,&wc);
  } else {
      XWindowChanges wc;
    wc.width=width;
    wc.height=height;
    unsigned int value = CWX | CWY;
    XConfigureWindow(dpy(),win,value,&wc);
  }
  XSync(dpy(),false);
  #endif
}

void NSystem::setWindowMinimumSize( WinHandle win, int minWidth, int minHeight )
{
  #ifdef __unix__
  XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PMinSize;
    size_hints->min_width  = minWidth;
    size_hints->min_height = minHeight;
    XSetNormalHints(dpy(),win,size_hints);
  XFree(size_hints);
  #endif
}

void NSystem::destroyWindow( WinHandle win )
{
  #ifdef __unix__
  XDestroyWindow(dpy(), win);
  #endif
}

int NSystem::pixelSize( int depth ) const
{
 int pixelsize;
 switch( depth )
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

void NSystem::setWindowDecoration( WinHandle win, bool on )
{
  #ifdef __unix__
  XSetWindowAttributes attribs;
  int vmask = CWOverrideRedirect;
  attribs.override_redirect = !on;
  XChangeWindowAttributes(dpy(), win, vmask, &attribs);
  #else
  if ( !on ) {
    SetWindowLongPtr(      
      win,
      GWL_STYLE,
      WS_POPUP
    );
  }   
  #endif
  
}

void NSystem::unmapWindow( WinHandle  win )
{
  if (isWindowMapped(win)) {
   #ifdef __unix__
   XUnmapWindow(dpy(), win);//, DefaultScreen(dpy()));
   #else
   ShowWindow( win, SW_HIDE );
   UpdateWindow( win );
   #endif
  }
}

void NSystem::mapWindow( WinHandle  win )
{
  #ifdef __unix__
  XMapWindow(dpy(), win);
  #else
  ShowWindow( win, 1 );
  UpdateWindow( win );
  #endif
}

bool NSystem::isWindowMapped( WinHandle win )
{
  #ifdef __unix__
  XWindowAttributes attr;
  XGetWindowAttributes( dpy(), win, &attr );
  return !(attr.map_state == IsUnmapped);
  #else
  return IsWindowVisible( win ) ;
  #endif
}

int NSystem::windowLeft( WinHandle win )
{
  #ifdef __unix__
  XWindowAttributes actual;
  XGetWindowAttributes(dpy(), win, &actual);
  Window cr; 
  int X,Y; 
  XTranslateCoordinates(dpy(), win, actual.root,0, 0, &X, &Y, &cr);
  return X;
  #else
  RECT r;
  GetWindowRect( win, &r );
  return r.left + GetSystemMetrics( SM_CXSIZEFRAME );
  #endif
}

int NSystem::windowTop( WinHandle win )
{
  #ifdef __unix__
  XWindowAttributes actual;
  XGetWindowAttributes(dpy(), win, &actual);
  Window cr;
  int X,Y;
  XTranslateCoordinates(dpy(), win, actual.root,0, 0, &X, &Y, &cr);
  return Y;
  #else
  RECT r;
  GetWindowRect( win, &r );
  return r.top + GetSystemMetrics( SM_CYSIZEFRAME ) + GetSystemMetrics( SM_CYCAPTION );
  #endif
}

void NSystem::setWindowGrab( WinHandle win, bool on )
{
  #ifdef __unix__
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
  #endif
}

int NSystem::screenWidth( ) const
{
  #ifdef __unix__
  return DisplayWidth(dpy(),screen());
  #else
  return GetSystemMetrics( SM_CXFULLSCREEN );
  #endif
}

int NSystem::screenHeight( ) const
{
  #ifdef __unix__
  return DisplayHeight(dpy(),screen());
  #else
  return GetSystemMetrics( SM_CYFULLSCREEN );
  #endif
}

void NSystem::setStayAbove( WinHandle win )
{
  #ifdef __unix__
  Atom atom = XInternAtom(dpy(),"ATOM",0);
  Atom _net_wm_state_above = XInternAtom(dpy(), "_NET_WM_STATE_ABOVE", 0);
  Atom data[1];
  data[0]=_net_wm_state_above;
  XChangeProperty(dpy(), win, _net_wm_state_above,atom, 32,PropModeReplace,
                   (const unsigned char*) &data, 1);
  XSync(dpy(),true);
  #endif
}

unsigned long NSystem::getXColorValue(int r, int g, int b )
{
 #ifdef __unix__
 if (isTrueColor()) {
   return color_converter_(r, g, b);
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
 #else
 return (r << 16) | (g << 8) | (b);
 #endif
}

void NSystem::matchVisual( )
{
  #ifdef __unix__
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
    #if !defined NDEBUG
      //std::cout << "ngrs: visual class: true-color, bypassing color map" << std::endl;
    #endif
    visual_ = visualList[0].visual;
    isTrueColor_ = true;
      #if !defined NDEBUG
        //std::cout << "ngrs: visual class: initializing color converter ... ";
      #endif
      color_converter_ = color_converter(visual_->red_mask, visual_->green_mask, visual_->blue_mask);
      #if !defined NDEBUG
        //std::cout << "ok." << std::endl;
      #endif

    colormap_ = XCreateColormap (dpy(), rootWindow(),visualList[0].visual, AllocNone);
  }
  else
  {
    // use colormap
    #if !defined NDEBUG
      std::cout << "ngrs: visual class: not true-color, using color map" << std::endl;
    #endif
  }

  XFree(visualList);
  #endif
}

bool NSystem::isTrueColor()
{
  return isTrueColor_;
}

bool NSystem::propertysActive( )
{
  return true;
}

#ifdef __unix__
MWMHints NSystem::getMotifHints( Window win ) const
{
   MWMHints hint;
   Atom type;
   int format;
   unsigned long count,bytes;
   unsigned char* data = 0;
   if ((XGetWindowProperty(dpy(), win , atoms().wm_motif_hint(), 0, 5, false, atoms().wm_motif_hint(), &type, &format, &count, &bytes, &data) == Success) && (type==atoms().wm_motif_hint()) && format == 32 && count >= 5) {
     hint = *(MWMHints*) data;
   } else {
     hint.flags = 0L;
     hint.functions = MWM_FUNC_ALL;
     hint.input_mode = 0L;
     hint.status = 0L;
   }

   if (data) XFree(data);

   return hint;
}

void NSystem::setMotifHints( Window win, MWMHints hints )
{
  if (hints.flags != 0l) {
        XChangeProperty(dpy(), win,atoms().wm_motif_hint(), atoms().wm_motif_hint(), 32,
                        PropModeReplace, (unsigned char *) &hints, 5);
    } else {
        XDeleteProperty(dpy(), win, atoms().wm_motif_hint());
    }
}

void NSystem::setMotifModalMode(Window win)
{
  MWMHints hints = getMotifHints(win);
  hints.input_mode = MWM_INPUT_PRIMARY_APPLICATION_MODAL;
  hints.flags |= MWM_HINTS_INPUT_MODE;
  setMotifHints(win,hints);
}
#endif

void NSystem::setModalMode( WinHandle win )
{
  #ifdef __unix__
  setMotifModalMode(win);
  Atom data[3];
  data[0] = atoms().net_wm_state_modal();
  data[1] = atoms().net_wm_state_above();
  data[2] = atoms().net_wm_state_stays_on_top();

  XChangeProperty(dpy_, win, atoms().net_wm_state(), XA_ATOM, 32, PropModeReplace, (unsigned char *) data, 3);
  #endif
}

#ifdef __unix__
const NAtoms & NSystem::atoms( ) const
{
  return *atoms_;
}
#endif

void NSystem::setFocus( NWindow* window )
{
  #ifdef __unix__
  if ( window->mapped() ) {
     XSetInputFocus(dpy(), window->win(), RevertToParent, CurrentTime);
  }
  #endif
}

NClipBoard & NSystem::clipBoard( )
{
  return clipBoard_;
}

const NClipBoard & NSystem::clipBoard( ) const
{
  return clipBoard_;
}
