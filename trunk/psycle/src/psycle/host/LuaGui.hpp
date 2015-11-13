// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Machine.hpp"
#include "Canvas.hpp"
#include "InputHandler.hpp"
#include "lua.hpp"
#include "LuaHelper.hpp"

namespace psycle { 
namespace host {

class LuaMachine;

struct LuaLock {
  static void lock(lua_State* L);
  static void unlock(lua_State* L);
};

struct LuaMenuBarBind {
  static int open(lua_State *L);
  static const char* meta;
private:
  static int create(lua_State *L);
  static int add(lua_State *L);
  static int gc(lua_State* L);
};

struct LuaMenuBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int add(lua_State *L);
  static int remove(lua_State *L);
  static int addseparator(lua_State *L);
  static int setlabel(lua_State *L);
  static int gc(lua_State* L);
};

struct LuaMenuItemBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int id(lua_State *L);
  static int setlabel(lua_State *L);
  static int label(lua_State *L);
  static int gc(lua_State* L);
  static int check(lua_State* L);
  static int uncheck(lua_State* L);
  static int checked(lua_State* L);
  static int addlistener(lua_State* L);
  static int notify(lua_State* L);
};

class LuaActionListener : public ActionListener {
 public:
  LuaActionListener(lua_State* state) : ActionListener(), L(state), mac_(0) {}
  virtual void OnNotify(ActionType action);
  void setmac(LuaMachine* mac) { mac_ = mac; }
 private:
  lua_State* L;
  LuaMachine* mac_;
};

struct LuaActionListenerBind {
  static const char* meta;
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int gc(lua_State* L);
};

class LuaFrameWnd : public ui::canvas::CanvasFrameWnd {
 public:
   LuaFrameWnd(lua_State* state) : CanvasFrameWnd(), L(state) {}  //Use OnCreate.
   virtual void OnEvent(int msg);
 private:
   lua_State* L;
};

struct LuaFrameWndBind {
  static const char* meta;
  static int open(lua_State *L);  
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int show(lua_State* L);
  static int hide(lua_State* L);  
  static int setcanvas(lua_State* L);
  static int settitle(lua_State* L);
  static int setpos(lua_State* L);
};

class LuaCanvas : public ui::canvas::Canvas {
 public:
  LuaCanvas(lua_State* state);
  virtual ui::canvas::Item* OnEvent(ui::canvas::Event* ev);
  virtual void OnSize(int cx, int cy) {
    LuaLock::lock(L);    
    lua_pushnumber(L, cx);
    lua_pushnumber(L, cy);
    try {
      LuaHelper::send_event(L, this, "onsize", 2, true);
    } catch (std::exception& e) {
      LuaLock::unlock(L); 
      throw psycle::host::exceptions::library_error::runtime_error(std::string(e.what()));
    }  
    LuaLock::unlock(L);
  }
 private:
  lua_State* L;
  LuaMachine* mac_;
};

class LuaItem : public ui::canvas::Item {  
 public:
  static std::string type() { return "canvasitem"; }
  LuaItem(lua_State* state) : ui::canvas::Item(), L(state) { }
  LuaItem(lua_State* state, ui::canvas::Group* parent) :
      ui::canvas::Item(parent), L(state) {    
  }
  virtual bool OnEvent(ui::canvas::Event* ev);
  virtual void Draw(ui::Graphics* g, ui::Region& repaint_region,
    ui::canvas::Canvas* widget);
  virtual void onupdateregion();
  virtual void OnMessage(ui::canvas::CanvasMsg msg) {
    if (msg == ui::canvas::ONWND) { }
  }  
 private:  
  lua_State* L;  
};

class LuaGroup : public ui::canvas::Group {
 public:
  static std::string type() { return "canvasgroup"; }
  LuaGroup(lua_State* state) : ui::canvas::Group(), L(state) {}
  LuaGroup(lua_State* state, ui::canvas::Group* parent) :
    ui::canvas::Group(parent, 0, 0), L(state) {}
  virtual bool OnEvent(ui::canvas::Event* ev);
 private:
    lua_State* L;
};

class LuaRect : public ui::canvas::Rect {
 public:
  static std::string type() { return "canvasrect"; }
  LuaRect(lua_State* state) : ui::canvas::Rect(), L(state) {}
  LuaRect(lua_State* state, ui::canvas::Group* parent) :
    ui::canvas::canvas::Rect(parent), L(state) {}
  virtual bool OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

class LuaLine : public ui::canvas::Line {
 public:
  static std::string type() { return "canvasline"; }
  LuaLine(lua_State* state) : ui::canvas::Line(), L(state) {}
  LuaLine(lua_State* state, ui::canvas::Group* parent) :
    ui::canvas::Line(parent), L(state) {}
  virtual bool OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

class LuaText : public ui::canvas::Text {
 public:
  static std::string type() { return "canvastext"; }
  LuaText(lua_State* state) : ui::canvas::Text(), L(state) {}
  LuaText(lua_State* state, ui::canvas::Group* parent) :
    ui::canvas::Text(parent), L(state) {}
  virtual bool OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

class LuaPic : public ui::canvas::Pic {
 public:
  static std::string type() { return "picitem"; }
  LuaPic(lua_State* state) : ui::canvas::Pic(), L(state) {}
  LuaPic(lua_State* state, ui::canvas::Group* parent) :
  ui::canvas::Pic(parent), L(state) {}
  virtual bool OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

struct LuaImageBind {
  static int open(lua_State *L);
  static const char* meta;   
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int size(lua_State* L);
  static int load(lua_State *L);
  static int settransparent(lua_State* L);
};

struct LuaRegionBind {
  static int open(lua_State *L);
  static const char* meta;  
  static int create(lua_State *L);
  static int setrect(lua_State *L);
  static int boundrect(lua_State *L);
  static int gc(lua_State* L);
};

struct LuaGraphicsBind {
  static int open(lua_State *L);
  static int translate(lua_State *L);
  static int setcolor(lua_State* L);
  static int color(lua_State* L);
  static int drawline(lua_State *L);
  static int drawrect(lua_State *L);
  static int drawroundrect(lua_State *L);
  static int drawoval(lua_State* L);
  static int fillrect(lua_State *L);
  static int fillroundrect(lua_State *L);
  static int filloval(lua_State* L);
  static int copyarea(lua_State* L);
  static int drawstring(lua_State* L);
  static int setfont(lua_State* L);
  static int font(lua_State* L);
  static int drawpolygon(lua_State* L);
  static int fillpolygon(lua_State* L);
  static int drawpolyline(lua_State* L);
  static int drawimage(lua_State* L);
  static const char* meta;  
  static int gc(lua_State* L);
};

class LuaButton : public ui::canvas::Button {
 public:
  static std::string type() { return "buttonitem"; }
  LuaButton(lua_State* state) : ui::canvas::Button(), L(state) {}
  LuaButton(lua_State* state, ui::canvas::Group* parent) 
    : ui::canvas::Button(parent), L(state) {}
  virtual void OnClick() {
    LuaLock::lock(L);
    lua_newtable(L);
    LuaHelper::send_event(L, this, "onclick");
    LuaLock::unlock(L);
  }
 private:
   lua_State* L;
};

class LuaEdit : public ui::canvas::Edit {
 public:
  static std::string type() { return "edititem"; }
  LuaEdit(lua_State* state) : ui::canvas::Edit(), L(state) {}
  LuaEdit(lua_State* state, ui::canvas::Group* parent) 
    : ui::canvas::Edit(parent), L(state) {}  
 private:
   lua_State* L;
};

class LuaScrollBar : public ui::canvas::ScrollBar {
 public:
  static std::string type() { return "buttonitem"; }
  LuaScrollBar(lua_State* state) : ui::canvas::ScrollBar(), L(state) {}
  LuaScrollBar(lua_State* state, ui::canvas::Group* parent) 
    : ui::canvas::ScrollBar(parent), L(state) {}  
  virtual void OnScroll(int pos) {
    LuaLock::lock(L);    
    lua_pushnumber(L, pos);        
    try {
      LuaHelper::send_event(L, this, "onscroll");
    } catch (std::exception& e) {
      LuaLock::unlock(L); 
      throw psycle::host::exceptions::library_error::runtime_error(std::string(e.what()));
    }  
  }
 private:
   lua_State* L;
};

struct LuaCanvasBind {
  static const char* meta;
  static int open(lua_State *L);    
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int root(lua_State* L);
  static int size(lua_State* L);
  static int setpreferredsize(lua_State* L);
  static int preferredsize(lua_State* L);
  static int setcolor(lua_State* L);
  static int color(lua_State* L);
  static int generate(lua_State* L);
  static int setcapture(lua_State* L);
  static int releasecapture(lua_State* L);
  static int setcursor(lua_State* L);
  static int hidecursor(lua_State* L);
  static int showcursor(lua_State* L);
  static int setcursorpos(lua_State* L);
  static int showscrollbar(lua_State* L);
  static int setscrollinfo(lua_State* L);
};

template<class T = LuaItem>
class LuaItemBind {
 public:    
  static const std::string meta;
  static int open(lua_State *L) { 
    return openex<LuaItemBind>(L, meta, setmethods, gc);
  }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"new", create}, 
      {"setpos", setpos},
      {"pos", pos},
      {"clientpos", clientpos},
      {"getfocus", getfocus},
      // {"setzoom", setzoom},
      {"show", show},
      {"hide", hide},
      {"updateregion", updateregion},
      {"enablepointerevents", enablepointerevents},
      {"disablepointerevents", disablepointerevents},
//      {"tostring", tostring},
      {"parent", parent},
      {"boundrect", boundrect},
      {"canvas", canvas},    
      //{"intersect", intersect},
      {"fls", fls},
//      {"setclip", setclip},
      { NULL, NULL }
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
  static int gc(lua_State* L) { return delete_userdata<T>(L, meta.c_str()); }  
  static int draw(lua_State* L) { return 0; }
  static int setpos(lua_State *L) {
    callstrict2(L, meta.c_str(), &T::SetXY);
    return chaining(L);
  }
  static int updateregion(lua_State *L) {
    call(L, meta.c_str(), &T::needsupdate);
    return chaining(L);
  }
  static int pos(lua_State *L) {
    return get2number2<T,double>(L, meta.c_str(), &T::pos);
  }
  static int clientpos(lua_State* L) {
    return get2number2<T,double>(L, meta.c_str(), &T::clientpos);
  }  
  static int fls(lua_State *L);    
  static int canvas(lua_State* L);
  static int show(lua_State* L) {
    call(L, meta, &T::Show);
    return chaining(L);
  }
  static int hide(lua_State* L) {
    call(L, meta, &T::Hide);
    return chaining(L);
  }
  static int enablepointerevents(lua_State* L) {
    call(L, meta, &T::EnablePointerEvents);
    return chaining(L);
  }
  static int disablepointerevents(lua_State* L) {
    LuaHelper::call(L, meta, &T::DisablePointerEvents);
    return chaining(L);
  }
  static int boundrect(lua_State* L) {
    return get4numbers(L, meta, &T::GetBoundRect);
  }
  static int parent(lua_State *L);
  static int getfocus(lua_State *L) {
    T* item = LuaHelper::check<T>(L, 1, meta);
    item->GetFocus();
    return chaining(L);
  }
};

template <class T>
const std::string LuaItemBind<T>::meta = T::type()+std::string("meta");

template <class T = LuaGroup>
class LuaGroupBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"setzoom", setzoom},
      {"items", getitems},
      {"remove", remove},
      {"removeall", removeall},
      {"add", add},      
      {"tostring", tostring},      
      {"setzorder", setzorder},
      {"zorder", zorder},            
      {"intersect", intersect},      
      {"setclip", setclip},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setzoom(lua_State* L);    
  static int getitems(lua_State* L);
  static int remove(lua_State* L);
  static int removeall(lua_State* L);
  static int add(lua_State* L);
  static int gc(lua_State* L);        
  static int setzorder(lua_State* L);
  static int zorder(lua_State* L);    
  static int intersect(lua_State* L);        
  static ui::canvas::Item* test(lua_State* L, int index);    
  static int setclip(lua_State* L);
  static int tostring(lua_State* L) {
    lua_pushstring(L, "group");
    return 1;
  }
};

template<class T = LuaRect>
class LuaRectBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {      
      {"setcolor", setcolor},
      {"color", color},
      {"setstrokecolor", setstrokecolor},
      {"strokecolor", color},
      {"setpos", setpos},
      {"pos", pos},      
      {"tostring", tostring},      
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setpos(lua_State *L) {
    const int n = lua_gettop(L);
    if (n==3) {
      T* rect = LuaHelper::check<T>(L, 1, meta);
      rect->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
      return LuaHelper::chaining(L);
    } else
    if (n==5) {
      T* rect = LuaHelper::check<T>(L, 1, meta);
      double x = luaL_checknumber(L, 2);
      double y = luaL_checknumber(L, 3);
      double w = luaL_checknumber(L, 4);
      double h = luaL_checknumber(L, 5);
      rect->SetPos(x, y, w, h);
    } else {
       return luaL_error(L, "Wrong number of arguments.");
    }
    return LuaHelper::chaining(L);
  }
  static int pos(lua_State *L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    T* rect = LuaHelper::check<T>(L, 1, meta);
    lua_pushnumber(L, rect->x());
    lua_pushnumber(L, rect->y());
    lua_pushnumber(L, rect->width());
    lua_pushnumber(L, rect->height());
    return 4;
  }
  // fill
  static int setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &T::SetFillColor);
    return LuaHelper::chaining(L);
  }
  static int color(lua_State* L) {
    return LuaHelper::getnumber<LuaRect, ARGB>(L, meta, &T::fillcolor);
  }  
  static int setstrokecolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &T::SetStrokeColor);
    return LuaHelper::chaining(L);
  }
  static int strokecolor(lua_State* L) {
    return LuaHelper::getnumber<T, ARGB>(L, meta, &T::strokecolor);
  }
  static int setborder(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &T::SetBorder);    
    return LuaHelper::chaining(L);
  }
  static int border(lua_State* L) {
    return LuaHelper::get2number2<T,double>(L, meta.c_str(), &T::border);        
  }
  static int tostring(lua_State* L) {
    lua_pushstring(L, "rect");
    return 1;
  }
};


namespace {
using namespace LuaHelper;
using namespace ui;

template <class T = LuaLine>
class LuaLineBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
      static const luaL_Reg methods[] = {      
      {"setcolor", setcolor},
      {"color", color},
      {"setpoints", setpoints},
      {"points", points},
      {"setpoint", setpoint},      
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &T::SetColor);
    return LuaHelper::chaining(L);
  }
  static int color(lua_State* L) {
    return LuaHelper::getnumber<T, ARGB>(L, meta, &T::color);
  }
  static int setpoints(lua_State* L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    T* line = LuaHelper::check<T>(L, 1, meta);
    std::vector<std::pair<double, double> > pts;
    size_t len = lua_rawlen(L, 2);
    for (size_t i = 1; i <= len; ++i) {
      lua_rawgeti(L, 2, i); // GetTable
      lua_rawgeti(L, -1, 1); // get px
      double x = luaL_checknumber(L, -1);
      lua_pop(L, 1);
      lua_rawgeti(L, -1, 2); // get py
      double y = luaL_checknumber(L, -1);
      lua_pop(L,2);
      pts.push_back(std::pair<double, double>(x, y));
    }
    line->SetPoints(pts);
    return LuaHelper::chaining(L);
  }
  static int setpoint(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 4, "self, idx, x, y");
    if (err!=0) return err;
    T* line = LuaHelper::check<T>(L, 1, meta);
    double idx = luaL_checknumber(L, 2);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);
    Point pt(x, y);
    line->SetPoint(idx-1,  pt);
    return LuaHelper::chaining(L);
  }
  static int points(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    T* line = LuaHelper::check<T>(L, 1, meta);
    Points pts = line->points();
    lua_newtable(L);
    Points::iterator it = pts.begin();
    int k = 1;
    for (; it != pts.end(); ++it) {
      lua_newtable(L);
      lua_pushnumber(L, it->first);
      lua_setfield(L, -2, "x");
      lua_pushnumber(L, it->second);
      lua_setfield(L, -2, "y");
      lua_rawseti(L, -2, k++);
    }
    return 1;
  }
};

template <class T = LuaText>
class LuaTextBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
     static const luaL_Reg methods[] = {      
      {"settext", settext},
      {"text", text},
      {"setcolor", setcolor},
      {"color", color},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int settext(lua_State* L) {
    LuaHelper::callstrictstr(L, meta, &T::SetText);
    return LuaHelper::chaining(L);
  }
  static int text(lua_State* L) {
    return LuaHelper::getstring(L, meta, &T::text);
  }
  static int setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &T::SetColor);
    return LuaHelper::chaining(L);
  }
  static int color(lua_State* L) {
    return LuaHelper::getnumber<T, ARGB>(L, meta, &T::color);
  }
  static int tostring(lua_State* L) {
    lua_pushstring(L, "text");
    return 1;
  }
};

template <class T = LuaPic>
class LuaPicBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {  
    static const luaL_Reg methods[] = {      
      {"setsource", setsource},
      {"setsize", setsize},
      {"setimage", setimage},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setsource(lua_State* L) {
    callstrict2(L, meta, &T::SetSource);
    return chaining(L);
  }
  static int setsize(lua_State* L) {
    callstrict2(L, meta, &T::SetSize);
    return chaining(L);
  }
  static int setimage(lua_State* L) {
    int err = check_argnum(L, 2, "self, image");
    if (err!=0) return err;
    T* pic = check<T>(L, 1, meta);
    pic->SetImage(check<Image>(L, 2, LuaImageBind::meta));    
    return LuaHelper::chaining(L);
  }
  static int tostring(lua_State* L) {
    lua_pushstring(L, "pic");
    return 1;
  }
};

template <class T = LuaButton>
class LuaButtonBind : public LuaItemBind<T> {
 public:   
  typedef LuaItemBind<T> B;   
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {      
       {NULL, NULL}
    };    
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
};

template <class T = LuaEdit>
class LuaEditBind : public LuaItemBind<T> { 
 public:      
  typedef LuaItemBind<T> B;   
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {      
       {"settext", settext},
       {"gettext", settext},
       {NULL, NULL}
    };    
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int settext(lua_State *L) {
    callstrictstr(L, meta, &T::SetText);
    return chaining(L);
  }
  static int gettext(lua_State *L) {
    getstringbyvalue(L, meta, &T::GetText);
    return chaining(L);
  }
};

template <class T = LuaScrollBar>
class LuaScrollBarBind : public LuaItemBind<T> {
 public:   
  typedef LuaItemBind<T> B;   
  static int open(lua_State *L) { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"setpos", setpos},
      {"setscrollpos", setscrollpos},
      {"setrange", setrange},
      {"setorientation", setorientation},
      {NULL, NULL}
    };    
    luaL_setfuncs(L, methods, 0);
    const char* const e[] = {"HORZ", "VERT"};
    buildenum(L, e, 2);
    return 0;
  }
  static int setscrollpos(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &LuaScrollBar::SetScrollPos);
    return chaining(L);
  }
  static int setrange(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &LuaScrollBar::SetRange);
    return chaining(L);
  }
  static int setorientation(lua_State* L) {
    LuaHelper::callstrict1INT(L, meta, &LuaScrollBar::SetOrientation);
    return chaining(L);
  }
  static int setpos(lua_State *L) {
    const int n = lua_gettop(L);
    if (n==3) {
      T* item = check<T>(L, 1, meta);
      item->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
      return chaining(L);
    } else
    if (n==5) {
      T* item = check<T>(L, 1, meta);
      double x = luaL_checknumber(L, 2);
      double y = luaL_checknumber(L, 3);
      double w = luaL_checknumber(L, 4);
      double h = luaL_checknumber(L, 5);
      item->SetPos(x, y, w, h);
    } else {
       return luaL_error(L, "Wrong number of arguments.");
    }
    return chaining(L);
  }
};

template class LuaRectBind<LuaRect>;
template class LuaLineBind<LuaLine>;
template class LuaTextBind<LuaText>;
template class LuaPicBind<LuaPic>;
template class LuaButtonBind<LuaButton>;
template class LuaEditBind<LuaEdit>;
template class LuaScrollBarBind<LuaScrollBar>;


} // unnamed
} // namespace host
} // namespace psycle