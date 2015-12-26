// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2015 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Canvas.hpp"
#include "CanvasItems.hpp"
#include "Menu.hpp"
#include "InputHandler.hpp"
#include "LuaHelper.hpp"

namespace psycle {
namespace host {
class LuaMachine;

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
  static int addseparator(lua_State *L) { LUAEXPORTM(L, meta, &ui::Menu::addseparator); }
  static int setlabel(lua_State *L) { LUAEXPORTM(L, meta, &ui::Menu::set_label); }
  static int gc(lua_State* L);
};

struct LuaMenuItemBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int id(lua_State *L) { LUAEXPORTM(L, meta, &ui::MenuItem::id); }
  static int setlabel(lua_State *L) { LUAEXPORTM(L, meta, &ui::MenuItem::set_label); }
  static int label(lua_State *L) { LUAEXPORTM(L, meta, &ui::MenuItem::label); }
  static int gc(lua_State* L);
  static int check(lua_State* L) { LUAEXPORTM(L, meta, &ui::MenuItem::check); }
  static int uncheck(lua_State* L) { LUAEXPORTM(L, meta, &ui::MenuItem::uncheck); }
  static int checked(lua_State* L) { LUAEXPORTM(L, meta, &ui::MenuItem::checked); }
  static int addlistener(lua_State* L);
  static int notify(lua_State* L);
};

class LuaCmdDefBind {
 public:
  static int open(lua_State* L);
  static int keytocmd(lua_State* L);  
};

class LuaActionListener : public ActionListener,  public LuaState {
 public:
  LuaActionListener(lua_State* L) : ActionListener(), LuaState(L), mac_(0) { }
  virtual void OnNotify(ActionType action);
  void setmac(LuaMachine* mac) { mac_ = mac; }
 private:  
  LuaMachine* mac_;
};

struct LuaActionListenerBind {
  static const char* meta;
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int gc(lua_State* L);
};

struct LuaFileOpenBind {
  static const char* meta;
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int show(lua_State *L);
  static int filename(lua_State *L);
};

struct LuaFileSaveBind {
  static const char* meta;
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int show(lua_State *L);
  static int filename(lua_State *L);
};

struct LuaSystemMetrics {
  static int open(lua_State *L); 
  static int screensize(lua_State *L);
};

template <class T>
class CanvasItem : public T, public LuaState {
 public:    
   CanvasItem(lua_State* L) : LuaState(L), T() {}
   virtual void OnMouseDown(ui::canvas::MouseEvent& ev) {
     SendMouseEvent(L, "onmousedown", ev, *this);
   }
   virtual void OnMouseUp(ui::canvas::MouseEvent& ev) {
     SendMouseEvent(L, "onmouseup", ev, *this);
   }
   virtual void OnMouseMove(ui::canvas::MouseEvent& ev) {
     SendMouseEvent(L, "onmousemove", ev, *this);
   }
   virtual void OnMouseOut(ui::canvas::MouseEvent& ev) {
     SendMouseEvent(L, "onmouseout", ev, *this);
   }
   virtual void OnKeyDown(ui::canvas::KeyEvent& ev) {
     SendKeyEvent(L, "onkeydown", ev, *this);
   }
   virtual void OnKeyUp(ui::canvas::KeyEvent& ev) {
     SendKeyEvent(L, "onkeyup", ev, *this);
   }
   virtual void OnSize(double cw, double ch);
   virtual bool onupdateregion();

 private:
  static void SendKeyEvent(lua_State* L,
                       const::std::string method,
                       ui::canvas::KeyEvent& ev, 
                       ui::canvas::Item& item);
  void static SendMouseEvent(lua_State* L,
                       const::std::string method,
                       ui::canvas::MouseEvent& ev, 
                       ui::canvas::Item& item);

};


class LuaItem : public CanvasItem<ui::canvas::Item> {
 public:  
  LuaItem(lua_State* L) : CanvasItem<ui::canvas::Item>(L) {}
  virtual void Draw(ui::Graphics* g, ui::Region& draw_rgn); 
  virtual void OnSize(double cw, double ch);    
};

typedef CanvasItem<ui::canvas::Group> LuaGroup;
typedef CanvasItem<ui::canvas::Canvas> LuaCanvas;
typedef CanvasItem<ui::canvas::Edit> LuaEdit;
typedef CanvasItem<ui::canvas::Rect> LuaRect;
typedef CanvasItem<ui::canvas::Line> LuaLine;
typedef CanvasItem<ui::canvas::Text> LuaText;
typedef CanvasItem<ui::canvas::Pic> LuaPic;
typedef CanvasItem<ui::canvas::Tree> LuaTree;
typedef CanvasItem<ui::canvas::ComboBox> LuaComboBox;

struct LuaKeyEventBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int keycode(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::keycode); }  
  static int shiftkey(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::shiftkey); }
  static int ctrlkey(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::ctrlkey); }
  static int preventdefault(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::PreventDefault); }
};

struct LuaImageBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int size(lua_State* L) { LUAEXPORTM(L, meta, &ui::Image::size); }
  static int load(lua_State *L);
  static int settransparent(lua_State* L);
};

struct LuaRegionBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int setrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Region::SetRect); }
  static int boundrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Region::BoundRect); }
  static int combine(lua_State *L) { LUAEXPORTML(L, meta, &ui::Region::Combine); }
  static int offset(lua_State *L) LUAEXPORTML(L, meta, &ui::Region::Offset);
  static int gc(lua_State* L);
};

struct LuaGraphicsBind {
  static int open(lua_State *L);
  static int translate(lua_State *L) { LUAEXPORTML(L, meta, &ui::Graphics::Translate); }
  static int setcolor(lua_State* L) { LUAEXPORTML(L, meta, &ui::Graphics::SetColor); }
  static int color(lua_State* L) { LUAEXPORTML(L, meta, &ui::Graphics::color); }
  static int drawline(lua_State *L) { LUAEXPORTML(L, meta, &ui::Graphics::DrawLine); }
  static int drawrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Graphics::DrawRect); }
  static int drawroundrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Graphics::DrawRoundRect); }
  static int drawoval(lua_State* L) { LUAEXPORTML(L, meta, &ui::Graphics::DrawOval); }
  static int fillrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Graphics::FillRect); }
  static int fillroundrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Graphics::FillRoundRect); }
  static int filloval(lua_State* L) { LUAEXPORTML(L, meta, &ui::Graphics::FillOval); }
  static int copyarea(lua_State* L) { LUAEXPORTML(L, meta, &ui::Graphics::CopyArea); }
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

class LuaButton : public ui::canvas::Button, public LuaState {
 public:  
  LuaButton(lua_State* state) : LuaState(state) {}  
  virtual void OnClick();
};

class LuaTextTreeItem : public ui::canvas::TextTreeItem, public LuaState {
 public:  
  LuaTextTreeItem(lua_State* state) : LuaState(state) {}  
  virtual void OnClick();
};

class LuaScintilla : public CanvasItem<ui::canvas::Scintilla> {
 public:  
  LuaScintilla(lua_State* L) : CanvasItem<ui::canvas::Scintilla>(L) {}
  virtual void OnFirstModified(); 
};

class LuaFrameWnd : public CanvasItem<ui::canvas::Frame> {
 public:   
   typedef boost::shared_ptr<LuaFrameWnd> Ptr;
   LuaFrameWnd(lua_State* L) : CanvasItem<ui::canvas::Frame>(L) {}
   virtual int OnFrameClose();
};

class LuaScrollBar : public ui::canvas::ScrollBar, public LuaState {
 public:  
  LuaScrollBar(lua_State* state) : LuaState(state) {}
  virtual void OnScroll(int pos);   
};

struct LuaItemStyleBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);  
  static int setalign(lua_State* L);
  static int align(lua_State* L);
  static int setmargin(lua_State* L);
  static int margin(lua_State* L);
};

template<class T = LuaItem>
class LuaItemBind {
 public:
  static const std::string meta;  
  typedef LuaItemBind B;
  static int open(lua_State *L) {
    return LuaHelper::openex(L, meta, setmethods, gc);
  }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setpos", setpos},
      {"setsize", setsize},    
      {"setblitpos", setblitpos},
      {"pos", pos},
      {"setwidth", setwidth},
      {"setheight", setheight},
      {"width", width},
      {"height", height},
      {"clientpos", clientpos},
      {"getfocus", getfocus},
      // {"setzoom", setzoom},
      {"show", show},
      {"hide", hide},
      {"updateregion", updateregion},
      {"enablepointerevents", enablepointerevents},
      {"disablepointerevents", disablepointerevents},
      {"parent", parent},
      {"boundrect", boundrect},
      {"canvas", canvas},      
      //{"intersect", intersect},      
      {"fls", fls},
      {"str", str},
      {"region", region},
      {"drawregion", drawregion},
      {"setclip", setclip},
      {"clip", clip},
      {"setstyle", setstyle},
      {"style", style},
      {"tostring", tostring},
      {"setfillcolor", setfillcolor},
      {"fillcolor", fillcolor},
      {"updatealign", updatealign},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int draw(lua_State* L) { return 0; }
  static int setpos(lua_State *L) {
    const int n = lua_gettop(L);
    if (n==3) {
      LUAEXPORT(L, &T::SetXY)
    } else {
      LUAEXPORT(L, &T::SetPos);
    }
  }
  static int setblitpos(lua_State *L) { LUAEXPORT(L, &T::SetBlitXY) }
  static int setsize(lua_State* L) { 
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    double v1 = luaL_checknumber(L, 2);
    double v2 = luaL_checknumber(L, 3);
    std::stringstream str;
    str << "LuaBind setsize" << v1 << "," << v2 << std::endl;
		TRACE(str.str().c_str());
    LUAEXPORT(L, &T::SetSize); 
  }
  static int updateregion(lua_State *L) { LUAEXPORT(L, &T::needsupdate) }
  static int pos(lua_State *L) { LUAEXPORT(L, &T::pos) }
  static int width(lua_State *L) { LUAEXPORT(L, &T::width) }
  static int height(lua_State *L) { LUAEXPORT(L, &T::height) }
  static int setwidth(lua_State *L) { LUAEXPORT(L, &T::SetWidth) }
  static int setheight(lua_State *L) { LUAEXPORT(L, &T::SetHeight) }
  static int clientpos(lua_State* L) { LUAEXPORT(L, &T::clientpos) }
  static int setfillcolor(lua_State* L) { LUAEXPORT(L, &T::set_fill_color) }
  static int fillcolor(lua_State* L) { LUAEXPORT(L, &T::fill_color) }
  static int fls(lua_State *L);
  static int str(lua_State *L) { LUAEXPORT(L, &T::STR) }
  static int canvas(lua_State* L)  { LUAEXPORT(L, &T::root) } 
  static int show(lua_State* L) { LUAEXPORT(L, &T::Show) }
  static int hide(lua_State* L) { LUAEXPORT(L, &T::Hide) }
  static int enablepointerevents(lua_State* L) { LUAEXPORT(L, &T::EnablePointerEvents); }
  static int disablepointerevents(lua_State* L) { LUAEXPORT(L, &T::DisablePointerEvents); }
  static int boundrect(lua_State* L) { LUAEXPORT(L, &T::BoundRect); }  
  static int parent(lua_State *L);
  static int getfocus(lua_State *L) { LUAEXPORT(L, &T::GetFocus); }  
  static int region(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Region* rgn = item->region().Clone();
    ui::Region ** ud = (ui::Region **)lua_newuserdata(L, sizeof(ui::Region *));
    *ud = rgn;
    luaL_setmetatable(L, LuaRegionBind::meta);
    return 1;
  }
  static int drawregion(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    std::auto_ptr<ui::Region> rgn = item->draw_region();    
    if (rgn.get()) {
      ui::Region ** ud = (ui::Region **)lua_newuserdata(L, sizeof(ui::Region *));      
      *ud = rgn.get();     
      rgn.release();
      luaL_setmetatable(L, LuaRegionBind::meta);
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  static int setclip(lua_State* L) { LUAEXPORT(L, &T::SetClip); }
  static int clip(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Region ** ud = (ui::Region **)lua_newuserdata(L, sizeof(ui::Region *));      
    *ud = item->clip().Clone();          
    luaL_setmetatable(L, LuaRegionBind::meta);    
    return 1;
  }
  static int style(lua_State* L) {
    using namespace ui::canvas;
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    luaL_requiref(L, "psycle.ui.canvas.itemstyle", LuaItemStyleBind::open, true);
    Item::StylePtr& style = LuaHelper::new_shared_userdata<ItemStyle>(L, LuaItemStyleBind::meta, 0, 2); 
    style = item->style();
    return 1;
  }
  static int setstyle(lua_State* L) {
    using namespace ui::canvas;
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::canvas::Item::StylePtr style = 
      LuaHelper::check_sptr<ItemStyle>(L, 2, LuaItemStyleBind::meta);
    item->set_style(style);
    return LuaHelper::chaining(L);
  }
  static int tostring(lua_State* L) {
    lua_pushstring(L, T::type().c_str());
    return 1;
  }
  static int updatealign(lua_State* L) { LUAEXPORT(L, &T::UpdateAlign); }
};

template <class T>
const std::string LuaItemBind<T>::meta = T::type();

template <class T = LuaGroup>
class LuaGroupBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;  
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
      {"setzoom", setzoom},
      {"itemcount", itemcount},
      {"items", getitems},
      {"remove", remove},
      {"removeall", removeall},
      {"add", add},      
      {"setzorder", setzorder},
      {"zorder", zorder},
      {"itemindex", zorder},
      {"intersect", intersect},     
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setzoom(lua_State* L) { LUAEXPORT(L, &T::setzoom); }
  static int itemcount(lua_State* L) { LUAEXPORT(L, &T::size); }
  static int getitems(lua_State* L);
  static int remove(lua_State* L);
  static int removeall(lua_State* L);
  static int add(lua_State* L);  
  static int setzorder(lua_State* L);
  static int zorder(lua_State* L);
  static int intersect(lua_State* L);
  static ui::canvas::Item::Ptr test(lua_State* L, int index); 
};

template <class T = LuaCanvas>
class LuaCanvasBind : public LuaGroupBind<T> {
 public:
  typedef LuaGroupBind<T> B;  
  static int open(lua_State *L); // { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
      {"setcolor", setcolor},
      {"color", color},
      {"clientsize", clientsize},
      {"setpreferredsize", setpreferredsize},
      {"preferredsize", preferredsize},
      {"mousecapture", setcapture},
      {"mouserelease", releasecapture},
      {"setcursor", setcursor},
      {"showcursor", showcursor},
      {"hidecursor", hidecursor},
      {"setcursorpos", setcursorpos},
      {"showscrollbar", showscrollbar},
      {"setscrollinfo", setscrollinfo},
      {"setfocus", setfocus},
      {"invalidate", invalidate},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int clientsize(lua_State* L) { LUAEXPORT(L, &T::ClientSize); }
  static int setpreferredsize(lua_State* L) { LUAEXPORT(L, &T::setpreferredsize); }
  static int preferredsize(lua_State* L) { LUAEXPORT(L, &T::preferredsize); }
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::SetColor); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::color); }
  static int setcapture(lua_State* L) { LUAEXPORT(L, &T::SetCapture); }
  static int releasecapture(lua_State* L) { LUAEXPORT(L, &T::ReleaseCapture); }  
  static int showcursor(lua_State* L) { LUAEXPORT(L, &T::ShowCursor); }
  static int hidecursor(lua_State* L) { LUAEXPORT(L, &T::HideCursor); }  
  static int setcursorpos(lua_State* L) { LUAEXPORT(L, &T::SetCursorPos); }
  static int invalidate(lua_State* L) { LUAEXPORT(L, &T::Invalidate); }
  static int setcursor(lua_State* L);
  static int showscrollbar(lua_State* L);
  static int setscrollinfo(lua_State* L);
  static int setfocus(lua_State* L);  
};

template<class T = LuaRect>
class LuaRectBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { 
    return LuaHelper::openex(L, meta, setmethods, gc);
  }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {    
      {"setcolor", setcolor},
      {"color", color},
      {"setstrokecolor", setstrokecolor},
      {"strokecolor", color},
      {"setpos", setpos},
      {"pos", pos},      
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setpos(lua_State *L) {    
    const int n = lua_gettop(L);    
    (n == 2) ? LuaHelper::bind(L, meta, &T::SetXY)
             : LuaHelper::bind(L, meta, &T::SetPos);               
    return LuaHelper::chaining(L);
  }
  static int pos(lua_State *L) { LUAEXPORT(L, &T::pos); }
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::SetFillColor); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::fillcolor); }
  static int setstrokecolor(lua_State* L) { LUAEXPORT(L, &T::SetStrokeColor); }
  static int strokecolor(lua_State* L) { LUAEXPORT(L, &T::strokecolor); }
  static int setborder(lua_State* L) { LUAEXPORT(L, &T::SetBorder); }
  static int border(lua_State* L) { LUAEXPORT(L, &T::border); }  
};

template <class T = LuaLine>
class LuaLineBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
      B::setmethods(L);
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
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::SetColor); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::color); }
  static int setpoints(lua_State* L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    boost::shared_ptr<T> line = LuaHelper::check_sptr<T>(L, 1, meta);
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
    boost::shared_ptr<T> line = LuaHelper::check_sptr<T>(L, 1, meta);
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
    boost::shared_ptr<T> line = LuaHelper::check_sptr<T>(L, 1, meta);
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
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
     B::setmethods(L);
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
  static int settext(lua_State* L) { LUAEXPORT(L, &T::SetText); }
  static int text(lua_State* L) { LUAEXPORT(L, &T::text); }
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::SetColor); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::color); } 
};

template <class T = LuaPic>
class LuaPicBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"setsource", setsource},
      {"setimage", setimage},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setsource(lua_State* L) { LUAEXPORT(L, &T::SetSource); }
  static int setimage(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, image");
    if (err!=0) return err;
    boost::shared_ptr<T> pic = LuaHelper::check_sptr<T>(L, 1, meta);
    pic->SetImage(LuaHelper::check_sptr<Image>(L, 2, LuaImageBind::meta).get());
    return LuaHelper::chaining(L);
  }  
};

template <class T = LuaButton>
class LuaButtonBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
};

template <class T = LuaComboBox>
class LuaComboBoxBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
};

template <class T = LuaTree>
class LuaTreeBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"add", add},
      {"clear", clear},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
 
  static int add(lua_State* L) {
    boost::shared_ptr<T> tree = LuaHelper::check_sptr<T>(L, 1, meta);
    using namespace ui::canvas;
    TreeItem::Ptr item = LuaHelper::check_sptr<TextTreeItem>(L, 2, LuaTextTreeItemBind::meta);
    tree->InsertItem(item);
    return LuaHelper::chaining(L);
  }

  static int gc(lua_State* L) {
    using namespace ui::canvas;
    typedef boost::shared_ptr<T> SPtr;
    SPtr tree = *(SPtr*) luaL_checkudata(L, 1, meta.c_str());
    TreeItem::TreeItemList subitems = tree->SubChildren(); 
    TreeItem::TreeItemList::iterator it = subitems.begin();
    for ( ; it != subitems.end(); ++it) {
      TreeItem::Ptr subitem = *it;
      LuaHelper::unregister_userdata<>(L, subitem.get());
    }   
    return LuaHelper::delete_shared_userdata<T>(L, meta);
  }

  static int clear(lua_State* L) {
    using namespace ui::canvas;
    boost::shared_ptr<T> tree = LuaHelper::check_sptr<T>(L, 1, meta);
    using namespace ui::canvas;
    TreeItem::TreeItemList subitems = tree->SubChildren(); 
    TreeItem::TreeItemList::iterator it = subitems.begin();
    for ( ; it != subitems.end(); ++it) {
      TreeItem::Ptr subitem = *it;
      LuaHelper::unregister_userdata<>(L, subitem.get());
    }   
    tree->Clear();
    return LuaHelper::chaining(L);
  }
};

class LuaTextTreeItemBind {
 public:
  static std::string meta;
  static int open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"settext", settext},
      {"text", text},
      {"add", add},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    return 1;
  }

  static int create(lua_State* L) {
    LuaHelper::create<ui::canvas::TextTreeItem>(L, meta, true);    
    return 1;
  }

  static int gc(lua_State* L) {
    return LuaHelper::delete_shared_userdata<LuaTextTreeItem>(L, meta);
  }

  static int settext(lua_State* L) { LUAEXPORTM(L, meta, &LuaTextTreeItem::set_text); }  
  static int text(lua_State* L) { LUAEXPORTM(L, meta, &LuaTextTreeItem::text); } 
  static int add(lua_State* L) {
    using namespace ui::canvas;
    TreeItem::Ptr treeitem = LuaHelper::check_sptr<LuaTextTreeItem>(L, 1, meta);    
    TreeItem::Ptr item = LuaHelper::check_sptr<TextTreeItem>(L, 2, LuaTextTreeItemBind::meta);
    treeitem->InsertItem(item);
    return LuaHelper::chaining(L);
  }
};

template <class T = LuaFrameWnd>
class LuaFrameItemBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"settitle", settitle},
      {"setcanvas", setcanvas},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
 
  static int gc(lua_State* L) {
    if (!LuaFrameItemBind<T>::mfcclosing) {    
      LuaFrameWnd::Ptr wnd = *(LuaFrameWnd::Ptr*) luaL_checkudata(L, 1, meta.c_str());    
      canvas::Canvas* c = wnd->canvas().lock().get();
      if (c) {
        LuaHelper::unregister_userdata(L, c);
      }
      LuaHelper::delete_shared_userdata<LuaFrameWnd>(L, meta);
    }
    return 0;
  }
  static int setcanvas(lua_State* L) {
    LuaFrameWnd::Ptr wnd = LuaHelper::check_sptr<LuaFrameWnd>(L, 1, meta);
    ui::canvas::Canvas::WeakPtr old_canvas = wnd->canvas();
    if (!old_canvas.expired()) {
      LuaHelper::unregister_userdata(L, old_canvas.lock().get());
    }
    LuaCanvas::Ptr canvas = LuaHelper::check_sptr<LuaCanvas>(L, 2, LuaCanvasBind<>::meta);
    if (canvas) {
      LuaHelper::register_userdata(L, canvas.get());
    }
    wnd->set_canvas(canvas);
    return LuaHelper::chaining(L);
  }
  static int settitle(lua_State* L)  { LUAEXPORT(L, &T::SetTitle); }
  static bool mfcclosing;
};

template <class T = LuaScintilla>
class LuaScintillaBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
       {"f", f},       
       {"gotoline", gotoline},
       {"length", length},
       {"loadfile", loadfile},
       {"savefile", savefile},
       {"filename", filename},
       {"hasfile", hasfile},
       {"setpos", setpos},
       {"addtext", addtext},
       {"findtext", findtext},
       {"selectionstart", selectionstart},
       {"selectionend", selectionend},
       {"setsel", setsel},
       {"hasselection", hasselection},
       {"setfindwholeword", setfindwholeword},
       {"setfindmatchcase", setfindmatchcase},
       {"setfindregexp", setfindregexp},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int f(lua_State *L) {
    boost::shared_ptr<T> sc = LuaHelper::check_sptr<LuaScintilla>(L, 1, meta);    
    int sci = luaL_checknumber(L, 2);
    WPARAM wparam(0);
    LPARAM lparam(0);    
    switch (lua_type(L, 3)) {
      case LUA_TSTRING: // strings
			  wparam = (WPARAM) lua_tostring(L, 3);
			break;
			case LUA_TBOOLEAN: // booleans
				wparam = (WPARAM) lua_toboolean(L, 3);
			break;
			case LUA_TNUMBER: // numbers
				wparam = (WPARAM) lua_tonumber(L, 3);
			break;
      default:
        luaL_error(L, "Wrong argument type");
    }
    switch (lua_type(L, 4)) {
      case LUA_TSTRING: // strings
			  lparam = (LPARAM) lua_tostring(L, 4);
			break;
			case LUA_TBOOLEAN: // booleans
				lparam = (LPARAM) lua_toboolean(L, 4);
			break;
			case LUA_TNUMBER: // numbers
				lparam = (LPARAM) lua_tonumber(L, 4);
			break;
      default:
        luaL_error(L, "Wrong argument type");
    }
    int r = sc->f(sci, wparam, lparam);
    lua_pushinteger(L, r);
    return 1;
  }

  static int setpos(lua_State *L) {
    const int n = lua_gettop(L);
    (n == 3) ? LuaHelper::bind(L, meta, &T::SetXY)
             : LuaHelper::bind(L, meta, &T::SetPos);               
    return LuaHelper::chaining(L);
  }

  static int setsel(lua_State *L) { LUAEXPORT(L, &T::SetSel); } 
  static int selectionstart(lua_State *L) { LUAEXPORT(L, &T::selectionstart); } 
  static int selectionend(lua_State *L) { LUAEXPORT(L, &T::selectionend); } 
  static int hasselection(lua_State *L) { LUAEXPORT(L, &T::has_selection); } 
  static int gotoline(lua_State *L) { LUAEXPORT(L, &T::GotoLine); }  
  static int length(lua_State *L) { LUAEXPORT(L, &T::length); }  
  static int addtext(lua_State *L) { LUAEXPORT(L, &T::AddText); } 
  static int findtext(lua_State *L) { LUAEXPORT(L, &T::FindText); }  
  static int clear(lua_State *L) { LUAEXPORT(L, &T::Clear); }  
  static int loadfile(lua_State *L) { LUAEXPORT(L, &T::LoadFile); }
  static int savefile(lua_State *L) { LUAEXPORT(L, &T::SaveFile); }
  static int filename(lua_State *L) { LUAEXPORT(L, &T::filename); }  
  static int hasfile(lua_State *L) { LUAEXPORT(L, &T::has_file); }
  static int setfindmatchcase(lua_State *L) { LUAEXPORT(L, &T::set_find_match_case); }
  static int setfindwholeword(lua_State *L) { LUAEXPORT(L, &T::set_find_whole_word); }
  static int setfindregexp(lua_State *L) { LUAEXPORT(L, &T::set_find_regexp); }
};

template <class T = LuaEdit>
class LuaEditBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
       {"settext", settext},
       {"text", text},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int settext(lua_State *L) { LUAEXPORT(L, &T::SetText); }
  static int text(lua_State *L) { LUAEXPORT(L, &T::text); }
};

template <class T = LuaScrollBar>
class LuaScrollBarBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"setpos", setpos},
      {"setscrollpos", setscrollpos},
      {"scrollpos", scrollpos},
      {"setscrollrange", setscrollrange},
      {"scrollrange", scrollrange},
      {"setorientation", setorientation},
      {"systemsize", systemsize},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    const char* const e[] = {"HORZ", "VERT"};
    LuaHelper::buildenum(L, e, 2);
    return 0;
  }
  static int setscrollpos(lua_State* L) { LUAEXPORT(L, &T::SetScrollPos); }
  static int scrollpos(lua_State* L) { LUAEXPORT(L, &T::scroll_pos); }
  static int setscrollrange(lua_State* L) { LUAEXPORT(L, &T::SetScrollRange); }
  static int scrollrange(lua_State* L) { LUAEXPORT(L, &T::scroll_range); }
  static int systemsize(lua_State* L) { LUAEXPORT(L, &T::SystemSize); }
  static int setorientation(lua_State* L) {
    boost::shared_ptr<T> scrollbar = LuaHelper::check_sptr<T>(L, 1, meta);
    int val = luaL_checkinteger(L, 2);
    scrollbar->SetOrientation(ui::canvas::Orientation(val));    
    return LuaHelper::chaining(L);
  }  
};

template class LuaRectBind<LuaRect>;
template class LuaLineBind<LuaLine>;
template class LuaTextBind<LuaText>;
template class LuaPicBind<LuaPic>;
template class LuaButtonBind<LuaButton>;
template class LuaEditBind<LuaEdit>;
template class LuaScrollBarBind<LuaScrollBar>;
template class LuaScintillaBind<LuaScintilla>;
template class LuaComboBoxBind<LuaComboBox>;
template class LuaTreeBind<LuaTree>;
template class LuaFrameItemBind<LuaFrameWnd>;

} // namespace host
} // namespace psycle