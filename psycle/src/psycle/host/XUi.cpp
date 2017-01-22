#ifdef __linux__
#include "XUi.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace psycle {
namespace host {
namespace ui {
namespace x {
	
 Display* display() {
    Display* dpy(0);
    App* app = Systems::instance().app();
    if (app) {
	AppImp* app_imp = dynamic_cast<ui::x::AppImp*>(app->imp());
	dpy = app_imp->dpy();
    }
    return dpy;
}	

AppImp* app_imp() {    
    AppImp* app_imp(0);
    App* app = Systems::instance().app();
    if (app) {
	app_imp = dynamic_cast<ui::x::AppImp*>(app->imp());	
    }
    return app_imp;
}	
	
::Window DummyWindow::dummy_wnd_(0);	
	
::Window DummyWindow::dummy() {
  if (!dummy_wnd_) {   
    Display* dpy(display());
    int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
    int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));	
     dummy_wnd_ = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 
				     200, 200, 0, blackColor, blackColor);     
   }
   return dummy_wnd_;
}

GraphicsImp::GraphicsImp(::Window w) {   
   Display* dpy(display());
   int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));		   
   x::FontImp* imp = dynamic_cast<x::FontImp*>(font_.imp());
   assert(imp);		
   XGCValues gr_values;
   gr_values.font = imp->xfont()->fid;
   gr_values.foreground = whiteColor;	   
   gc_ = XCreateGC(dpy, w, GCFont+GCForeground, &gr_values);   
   w_ = w;
}

void GraphicsImp::Init() {

}

void GraphicsImp::DevSetColor(ARGB color) {      
  //if (argb_color_ != color) {
  argb_color_ = color;
  AppImp* app_i = app_imp();
  assert(app_i);
  Display* dpy(app_i->dpy());
  Colormap screen_colormap = app_i->default_colormap();
  int red = color & 0xff;
  int green = (color >> 8) & 0xff;
  int blue = (color >>16) & 0xff;
  XColor x_color;
  x_color.red = 0xFF * red;
  x_color.green = 0xFF * green;
  x_color.blue = 0xFF * blue;
  Status rc = XAllocColor(dpy, screen_colormap, &x_color);
  XSetForeground(dpy, gc_, x_color.pixel);
  XFlush(display());  
  //}
}

void GraphicsImp::DevFillRect(const Rect& rect) {   
   XFillRectangle(display(), w_, gc_, rect.left(), rect.top(), rect.width(), rect.height());   
   XFlush(display());
}

RegionImp::RegionImp() { 
  rgn_ = XCreateRegion();  
  XRectangle rect;
  memset(&rect, 0, sizeof(XRectangle));	
  XUnionRectWithRegion(&rect, rgn_, rgn_);
}
  
RegionImp::RegionImp(const ::Region& rgn) {
  rgn_ = XCreateRegion();  	  
  XUnionRegion(rgn, rgn_, rgn_);
}

RegionImp::~RegionImp() {
   XDestroyRegion(rgn_);
}

RegionImp* RegionImp::DevClone() const {
  return new RegionImp(rgn_);  
}
  
void RegionImp::DevOffset(double dx, double dy) {  
   XOffsetRegion(rgn_, dx, dy);
}

 void RegionImp::DevUnion(const Region& other) {
   x::RegionImp* imp = dynamic_cast<x::RegionImp*>(other.imp());
   assert(imp);
   XUnionRegion(rgn_,  imp->xrgn(), rgn_);
}

ui::Rect RegionImp::DevBounds() const { 
  XRectangle rc;  
  XClipBox(rgn_, &rc);
  return ui::Rect(ui::Point(rc.x, rc.y), ui::Dimension(rc.width, rc.height));
}

bool RegionImp::DevIntersect(double x, double y) const {
  return XPointInRegion(rgn_, static_cast<int>(x), static_cast<int>(y)) != 0;      
}

bool RegionImp::DevIntersectRect(const ui::Rect& rect) const {  	
  return XRectInRegion(rgn_, rect.left(), rect.top(), rect.width(), rect.height()) != RectangleOut;
}
  
void RegionImp::DevSetRect(const ui::Rect& rect) {
  XDestroyRegion(rgn_);	
  rgn_ = XCreateRegion();  
  XRectangle rc;
  rc.x = rect.left();
  rc.y = rect.top();
  rc.width = rect.width();
  rc.height = rect.height();  
  XUnionRectWithRegion(&rc, rgn_, rgn_);
}

void RegionImp::DevClear() {
  DevSetRect(ui::Rect());  
}
	
AppImp::AppImp() {
    dpy_ = XOpenDisplay(NIL);     
    if (dpy_ == 0) {
      printf("Cannot open X Display. Abort App.\n");
      exit(EXIT_FAILURE);
    }    
    default_colormap_ = DefaultColormap(dpy_, DefaultScreen(dpy_));  
}

AppImp::~AppImp() {
  XCloseDisplay(dpy_);
}

void AppImp::DevRun() {
   XEvent e;   
   Display* dpy(display());
   while(1) {
     XNextEvent(dpy, &e);
      /* draw or redraw the window */
     std::cout << e.xany.window << "," << std::endl;
     if(e.type==Expose) {	
        ui::WindowImp* window_imp = find(e.xany.window);
	if (window_imp) {          
          ui::Graphics g(new ui::x::GraphicsImp(e.xany.window));
	  ui::Region draw_rgn(0); //new ui::mfc::RegionImp(rgn));    
          assert(window_imp->window());
          window_imp->window()->DrawBackground(&g, draw_rgn);    
	  // g.Translate(Point(padding_.left() + border_space_.left(),
          // padding_.top() + border_space_.top()));
	  window_imp->OnDevDraw(&g, draw_rgn);
	}       
     }
     /* exit on key press */
     if(e.type==KeyPress) {	     
       break;
     }
     if (e.type == ButtonPress) {
        ui::WindowImp* window_imp = find(e.xany.window);
	if (window_imp && window_imp->window()) {
          MouseEvent ev(ui::Point(0, 0), 1, 0);
	  window_imp->OnDevMouseDown(ev);
	}       
     }
     if(e.type == ButtonRelease) {
        ui::WindowImp* window_imp = find(e.xany.window);
	if (window_imp && window_imp->window()) {
          MouseEvent ev(ui::Point(0, 0), 1, 0);
	  window_imp->OnDevMouseUp(ev);
	}
     }
     // Handle Windows Close Event
     if(e.type==ClientMessage)
        break;
   }
}

ui::WindowImp* AppImp::find(::Window w) {    
   std::map< ::Window, ui::WindowImp*>::iterator it = windows_.find(w);
   return it != windows_.end() ? it->second : 0;   	
}

void GraphicsImp::DevDispose() {
  if (gc_ != NIL) {    
    XFreeGC(display(), gc_);
  }
}

template<typename I>
void WindowTemplateImp<I>::dev_set_parent(Window*  parent) {	  
    Display* dpy(display());
    if (parent && parent->imp()) {        
      ::Window parent_win = dynamic_cast<WindowImp*>(parent->imp())->x_window();      
      ui::Rect pos = dev_position();
      XReparentWindow(dpy, x_window_, parent_win, pos.left(), pos.top());
      XMapWindow(dpy, x_window_); 
    } else {
      XReparentWindow(dpy, x_window_, DummyWindow::dummy(), 0, 0);    
      XMapWindow(dpy, x_window_); 
    }
  }
  
template<typename I>
void WindowTemplateImp<I>::dev_set_position(const ui::Rect& pos) {
  ui::Point top_left = pos.top_left();
  top_left.offset(margin_.left(), margin_.top());
  /*if (window() && window()->parent()) {
	  top_left.offset(window()->parent()->border_space().left() +
			                window()->parent()->padding().left(),
									  window()->parent()->border_space().top() +
			                window()->parent()->padding().top());
  }*/
  Display* dpy(display());  
  XMoveResizeWindow(dpy,
	                          x_window_,
                                  static_cast<int>(top_left.x()),
		                  static_cast<int>(top_left.y()),
		                  static_cast<int>(pos.width() + padding_.width() + border_space_.width()),
		                  static_cast<int>(pos.height() + padding_.height() + border_space_.height()));      
}

template<typename I>
ui::Rect WindowTemplateImp<I>::dev_position() const {
   Display* dpy(display());	
   int x, y;
   Window child;	
   XWindowAttributes xwa;
   // XTranslateCoordinates( dpy, window, root_window, 0, 0, &x, &y, &child );
   XGetWindowAttributes( dpy, x_window_, &xwa );	
   return ui::Rect(ui::Point(xwa.x, xwa.y), ui::Dimension(xwa.width, xwa.height));	
}

WindowImp::WindowImp(::Window parent) {   	
   Display* dpy(display());
   int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
   int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));	
   x_window_ = XCreateSimpleWindow(dpy, parent, 0, 0, 
       200, 200, 0, blackColor, blackColor);   
   XSelectInput(dpy, x_window_, StructureNotifyMask | ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
   App* app = Systems::instance().app();
   if (app) {
     AppImp* app_imp = dynamic_cast<ui::x::AppImp*>(app->imp());
     app_imp->RegisterWindow(x_window_, this);
   }
}

ButtonImp::ButtonImp(::Window parent) : checked_(false) {   	
   Display* dpy(display());
   int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
   int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));	
   x_window_ = XCreateSimpleWindow(dpy, parent, 0, 0, 
       200, 200, 0, blackColor, blackColor);   
   XSelectInput(dpy, x_window_, StructureNotifyMask | ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
   App* app = Systems::instance().app();
   if (app) {
     AppImp* app_imp = dynamic_cast<ui::x::AppImp*>(app->imp());
     app_imp->RegisterWindow(x_window_, this);
   }
}

void ButtonImp::OnDevDraw(Graphics* g, Region& draw_region) {
   if (!checked_) {	
     g->set_color(0xFFFFFFFF);
   } else {
     g->set_color(0xFFAAAAAA);
   }
   g->FillRect(ui::Rect(ui::Point(), ui::Dimension(100, 20)));	
}

void ButtonImp::OnDevMouseDown(MouseEvent& ev) {
  checked_ = true;	
  ui::Graphics g(new ui::x::GraphicsImp(x_window_));
  ui::Region draw_rgn(0);
  OnDevDraw(&g, draw_rgn);	  
}

void ButtonImp::OnDevMouseUp(MouseEvent& ev) {
  checked_ = false;	
  ui::Graphics g(new ui::x::GraphicsImp(x_window_));
  ui::Region draw_rgn(0);
  OnDevDraw(&g, draw_rgn);
}

FrameImp::FrameImp() {
   Display* dpy(display());
   int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
   int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));	
   x_window_ = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 
       200, 100, 0, blackColor, blackColor);   
   XSelectInput(dpy, x_window_, StructureNotifyMask | ExposureMask | KeyPressMask);
   App* app = Systems::instance().app();
   if (app) {
     AppImp* app_imp = dynamic_cast<ui::x::AppImp*>(app->imp());
     app_imp->RegisterWindow(x_window_, this);
   }
}

void FrameImp::DevShow() {
    Display* dpy(display());    
    XMapWindow(dpy, x_window_);    
    // Wait for the MapNotify event
    for(;;) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    if (e.type == MapNotify)
		  break;
    }    
    XFlush(dpy);
}

void FrameImp::dev_set_viewport(const ui::Window::Ptr& viewport) {
    /*if (viewport_) {
      CWnd* wnd = dynamic_cast<CWnd*>(viewport_->imp());
      ::Window wnd = dynamic_cast<WindowImp*>(viewport_->imp())->x_window();	    
	    
      if (wnd) {
        wnd->SetParent(DummyWindow::dummy());
      }
    }*/
    if (viewport) {
      ::Window wnd = dynamic_cast<x::WindowImp*>(viewport->imp())->x_window();      
      if (wnd) {        
        XReparentWindow(display(), wnd, x_window_, 0, 0);
      }
    }
    //viewport_ = viewport;
  }

ui::WindowImp* ImpFactory::CreateWindowImp() {	   
  ui::WindowImp* imp = WindowImp::Make(0, DummyWindow::dummy());  	
  return imp;
}

template class WindowTemplateImp<ui::WindowImp>;
template class WindowTemplateImp<ui::ButtonImp>;

} // namespace x
} // namespace ui
} // namespace host
} // namespace psycle
#endif
