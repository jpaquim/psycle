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
     if (!SendMouseEvent(L, "onmousedown", ev, *this)) {
       T::OnMouseDown(ev);
     }
   }
   virtual void OnMouseUp(ui::canvas::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmouseup", ev, *this)) {
       T::OnMouseUp(ev);
     }
   }
   virtual void OnMouseMove(ui::canvas::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmousemove", ev, *this)) {
       T::OnMouseMove(ev);
     }
   }
   virtual void OnMouseEnter(ui::canvas::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmouseenter", ev, *this)) {
       T::OnMouseEnter(ev);
     }
   }
   virtual void OnMouseOut(ui::canvas::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmouseout", ev, *this)) {
       T::OnMouseOut(ev);
     }
   }
   virtual void OnKeyDown(ui::canvas::KeyEvent& ev) {
     if (!SendKeyEvent(L, "onkeydown", ev, *this)) {
       T::OnKeyDown(ev);
     }
   }
   virtual void OnKeyUp(ui::canvas::KeyEvent& ev) {
     if (!SendKeyEvent(L, "onkeyup", ev, *this)) {
       T::OnKeyUp(ev);
     }
   }
   virtual void OnFocus(ui::canvas::Event& ev) {
     if (!SendEvent(L, "onfocus", ev, *this)) {
       T::OnFocus(ev);
     }
   }
   virtual void OnKillFocus();
   virtual void OnSize(double cw, double ch);
   virtual bool OnUpdateArea();
 
   static bool SendEvent(lua_State* L,
                         const::std::string method,
                         ui::Event& ev, 
                         ui::Window& item);
   static bool SendKeyEvent(lua_State* L,
                            const::std::string method,
                            ui::KeyEvent& ev, 
                            ui::Window& item);
   static bool SendMouseEvent(lua_State* L,
                              const::std::string method,
                              ui::MouseEvent& ev, 
                              ui::Window& item);
};


class LuaRun : public LuaState {
 public:
   LuaRun(lua_State* L) : LuaState(L) {}
  void Run();
};

struct LuaRunBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L); 
  static int gc(lua_State* L);  
};


class LuaItem : public CanvasItem<ui::Window> {
 public:  
  LuaItem(lua_State* L) : CanvasItem<ui::Window>(L) {}
  virtual void Draw(ui::Graphics* g, ui::Region& draw_rgn); 
  virtual void OnSize(double cw, double ch);  
  virtual std::string GetType() const { return "luaitem"; }
};

typedef CanvasItem<ui::Group> LuaGroup;
typedef CanvasItem<ui::canvas::Canvas> LuaCanvas;
typedef CanvasItem<ui::canvas::Rect> LuaRect;
typedef CanvasItem<ui::canvas::Line> LuaLine;
typedef CanvasItem<ui::canvas::Text> LuaText;
typedef CanvasItem<ui::canvas::Pic> LuaPic;

class LuaComboBox : public CanvasItem<ui::canvas::ComboBox> {
 public:  
  LuaComboBox(lua_State* L) : CanvasItem<ui::canvas::ComboBox>(L) {}

  void MakeImp() {
    ui::ComboBoxImp* imp  = ui::ImpFactory::instance().CreateComboBoxImp();
    this->set_imp(imp);
    imp->set_window(this);
  }
};

class LuaEdit : public CanvasItem<ui::Edit> {
 public:  
  LuaEdit(lua_State* L) : CanvasItem<ui::Edit>(L) {}

  void MakeImp() {
    ui::EditImp* imp  = ui::ImpFactory::instance().CreateEditImp();
    this->set_imp(imp);
    imp->set_window(this);
  }
};

class LuaTree : public CanvasItem<ui::Tree> {
 public:  
  LuaTree(lua_State* L) : CanvasItem<ui::Tree>(L) {}

  void MakeImp() {
    ui::TreeImp* imp  = ui::ImpFactory::instance().CreateTreeImp();
    this->set_imp(imp);
    imp->set_window(this);
    imp->set_tree(this);
  }

  virtual void OnClick(boost::shared_ptr<ui::Node> node);
};

class LuaTable : public CanvasItem<ui::Table> {
 public:  
  LuaTable(lua_State* L) : CanvasItem<ui::Table>(L) {}

  void MakeImp() {
    ui::TableImp* imp  = ui::ImpFactory::instance().CreateTableImp();
    this->set_imp(imp);
    imp->set_window(this);
  }
};

struct LuaEventBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int preventdefault(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::Event::PreventDefault); }
  static int isdefaultprevented(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::Event::is_default_prevented); }
};

struct LuaKeyEventBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int keycode(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::keycode); }  
  static int shiftkey(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::shiftkey); }
  static int ctrlkey(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::ctrlkey); }
  static int preventdefault(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::KeyEvent::PreventDefault); }
  static int isdefaultprevented(lua_State* L) { LUAEXPORTM(L, meta, &ui::canvas::Event::is_default_prevented); }
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
  static int setrect(lua_State *L) { 
    ui::Region* rgn= *(ui::Region **)luaL_checkudata(L, 1, meta);
    ui::Rect rect(ui::Point(luaL_checknumber(L, 2),
                            luaL_checknumber(L, 3)),
                  ui::Dimension(luaL_checknumber(L, 4),
                                luaL_checknumber(L, 5))
                  );
    rgn->SetRect(rect);
    return LuaHelper::chaining(L);
  }
   
  static int boundrect(lua_State *L) {
    ui::Region* rgn= *(ui::Region **)luaL_checkudata(L, 1, meta);
    ui::Rect bounds = rgn->bounds();
    lua_pushnumber(L, bounds.left());
    lua_pushnumber(L, bounds.top());
    lua_pushnumber(L, bounds.width());
    lua_pushnumber(L, bounds.height());
    return 4;
  }
  static int combine(lua_State *L) { LUAEXPORTML(L, meta, &ui::Region::Combine); }
  static int offset(lua_State *L) LUAEXPORTML(L, meta, &ui::Region::Offset);
  static int gc(lua_State* L);
};

struct LuaAreaBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  //static int setrect(lua_State *L) { LUAEXPORTML(L, meta, &ui::Area:: ::SetRect); }
  static int boundrect(lua_State *L) {
    ui::Area* rgn= *(ui::Area **)luaL_checkudata(L, 1, meta);
    ui::Rect bounds = rgn->bounds();
    lua_pushnumber(L, bounds.left());
    lua_pushnumber(L, bounds.top());
    lua_pushnumber(L, bounds.width());
    lua_pushnumber(L, bounds.height());
    return 4;
  }
  static int combine(lua_State *L) { LUAEXPORTML(L, meta, &ui::Area::Combine); }
  static int offset(lua_State *L) LUAEXPORTML(L, meta, &ui::Area::Offset);
  static int gc(lua_State* L);
};

struct LuaGraphicsBind {
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int translate(lua_State *L) { LUAEXPORTM(L, meta, &ui::Graphics::Translate); }
  static int setcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::SetColor); }
  static int color(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::color); }
  static int drawline(lua_State *L) { LUAEXPORTM(L, meta, &ui::Graphics::DrawLine); }
  static int drawrect(lua_State *L);
  static int drawroundrect(lua_State *L) { LUAEXPORTM(L, meta, &ui::Graphics::DrawRoundRect); }
  static int drawoval(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::DrawOval); }
  static int fillrect(lua_State *L) { LUAEXPORTM(L, meta, &ui::Graphics::FillRect); }
  static int fillroundrect(lua_State *L) { LUAEXPORTM(L, meta, &ui::Graphics::FillRoundRect); }
  static int filloval(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::FillOval); }
  static int copyarea(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::CopyArea); }
  static int drawstring(lua_State* L);
  static int setfont(lua_State* L);
  static int font(lua_State* L);
  static int drawpolygon(lua_State* L);
  static int fillpolygon(lua_State* L);
  static int drawpolyline(lua_State* L);
  static int drawimage(lua_State* L);
  static int textsize(lua_State* L);
  static const char* meta;
  static int gc(lua_State* L);
};

class LuaButton : public CanvasItem<ui::Button> {
 public:  
  LuaButton(lua_State* L) : CanvasItem<ui::Button>(L) {}

  void MakeImp() {
    ui::ButtonImp* imp  = ui::ImpFactory::instance().CreateButtonImp();
    this->set_imp(imp);
    imp->set_window(this);
  }

  virtual void OnClick();
};

/*class LuaTreeNode : public ui::Node, public LuaState {
 public:  
  LuaTreeNode(lua_State* state) : LuaState(state) {}  
  virtual void OnClick();
};*/

class LuaScintilla : public CanvasItem<ui::Scintilla> {
 public:  
  LuaScintilla(lua_State* L) : CanvasItem<ui::Scintilla>(L) {}

  void MakeImp() {
    ui::ScintillaImp* imp  = ui::ImpFactory::instance().CreateScintillaImp();
    this->set_imp(imp);
    imp->set_window(this);
  }

  virtual void OnFirstModified(); 
};

class LuaFrameWnd : public CanvasItem<ui::Frame> {
 public:   
   typedef boost::shared_ptr<LuaFrameWnd> Ptr;
   LuaFrameWnd(lua_State* L) : CanvasItem<ui::Frame>(L) {}
   
   void MakeImp() {
     ui::FrameImp* imp  = ui::ImpFactory::instance().CreateFrameImp();
     this->set_imp(imp);
     imp->set_window(this);
   }

   virtual void OnClose();
};

class LuaScrollBar : public ui::ScrollBar, public LuaState {
 public:  
  LuaScrollBar(lua_State* state) : LuaState(state) {}

  void MakeImp(ui::Orientation orientation) {
    ui::ScrollBarImp* imp  = ui::ImpFactory::instance().CreateScrollBarImp(orientation);
    this->set_imp(imp);
    imp->set_window(this);
  }

  virtual void OnScroll(int pos);   
};

struct OrnamentFactoryBind {
  static int open(lua_State *L);
  static std::string meta;
  static int create(lua_State *L); 
  static int createlineborder(lua_State* L);
  static int createwallpaper(lua_State* L);
  static int createfill(lua_State* L);
  static int createboundfill(lua_State* L);
};

struct LineBorderBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L); 
  static int gc(lua_State* L);
  static int setborderradius(lua_State* L);
  static int setborderstyle(lua_State* L);
};

struct WallpaperBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L); 
  static int gc(lua_State* L);  
};

struct FillBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L); 
  static int gc(lua_State* L);  
};

template<class T = LuaItem>
class LuaItemBind {
 public:
  static const std::string meta;  
  typedef LuaItemBind B;
  static int open(lua_State *L) {
    LuaHelper::openex(L, meta, setmethods, gc);
    LuaHelper::setfield(L, "ALLEFT", canvas::ALLEFT);
    LuaHelper::setfield(L, "ALTOP", canvas::ALTOP);
    LuaHelper::setfield(L, "ALRIGHT", canvas::ALRIGHT);
    LuaHelper::setfield(L, "ALBOTTOM", canvas::ALBOTTOM);  
    LuaHelper::setfield(L, "ALCLIENT", canvas::ALCLIENT);
    LuaHelper::setfield(L, "ALCENTER", canvas::ALCENTER);
    static const char* const e[] = {
		  "DOTTED", "DASHED", "SOLID", "DOUBLE", "GROOVE", "RIDGE", "INSET", 
      "OUTSET", "NONE", "HIDDEN"
	  };
    size_t size = sizeof(e)/sizeof(e[0]);    
    LuaHelper::buildenum(L, e, size); 
    return 1;
  }
  static int setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setpos", setpos},
     // {"setsize", setsize},    
     // {"setblitpos", setblitpos},
      {"pos", pos},
      {"clientpos", clientpos},
      {"desktoppos", desktoppos},
    //  {"setwidth", setwidth},
    //  {"setheight", setheight},
      {"setautosize", setautosize},
      {"setdebugtext", setdebugtext},      
      {"getfocus", getfocus},
      // {"setzoom", setzoom},
      {"show", show},
      {"hide", hide},
      {"visible", visible},
      {"updatearea", updatearea},
      {"enablepointerevents", enablepointerevents},
      {"disablepointerevents", disablepointerevents},
      {"parent", parent},
      {"bounds", bounds},
      {"canvas", canvas},      
      //{"intersect", intersect},      
      {"fls", fls},
      {"invalidate", invalidate},
      {"preventfls", preventfls},
      {"enablefls", enablefls},      
      {"isflsprevented", isflsprevented},
      {"str", str},
      {"area", area},
      {"drawregion", drawregion},
      {"setclip", setclip},
      {"clip", clip},
      {"tostring", tostring},   
      {"setornament", setornament},
      {"ornament", ornament},
      {"setcursor", setcursor},
      {"setclipchildren", setclipchildren},
      {"addstyle", addstyle},
      {"removestyle", removestyle},
      {"getfocus", getfocus},
      {"setalign", setalign},
      {"setmargin", setmargin},
      {"margin", margin},
      {"setpadding", setmargin},
      {"padding", padding},
      {"align", align},
      {"mousecapture", setcapture},
      {"mouserelease", releasecapture},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int draw(lua_State* L) { return 0; }
  static int invalidate(lua_State* L) { LUAEXPORT(L, &T::Invalidate); }
  static int setcursor(lua_State* L);
  static int setalign(lua_State* L);
  static int align(lua_State* L);
  static int setmargin(lua_State* L);
  static int margin(lua_State* L);
  static int setpadding(lua_State* L);
  static int padding(lua_State* L);

  static int addstyle(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    UINT style = (unsigned int) luaL_checknumber(L, 2);    
    item->add_style(style);
    return LuaHelper::chaining(L);
  }

  static int removestyle(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    UINT style = (unsigned int) luaL_checknumber(L, 2);    
    item->remove_style(style);
    return LuaHelper::chaining(L);
  }

  static int setpos(lua_State *L) {
    const int n = lua_gettop(L);
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    if (n==3) {
      double v1 = luaL_checknumber(L, 2);
      double v2 = luaL_checknumber(L, 3);
      item->set_pos(ui::Rect(ui::Point(v1, v2), ui::Dimension(item->pos().width(), item->pos().height())));
    } else {
      double v1 = luaL_checknumber(L, 2);
      double v2 = luaL_checknumber(L, 3);
      double v3 = luaL_checknumber(L, 4);
      double v4 = luaL_checknumber(L, 5);
      item->set_pos(ui::Rect(ui::Point(v1, v2), ui::Dimension(v3, v4)));
    }
    return LuaHelper::chaining(L);
  }
//  static int setblitpos(lua_State *L) { LUAEXPORT(L, &T::SetBlitXY) }
  /*static int setsize(lua_State* L) { 
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    double v1 = luaL_checknumber(L, 2);
    double v2 = luaL_checknumber(L, 3);
    std::stringstream str;
    str << "LuaBind setsize" << v1 << "," << v2 << std::endl;
		TRACE(str.str().c_str());
    LUAEXPORT(L, &T::SetSize); 
  }*/
  static int getfocus(lua_State *L) { LUAEXPORT(L, &T::GetFocus) }
  static int updatearea(lua_State *L) { LUAEXPORT(L, &T::needsupdate) }
  static int pos(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, item->pos().left());
    lua_pushnumber(L, item->pos().top());
    lua_pushnumber(L, item->pos().width());
    lua_pushnumber(L, item->pos().height());
    return 4;
  }  
//  static int setwidth(lua_State *L) { LUAEXPORT(L, &T::SetWidth) }
//  static int setheight(lua_State *L) { LUAEXPORT(L, &T::SetHeight) }
  static int setautosize(lua_State *L) { LUAEXPORT(L, &T::set_auto_size) }
  static int clientpos(lua_State* L) { 
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, item->abs_pos().left());
    lua_pushnumber(L, item->abs_pos().top());
    lua_pushnumber(L, item->abs_pos().width());
    lua_pushnumber(L, item->abs_pos().height());
    return 4;    
  }
  static int desktoppos(lua_State* L) { 
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, item->desktop_pos().left());
    lua_pushnumber(L, item->desktop_pos().top());
    lua_pushnumber(L, item->desktop_pos().width());
    lua_pushnumber(L, item->desktop_pos().height());
    return 4;    
  }
  static int fls(lua_State *L);
  static int preventfls(lua_State *L) { LUAEXPORT(L, &T::PreventFls) }
  static int enablefls(lua_State *L) { LUAEXPORT(L, &T::EnableFls) }
  static int isflsprevented(lua_State *L) { LUAEXPORT(L, &T::is_fls_prevented) }
  static int str(lua_State *L) { LUAEXPORT(L, &T::STR) }
  static int canvas(lua_State* L);
  static int show(lua_State* L) { LUAEXPORT(L, &T::Show) }
  static int hide(lua_State* L) { LUAEXPORT(L, &T::Hide) }
  static int enablepointerevents(lua_State* L) { LUAEXPORT(L, &T::EnablePointerEvents); }
  static int disablepointerevents(lua_State* L) { LUAEXPORT(L, &T::DisablePointerEvents); }
  static int setclipchildren(lua_State* L) { LUAEXPORT(L, &T::set_clip_children); }
  static int bounds(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Rect bounds = item->area().bounds();
    lua_pushnumber(L, bounds.left());
    lua_pushnumber(L, bounds.top());
    lua_pushnumber(L, bounds.width());
    lua_pushnumber(L, bounds.height());
    return 4;
  }  
  static int parent(lua_State *L);  
  static int area(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Area* rgn = item->area().Clone();
    ui::Area ** ud = (ui::Area **)lua_newuserdata(L, sizeof(ui::Area *));
    *ud = rgn;
    luaL_setmetatable(L, LuaAreaBind::meta);
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
  static int tostring(lua_State* L) {
    lua_pushstring(L, T::type().c_str());
    return 1;
  }
  static int setdebugtext(lua_State* L) {
    LUAEXPORT(L, &T::set_debug_text);
  }

  static int visible(lua_State* L) {
    LUAEXPORT(L, &T::visible);
  }

  static int setornament(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    boost::shared_ptr<ui::Ornament> ornament;
    if (!lua_isnil(L, 2)) {      
      ornament = LuaHelper::test_sptr<ui::canvas::LineBorder>(L, 2, LineBorderBind::meta);
      if (!ornament) {
        ornament = LuaHelper::test_sptr<ui::canvas::Wallpaper>(L, 2, WallpaperBind::meta);
        if (!ornament) {
          ornament = LuaHelper::test_sptr<ui::canvas::Fill>(L, 2, FillBind::meta);
        }
      }            
    }    
    lua_setfield(L, -2, "_ornament");
    item->set_ornament(ornament);
    return LuaHelper::chaining(L);
  }

  static int ornament(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    if (item->ornament().expired()) {
      lua_pushnil(L);
    } else {
      /*luaL_requiref(L, "psycle.ui.canvas.ornament", LuaOrnamentBind::open, true);
      int n = lua_gettop(L);                  
      ui::Ornament* ornament = item->ornament().lock()->Clone();
      LuaHelper::new_shared_userdata<>(L, LuaOrnamentBind::meta, item->ornament().lock(), n, true);     */
      lua_pushnumber(L, 1); // todo
    }
    return 1;
  }

  static int setcapture(lua_State* L) { LUAEXPORT(L, &T::SetCapture); }
  static int releasecapture(lua_State* L) { LUAEXPORT(L, &T::ReleaseCapture); }
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
      {"new", create},      
      {"itemcount", itemcount},
      {"items", getitems},
      {"remove", remove},
      {"removeall", removeall},
      {"add", add},      
      {"setzorder", setzorder},
      {"zorder", zorder},
      {"itemindex", zorder},
      {"intersect", intersect},     
      {"updatealign", updatealign},
      {"flagnotaligned", flagnotaligned},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);  
  static int itemcount(lua_State* L) { LUAEXPORT(L, &T::size); }
  static int getitems(lua_State* L);
  static int remove(lua_State* L);
  static int removeall(lua_State* L);
  static int add(lua_State* L);  
  static int setzorder(lua_State* L);
  static int zorder(lua_State* L);
  static int intersect(lua_State* L);
  static int updatealign(lua_State* L) { LUAEXPORT(L, &T::UpdateAlign); }
  static int flagnotaligned(lua_State* L) { LUAEXPORT(L, &T::FlagNotAligned); }
  static ui::Window::Ptr test(lua_State* L, int index); 
};

template <class T = LuaCanvas>
class LuaCanvasBind : public LuaGroupBind<T> {
 public:
  typedef LuaGroupBind<T> B;  
  static int open(lua_State *L); // { return openex<B>(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {  
      {"new", create},      
      {"showcursor", showcursor},
      {"hidecursor", hidecursor},
      {"setcursorpos", setcursorpos},
      {"showscrollbar", showscrollbar},
      {"setscrollinfo", setscrollinfo},
      {"invalidateontimer", invalidateontimer},
      {"invalidatedirect", invalidatedirect},
      {"setfocus", setfocus},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }      
  static int create(lua_State* L);  
  static int showcursor(lua_State* L) { LUAEXPORT(L, &T::ShowCursor); }
  static int hidecursor(lua_State* L) { LUAEXPORT(L, &T::HideCursor); }  
  static int setcursorpos(lua_State* L) { LUAEXPORT(L, &T::SetCursorPos); }    
  static int showscrollbar(lua_State* L);
  static int setscrollinfo(lua_State* L);
  static int setfocus(lua_State* L);
  static int invalidateontimer(lua_State* L);
  static int invalidatedirect(lua_State* L);
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
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
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
    ui::Points pts;
    size_t len = lua_rawlen(L, 2);
    for (size_t i = 1; i <= len; ++i) {
      lua_rawgeti(L, 2, i); // GetTable
      lua_rawgeti(L, -1, 1); // get px
      double x = luaL_checknumber(L, -1);
      lua_pop(L, 1);
      lua_rawgeti(L, -1, 2); // get py
      double y = luaL_checknumber(L, -1);
      lua_pop(L,2);
      pts.push_back(ui::Point(x, y));
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
    ui::Point pt(x, y);
    line->SetPoint(idx-1,  pt);
    return LuaHelper::chaining(L);
  }
  static int points(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    boost::shared_ptr<T> line = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Points pts = line->points();
    lua_newtable(L);
    ui::Points::iterator it = pts.begin();
    int k = 1;
    for (; it != pts.end(); ++it) {
      lua_newtable(L);
      lua_pushnumber(L, (*it).x());
      lua_setfield(L, -2, "x");
      lua_pushnumber(L, (*it).y());
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
      {"setfont", setfont},
      {"setalignment", setalignment},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int settext(lua_State* L) { LUAEXPORT(L, &T::set_text); }
  static int text(lua_State* L) { LUAEXPORT(L, &T::text); }
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::set_color); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::color); } 
  static int setfont(lua_State* L);
  static int setalignment(lua_State* L);
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
    pic->SetImage(LuaHelper::check_sptr<ui::Image>(L, 2, LuaImageBind::meta).get());
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
       {"new", create},
       {"settext", settext},
       {"text", text},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
  static int settext(lua_State *L) { LUAEXPORT(L, &T::set_text); }
  static int text(lua_State *L) { LUAEXPORT(L, &T::text); }
};

template <class T = LuaComboBox>
class LuaComboBoxBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
       {"new", create},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
};


struct LuaMenuBarBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int add(lua_State *L);  
  static int gc(lua_State* L);
  static int setrootnode(lua_State* L);  
  static int update(lua_State* L) { LUAEXPORTM(L, meta, &ui::MenuBar::Update); }
};

template <class T = LuaTree>
class LuaTreeBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setrootnode", setrootnode},
      {"addnode", addnode},
//      {"clear", clear},
      {"settextcolor", settextcolor},
      {"setbackgroundcolor", setbackgroundcolor},
      {"backgroundcolor", backgroundcolor},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  
  static int create(lua_State* L);
  static int settextcolor(lua_State* L) { LUAEXPORTM(L, meta, &T::set_text_color); } 
  static int setbackgroundcolor(lua_State* L) { LUAEXPORT(L, &T::set_background_color) }
  static int backgroundcolor(lua_State* L) { LUAEXPORT(L, &T::background_color) }

  static int setrootnode(lua_State* L) {
    boost::shared_ptr<T> tree = LuaHelper::check_sptr<T>(L, 1, meta);
    using namespace ui::canvas;
    boost::shared_ptr<Node> node = 
      boost::dynamic_pointer_cast<Node>(LuaHelper::check_sptr<ui::canvas::Node>(L, 2, LuaTreeNodeBind::meta));
    tree->set_root_node(node);
    tree->UpdateTree();
    return LuaHelper::chaining(L);
  }

  static int addnode(lua_State* L) {    
    return LuaHelper::chaining(L);
  }

  static int gc(lua_State* L) {
    // using namespace ui::canvas;
    // typedef boost::shared_ptr<T> SPtr;
    /*SPtr tree = *(SPtr*) luaL_checkudata(L, 1, meta.c_str());
    TreeItem::TreeItemList subitems = tree->SubChildren(); 
    TreeItem::TreeItemList::iterator it = subitems.begin();
    for ( ; it != subitems.end(); ++it) {
      TreeItem::Ptr subitem = *it;
      LuaHelper::unregister_userdata<>(L, subitem.get());
    } */  
    return LuaHelper::delete_shared_userdata<T>(L, meta);
  }

  /*static int clear(lua_State* L) {
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
  }*/
};

class LuaTreeNodeBind {
 public:
  static std::string meta;
  static int open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"settext", settext},
      {"text", text},
      {"add", add},
      {"size", size},
      {"at", at},
      {"remove", remove},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    return 1;
  }

  static int create(lua_State* L) {
    LuaHelper::new_shared_userdata(L, meta.c_str(), new ui::Node());    
    return 1;
  }

  static int gc(lua_State* L) {
    return LuaHelper::delete_shared_userdata<ui::Node>(L, meta);
  }

  static int settext(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::set_text); }  
  static int text(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::text); } 
  static int add(lua_State* L) {
    using namespace ui::canvas;
    boost::shared_ptr<ui::Node> treenode = LuaHelper::check_sptr<ui::Node>(L, 1, meta);    
    boost::shared_ptr<ui::Node> treenode2 = LuaHelper::check_sptr<ui::Node>(L, 2, LuaTreeNodeBind::meta);
    treenode->AddNode(treenode2);
    return LuaHelper::chaining(L);
  }
  static int size(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::size); }
  static int at(lua_State *L) {
    if (lua_isnumber(L, 2)) {
      boost::shared_ptr<ui::Node> treenode = LuaHelper::check_sptr<ui::Node>(L, 1, meta);  
      int index = luaL_checknumber(L, 2);
      if (index <0 && index >= treenode->size()) {
        luaL_error(L, "index out of range");
      }
      luaL_requiref(L, "psycle.ui.canvas.treenode", LuaTreeNodeBind::open, true);
      int n = lua_gettop(L);      
      boost::shared_ptr<ui::Node> tn = *(treenode->begin() + index);      
      LuaHelper::new_shared_userdata<>(L, LuaTreeNodeBind::meta, tn.get(), n, true);     
      return 1;
    }
    return 0;
  }

  static int remove(lua_State *L) {
    if (lua_isnumber(L, 2)) {
      boost::shared_ptr<ui::Node> treenode = LuaHelper::check_sptr<ui::Node>(L, 1, meta);  
      int index = luaL_checknumber(L, 2);
      if (index <0 && index >= treenode->size()) {
        luaL_error(L, "index out of range");
      }
      treenode->erase(treenode->begin() + index);
      lua_gc(L, LUA_GCCOLLECT, 0);
      return 1;
    }
    return 0;
  }

};

template <class T = LuaTable>
class LuaTableBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"insertcolumn", insertcolumn},
      {"inserttext", insertext},
      {"settext", settext},
      {"autosize", autosize},
      {"settextcolor", settextcolor},
      {"setbackgroundcolor", setbackgroundcolor},
      {"backgroundcolor", backgroundcolor},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int create(lua_State* L);  
  static int gc(lua_State* L) { return LuaHelper::delete_shared_userdata<T>(L, meta); }
  static int insertcolumn(lua_State* L) { LUAEXPORTM(L, meta, &T::InsertColumn); }  
  static int insertext(lua_State* L) { LUAEXPORTM(L, meta, &T::InsertText); } 
  static int settext(lua_State* L) { LUAEXPORTM(L, meta, &T::SetText); } 
  static int autosize(lua_State* L) { LUAEXPORTM(L, meta, &T::AutoSize); } 
  static int settextcolor(lua_State* L) { LUAEXPORTM(L, meta, &T::set_text_color); } 
  static int setbackgroundcolor(lua_State* L) { LUAEXPORT(L, &T::set_background_color) }
  static int backgroundcolor(lua_State* L) { LUAEXPORT(L, &T::background_color) }
};

template <class T = LuaFrameWnd>
class LuaFrameItemBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"new", create},
      {"settitle", settitle},
      {"setview", setview},
      {"showdecoration", showdecoration},
      {"hidedecoration", hidedecoration},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
 
  static int create(lua_State* L);
  static int gc(lua_State* L) {
    LuaHelper::delete_shared_userdata<LuaFrameWnd>(L, meta);    
    return 0;
  }
  static int setview(lua_State* L) {
    LuaFrameWnd::Ptr frame = LuaHelper::check_sptr<LuaFrameWnd>(L, 1, meta);    
/*    ui::canvas::Canvas::WeakPtr old_canvas = wnd->canvas();
    if (!old_canvas.expired()) {
      LuaHelper::unregister_userdata(L, old_canvas.lock().get());
    }*/
    LuaCanvas::Ptr canvas = LuaHelper::check_sptr<LuaCanvas>(L, 2, LuaCanvasBind<>::meta);
    if (canvas) {
      LuaHelper::register_userdata(L, canvas.get());
      frame->set_view(canvas);
    }    
    return LuaHelper::chaining(L);
  }
  static int settitle(lua_State* L)  { LUAEXPORT(L, &T::set_title); }
  static int showdecoration(lua_State* L)  { LUAEXPORT(L, &T::ShowDecoration); }
  static int hidedecoration(lua_State* L)  { LUAEXPORT(L, &T::HideDecoration); }  
};


class LuaLexerBind {
 public:
  static std::string meta;
  static int open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setkeywords", setkeywords},
      {"setcommentcolor", setcommentcolor},
      {"commentcolor", commentcolor},
      {"setcommentlinecolor", setcommentlinecolor},
      {"commentlinecolor", commentlinecolor},
      {"setcommentdoccolor", setcommentdoccolor},
      {"commentdoccolor", commentdoccolor},
      {"setnumbercolor", setnumbercolor},
      {"numbercolor", numbercolor},
      {"setwordcolor", setwordcolor},
      {"wordcolor", wordcolor},
      {"setstringcolor", setstringcolor},
      {"stringcolor", stringcolor},
      {"setoperatorcolor", setoperatorcolor},
      {"operatorcolor", operatorcolor},
      {"setcharactercodecolor", setcharactercodecolor},
      {"charactercodecolor", charactercodecolor},
      {"setpreprocessorcolor", setpreprocessorcolor},
      {"preprocessorcolor", preprocessorcolor},
      {"setidentifiercolor", setidentifiercolor},
      {"identifiercolor", identifiercolor},
      {"setfoldingcolor", setfoldingcolor},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    return 1;
  }

  static int create(lua_State* L) {
    LuaHelper::new_shared_userdata(L, meta, new ui::Lexer());
    return 1;
  }

  static int gc(lua_State* L) {
    return LuaHelper::delete_shared_userdata<ui::Lexer>(L, meta);
  }

  static int setkeywords(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_keywords); }  
  static int setcommentcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_comment_color); } 
  static int commentcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::comment_color); } 
  static int setcommentlinecolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_comment_line_color); } 
  static int commentlinecolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::comment_line_color); } 
  static int setcommentdoccolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_comment_doc_color); }   
  static int commentdoccolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::comment_doc_color); } 
  static int setnumbercolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_number_color); }
  static int numbercolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::number_color); }
  static int setwordcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_word_color); }
  static int wordcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::word_color); }
  static int setstringcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_string_color); }
  static int stringcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::string_color); }
  static int setoperatorcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_operator_color); }
  static int operatorcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::operator_color); }
  static int setcharactercodecolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_character_code_color); }
  static int charactercodecolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::character_code_color); }
  static int setpreprocessorcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_preprocessor_color); }
  static int preprocessorcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::preprocessor_color); }
  static int setidentifiercolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_identifier_color); }
  static int identifiercolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::identifier_color); }
  static int setfoldingcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Lexer::set_folding_color); }
};

template <class T = LuaScintilla>
class LuaScintillaBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
       {"new", create},
       {"f", f},       
       {"gotoline", gotoline},
       {"length", length},
       {"loadfile", loadfile},
       {"savefile", savefile},
       {"filename", filename},
       {"hasfile", hasfile},
       {"addtext", addtext},
       {"findtext", findtext},
       {"selectionstart", selectionstart},
       {"selectionend", selectionend},
       {"setsel", setsel},
       {"hasselection", hasselection},
       {"setfindwholeword", setfindwholeword},
       {"setfindmatchcase", setfindmatchcase},
       {"setfindregexp", setfindregexp},
       {"setforegroundcolor", setforegroundcolor},
       {"foregroundcolor", foregroundcolor},   
       {"setbackgroundcolor", setbackgroundcolor},
       {"backgroundcolor", backgroundcolor},
       {"setlinenumberforegroundcolor", setlinenumberforegroundcolor},
       {"linenumberforegroundcolor", linenumberforegroundcolor},   
       {"setlinenumberbackgroundcolor", setlinenumberbackgroundcolor},
       {"linenumberbackgroundcolor", linenumberbackgroundcolor},
       {"setmarginbackgroundcolor", setmarginbackgroundcolor},
       {"marginbackgroundcolor", marginbackgroundcolor},
       {"setselforegroundcolor", setselforegroundcolor},
       {"setselbackgroundcolor", setselbackgroundcolor},
       {"setselalpha", setselalpha},
       {"setcaretcolor", setcaretcolor},
       {"caretcolor", caretcolor},
       {"setidentcolor", setidentcolor},
       {"styleclearall", styleclearall},
       {"setlexer", setlexer},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);

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
    int r = sc->f(sci, (void*) wparam, (void*) lparam);
    lua_pushinteger(L, r);
    return 1;
  }

  static int setsel(lua_State *L) { LUAEXPORT(L, &T::SetSel); } 
  static int selectionstart(lua_State *L) { LUAEXPORT(L, &T::selectionstart); } 
  static int selectionend(lua_State *L) { LUAEXPORT(L, &T::selectionend); } 
  static int hasselection(lua_State *L) { LUAEXPORT(L, &T::has_selection); } 
  static int gotoline(lua_State *L) { LUAEXPORT(L, &T::GotoLine); }  
  static int length(lua_State *L) { LUAEXPORT(L, &T::length); }  
  static int addtext(lua_State *L) { LUAEXPORT(L, &T::AddText); } 
  static int findtext(lua_State *L) { LUAEXPORT(L, &T::FindText); }  
  static int clear(lua_State *L) { LUAEXPORT(L, &T::RemoveAll); }  
  static int loadfile(lua_State *L) { LUAEXPORT(L, &T::LoadFile); }
  static int savefile(lua_State *L) { LUAEXPORT(L, &T::SaveFile); }
  static int filename(lua_State *L) { LUAEXPORT(L, &T::filename); }  
  static int hasfile(lua_State *L) { LUAEXPORT(L, &T::has_file); }
  static int setfindmatchcase(lua_State *L) { LUAEXPORT(L, &T::set_find_match_case); }
  static int setfindwholeword(lua_State *L) { LUAEXPORT(L, &T::set_find_whole_word); }
  static int setfindregexp(lua_State *L) { LUAEXPORT(L, &T::set_find_regexp); }
  static int setforegroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_foreground_color); }
  static int foregroundcolor(lua_State *L) { LUAEXPORT(L, &T::foreground_color); }
  static int setbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_background_color); }
  static int backgroundcolor(lua_State *L) { LUAEXPORT(L, &T::background_color); }  
  static int setlinenumberforegroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_linenumber_foreground_color); }
  static int linenumberforegroundcolor(lua_State *L) { LUAEXPORT(L, &T::linenumber_foreground_color); }
  static int setlinenumberbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_linenumber_background_color); }
  static int linenumberbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::linenumber_background_color); }
  static int setmarginbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_margin_background_color); }
  static int marginbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::margin_background_color); }
  static int setselforegroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_sel_foreground_color); }
  //static int selforegroundcolor(lua_State *L) { LUAEXPORT(L, &T::sel_foreground_color); }
  static int setselbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_sel_background_color); }
  //static int selbackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::sel_background_color); }
  static int setselalpha(lua_State *L) { LUAEXPORT(L, &T::set_sel_alpha); }
  static int setcaretcolor(lua_State *L) { LUAEXPORT(L, &T::set_caret_color); }
  static int caretcolor(lua_State *L) { LUAEXPORT(L, &T::caret_color); }
  static int setidentcolor(lua_State *L) { LUAEXPORT(L, &T::set_ident_color); }
  static int styleclearall(lua_State *L) { LUAEXPORT(L, &T::StyleClearAll); }
  static int setlexer(lua_State *L) { 
    LuaHelper::bindud<T, ui::Lexer>(L, meta, LuaLexerBind::meta, &T::set_lexer); 
    return LuaHelper::chaining(L);
  }
};

template <class T = LuaEdit>
class LuaEditBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
       {"new", create},
       {"settext", settext},
       {"text", text},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int create(lua_State *L);
  static int settext(lua_State *L) { LUAEXPORT(L, &T::set_text); }
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
      {"new", create},
      {"setpos", setpos},
      {"setscrollpos", setscrollpos},
      {"scrollpos", scrollpos},
      {"setscrollrange", setscrollrange},
      {"scrollrange", scrollrange},
      {"systemsize", systemsize},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    const char* const e[] = {"HORZ", "VERT"};
    LuaHelper::buildenum(L, e, 2);
    return 0;
  }
  static int create(lua_State *L);
  static int setscrollpos(lua_State* L) { LUAEXPORT(L, &T::set_scroll_pos); }
  static int scrollpos(lua_State* L) { LUAEXPORT(L, &T::scroll_pos); }
  static int setscrollrange(lua_State* L) { LUAEXPORT(L, &T::set_scroll_range); }
  static int scrollrange(lua_State* L) { LUAEXPORT(L, &T::scroll_range); }
  static int systemsize(lua_State* L) { LUAEXPORT(L, &T::system_size); }   
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
template class LuaTableBind<LuaTable>;
template class LuaFrameItemBind<LuaFrameWnd>;

} // namespace host
} // namespace psycle