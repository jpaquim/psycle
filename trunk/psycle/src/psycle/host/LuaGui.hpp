// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2015 members of the psycle project http://psycle.sourceforge.net

#pragma once
// #include <psycle/host/detail/project.hpp>
#include "Canvas.hpp"
#include "CanvasItems.hpp"
#include "LuaHelper.hpp"

namespace psycle {
namespace host {

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

class LuaFileObserver : public ui::FileObserver, public LuaState {
 public:
   LuaFileObserver(lua_State* L) : LuaState(L), ui::FileObserver() {}
   
   virtual void OnCreateFile(const std::string& path);
   virtual void OnDeleteFile(const std::string& path);
   virtual void OnChangeFile(const std::string& path);
};

struct LuaFileObserverBind {
    static const char* meta;
    static int open(lua_State *L);    
    static int create(lua_State* L);    
    static int gc(lua_State* L);
    static int setdirectory(lua_State* L);
    static int startwatching(lua_State* L);
    static int stopwatching(lua_State* L);
  };

struct LuaPointBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L);
  static int gc(lua_State* L);  
  static int setxy(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::set_xy); }
  static int setx(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::set_x); }
  static int x(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::x); }
  static int sety(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::set_y); }
  static int y(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::y); }
};

struct LuaDimensionBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int set(lua_State* L) { LUAEXPORTM(L, meta, &ui::Dimension::set); }
  static int setwidth(lua_State* L) { LUAEXPORTM(L, meta, &ui::Dimension::set_width); }
  static int width(lua_State* L) { LUAEXPORTM(L, meta, &ui::Dimension::width); }
  static int setheight(lua_State* L) { LUAEXPORTM(L, meta, &ui::Dimension::set_height); }
  static int height(lua_State* L) { LUAEXPORTM(L, meta, &ui::Dimension::height); }
};

struct LuaUiRectBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int set(lua_State* L);
  static int left(lua_State *L) { LUAEXPORTM(L, meta, &ui::Rect::left); }
  static int top(lua_State *L) { LUAEXPORTM(L, meta, &ui::Rect::top); }
  static int right(lua_State *L) { LUAEXPORTM(L, meta, &ui::Rect::right); }
  static int bottom(lua_State *L) { LUAEXPORTM(L, meta, &ui::Rect::bottom); }
  static int width(lua_State *L) { LUAEXPORTM(L, meta, &ui::Rect::width); }
  static int height(lua_State *L) { LUAEXPORTM(L, meta, &ui::Rect::height); }
};

struct LuaSystemMetrics {
  static int open(lua_State *L); 
  static int screensize(lua_State *L);
};

template <class T>
class CanvasItem : public T, public LuaState {
 public:    
   CanvasItem(lua_State* L) : LuaState(L), T() {}
   CanvasItem(lua_State* L, ui::WindowImp* imp) : LuaState(L), T(0) {}

   virtual void OnMouseDown(ui::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmousedown", ev, *this)) {
       T::OnMouseDown(ev);
     }
   }
   virtual void OnDblclick(ui::MouseEvent& ev) {
     if (!SendMouseEvent(L, "ondblclick", ev, *this)) {
       T::OnDblclick(ev);
     }
   }   
   virtual void OnMouseUp(ui::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmouseup", ev, *this)) {
       T::OnMouseUp(ev);
     }
   }
   virtual void OnMouseMove(ui::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmousemove", ev, *this)) {
       T::OnMouseMove(ev);
     }
   }
   virtual void OnMouseEnter(ui::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmouseenter", ev, *this)) {
       T::OnMouseEnter(ev);
     }
   }
   virtual void OnMouseOut(ui::MouseEvent& ev) {
     if (!SendMouseEvent(L, "onmouseout", ev, *this)) {
       T::OnMouseOut(ev);
     }
   }
   virtual void OnKeyDown(ui::KeyEvent& ev) {
     if (!SendKeyEvent(L, "onkeydown", ev, *this)) {
       T::OnKeyDown(ev);
     }
   }
   virtual void OnKeyUp(ui::KeyEvent& ev) {
     if (!SendKeyEvent(L, "onkeyup", ev, *this)) {
       T::OnKeyUp(ev);
     }
   }
   virtual void OnFocus(ui::Event& ev) {
     if (!SendEvent(L, "onfocus", ev, *this)) {
       T::OnFocus(ev);
     }
   }
   virtual void OnKillFocus();
   virtual void OnSize(const ui::Dimension &dimension);   
	 virtual ui::Dimension OnCalcAutoDimension() const;
 
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
typedef CanvasItem<ui::HeaderGroup> LuaHeaderGroup;
typedef CanvasItem<ui::Canvas> LuaCanvas;
typedef CanvasItem<ui::RectangleBox> LuaRectangleBox;
typedef CanvasItem<ui::Line> LuaLine;
typedef CanvasItem<ui::Text> LuaText;

class LuaPic : public CanvasItem<ui::Pic> {
 public:  
  LuaPic(lua_State* L) : CanvasItem<ui::Pic>(L) {}  
  
  virtual void Draw(ui::Graphics* g, ui::Region& draw_rgn);
};

class LuaScrollBox : public CanvasItem<ui::ScrollBox> {
 public:  
  LuaScrollBox(lua_State* L) : CanvasItem<ui::ScrollBox>(L) {}  
};


class LuaButton : public CanvasItem<ui::Button> {
 public:  
  LuaButton(lua_State* L) : CanvasItem<ui::Button>(L) {}
  
  virtual void OnClick();
};

class LuaRadioButton : public CanvasItem<ui::RadioButton> {
public:
	LuaRadioButton(lua_State* L) : CanvasItem<ui::RadioButton>(L) {}

	virtual void OnClick();
};

class LuaGroupBox : public CanvasItem<ui::GroupBox> {
public:
	LuaGroupBox(lua_State* L) : CanvasItem<ui::GroupBox>(L) {}

	virtual void OnClick();
};

class LuaEdit : public CanvasItem<ui::Edit> {
 public:  
  LuaEdit(lua_State* L) : CanvasItem<ui::Edit>(L) {}  
};

class LuaComboBox : public CanvasItem<ui::ComboBox> {
 public:  
  LuaComboBox(lua_State* L) : CanvasItem<ui::ComboBox>(L) {}

  virtual void OnSelect();
};

class LuaTreeView : public CanvasItem<ui::TreeView> {
 public:  
  LuaTreeView(lua_State* L) : CanvasItem<ui::TreeView>(L) {}
  
  virtual void OnChange(const ui::Node::Ptr& node);
  virtual void OnRightClick(const ui::Node::Ptr& node);
  virtual void OnEditing(const ui::Node::Ptr& node, const std::string& text);
  virtual void OnEdited(const ui::Node::Ptr& node, const std::string& text);
  virtual void OnContextPopup(ui::Event&, const ui::Point& mouse_point, const ui::Node::Ptr& node);
};

class LuaListView : public CanvasItem<ui::ListView> {
 public:  
  LuaListView(lua_State* L) : CanvasItem<ui::ListView>(L) {}
  
  virtual void OnChange(const ui::Node::Ptr& node);
  virtual void OnRightClick(const ui::Node::Ptr& node);
  virtual void OnEditing(const ui::Node::Ptr& node, const std::string& text);
  virtual void OnEdited(const ui::Node::Ptr& node, const std::string& text);
};

struct LuaEventBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int preventdefault(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::PreventDefault); }
  static int isdefaultprevented(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::is_default_prevented); }
  static int stoppropagation(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::StopPropagation); }
  static int ispropagationstopped(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::is_propagation_stopped); }
};

struct LuaKeyEventBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int keycode(lua_State* L) { LUAEXPORTM(L, meta, &ui::KeyEvent::keycode); }  
  static int shiftkey(lua_State* L) { LUAEXPORTM(L, meta, &ui::KeyEvent::shiftkey); }
  static int ctrlkey(lua_State* L) { LUAEXPORTM(L, meta, &ui::KeyEvent::ctrlkey); }
  static int preventdefault(lua_State* L) { LUAEXPORTM(L, meta, &ui::KeyEvent::PreventDefault); }
  static int isdefaultprevented(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::is_default_prevented); }
  static int stoppropagation(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::StopPropagation); }
  static int ispropagationstopped(lua_State* L) { LUAEXPORTM(L, meta, &ui::Event::is_propagation_stopped); }
};

struct LuaMouseEventBind {
	static int open(lua_State *L);
	static const char* meta;
	static int create(lua_State *L);
	static int gc(lua_State* L);	
	static int clientx(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::cx); }
	static int clienty(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::cy); }	
	static int button(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::button); }
	static int preventdefault(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::PreventDefault); }
	static int isdefaultprevented(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::is_default_prevented); }
	static int stoppropagation(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::StopPropagation); }
	static int ispropagationstopped(lua_State* L) { LUAEXPORTM(L, meta, &ui::MouseEvent::is_propagation_stopped); }
};

struct LuaImageBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int graphics(lua_State* L);
  static int size(lua_State* L);
  static int resize(lua_State* L);  
  static int rotate(lua_State* L);
  static int reset(lua_State *L);
  static int load(lua_State *L) { LUAEXPORTM(L, meta, &ui::Image::Load); }
  static int save(lua_State *L);
  static int cut(lua_State *L);
  static int settransparent(lua_State* L);
  static int setpixel(lua_State *L);
  static int getpixel(lua_State *L);
};

class LuaColorHelper {
 public:
  static int open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"encodeargb", encodeargb},
    {"decodeargb", decodeargb},			
    { NULL, NULL }
  };
  luaL_newlib(L, funcs);
  return 1;
  }

  static int encodeargb(lua_State* L) {
		int red = static_cast<int>(luaL_checkinteger(L, 1));
	  int green = static_cast<int>(luaL_checkinteger(L, 2));
	  int blue = static_cast<int>(luaL_checkinteger(L, 3));
	  int alpha = static_cast<int>(luaL_checkinteger(L, 4));
	  int argb = (blue << 0) | (green << 8) | (red << 16) | (alpha << 24);
	  lua_pushinteger(L, argb);
	  return 1;
  }

  static int decodeargb(lua_State* L) {
	  int argb = static_cast<int>(luaL_checkinteger(L, 1));
	  int alpha = (argb >> 24) & 0xff;
	  int red = (argb >> 16) & 0xff;
	  int green = (argb >> 8) & 0xff;
	  int blue = argb & 0xff;
	  lua_pushinteger(L, red);
	  lua_pushinteger(L, green);
	  lua_pushinteger(L, blue);
	  lua_pushinteger(L, alpha);
	  return 4;
  }
};

class LuaImagesBind {
 public:
  static std::string meta;
  static int open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},                  
      {"add", add},      
      {"size", size},
      {"at", at},      	  
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    return 1;
  }

  static int create(lua_State* L) {
    LuaHelper::new_shared_userdata(L, meta.c_str(), new ui::Images());
    lua_newtable(L);
    lua_setfield(L, -2, "_children");
    return 1;
  }

  static int gc(lua_State* L) {
    return LuaHelper::delete_shared_userdata<ui::Images>(L, meta);
  }    
  static int add(lua_State* L) {    
    boost::shared_ptr<ui::Images> images = LuaHelper::check_sptr<ui::Images>(L, 1, meta);
    boost::shared_ptr<ui::Image> image = LuaHelper::check_sptr<ui::Image>(L, 2, LuaImageBind::meta);
    images->Add(image);
    lua_getfield(L, -2, "_children");
    int len = lua_rawlen(L, -1);
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, len+1);
    return LuaHelper::chaining(L);
  }
    
  static int size(lua_State* L) { LUAEXPORTM(L, meta, &ui::Images::size); }
  static int at(lua_State *L) {
    if (lua_isnumber(L, 2)) {
      ui::Images::Ptr images = LuaHelper::check_sptr<ui::Images>(L, 1, meta);
      int index = static_cast<int>(luaL_checkinteger(L, 2));
      if (index < 1 && index >= images->size()) {
        luaL_error(L, "index out of range");
      }            
      ui::Image::Ptr tn = *(images->begin() + index - 1);
      if (tn.get()) {
        LuaHelper::find_weakuserdata(L, tn.get());
      } else {
        lua_pushnil(L);
      }      
      return 1;
    }
    return 0;
  }

};

struct LuaRegionBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int setrect(lua_State *L) { 
	ui::Region::Ptr rgn = LuaHelper::check_sptr<ui::Region>(L, 1, meta);    
    ui::Rect rect(ui::Point(luaL_checknumber(L, 2),
                            luaL_checknumber(L, 3)),
                  ui::Dimension(luaL_checknumber(L, 4),
                                luaL_checknumber(L, 5))
                  );
    rgn->SetRect(rect);
    return LuaHelper::chaining(L);
  }
   
  static int bounds(lua_State *L) {
	ui::Region::Ptr rgn = LuaHelper::check_sptr<ui::Region>(L, 1, meta);
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
  static int boundrect(lua_State *L) {
		boost::shared_ptr<ui::Area> rgn = LuaHelper::check_sptr<ui::Area>(L, 1, meta);
    ui::Rect bounds = rgn->bounds();
    lua_pushnumber(L, bounds.left());
    lua_pushnumber(L, bounds.top());
    lua_pushnumber(L, bounds.width());
    lua_pushnumber(L, bounds.height());
    return 4;
  }
  static int combine(lua_State *L) { LUAEXPORTML(L, meta, &ui::Area::Combine); }
	static int offset(lua_State *L) { LUAEXPORTML(L, meta, &ui::Area::Offset); }
	static int clear(lua_State* L) { LUAEXPORTML(L, meta, &ui::Area::Clear); }
	static int setrect(lua_State* L) {
		boost::shared_ptr<ui::Area> rgn = LuaHelper::check_sptr<ui::Area>(L, 1, meta);
		rgn->Combine(ui::Area(ui::Rect(ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)), ui::Point(luaL_checknumber(L, 4), luaL_checknumber(L, 5)))), RGN_OR);
		return LuaHelper::chaining(L);
	}
  static int gc(lua_State* L);
};

struct LuaGraphicsBind {
  static int open(lua_State *L);
  static int create(lua_State *L);
  static int translate(lua_State *L) { LUAEXPORTM(L, meta, &ui::Graphics::Translate); }
  static int setcolor(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::SetColor); }
  static int color(lua_State* L) { LUAEXPORTM(L, meta, &ui::Graphics::color); }
  static int drawline(lua_State *L) {
    ui::Graphics::Ptr g = LuaHelper::check_sptr<ui::Graphics>(L, 1, meta);
    typedef boost::shared_ptr<ui::Point> PointPtr;
    PointPtr from = LuaHelper::check_sptr<ui::Point>(L, 2, LuaPointBind::meta);
    PointPtr to = LuaHelper::check_sptr<ui::Point>(L, 3, LuaPointBind::meta);
    g->DrawLine(*from.get(), *to.get());
    return LuaHelper::chaining(L);
  }
  static int drawrect(lua_State *L);
  static int drawroundrect(lua_State *L) {
    LuaHelper::check_sptr<ui::Graphics>(L, 1, meta)->FillRoundRect(ui::Rect(
      ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)),
      ui::Dimension(luaL_checknumber(L, 4), luaL_checknumber(L, 5))),
      ui::Dimension(luaL_checknumber(L, 6), luaL_checknumber(L, 7)));
    return LuaHelper::chaining(L);
  }
  static int drawoval(lua_State* L) { 
    LuaHelper::check_sptr<ui::Graphics>(L, 1, meta)->DrawOval(ui::Rect(
      ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)),
      ui::Dimension(luaL_checknumber(L, 4), luaL_checknumber(L, 5))));
    return LuaHelper::chaining(L);
  }
  static int fillrect(lua_State *L) {        
    ui::Graphics::Ptr g = LuaHelper::check_sptr<ui::Graphics>(L, 1, meta);
    boost::shared_ptr<ui::Rect> rect =
      LuaHelper::check_sptr<ui::Rect>(L, 2, LuaUiRectBind::meta);    
    g->FillRect(*rect.get());
    return LuaHelper::chaining(L);
  }
  static int fillroundrect(lua_State *L) {
    LuaHelper::check_sptr<ui::Graphics>(L, 1, meta)->FillRoundRect(ui::Rect(
      ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)),
      ui::Dimension(luaL_checknumber(L, 4), luaL_checknumber(L, 5))),
      ui::Dimension(luaL_checknumber(L, 6), luaL_checknumber(L, 7)));
    return LuaHelper::chaining(L);  
  }
  static int filloval(lua_State* L) {
    LuaHelper::check_sptr<ui::Graphics>(L, 1, meta)->FillOval(ui::Rect(
      ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)),
      ui::Dimension(luaL_checknumber(L, 4), luaL_checknumber(L, 5))));
    return LuaHelper::chaining(L);
  }
  static int copyarea(lua_State* L) {
    LuaHelper::check_sptr<ui::Graphics>(L, 1, meta)->CopyArea(ui::Rect(
      ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)),
      ui::Dimension(luaL_checknumber(L, 4), luaL_checknumber(L, 5))),
      ui::Point(luaL_checknumber(L, 6), luaL_checknumber(L, 7)));
    return LuaHelper::chaining(L);
  }
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


class LuaMenuBar : public ui::MenuBar, public LuaState {
 public:  
  LuaMenuBar(lua_State* state) : LuaState(state) {}

  virtual void OnMenuItemClick(boost::shared_ptr<ui::Node> node);
};

class LuaPopupMenu : public ui::PopupMenu, public LuaState {
 public:  
  LuaPopupMenu(lua_State* state) : LuaState(state) {}

  virtual void OnMenuItemClick(boost::shared_ptr<ui::Node> node);
};

class LuaScintilla : public CanvasItem<ui::Scintilla> {
 public:  
  LuaScintilla(lua_State* L) : CanvasItem<ui::Scintilla>(L) {}    
};

class LuaFrameWnd : public CanvasItem<ui::Frame> {
 public:   
   typedef boost::shared_ptr<LuaFrameWnd> Ptr;
   LuaFrameWnd(lua_State* L) : CanvasItem<ui::Frame>(L) {}   
      
   virtual void OnClose();
   virtual void OnShow();
   virtual void OnContextPopup(ui::Event&, const ui::Point& mouse_point, const ui::Node::Ptr& node);
};

class LuaPopupFrameWnd : public CanvasItem<ui::PopupFrame> {
 public:   
   typedef boost::shared_ptr<LuaPopupFrameWnd> Ptr;
   LuaPopupFrameWnd(lua_State* L) : CanvasItem<ui::PopupFrame>(L) {}   
      
   virtual void OnClose();
   virtual void OnShow();
};

class LuaSplitter : public ui::Splitter, public LuaState {
 public:  
  LuaSplitter(lua_State* L) : LuaState(L) {}
  LuaSplitter(lua_State* L, const ui::Orientation& orientation) 
    : ui::Splitter(orientation), 
      LuaState(L) {
  }  
};

class LuaScrollBar : public ui::ScrollBar, public LuaState {
 public:  
  LuaScrollBar(lua_State* state) : LuaState(state) {}
  LuaScrollBar(lua_State* state, const ui::Orientation& orientation) 
    : ui::ScrollBar(orientation), 
      LuaState(state) {
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

class LuaGameController : public ui::GameController,  public LuaState {
 public:
  LuaGameController() : ui::GameController(), LuaState(0) {}
  LuaGameController(lua_State* L) : ui::GameController(), LuaState(L) {}

  virtual void OnButtonDown(int button);
  virtual void OnButtonUp(int button);
  virtual void OnXAxis(int pos, int old_pos);
  virtual void OnYAxis(int pos, int old_pos);
  virtual void OnZAxis(int pos, int old_pos);
};

struct LuaGameControllerBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L); 
  static int gc(lua_State* L);  
  static int xposition(lua_State* L) { LUAEXPORTM(L, LuaGameControllerBind::meta, &ui::GameController::xposition); }
  static int yposition(lua_State* L) { LUAEXPORTM(L, LuaGameControllerBind::meta, &ui::GameController::yposition); }
  static int zposition(lua_State* L) { LUAEXPORTM(L, LuaGameControllerBind::meta, &ui::GameController::zposition); }
  static int buttons(lua_State* L);    
};

struct LuaGameControllersBind {
  static std::string meta;
  static int open(lua_State *L);  
  static int create(lua_State *L); 
  static int gc(lua_State* L);  
  static int update(lua_State* L) { LUAEXPORTM(L, LuaGameControllersBind::meta, &ui::GameControllers<LuaGameController>::Update); }
  static int controllers(lua_State* L);
};

template<class T = LuaItem>
class LuaItemBind {
 public:
  static const std::string meta;  
  typedef LuaItemBind B;
  static int open(lua_State *L) {
    LuaHelper::openex(L, meta, setmethods, gc);
    LuaHelper::setfield(L, "ALLEFT", ui::ALLEFT);
    LuaHelper::setfield(L, "ALTOP", ui::ALTOP);
    LuaHelper::setfield(L, "ALRIGHT", ui::ALRIGHT);
    LuaHelper::setfield(L, "ALBOTTOM", ui::ALBOTTOM);  
    LuaHelper::setfield(L, "ALCLIENT", ui::ALCLIENT);
    LuaHelper::setfield(L, "ALCENTER", ui::ALCENTER);
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
      {"setposition", setposition},
//      {"scrollto", scrollto},
     // {"setsize", setsize},     
      {"position", position},
      {"clientposition", clientposition},
      {"desktopposition", desktopposition},
      {"setautosize", setautosize},
			{"autosize", autosize},
      {"setdebugtext", setdebugtext},      
      {"setfocus", setfocus},    
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
      {"area", area},
      {"drawregion", drawregion},
      {"setclip", setclip},
      {"clip", clip},
      {"tostring", tostring},   
      {"addornament", addornament},
			{"removeornaments", removeornaments},
      {"ornaments", ornaments},
      {"setcursor", setcursor},
      {"setclipchildren", setclipchildren},
      {"addstyle", addstyle},
      {"removestyle", removestyle},      
      {"setalign", setalign},
      {"setmargin", setmargin},
      {"margin", margin},
      {"setpadding", setpadding},
      {"padding", padding},
      {"align", align},
      {"mousecapture", setcapture},
      {"mouserelease", releasecapture},
      {"setaligner", setaligner},
      {"enable", enable},
      {"disable", disable},
			{"showcursor", showcursor},
      {"hidecursor", hidecursor},
      {"setcursorpos", setcursorposition},			
	    {"viewdoublebuffered", viewdoublebuffered},
	    {"viewsinglebuffered", viewsinglebuffered},
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
  static int enable(lua_State *L) { LUAEXPORT(L, &T::Enable); }
  static int disable(lua_State *L) { LUAEXPORT(L, &T::Disable); }
	static int showcursor(lua_State* L) { LUAEXPORT(L, &T::ShowCursor); }
  static int hidecursor(lua_State* L) { LUAEXPORT(L, &T::HideCursor); }
  static int setcursorposition(lua_State* L);
  static int viewdoublebuffered(lua_State* L) { LUAEXPORT(L, &T::ViewDoubleBuffered); }
  static int viewsinglebuffered(lua_State* L) { LUAEXPORT(L, &T::ViewSingleBuffered); }
  static int addstyle(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    UINT style = (unsigned int) luaL_checknumber(L, 2);    
    item->add_style(style);
    return LuaHelper::chaining(L);
  }
	static int autosize(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);		
		lua_pushboolean(L, item->auto_size_width());
		lua_pushboolean(L, item->auto_size_height());    
		return 2;
  }	
  static int removestyle(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    UINT style = (unsigned int) luaL_checknumber(L, 2);    
    item->remove_style(style);
    return LuaHelper::chaining(L);
  }
  static int setposition(lua_State *L) {        
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    boost::shared_ptr<ui::Rect> pos = LuaHelper::check_sptr<ui::Rect>(L, 2, LuaUiRectBind::meta);
    item->set_position(*pos);    
    return LuaHelper::chaining(L);
  }
  static int updatearea(lua_State *L) { LUAEXPORT(L, &T::needsupdate) }
  static int position(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, item->position().left());
    lua_pushnumber(L, item->position().top());
    lua_pushnumber(L, item->position().width());
    lua_pushnumber(L, item->position().height());
    return 4;
  }  
  static int setautosize(lua_State *L) { LUAEXPORT(L, &T::set_auto_size) }
  static int clientposition(lua_State* L) { 
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, item->abs_position().left());
    lua_pushnumber(L, item->abs_position().top());
    lua_pushnumber(L, item->abs_position().width());
    lua_pushnumber(L, item->abs_position().height());
    return 4;    
  }
  static int desktopposition(lua_State* L) { 
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushnumber(L, item->desktop_position().left());
    lua_pushnumber(L, item->desktop_position().top());
    lua_pushnumber(L, item->desktop_position().width());
    lua_pushnumber(L, item->desktop_position().height());
    return 4;    
  }
  static int fls(lua_State *L);
  static int preventfls(lua_State *L) { LUAEXPORT(L, &T::PreventFls) }
  static int enablefls(lua_State *L) { LUAEXPORT(L, &T::EnableFls) }
  static int isflsprevented(lua_State *L) { LUAEXPORT(L, &T::is_fls_prevented) }  
  static int canvas(lua_State* L);
  static int show(lua_State* L) { 
    int n = lua_gettop(L);
    if (n==1) {
      LUAEXPORT(L, &T::Show) 
    } else {
      boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
      boost::shared_ptr<ui::WindowShowStrategy> window_show_strategy = LuaHelper::check_sptr<ui::WindowShowStrategy>(L, 2, LuaCenterToScreenBind::meta);
      item->Show(window_show_strategy);
    }
    return LuaHelper::chaining(L);
  }
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
  static int setclip(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    item->SetClip(ui::Rect(ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)), 
                           ui::Point(luaL_checknumber(L, 3), luaL_checknumber(L, 4))));
    return LuaHelper::chaining(L);
  }
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

  static int addornament(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    boost::shared_ptr<ui::Ornament> ornament;
    if (!lua_isnil(L, 2)) {      
      ornament = LuaHelper::test_sptr<ui::LineBorder>(L, 2, LineBorderBind::meta);
      if (!ornament) {
        ornament = LuaHelper::test_sptr<ui::Wallpaper>(L, 2, WallpaperBind::meta);
        if (!ornament) {
          ornament = LuaHelper::test_sptr<ui::Fill>(L, 2, FillBind::meta);
        }
      }            
    }    
		lua_getfield(L, 1, "_ornaments");
		if (lua_isnil(L, -1)) {
			lua_newtable(L);
			lua_setfield(L, 1, "_ornaments");
		}
		lua_getfield(L, 1, "_ornaments");
		int n = lua_rawlen(L, -1);
		lua_pushvalue(L, 2);
		lua_rawseti(L, -2, n + 1);    
    item->add_ornament(ornament);
    return LuaHelper::chaining(L);
  }

	static int removeornaments(lua_State* L) {		
		lua_pushnil(L);
		lua_setfield(L, 1, "_ornaments");
    LUAEXPORT(L, &T::RemoveOrnaments);
  }

  static int ornaments(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    if (item->ornaments().empty()) {
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

  static int setaligner(lua_State* L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    int row_num = static_cast<int>(luaL_checkinteger(L, 2));
    int col_num = static_cast<int>(luaL_checkinteger(L, 3));
    boost::shared_ptr<ui::GridAligner> aligner(new ui::GridAligner(row_num, col_num));
    item->set_aligner(aligner);
    return LuaHelper::chaining(L);
  }

  static int setcapture(lua_State* L) { LUAEXPORT(L, &T::SetCapture); }
  static int releasecapture(lua_State* L) { LUAEXPORT(L, &T::ReleaseCapture); }
  static int setfocus(lua_State *L) { LUAEXPORT(L, &T::SetFocus) }
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

template <class T = LuaHeaderGroup>
class LuaHeaderGroupBind : public LuaGroupBind<T> {
 public:
  typedef LuaGroupBind<T> B;  
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {   
      {"settitle", settitle},            
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int settitle(lua_State* L) { LUAEXPORT(L, &T::set_title); }  
};

template <class T = LuaScrollBox>
class LuaScrollBoxBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;  
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
      {"scrollby", scrollby},
			{"scrollto", scrollto},
			{"updatescrollrange", updatescrollrange},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int scrollby(lua_State* L) { LUAEXPORT(L, &T::ScrollBy); }
	static int scrollto(lua_State* L) { 
		boost::shared_ptr<T> box = LuaHelper::check_sptr<T>(L, 1, meta);
		box->ScrollTo(ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
		return LuaHelper::chaining(L);
	}
	static int updatescrollrange(lua_State* L) { LUAEXPORT(L, &T::UpdateScrollRange); }
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
      {"showscrollbar", showscrollbar},
      {"setscrollinfo", setscrollinfo},
      {"invalidateontimer", invalidateontimer},
      {"invalidatedirect", invalidatedirect},      
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }      
  static int create(lua_State* L);  
  static int showscrollbar(lua_State* L);
  static int setscrollinfo(lua_State* L);
  static int invalidateontimer(lua_State* L);
  static int invalidatedirect(lua_State* L);
};

template<class T = LuaRectangleBox>
class LuaRectangleBoxBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { 
    return LuaHelper::openex(L, meta, setmethods, gc);
  }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {"setcolor", setcolor},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::SetColor); }
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
    int idx = static_cast<int>(luaL_checkinteger(L, 2));
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
  static int open(lua_State *L) { 
    LuaHelper::openex(L, meta, setmethods, gc); 
    LuaHelper::setfield(L, "LEFTJUSTIFY", ui::LEFTJUSTIFY);
    LuaHelper::setfield(L, "CENTERJUSTIFY", ui::CENTERJUSTIFY);
    LuaHelper::setfield(L, "RIGHTJUSTIFY", ui::RIGHTJUSTIFY);
    return 1;    
  }
  static int setmethods(lua_State* L) {
     B::setmethods(L);
     static const luaL_Reg methods[] = {
      {"settext", settext},
      {"text", text},
      {"setcolor", setcolor},
      {"color", color},
      {"setfont", setfont},
      {"setverticalalignment", setverticalalignment},
      {"setjustify", setjustify},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int settext(lua_State* L) { LUAEXPORT(L, &T::set_text); }
  static int text(lua_State* L) { LUAEXPORT(L, &T::text); }
  static int setcolor(lua_State* L) { LUAEXPORT(L, &T::set_color); }
  static int color(lua_State* L) { LUAEXPORT(L, &T::color); } 
  static int setfont(lua_State* L) {    
    boost::shared_ptr<LuaText> text = LuaHelper::check_sptr<LuaText>(L, 1, meta);    
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "name");
    lua_getfield(L, 2, "height");
    const char *name = luaL_checkstring(L, -2);
    int height = static_cast<int>(luaL_checknumber(L, -1));
    std::auto_ptr<ui::Font> font(ui::Systems::instance().CreateFont());
    ui::FontInfo font_info;
    font_info.name = name;
    font_info.height = height;
    font->set_info(font_info);
    text->set_font(*font);
    return LuaHelper::chaining(L);
  }  
  static int setverticalalignment(lua_State* L) {    
    LuaHelper::check_sptr<LuaText>(L, 1, meta)
      ->set_vertical_alignment(static_cast<ui::AlignStyle>(luaL_checkinteger(L, 2)));
    return LuaHelper::chaining(L);
  }
  static int setjustify(lua_State* L) {    
    LuaHelper::check_sptr<LuaText>(L, 1, meta)
      ->set_justify(static_cast<ui::JustifyStyle>(luaL_checkinteger(L, 2)));
    return LuaHelper::chaining(L);
  }
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
	  {"setzoom", setzoom},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setsource(lua_State* L) {
		int err = LuaHelper::check_argnum(L, 3, "self, x, y");
    if (err!=0) return err;
    boost::shared_ptr<T> pic = LuaHelper::check_sptr<T>(L, 1, meta);
		pic->SetSource(ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
    return LuaHelper::chaining(L);		
	}
	static int setzoom(lua_State* L) {
		int err = LuaHelper::check_argnum(L, 3, "self, x, y");
    if (err!=0) return err;
    boost::shared_ptr<T> pic = LuaHelper::check_sptr<T>(L, 1, meta);
		pic->set_zoom(ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
    return LuaHelper::chaining(L);		
	}
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
       {"settext", settext},
       {"text", text},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int settext(lua_State *L) { LUAEXPORT(L, &T::set_text); }
  static int text(lua_State *L) { LUAEXPORT(L, &T::text); }
};


template <class T = LuaSplitter>
class LuaSplitterBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) {
	LuaHelper::openex(L, meta, setmethods, gc);
	LuaHelper::setfield(L, "HORZ", ui::HORZ);
	LuaHelper::setfield(L, "VERT", ui::VERT);
	return 1;
  }
  static int create(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments  
    boost::shared_ptr<LuaGroup> group;
	ui::Orientation orientation = ui::VERT;
    if (n>=2 && !lua_isnil(L, 2)) {
      group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
      if (!group) {
        group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
		if (!group) {
          group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaHeaderGroupBind<>::meta);
		  if (!group) {
			group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaScrollBoxBind<>::meta);
		  }
        }
      }    
	  if (n==3) {
		orientation = (ui::Orientation) luaL_checkinteger(L, 3);
	  }
    }
    boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L, orientation));    
    LuaHelper::register_weakuserdata(L, item.get());
    if (group) {    
      group->Add(item);
      LuaHelper::register_userdata(L, item.get());
    }  
    LuaHelper::new_lua_module(L, "psycle.signal");  
    lua_setfield(L, -2, "keydown");
    return 1;
  }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
	   {"new", create},   
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  } 
};

template <class T = LuaGroupBox>
class LuaGroupBoxBind : public LuaItemBind<T> {
public:
	typedef LuaItemBind<T> B;
	static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
	static int setmethods(lua_State* L) {
		B::setmethods(L);
		static const luaL_Reg methods[] = {
			{ "settext", settext },
			{ "text", text },
			{ NULL, NULL }
		};
		luaL_setfuncs(L, methods, 0);
		return 0;
	}
	static int settext(lua_State *L) { LUAEXPORT(L, &T::set_text); }
	static int text(lua_State *L) { LUAEXPORT(L, &T::text); }
};

template <class T = LuaRadioButton>
class LuaRadioButtonBind : public LuaItemBind<T> {
public:
	typedef LuaItemBind<T> B;
	static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
	static int setmethods(lua_State* L) {
		B::setmethods(L);
		static const luaL_Reg methods[] = {
			{"settext", settext},
			{"text", text},
			{"check", check},
			{"uncheck", uncheck},
			{"checked", checked},
			{NULL, NULL}
		};
		luaL_setfuncs(L, methods, 0);
		return 0;
	}
	static int settext(lua_State *L) { LUAEXPORT(L, &T::set_text); }
	static int text(lua_State *L) { LUAEXPORT(L, &T::text); }
	static int check(lua_State *L) { LUAEXPORT(L, &T::Check); }
	static int uncheck(lua_State *L) { LUAEXPORT(L, &T::UnCheck); }
	static int checked(lua_State *L) { LUAEXPORT(L, &T::checked); }
};

template <class T = LuaComboBox>
class LuaComboBoxBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
       {"setitems", setitems},
       {"items", items},
       {"setitemindex", setitemindex},
       {"itemindex", itemindex},
       {"additem", additem},
       {"text", text},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  static int setitems(lua_State* L) {
    boost::shared_ptr<T> combo_box = LuaHelper::check_sptr<T>(L, 1, meta);
    std::vector<std::string> itemlist;
    luaL_checktype(L, 2, LUA_TTABLE);
    int n = lua_rawlen(L, 2);
    for (int i = 1; i <= n; ++i) {
      lua_rawgeti(L, 2, i);
      const char* str = luaL_checkstring(L, -1);
      lua_pop(L, 1);
      itemlist.push_back(str);
    }
    combo_box->set_items(itemlist);
    return LuaHelper::chaining(L);
  }
  static int additem(lua_State* L) {
    boost::shared_ptr<T> combo_box = LuaHelper::check_sptr<T>(L, 1, meta);    
    const char* str = luaL_checkstring(L, -1);    
    combo_box->add_item(str);
    return LuaHelper::chaining(L);
  }
  static int items(lua_State* L) {
    boost::shared_ptr<T> combo_box = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_newtable(L);
    std::vector<std::string> itemlist = combo_box->items();
    std::vector<std::string>::iterator it = itemlist.begin();
    for (int i = 1; it != itemlist.end(); ++it, ++i) {
      lua_pushstring(L, (*it).c_str());
      lua_rawseti(L, -2, i);
    }    
    return 1;
  }
  static int itemindex(lua_State* L) {
    boost::shared_ptr<T> combo_box = LuaHelper::check_sptr<T>(L, 1, meta);
    lua_pushinteger(L, combo_box->item_index() + 1);
    return 1;
  };
  static int setitemindex(lua_State* L) {
    boost::shared_ptr<T> combo_box = LuaHelper::check_sptr<T>(L, 1, meta);
    int item_index = static_cast<int>(luaL_checkinteger(L, 2) - 1);
    combo_box->set_item_index(item_index);
    return LuaHelper::chaining(L);
  };
  static int text(lua_State* L) { LUAEXPORT(L, &T::text); }
};

struct LuaCenterToScreenBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int gc(lua_State* L);
  static int sizetoscreen(lua_State* L) { LUAEXPORTM(L, LuaCenterToScreenBind::meta, &ui::WindowCenterToScreen::SizeToScreen) };
};

struct LuaMenuBarBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int add(lua_State *L);  
  static int gc(lua_State* L);
  static int setrootnode(lua_State* L);  
  static int update(lua_State* L) { LUAEXPORTM(L, meta, &LuaMenuBar::Update); }
  static int invalidate(lua_State* L) { LUAEXPORTM(L, meta, &LuaMenuBar::Invalidate); }
};

struct LuaPopupMenuBind {
  static int open(lua_State *L);
  static const char* meta;
  static int create(lua_State *L);
  static int add(lua_State *L);  
  static int gc(lua_State* L);
  static int setrootnode(lua_State* L);  
  static int update(lua_State* L) { LUAEXPORTM(L, meta, &LuaPopupMenu::Update); }
  static int invalidate(lua_State* L) { LUAEXPORTM(L, meta, &LuaPopupMenu::Invalidate); }
  static int track(lua_State* L);
};

template <class T = LuaTreeView>
class LuaTreeViewBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
      {"setrootnode", setrootnode},
      {"addnode", addnode},      
//      {"clear", clear},
      {"settextcolor", settextcolor},
      {"setbackgroundcolor", setbackgroundcolor},
      {"backgroundcolor", backgroundcolor},
      {"isediting", isediting},
      {"updatetree", updatetree},
      {"selectnode", selectnode},
      {"editnode", editnode},
      {"selected", selected},
      {"showlines", showlines},
      {"hidelines", hidelines},
      {"showbuttons", showbuttons},
      {"hidebuttons", hidebuttons},
      {"setimages", setimages},
      {"setpopupmenu", setpopupmenu},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
    
  static int showlines(lua_State* L) { LUAEXPORT(L, &T::ShowLines); }
  static int hidelines(lua_State* L) { LUAEXPORT(L, &T::HideLines); }
  static int showbuttons(lua_State* L) { LUAEXPORT(L, &T::ShowButtons); }
  static int hidebuttons(lua_State* L) { LUAEXPORT(L, &T::HideButtons); }
  static int settextcolor(lua_State* L) { LUAEXPORTM(L, meta, &T::set_text_color); } 
  static int setbackgroundcolor(lua_State* L) { LUAEXPORT(L, &T::set_background_color) }
  static int backgroundcolor(lua_State* L) { LUAEXPORT(L, &T::background_color) }
  static int isediting(lua_State* L) { LUAEXPORT(L, &T::is_editing) }
  static int updatetree(lua_State* L) { LUAEXPORT(L, &T::UpdateTree); }
  static int selectnode(lua_State* L) { 
    boost::shared_ptr<T> tree_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node = 
      boost::dynamic_pointer_cast<ui::Node>(LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta));
    tree_view->select_node(node);
    return LuaHelper::chaining(L);
  }
  static int editnode(lua_State* L) { 
    boost::shared_ptr<T> tree_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node = 
      boost::dynamic_pointer_cast<ui::Node>(LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta));
    tree_view->EditNode(node);
    return LuaHelper::chaining(L);
  }
  static int selected(lua_State* L) {
    boost::shared_ptr<T> tree_view = LuaHelper::check_sptr<T>(L, 1, meta);
    boost::shared_ptr<ui::Node> tn = tree_view->selected().lock();
    if (tn.get()) {
      LuaHelper::find_weakuserdata(L, tn.get());
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  static int setrootnode(lua_State* L) {
    boost::shared_ptr<T> tree_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node = 
      boost::dynamic_pointer_cast<ui::Node>(LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta));
    tree_view->set_root_node(node);
    tree_view->UpdateTree();
    return LuaHelper::chaining(L);
  }
  static int addnode(lua_State* L) {    
    return LuaHelper::chaining(L);
  }
  static int gc(lua_State* L) {    
    return LuaHelper::delete_shared_userdata<T>(L, meta);
  }

  static int setimages(lua_State* L) {    
    boost::shared_ptr<T> tree_view = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Images::Ptr images = LuaHelper::check_sptr<ui::Images>(L, 2, LuaImagesBind::meta);
    tree_view->set_images(images);
    return LuaHelper::chaining(L);
  }

  static int setpopupmenu(lua_State* L) {    
    boost::shared_ptr<T> tree_view = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::PopupMenu::Ptr popup_menu = LuaHelper::check_sptr<ui::PopupMenu>(L, 2, LuaPopupMenuBind::meta);
    tree_view->set_popup_menu(popup_menu);
    return LuaHelper::chaining(L);
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

template <class T = LuaListView>
class LuaListViewBind : public LuaItemBind<T> {
 public:
  typedef LuaItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {      
      {"setrootnode", setrootnode},
      {"addcolumn", addcolumn},
//      {"clear", clear},
      {"settextcolor", settextcolor},
      {"setbackgroundcolor", setbackgroundcolor},
      {"backgroundcolor", backgroundcolor},
      {"isediting", isediting},
      {"updatelist", updatelist},
      {"selectnode", selectnode},
      {"selectednodes", selectednodes},
      {"editnode", editnode},
      {"selected", selected},
      {"setimages", setimages},
      {"viewlist", viewlist},
      {"viewreport", viewreport},
      {"viewicon", viewicon},
      {"viewsmallicon", viewsmallicon},
      {"enablerowselect", enablerowselect},
      {"diablerowselect", disablerowselect},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
    
  static int settextcolor(lua_State* L) { LUAEXPORTM(L, meta, &T::set_text_color); } 
  static int setbackgroundcolor(lua_State* L) { LUAEXPORT(L, &T::set_background_color) }
  static int backgroundcolor(lua_State* L) { LUAEXPORT(L, &T::background_color) }
  static int isediting(lua_State* L) { LUAEXPORT(L, &T::is_editing) }
  static int updatelist(lua_State* L) { LUAEXPORT(L, &T::UpdateList); }
  static int viewlist(lua_State* L) { LUAEXPORT(L, &T::ViewList); }
  static int viewreport(lua_State* L) { LUAEXPORT(L, &T::ViewReport); }
  static int viewicon(lua_State* L) { LUAEXPORT(L, &T::ViewIcon); }
  static int viewsmallicon(lua_State* L) { LUAEXPORT(L, &T::ViewSmallIcon); }
  static int selectednodes(lua_State* L) { 
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    lua_newtable(L);
    std::vector<ui::Node::Ptr> nodes = list_view->selected_nodes();
    std::vector<ui::Node::Ptr>::iterator it = nodes.begin();
    for (int i = 1; it != nodes.end(); ++i, ++it) {
      LuaHelper::find_weakuserdata(L, (*it).get());
      lua_rawseti(L, -2, i);
    }      
    return 1;
  }
  static int selectnode(lua_State* L) { 
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node = 
      boost::dynamic_pointer_cast<ui::Node>(LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta));
    list_view->select_node(node);
    return LuaHelper::chaining(L);
  }
  static int editnode(lua_State* L) { 
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node = 
      boost::dynamic_pointer_cast<ui::Node>(LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta));
    list_view->EditNode(node);
    return LuaHelper::chaining(L);
  }
  static int selected(lua_State* L) {
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);
    boost::shared_ptr<ui::Node> tn = list_view->selected().lock();
    if (tn.get()) {
      LuaHelper::find_weakuserdata(L, tn.get());
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  static int setrootnode(lua_State* L) {
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node = 
      boost::dynamic_pointer_cast<ui::Node>(LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta));
    list_view->set_root_node(node);
    list_view->UpdateList();
    return LuaHelper::chaining(L);
  }  
  static int addcolumn(lua_State* L) { 
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);
    list_view->AddColumn(luaL_checkstring(L, 2), static_cast<int>(luaL_checkinteger(L, 3)));
    return LuaHelper::chaining(L);
  }
  static int gc(lua_State* L) {    
    return LuaHelper::delete_shared_userdata<T>(L, meta);
  }
  static int setimages(lua_State* L) {    
    boost::shared_ptr<T> list_view = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Images::Ptr images = LuaHelper::check_sptr<ui::Images>(L, 2, LuaImagesBind::meta);
    list_view->set_images(images);
    return LuaHelper::chaining(L);
  }
  static int enablerowselect(lua_State* L) { LUAEXPORT(L, &T::EnableRowSelect); }
  static int disablerowselect(lua_State* L) { LUAEXPORT(L, &T::DisableRowSelect); }
  
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

class LuaNodeBind {
 public:
  static std::string meta;
  static int open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"settext", settext},
      {"setimage", setimage},
      {"setimageindex", setimageindex},
      {"setselectedimageindex", setselectedimageindex},
      {"text", text},
      {"add", add},
      {"insertafter", insertafter},
      {"size", size},
      {"at", at},
      {"remove", remove},
      {"clear", clear},
      {"parent", parent},
      {"level", level},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    return 1;
  }

  static int create(lua_State* L) {
    LuaHelper::new_shared_userdata(L, meta.c_str(), new ui::Node());
    lua_newtable(L);
    lua_setfield(L, -2, "_children");
    return 1;
  }

  static int gc(lua_State* L) {
    return LuaHelper::delete_shared_userdata<ui::Node>(L, meta);
  }

  static int settext(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::set_text); }
  static int text(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::text); }
  static int setimage(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, image");
    if (err!=0) return err;
    boost::shared_ptr<ui::Node> node = LuaHelper::check_sptr<ui::Node>(L, 1, meta);
    node->set_image(LuaHelper::check_sptr<ui::Image>(L, 2, LuaImageBind::meta));
    return LuaHelper::chaining(L);
  }
  static int setimageindex(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::set_image_index); }
  static int setselectedimageindex(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::set_selected_image_index); }
  static int add(lua_State* L) {
    boost::shared_ptr<ui::Node> treenode = LuaHelper::check_sptr<ui::Node>(L, 1, meta);    
    boost::shared_ptr<ui::Node> treenode2 = LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta);
    treenode->AddNode(treenode2);
    lua_getfield(L, -2, "_children");
    int len = lua_rawlen(L, -1);
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, len+1);
    return LuaHelper::chaining(L);
  }

  static int insertafter(lua_State* L) {
    boost::shared_ptr<ui::Node> node = LuaHelper::check_sptr<ui::Node>(L, 1, meta);    
    boost::shared_ptr<ui::Node> node1 = LuaHelper::check_sptr<ui::Node>(L, 2, LuaNodeBind::meta);
    boost::shared_ptr<ui::Node> node2 = LuaHelper::check_sptr<ui::Node>(L, 3, LuaNodeBind::meta);

    for (ui::Node::iterator it = node->begin(); it != node->end(); ++it) {
      if ((*it) == node2) {
        ++it;
        node->insert(it, node1);
        break;
      }
    }    
    lua_getfield(L, 1, "_children");
    int len = lua_rawlen(L, -1);
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, len+1);
    return LuaHelper::chaining(L);
  }
  static int remove(lua_State *L) {
    if (lua_isnumber(L, 2)) {
      boost::shared_ptr<ui::Node> treenode = LuaHelper::check_sptr<ui::Node>(L, 1, meta);
      int index = static_cast<int>(luaL_checkinteger(L, 2));
      if (index < 1 && index > treenode->size()) {
        luaL_error(L, "index out of range");
      }
      treenode->erase(treenode->begin() + index - 1);
      lua_getfield(L, -2, "_children");
      lua_pushnil(L);
      lua_rawseti(L, -2, index);      
      lua_gc(L, LUA_GCCOLLECT, 0);
      return 1;
    } else {
      boost::shared_ptr<ui::Node> node = LuaHelper::check_sptr<ui::Node>(L, 1, meta);
      boost::shared_ptr<ui::Node> node1 = LuaHelper::check_sptr<ui::Node>(L, 2, meta);

      lua_getfield(L, -2, "_children");
      int n = lua_rawlen(L, -1);
      for (int i = 1; i <= n; ++i) {
        lua_rawgeti(L, -1, i);
        boost::shared_ptr<ui::Node> node2 = LuaHelper::check_sptr<ui::Node>(L, lua_gettop(L), meta);
        if (node2 == node1) {          
          lua_pushnil(L);
          lua_rawseti(L, -2, i);          
          break;
        }
        lua_remove(L, -1);
      }
      for (ui::Node::iterator it = node->begin(); it != node->end(); ++it) {
        if ((*it) == node1) {          
          node->erase(it);
          break;
        }
      }
      lua_gc(L, LUA_GCCOLLECT, 0);
    }    
    return LuaHelper::chaining(L);
  }

  static int clear(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::clear); }

  static int parent(lua_State *L) {
    boost::shared_ptr<ui::Node> node = LuaHelper::check_sptr<ui::Node>(L, 1, meta);
    LuaHelper::find_weakuserdata(L, node->parent().lock().get());
    return 1;
  }

  static int size(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::size); }
  static int at(lua_State *L) {
    if (lua_isnumber(L, 2)) {
      boost::shared_ptr<ui::Node> treenode = LuaHelper::check_sptr<ui::Node>(L, 1, meta);  
      int index = static_cast<int>(luaL_checkinteger(L, 2));
      if (index < 1 && index >= treenode->size()) {
        luaL_error(L, "index out of range");
      }            
      boost::shared_ptr<ui::Node> tn = *(treenode->begin() + index - 1);
      if (tn.get()) {
        LuaHelper::find_weakuserdata(L, tn.get());
      } else {
        lua_pushnil(L);
      }      
      return 1;
    }
    return 0;
  }
  static int level(lua_State* L) { LUAEXPORTM(L, meta, &ui::Node::level); }
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
      {"title", title},
      {"setviewport", setviewport},
      {"view", view},
      {"showdecoration", showdecoration},
      {"hidedecoration", hidedecoration},
      {"setpopupmenu", setpopupmenu},
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
 
	static int create(lua_State* L) {		
		boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
		LuaHelper::register_weakuserdata(L, item.get());
		return 1;
	}
  static int gc(lua_State* L) {
    LuaHelper::delete_shared_userdata<LuaFrameWnd>(L, meta);    
    return 0;
  }
  static int setviewport(lua_State* L) {
    LuaFrameWnd::Ptr frame = LuaHelper::check_sptr<LuaFrameWnd>(L, 1, meta);    
/*    ui::Canvas::WeakPtr old_canvas = wnd->canvas();
    if (!old_canvas.expired()) {
      LuaHelper::unregister_userdata(L, old_canvas.lock().get());
    }*/
    LuaCanvas::Ptr canvas = LuaHelper::check_sptr<LuaCanvas>(L, 2, LuaCanvasBind<>::meta);
    if (canvas) {
      LuaHelper::register_userdata(L, canvas.get());
      frame->set_viewport(canvas);      
    }    
    return LuaHelper::chaining(L);
  }
  static int settitle(lua_State* L)  { LUAEXPORT(L, &T::set_title); }
  static int title(lua_State* L)  { LUAEXPORT(L, &T::title); }
  static int showdecoration(lua_State* L)  { LUAEXPORT(L, &T::ShowDecoration); }
  static int hidedecoration(lua_State* L)  { LUAEXPORT(L, &T::HideDecoration); }  
	static int view(lua_State* L) {		
		boost::shared_ptr<T> frame = LuaHelper::check_sptr<T>(L, 1, meta);
		LuaHelper::find_weakuserdata<>(L, frame->viewport().get());
		return 1;		
	}
  static int setpopupmenu(lua_State* L) {    
    boost::shared_ptr<T> frame = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::PopupMenu::Ptr popup_menu = LuaHelper::check_sptr<ui::PopupMenu>(L, 2, LuaPopupMenuBind::meta);
    frame->set_popup_menu(popup_menu);
    return LuaHelper::chaining(L);
  }
};

typedef LuaFrameItemBind<LuaPopupFrameWnd> LuaPopupFrameItemBind;

/*template <class T = LuaPopupFrameWnd>
class LuaPopupFrameItemBind : public LuaFrameItemBind<T> {
 public:
  typedef LuaFrameItemBind<T> B;
  static int open(lua_State *L) { return LuaHelper::openex(L, meta, setmethods, gc); }
  static int setmethods(lua_State* L) {
    B::setmethods(L);
    static const luaL_Reg methods[] = {
      {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }   
};*/



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
       {"f", f},       
       {"gotoline", gotoline},
       {"length", length},
       {"loadfile", loadfile},
       {"reload", reload},
       {"savefile", savefile},
       {"filename", filename},
       {"hasfile", hasfile},
       {"addtext", addtext},       
			 {"clearall", clearall},
       {"findtext", findtext},
       {"addmarker", addmarker},
       {"deletemarker", deletemarker},
       {"definemarker", definemarker},
       {"selectionstart", selectionstart},
       {"selectionend", selectionend},
       {"setsel", setsel},
       {"hasselection", hasselection},
       {"replacesel", replacesel},
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
       {"setfont", setfont},
       {"line", line},
       {"column", column},
       {"ovrtype", ovrtype},
       {"modified", modified},
       {"showcaretline", showcaretline},
       {"hidecaretline", hidecaretline},
       {"setcaretlinebackgroundcolor", setcaretlinebackgroundcolor},
       {"undo", undo},
       {"redo", redo},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  

  static int f(lua_State *L) {
    boost::shared_ptr<T> sc = LuaHelper::check_sptr<LuaScintilla>(L, 1, meta);    
    int sci = static_cast<int>(luaL_checkinteger(L, 2));
    WPARAM wparam(0);
    LPARAM lparam(0);    
    switch (lua_type(L, 3)) {
      case LUA_TSTRING:
			  wparam = (WPARAM) lua_tostring(L, 3);
			break;
			case LUA_TBOOLEAN:
				wparam = (WPARAM) lua_toboolean(L, 3);
			break;
			case LUA_TNUMBER:
				wparam = (WPARAM) lua_tonumber(L, 3);
			break;
      default:
        luaL_error(L, "Wrong argument type");
    }
    switch (lua_type(L, 4)) {
      case LUA_TSTRING:
			  lparam = (LPARAM) lua_tostring(L, 4);
			break;
			case LUA_TBOOLEAN:
				lparam = (LPARAM) lua_toboolean(L, 4);
			break;
			case LUA_TNUMBER:
				lparam = (LPARAM) lua_tonumber(L, 4);
			break;
      default:
        luaL_error(L, "Wrong argument type");
    }
    int r = sc->f(sci, (void*) wparam, (void*) lparam);
    lua_pushinteger(L, r);
    return 1;
  }

  static int definemarker(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    int val1 = static_cast<int>(luaL_checkinteger(L, 2));
    int val2 = static_cast<int>(luaL_checkinteger(L, 3));
    int val3 = static_cast<int>(luaL_checkinteger(L, 4));
    int val4 = static_cast<int>(luaL_checkinteger(L, 5));
    item->define_marker(val1, val2, val3, val4);  
    return 0;
  }  
  static int showcaretline(lua_State *L) { LUAEXPORT(L, &T::ShowCaretLine); } 
  static int hidecaretline(lua_State *L) { LUAEXPORT(L, &T::HideCaretLine); }   
  static int addmarker(lua_State *L) { LUAEXPORT(L, &T::add_marker); } 
  static int setcaretlinebackgroundcolor(lua_State *L) { LUAEXPORT(L, &T::set_caret_line_background_color); } 
  static int deletemarker(lua_State *L) { LUAEXPORT(L, &T::delete_marker); } 
  static int setsel(lua_State *L) { LUAEXPORT(L, &T::SetSel); } 
  static int selectionstart(lua_State *L) { LUAEXPORT(L, &T::selectionstart); } 
  static int selectionend(lua_State *L) { LUAEXPORT(L, &T::selectionend); } 
  static int hasselection(lua_State *L) { LUAEXPORT(L, &T::has_selection); } 
  static int replacesel(lua_State *L) { LUAEXPORT(L, &T::ReplaceSel); } 
  static int gotoline(lua_State *L) { LUAEXPORT(L, &T::GotoLine); }  
  static int length(lua_State *L) { LUAEXPORT(L, &T::length); }  
  static int addtext(lua_State *L) { LUAEXPORT(L, &T::AddText); } 
  static int findtext(lua_State *L) { LUAEXPORT(L, &T::FindText); }  
  static int clear(lua_State *L) { LUAEXPORT(L, &T::RemoveAll); }  
  static int loadfile(lua_State *L) { LUAEXPORT(L, &T::LoadFile); }
  static int reload(lua_State *L) { LUAEXPORT(L, &T::Reload); }
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
  static int setfont(lua_State *L) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);    
    ui::FontInfo font_info;
    font_info.name = luaL_checkstring(L, 2);
    font_info.height = static_cast<int>(luaL_checkinteger(L, 3));
    item->set_font(font_info);
    return LuaHelper::chaining(L);
  }

  static int line(lua_State *L) { LUAEXPORT(L, &T::line); }
  static int column(lua_State *L) { LUAEXPORT(L, &T::column); }
  static int ovrtype(lua_State *L) { LUAEXPORT(L, &T::ovr_type); }
  static int modified(lua_State *L) { LUAEXPORT(L, &T::modified); }
	static int clearall(lua_State *L) { LUAEXPORT(L, &T::ClearAll); }
  static int undo(lua_State *L) { LUAEXPORT(L, &T::Undo); } 
  static int redo(lua_State *L) { LUAEXPORT(L, &T::Redo); } 
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
       {"settextcolor", settextcolor},
       {"setbackgroundcolor", setbackgroundcolor},
       {"backgroundcolor", backgroundcolor},
       {"setfont", setfont},
       {NULL, NULL}
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }  
  static int settext(lua_State *L) { LUAEXPORT(L, &T::set_text); }
  static int text(lua_State *L) { LUAEXPORT(L, &T::text); }
  static int settextcolor(lua_State* L) { LUAEXPORTM(L, meta, &T::set_text_color); } 
  static int setbackgroundcolor(lua_State* L) { LUAEXPORT(L, &T::set_background_color) }
  static int backgroundcolor(lua_State* L) { LUAEXPORT(L, &T::background_color) }
  static int setfont(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 2, "self, font");
    if (err!=0) return err;
    boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "name");
    lua_getfield(L, 2, "height");
    const char *name = luaL_checkstring(L, -2);
    double height = luaL_checknumber(L, -1);
    ui::Font font;
    ui::FontInfo font_info;
    font_info.name = name;
    font_info.height = static_cast<int>(height);
    font.set_info(font_info);
    window->set_font(font);
    return LuaHelper::chaining(L);
  }
  
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
      {"setpos", setposition},
      {"setscrollpos", setscrollposition},
      {"scrollpos", scrollposition},
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
	static int create(lua_State *L) {		
		int n = lua_gettop(L);  // Number of arguments  
		boost::shared_ptr<LuaGroup> group;
		if (n >= 2 && !lua_isnil(L, 2)) {
			group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
			if (!group) {
				group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaHeaderGroupBind<>::meta);
				if (!group) {
					group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
				}
			}
		}
		ui::Orientation orientation = ui::VERT;
		if (n == 3) {
			orientation = static_cast<ui::Orientation>(luaL_checkinteger(L, 3));
		}
		boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L, orientation));
		if (group) {
			group->Add(item);
			LuaHelper::register_userdata(L, item.get());
		}
		LuaHelper::new_lua_module(L, "psycle.signal");
		lua_setfield(L, -2, "keydown");
		return 1;		
	}
  static int setscrollposition(lua_State* L) { LUAEXPORT(L, &T::set_scroll_position); }
  static int scrollposition(lua_State* L) { LUAEXPORT(L, &T::scroll_position); }
  static int setscrollrange(lua_State* L) { LUAEXPORT(L, &T::set_scroll_range); }
  static int scrollrange(lua_State* L) { LUAEXPORT(L, &T::scroll_range); }
  static int systemsize(lua_State* L) { LUAEXPORT(L, &T::system_size); }   
};

template class LuaRectangleBoxBind<LuaRectangleBox>;
template class LuaLineBind<LuaLine>;
template class LuaTextBind<LuaText>;
template class LuaPicBind<LuaPic>;
template class LuaButtonBind<LuaButton>;
template class LuaRadioButtonBind<LuaRadioButton>;
template class LuaGroupBoxBind<LuaGroupBox>;
template class LuaEditBind<LuaEdit>;
template class LuaScrollBarBind<LuaScrollBar>;
template class LuaScintillaBind<LuaScintilla>;
template class LuaComboBoxBind<LuaComboBox>;
template class LuaTreeViewBind<LuaTreeView>;
template class LuaListViewBind<LuaListView>;
template class LuaFrameItemBind<LuaFrameWnd>;
template class LuaFrameItemBind<LuaPopupFrameWnd>;
template class LuaScrollBoxBind<LuaScrollBox>;
template class LuaSplitterBind<LuaSplitter>;


static int lua_ui_requires(lua_State* L) {
  // ui binds
  LuaHelper::require<LuaPointBind>(L, "psycle.ui.point");
  LuaHelper::require<LuaDimensionBind>(L, "psycle.ui.dimension");
  LuaHelper::require<LuaUiRectBind>(L, "psycle.ui.rect");
  LuaHelper::require<LuaRegionBind>(L, "psycle.ui.region");
	LuaHelper::require<LuaAreaBind>(L, "psycle.ui.area");
  LuaHelper::require<LuaImageBind>(L, "psycle.ui.image");
  LuaHelper::require<LuaImagesBind>(L, "psycle.ui.images");
  LuaHelper::require<LuaGraphicsBind>(L, "psycle.ui.graphics");
  LuaHelper::require<LuaColorHelper>(L, "psycle.ui.colorhelper");
  LuaHelper::require<LuaGameControllersBind>(L, "psycle.ui.gamecontrollers");
  LuaHelper::require<LuaGameControllerBind>(L, "psycle.ui.gamecontroller");
  // filedialog
  LuaHelper::require<LuaFileOpenBind>(L, "psycle.ui.fileopen");
  LuaHelper::require<LuaFileSaveBind>(L, "psycle.ui.filesave");
  // ui menu binds
  LuaHelper::require<LuaMenuBarBind>(L, "psycle.ui.menubar");
  LuaHelper::require<LuaPopupMenuBind>(L, "psycle.ui.popupmenu");
  LuaHelper::require<LuaSystemMetrics>(L, "psycle.ui.systemmetrics");  
  // ui canvas binds
  LuaHelper::require<LuaCanvasBind<> >(L, "psycle.ui.canvas");
  LuaHelper::require<LuaFrameItemBind<> >(L, "psycle.ui.canvas.frame");
  LuaHelper::require<LuaPopupFrameItemBind >(L, "psycle.ui.canvas.popupframe");
  LuaHelper::require<LuaCenterToScreenBind>(L, "psycle.ui.canvas.centertoscreen");
  LuaHelper::require<LuaGroupBind<> >(L, "psycle.ui.canvas.group");
  LuaHelper::require<LuaHeaderGroupBind<> >(L, "psycle.ui.canvas.headergroup");
  LuaHelper::require<LuaScrollBoxBind<> >(L, "psycle.ui.canvas.scrollbox");
  LuaHelper::require<LuaItemBind<> >(L, "psycle.ui.canvas.item");
  LuaHelper::require<LuaLineBind<> >(L, "psycle.ui.canvas.line");
  LuaHelper::require<LuaPicBind<> >(L, "psycle.ui.canvas.pic");  
  LuaHelper::require<LuaRectangleBoxBind<> >(L, "psycle.ui.canvas.rectanglebox");
  LuaHelper::require<LuaTextBind<> >(L, "psycle.ui.canvas.text");
  LuaHelper::require<LuaSplitterBind<> >(L, "psycle.ui.canvas.splitter");
  LuaHelper::require<LuaTreeViewBind<> >(L, "psycle.ui.canvas.treeview");
  LuaHelper::require<LuaListViewBind<> >(L, "psycle.ui.canvas.listview");
  LuaHelper::require<LuaNodeBind>(L, "psycle.node");  
  LuaHelper::require<LuaButtonBind<> >(L, "psycle.ui.canvas.button");
  LuaHelper::require<LuaRadioButtonBind<> >(L, "psycle.ui.canvas.radiobutton");
  LuaHelper::require<LuaGroupBoxBind<> >(L, "psycle.ui.canvas.groupbox");
  LuaHelper::require<LuaComboBoxBind<> >(L, "psycle.ui.canvas.combobox");
  LuaHelper::require<LuaEditBind<> >(L, "psycle.ui.canvas.edit");
  LuaHelper::require<LuaLexerBind>(L, "psycle.ui.canvas.lexer");
  LuaHelper::require<LuaScintillaBind<> >(L, "psycle.ui.canvas.scintilla");
  LuaHelper::require<LuaScrollBarBind<> >(L, "psycle.ui.canvas.scrollbar");
  LuaHelper::require<LuaEventBind>(L, "psycle.ui.canvas.event");
  LuaHelper::require<LuaKeyEventBind>(L, "psycle.ui.canvas.keyevent");
  LuaHelper::require<LuaMouseEventBind>(L, "psycle.ui.canvas.mouseevent");
  LuaHelper::require<OrnamentFactoryBind>(L, "psycle.ui.canvas.ornamentfactory");
  LuaHelper::require<LineBorderBind>(L, "psycle.ui.canvas.lineborder");
  LuaHelper::require<WallpaperBind>(L, "psycle.ui.canvas.wallpaper");
  LuaHelper::require<FillBind>(L, "psycle.ui.canvas.fill");
  return 0;
}

} // namespace host
} // namespace psycle