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

class LuaFrameWnd : public ui::canvas::CanvasFrameWnd { 
 public:
   static std::string type() { return "canvasframe"; }
   typedef boost::shared_ptr<LuaFrameWnd> Ptr;
   LuaFrameWnd(lua_State* state) : CanvasFrameWnd(), L(state) {}  //Use OnCreate.
   virtual int OnFrameClose();   
 private:
   lua_State* L;
};


struct LuaFrameWndBind {
  typedef LuaFrameWnd T;
  static std::string meta;
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int show(lua_State* L);
  static int hide(lua_State* L);
  static int setcanvas(lua_State* L);
  static int settitle(lua_State* L)  { LUAEXPORT(L, &T::SetTitle); }
  static int setpos(lua_State* L) { LUAEXPORT(L, &T::SetPos); }
  static bool mfcclosing;
};


class LuaCanvas : public ui::canvas::Canvas {
 public:
  typedef boost::shared_ptr<LuaCanvas> Ptr;
  typedef boost::weak_ptr<LuaCanvas> WeakPtr;

  static std::string type() { return "canvas"; }
  
  LuaCanvas(lua_State* state);

  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
  virtual void OnSize(int cw, int ch) {
    ui::canvas::Canvas::OnSize(cw, ch);    
    if (lua_in_.open("onsize")) {
      lua_in_ << cw << ch << pcall(0);
      lua_in_.close();
    }
  }
 private:
  lua_State* L;
  LuaImport lua_in_;
};


class LuaItem : public ui::canvas::Item {
 public:
  static std::string type() { return "canvasitem"; }
  LuaItem(lua_State* L);  
  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
  virtual void Draw(ui::Graphics* g, ui::Region& draw_rgn);  
  virtual void onupdateregion();
  virtual void OnMessage(ui::canvas::CanvasMsg msg) {
    if (msg == ui::canvas::ONWND) { }
  }

 private:
  lua_State* L;
  LuaImport lua_in_;
};

class LuaGroup : public ui::canvas::Group {
 public:
  static std::string type() { return "canvasgroup"; }
  LuaGroup(lua_State* state) : ui::canvas::Group(), L(state) { }  
  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
 private:
    lua_State* L;
};

class LuaRect : public ui::canvas::Rect {
 public:
  static std::string type() { return "canvasrect"; }
  LuaRect(lua_State* state) : ui::canvas::Rect(), L(state) {}    
  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

class LuaLine : public ui::canvas::Line {
 public:
  static std::string type() { return "canvasline"; }
  LuaLine(lua_State* state) : ui::canvas::Line(), L(state) {}  
  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

class LuaText : public ui::canvas::Text {
 public:
  static std::string type() { return "canvastext"; }
  LuaText(lua_State* state) : ui::canvas::Text(), L(state) {}  
  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
 private:
  lua_State* L;
};

class LuaPic : public ui::canvas::Pic {
 public:
  static std::string type() { return "picitem"; }
  LuaPic(lua_State* state) : ui::canvas::Pic(), L(state) {}  
  virtual ui::canvas::Item::WeakPtr OnEvent(ui::canvas::Event* ev);
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

class LuaRegion {
 public:
  LuaRegion(ui::Region* rgn, bool shared=false) : rgn_(rgn), shared_(shared)  { }
  LuaRegion(const ui::Region& rgn, bool shared=false) : shared_(shared)  { 
    rgn_ = rgn.Clone();
  }
  ~LuaRegion() {
    if (!shared_) {
      delete rgn_;
    }
  }
  ui::Region* get() { return rgn_; }
 private:
  ui::Region* rgn_;
  bool shared_;  
};

struct LuaRegionBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int setrect(lua_State *L);
  static int boundrect(lua_State *L);
  static int combine(lua_State *L);
  static int offset(lua_State *L);
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
  LuaButton(lua_State* state) : ui::canvas::Button(), L(state) { }  
  virtual void OnClick();
  
 private:
   lua_State* L;
};

class LuaScintilla : public ui::canvas::Scintilla {
 public:
  static std::string type() { return "scintillaitem"; }
  LuaScintilla(lua_State* state) : ui::canvas::Scintilla(), L(state), 
      last_modified_(false) { }
  virtual void OnFirstModified();  
 private:
   bool last_modified_;
   lua_State* L;
};

class LuaEdit : public ui::canvas::Edit {
 public:
  static std::string type() { return "edititem"; }
  LuaEdit(lua_State* state) : ui::canvas::Edit(), L(state) {}  
 private:
   lua_State* L;
};

class LuaScrollBar : public ui::canvas::ScrollBar {
 public:
  static std::string type() { return "buttonitem"; }
  LuaScrollBar(lua_State* state) : ui::canvas::ScrollBar(), L(state) {}  
  virtual void OnScroll(int pos);  
 private:
   lua_State* L;
};

template<class T = LuaItem>
class LuaItemBind {
 public:
  static const std::string meta;
  typedef LuaItemBind B;
  static int open(lua_State *L) {
    return LuaHelper::openex<B>(L, meta, setmethods, gc);
  }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setpos", setpos},
      {"setblitpos", setblitpos},
      {"pos", pos},
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
      {"tostring", tostring},
      { NULL, NULL }
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int draw(lua_State* L) { return 0; }
  static int setpos(lua_State *L) { LUAEXPORT(L, &T::SetXY) }
  static int setblitpos(lua_State *L) { LUAEXPORT(L, &T::SetBlitXY) }
  static int updateregion(lua_State *L) { LUAEXPORT(L, &T::needsupdate) }
  static int pos(lua_State *L) { LUAEXPORT(L, &T::pos) }
  static int clientpos(lua_State* L) { LUAEXPORT(L, &T::clientpos) }
  static int fls(lua_State *L);
  static int str(lua_State *L) { LUAEXPORT(L, &T::STR) }
  static int canvas(lua_State* L);
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
    LuaRegion ** ud = (LuaRegion **)lua_newuserdata(L, sizeof(LuaRegion *));
    *ud = new LuaRegion(rgn, false);
    luaL_setmetatable(L, LuaRegionBind::meta);
    return 1;
  }
  static int drawregion(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    std::auto_ptr<ui::Region> rgn = item->draw_region();    
    if (rgn.get()) {
      LuaRegion ** ud = (LuaRegion **)lua_newuserdata(L, sizeof(LuaRegion *));      
      *ud = new LuaRegion(rgn.get(), false);      
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
    LuaRegion ** ud = (LuaRegion **)lua_newuserdata(L, sizeof(LuaRegion *));      
    *ud = new LuaRegion(item->clip(), false);            
    luaL_setmetatable(L, LuaRegionBind::meta);    
    return 1;
  }
   static int tostring(lua_State* L) {
    lua_pushstring(L, T::type().c_str());
    return 1;
  }
};

template <class T>
const std::string LuaItemBind<T>::meta = T::type();

template <class T = LuaGroup>
class LuaGroupBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
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
  typedef LuaItemBind<T> B;  
  static int open(lua_State *L); // { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
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
  static int setcursor(lua_State* L);
  static int showscrollbar(lua_State* L);
  static int setscrollinfo(lua_State* L);
};


template<class T = LuaRect>
class LuaRectBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { 
    return LuaHelper::openex<B>(L, meta, setmethods, gc);
  }
  static int setmethods(lua_State* L) {
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
  static int pos(lua_State *L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    boost::shared_ptr<T> rect = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, rect->x());
    lua_pushnumber(L, rect->y());
    lua_pushnumber(L, rect->width());
    lua_pushnumber(L, rect->height());
    return 4;
  }
  // fill
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
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
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
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
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
  static int settext(lua_State* L) { LUAEXPORT(L, &T::SetText); }
  static int text(lua_State* L) { LUAEXPORT(L, &T::text); }
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::SetColor); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::color); } 
};

template <class T = LuaPic>
class LuaPicBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
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
  static int setsource(lua_State* L) { LUAEXPORT(L, &T::SetSource); }
  static int setsize(lua_State* L) { LUAEXPORT(L, &T::SetSize); }
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
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
};

template <class T = LuaScintilla>
class LuaScintillaBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {      
       {"f", f},       
       {"loadfile", loadfile},
       {"savefile", savefile},
       {"filename", filename},
       {"hasfile", hasfile},
       {"setpos", setpos},
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
    (n == 2) ? LuaHelper::bind(L, meta, &T::SetXY)
             : LuaHelper::bind(L, meta, &T::SetPos);               
    return LuaHelper::chaining(L);
  }

  static int clear(lua_State *L) { LUAEXPORT(L, &T::Clear);}  
  static int loadfile(lua_State *L) { LUAEXPORT(L, &T::LoadFile); }
  static int savefile(lua_State *L) { LUAEXPORT(L, &T::SaveFile); }
  static int filename(lua_State *L) { LUAEXPORT(L, &T::filename); }  
  static int hasfile(lua_State *L) { LUAEXPORT(L, &T::has_file); }
};

template <class T = LuaEdit>
class LuaEditBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
       {"settext", settext},
       {"gettext", settext},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int settext(lua_State *L) { LUAEXPORT(L, &T::SetText); }
  static int gettext(lua_State *L) { LUAEXPORT(L, &T::GetText); }
};

template <class T = LuaScrollBar>
class LuaScrollBarBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
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
  static int setpos(lua_State *L) {
    const int n = lua_gettop(L);
    if (n==2) {
      LUAEXPORT(L, &T::SetXY)
    } else {
      LUAEXPORT(L, &T::SetPos);
    }
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

} // namespace host
} // namespace psycle