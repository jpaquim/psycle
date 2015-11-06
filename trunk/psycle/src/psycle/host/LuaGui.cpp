// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "LuaGui.hpp"

#include "PsycleConfig.hpp"
#include "LuaHelper.hpp"
#include "LuaHost.hpp"
#include "LuaPlugin.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

namespace psycle { namespace host {
  using namespace ui;  

  ///////////////////////////////////////////////////////////////////////////////
  // MenuBinds
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaMenuBarBind::meta = "psymenubarmeta";

  int LuaMenuBarBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"add", add},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaMenuBarBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    MenuBar* menubar = new mfc::MenuBar();
    LuaHelper::new_userdata<>(L, meta, menubar);
    return 1;
  }

  int LuaMenuBarBind::add(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, menu");
    if (err!=0) return err;
    MenuBar* menubar = LuaHelper::check<MenuBar>(L, 1, meta);
    Menu* menu = LuaHelper::check<Menu>(L, 2, LuaMenuBind::meta);
    menubar->add(menu);
    LuaHelper::register_userdata<>(L, menu);
    return LuaHelper::chaining(L);
  }

  int LuaMenuBarBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<MenuBar>(L, meta);
  }
  
  //menu
  const char* LuaMenuBind::meta = "psymenumeta";

  int LuaMenuBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"add", add},
      {"remove", remove},
      {"setlabel", setlabel},
      {"addseparator", addseparator},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaMenuBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, menuname");
    if (err!=0) return err;
    Menu* menu = new mfc::Menu();
    menu->cmenu()->CreatePopupMenu();
    const char* label = luaL_checkstring(L, 2);
    menu->set_label(label);
    LuaHelper::new_userdata<>(L, meta, menu);
    return 1;
  }

  int LuaMenuBind::remove(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, menu or menuitem");
    if (err!=0) return err;
    Menu* menu = LuaHelper::check<Menu>(L, 1, meta);
    MenuItem* item = 0;
    item = LuaHelper::test<MenuItem>(L, 2, LuaMenuItemBind::meta);
    if (!item) {
      /*LuaMenu* new_menu= LuaHelper::test<LuaMenu>(L, 2, LuaMenuBind::meta);
      if (new_menu) {
        int pos = menu->menu()->GetMenuItemCount()-1;
        menu->menu()->AppendMenu(MF_POPUP, (UINT_PTR)new_menu->menu()->m_hMenu, new_menu->label().c_str());
        new_menu->set_parent(menu);
        new_menu->set_pos(pos);
        LuaHelper::register_userdata<>(L, new_menu);
      }*/
    } else {
      menu->remove(item);
      LuaHelper::unregister_userdata(L, item);
    }
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::add(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, menu or menuitem");
    if (err!=0) return err;
    Menu* menu = LuaHelper::check<Menu>(L, 1, meta);
    MenuItem* item = 0;
    item = LuaHelper::test<MenuItem>(L, 2, LuaMenuItemBind::meta);
    if (!item) {
      Menu* new_menu= LuaHelper::test<Menu>(L, 2, LuaMenuBind::meta);
      if (new_menu) {
        menu->add(new_menu);
        LuaHelper::register_userdata(L, new_menu);
      } else {
        return luaL_error(L, "Argument is no menu or menuitem.");
      }
    } else {
      menu->add(item);
      LuaHelper::register_userdata(L, item);
    }
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::addseparator(lua_State* L) {
    LuaHelper::call(L, meta, &Menu::addseparator);
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::setlabel(lua_State* L) {
    LuaHelper::callstrictstr(L, meta, &Menu::set_label);
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<Menu>(L, meta);
  }
    
  //menuitem
  const char* LuaMenuItemBind::meta = "psymenuitemmeta";  

  int LuaMenuItemBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setlabel", setlabel},
      {"label", label},
      {"id", id},
      {"check", check},
      {"uncheck", uncheck},
      {"checked", checked},
      {"addlistener", addlistener},
      {"notify", notify},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaMenuItemBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, itemname");
    if (err!=0) return err;
    const char* label = luaL_checkstring(L, 2);
    MenuItem* item = new mfc::MenuItem();
    item->set_id(item->id_counter);
    item->id_counter++;
    item->set_label(label);
    LuaHelper::new_userdata(L, meta, item);
    lua_newtable(L);
    lua_setfield(L, -2, "listener_");
    return 1;
  }

  int LuaMenuItemBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<MenuItem>(L, meta);
  }

  int LuaMenuItemBind::setlabel(lua_State* L) {
    LuaHelper::callstrictstr(L, meta, &MenuItem::set_label);
    return LuaHelper::chaining(L);
  }

  int LuaMenuItemBind::label(lua_State* L) {
    return LuaHelper::getstring(L, meta, &MenuItem::label);
  }

  int LuaMenuItemBind::checked(lua_State* L) {
    return LuaHelper::getbool(L, meta, &MenuItem::checked);
  }

  int LuaMenuItemBind::id(lua_State* L) {
    return LuaHelper::getnumber(L, meta, &MenuItem::id);
  }

  int LuaMenuItemBind::check(lua_State* L) {
    LuaHelper::call(L, meta, &MenuItem::check);
    return LuaHelper::chaining(L);
  }

  int LuaMenuItemBind::uncheck(lua_State* L) {
    LuaHelper::call(L, meta, &MenuItem::uncheck);
    return LuaHelper::chaining(L);
  }

  int LuaMenuItemBind::addlistener(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, index");
    if (err!=0) return err;
    lua_getfield(L, 1, "listener_");
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, lua_rawlen(L, -2)+1);
    return LuaHelper::chaining(L);
  }

  int LuaMenuItemBind::notify(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    lua_getfield(L, 1, "listener_");
    size_t len = lua_rawlen(L, -1);
    for (size_t i = 1; i <= len; ++i) {
      lua_rawgeti(L, 2, i);
      lua_getfield(L, -1, "onmenu");
      lua_pushvalue(L, -2);
      lua_pushvalue(L, 1);
      lua_pcall(L, 2, 0, 0);
      lua_pop(L, 1);
    }
    return 0;
  }

  /////////////////////////////////////////////////////////////////////////////
  // PsycleActions + Lua Bind
  /////////////////////////////////////////////////////////////////////////////

  void LuaActionListener::OnNotify(ActionType action) {
    if (mac_) mac_->lock();
    LuaHelper::find_userdata<>(L, this);
    lua_getfield(L, -1, "onnotify");
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    lua_pushnumber(L, action);
    int status = lua_pcall(L, 2, 0, 0);
    if (status) {
      if (mac_) mac_->unlock();
      const char* msg = lua_tostring(L, -1);
      throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));
    }
    if (mac_) mac_->unlock();
  }

  const char* LuaActionListenerBind::meta = "psyactionlistenermeta";

  int LuaActionListenerBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      { NULL, NULL }
    };
    LuaHelper::open(L, meta, methods,  gc);    
    static const char* const e[] = {
      "TPB", "BPM", "TRKNUM", "PLAY", "PLAYSTART", "PLAYSEQ", "STOP", "REC",
      "SEQSEL", "SEQFOLLOWSONG"
    };
    LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]), 1);
    return 1;
  }

  int LuaActionListenerBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    LuaActionListener* listener = new LuaActionListener(L);
    LuaHelper::new_userdata<>(L, meta, listener);
    LuaHelper::register_userdata<>(L, listener);
    return 1;
  }

  int LuaActionListenerBind::gc(lua_State* L) {
    LuaActionListener* listener = *(LuaActionListener **)luaL_checkudata(L, 1, meta);
    PsycleGlobal::actionHandler().RemoveListener(listener);
    return LuaHelper::delete_userdata<LuaActionListener>(L, meta);
  }

 
  /////////////////////////////////////////////////////////////////////////////
  // LuaFrameWnd+Bind
  /////////////////////////////////////////////////////////////////////////////

  void LuaFrameWnd::OnEvent(int msg) {    
    int n1 = lua_gettop(L);    
    LuaHelper::find_weakuserdata<>(L, this);    
    bool has_event_method = false;    
    if (!lua_isnil(L, -1)) {
      switch (msg) {
        case 1:
          lua_getfield(L, -1, "onclose");
        break;        
        default: return;
      }
      has_event_method = !lua_isnil(L, -1);
      if (has_event_method) {
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_newtable(L); // build event table        
        lua_pushnumber(L, 1);
        lua_setfield(L, -2, "dummy");        
        int status = lua_pcall(L, 2, 0, 0);
        if (status) {
          const char* msg = lua_tostring(L, -1);
          throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));
        }
      } else {
        lua_pop(L, 2);
      }
    } else {
      lua_pop(L, 1);
    }
    int n2 = lua_gettop(L);
    assert(n1==n2);
    lua_gc(L, LUA_GCCOLLECT, 0);
    //return has_event_method;
  }
  
  const char* LuaFrameWndBind::meta = "psyframemeta";

  int LuaFrameWndBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setpos", setpos},
      {"settitle", settitle},      
      {"show", show},
      {"hide", hide},
      {"setcanvas", setcanvas},      
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);    
    return 1;
  }

  int LuaFrameWndBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    LuaFrameWnd* frame = new LuaFrameWnd(L);
    frame->Create(NULL,"WindowsApplication",WS_OVERLAPPEDWINDOW, 
                       CRect(120, 100, 700, 480), NULL);
    LuaHelper::new_userdata(L, meta, frame);    
    LuaHelper::register_weakuserdata(L, frame);
    return 1;
  }

  int LuaFrameWndBind::gc(lua_State* L) {
    LuaFrameWnd* wnd = *(LuaFrameWnd**) luaL_checkudata(L, 1, meta);
    if (wnd->canvas()) {
      LuaHelper::unregister_userdata(L, wnd->canvas());
    }    
    wnd->DestroyWindow();  
    return 0;
  }

  int LuaFrameWndBind::show(lua_State* L) {
    LuaFrameWnd* wnd = LuaHelper::check<LuaFrameWnd>(L, 1, meta);
    wnd->ShowWindow(SW_SHOWNORMAL);
    wnd->PostOpenWnd();
    return LuaHelper::chaining(L);
  }

  int LuaFrameWndBind::hide(lua_State* L) {
    LuaFrameWnd* wnd = LuaHelper::check<LuaFrameWnd>(L, 1, meta);
    wnd->ShowWindow(SW_HIDE);
    return LuaHelper::chaining(L);
  }

  int LuaFrameWndBind::setcanvas(lua_State* L) {
    LuaFrameWnd* wnd = LuaHelper::check<LuaFrameWnd>(L, 1, meta);
    if (wnd->canvas()) {
      LuaHelper::unregister_userdata(L, wnd->canvas());
    }
    canvas::Canvas* canvas = LuaHelper::check<LuaCanvas>(L, 2, LuaCanvasBind::meta);
    LuaHelper::register_userdata(L, canvas);
    wnd->set_canvas(canvas);    
    return LuaHelper::chaining(L);
  }

  int LuaFrameWndBind::settitle(lua_State* L) {
    LuaFrameWnd* wnd = LuaHelper::check<LuaFrameWnd>(L, 1, meta);
    const char* title = luaL_checkstring(L, 2);
    wnd->SetTitle(title);
    return LuaHelper::chaining(L);
  }

  int LuaFrameWndBind::setpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &LuaFrameWnd::SetPos);    
    return LuaHelper::chaining(L);
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // LuaCanvasBind
  ///////////////////////////////////////////////////////////////////////////////
  template<class T>
  bool CallEvents(lua_State* L, canvas::Event* ev, T* that, bool is_canvas = false) {
    int n1 = lua_gettop(L);
    if (is_canvas) {
      LuaHelper::find_weakuserdata<>(L, that);
    } else {
      LuaHelper::find_userdata<>(L, that);
    }
    bool has_event_method = false;
    bool is_key = false;
    if (!lua_isnil(L, -1)) {
      switch (ev->type()) {
        case canvas::Event::BUTTON_PRESS:
          lua_getfield(L, -1, "onmousedown");
        break;
        case canvas::Event::BUTTON_RELEASE:
          lua_getfield(L, -1, "onmouseup");
        break;
        case canvas::Event::BUTTON_2PRESS:
          lua_getfield(L, -1, "ondblclick");
        break;
        case canvas::Event::MOTION_NOTIFY:
          lua_getfield(L, -1, "onmousemove");
        break;
        case canvas::Event::MOTION_OUT:
          lua_getfield(L, -1, "onmouseout");
        break;
        case canvas::Event::KEY_DOWN:
          is_key = true;
          lua_getfield(L, -1, "onkeydown");
        break;
        case canvas::Event::KEY_UP:
          is_key = true;
          lua_getfield(L, -1, "onkeyup");
        break;
        case canvas::Event::SCROLL:
          lua_getfield(L, -1, "onscroll");
        break;
        case canvas::Event::ONSIZE:
          lua_getfield(L, -1, "onsize");
        break;
        case canvas::Event::ONTIMER:
          lua_getfield(L, -1, "ontimer");
        break;
        default: return true;
      }
      has_event_method = !lua_isnil(L, -1);
      if (has_event_method) {
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_newtable(L); // build event table
        if (ev->type() == canvas::Event::ONTIMER) {
        } else
        if (ev->type() == canvas::Event::ONSIZE) {
          lua_pushnumber(L, ev->cx());
          lua_setfield(L, -2, "width");
          lua_pushnumber(L, ev->cy());
          lua_setfield(L, -2, "height");
        } else
        if (ev->type() == canvas::Event::SCROLL) {
          lua_pushnumber(L, ev->cx());
          lua_setfield(L, -2, "hpos");
          lua_pushnumber(L, ev->cy());
          lua_setfield(L, -2, "vpos");
        } else {
          lua_pushnumber(L, ev->shift());
          lua_setfield(L, -2, "shift");
          lua_pushboolean(L, MK_SHIFT & ev->shift());
          lua_setfield(L, -2, "shiftkey");
          lua_pushboolean(L, MK_CONTROL & ev->shift());
          lua_setfield(L, -2, "ctrlkey");
          lua_pushboolean(L, MK_ALT & ev->shift());
          lua_setfield(L, -2, "altkey");
          if (!is_key) {
            lua_pushnumber(L, ev->cx());
            lua_setfield(L, -2, "clientx");
            lua_pushnumber(L, ev->cy());
            lua_setfield(L, -2, "clienty");
            lua_pushnumber(L, ev->cx()-that->zoomabsx());
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, ev->cy()-that->zoomabsy());
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, ev->button());
            lua_setfield(L, -2, "button");
          } else {
            // todo key events
            lua_pushnumber(L, ev->button());
            lua_setfield(L, -2, "keycode");
          }
        }
        int status = lua_pcall(L, 2, 0, 0);
        if (status) {
          const char* msg = lua_tostring(L, -1);
          throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));
        }
      } else {
        lua_pop(L, 2);
      }
    } else {
      lua_pop(L, 1);
    }
    int n2 = lua_gettop(L);
    assert(n1==n2);
    return has_event_method;
  }

  bool LuaGroup::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, this); }
  bool LuaRect::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, this); }
  bool LuaLine::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, this); }  
  bool LuaText::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, this); }
  
  
  /////////////////////////////////////////////////////////////////////////////
  // LuaCanvas+Bind
  /////////////////////////////////////////////////////////////////////////////

  const char* LuaCanvasBind::meta = "psycanvasmeta";

  canvas::Item* LuaCanvas::OnEvent(canvas::Event* ev) {
    canvas::Item* item = canvas::Canvas::OnEvent(ev);
    CallEvents(L, ev, this, true);
    return item;
  }

  int LuaCanvasBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"root", root},
      {"setcolor", setcolor},
      {"color", color},
      {"size", size},
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
    LuaHelper::open(L, meta, methods,  gc);    
    static const char* const e[] = {
      "AUTO", "MOVE", "NO_DROP", "COL_RESIZE", "ALL_SCROLL", "POINTER",
      "NOT_ALLOWED", "ROW_RESIZE", "CROSSHAIR", "PROGRESS", "E_RESIZE",
      "NE_RESIZE", "DEFAULT", "TEXT", "N_RESIZE", "NW_RESIZE", "HELP",
      "VERTICAL_TEXT", "S_RESIZE", "SE_RESIZE", "INHERIT", "WAIT",
      "W_RESIZE", "SW_RESIZE"
    };
    lua_newtable(L); // setcursor enum          
    LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]));
    lua_setfield(L, -2, "CURSOR");
    return 1;
  }

  int LuaCanvasBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    canvas::Canvas* canvas = new LuaCanvas(L);
    LuaHelper::new_userdata(L, meta, canvas);
    luaL_requiref(L, LuaGroupBind<>::meta.c_str(), LuaGroupBind<>::open, 0);
    LuaHelper::new_userdata(L, LuaGroupBind<>::meta, canvas->root(), 3);
    LuaHelper::register_userdata(L, canvas->root());
    lua_pushvalue(L, 2);
    LuaHelper::register_weakuserdata<>(L, canvas);
    return 1;
  }

  int LuaCanvasBind::gc(lua_State* L) {
    LuaCanvas** canvas = (LuaCanvas**) luaL_checkudata(L, -1, meta);
    LuaHelper::unregister_userdata(L, (*canvas)->root());
    return LuaHelper::delete_userdata<LuaCanvas>(L, meta);
  }

  int LuaCanvasBind::root(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    LuaHelper::find_userdata(L, canvas->root());
    return 1;
  }

  int LuaCanvasBind::size(lua_State* L) {
    return LuaHelper::get2numbers<canvas::Canvas, int, int>(L, meta, &canvas::Canvas::cw, &canvas::Canvas::ch);
  }

  int LuaCanvasBind::preferredsize(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    double w, h;
    canvas->preferredsize(w, h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
  }

  int LuaCanvasBind::setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Canvas::SetColor);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::color(lua_State* L) {
    return LuaHelper::getnumber(L, meta, &LuaCanvas::color);
  }

  int LuaCanvasBind::setpreferredsize(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::Canvas::setpreferredsize);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::setcapture(lua_State* L) {
    LuaHelper::call(L, meta, &LuaCanvas::SetCapture);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::releasecapture(lua_State* L) {
    LuaHelper::call(L, meta, &LuaCanvas::ReleaseCapture);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::setcursor(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "Wrong number of arguments.");
    if (err!=0) return err;
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    CursorStyle style = (CursorStyle) (int) luaL_checknumber(L, 2);
    canvas->SetCursor(style);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::showcursor(lua_State* L) {
    LuaHelper::call(L, meta, &LuaCanvas::ShowCursor);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::hidecursor(lua_State* L) {
    LuaHelper::call(L, meta, &LuaCanvas::HideCursor);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::setcursorpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::Canvas::SetCursorPos);
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::showscrollbar(lua_State* L) {
    LuaCanvas* canvas = LuaHelper::check<LuaCanvas>(L, 1, meta);
    canvas->show_scrollbar = true;
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::setscrollinfo(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, nposh, nposv");
    if (err!=0) return err;
    LuaCanvas* canvas = LuaHelper::check<LuaCanvas>(L, 1, meta);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "nposh");
    lua_getfield(L, 2, "nposv");
    canvas->nposh = luaL_checknumber(L, -2);
    canvas->nposv = luaL_checknumber(L, -1);
    canvas->show_scrollbar = true;
    return LuaHelper::chaining(L);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaItemBind
  ///////////////////////////////////////////////////////////////////////////////

  template <class T>
  int LuaItemBind<T>::open(lua_State *L) {   
    LuaHelper::openmeta(L, meta, gc);    
    lua_newtable(L);
    setmethods(L);
    return 1; // LuaHelper::open(L, meta.c_str(), methods,  gc);
  }

  template <class T>
  int LuaItemBind<T>::setmethods(lua_State* L) {
    static const luaL_Reg methods[] = {
      {"new", create},    
      {"setpos", setpos},
      {"pos", pos},
      {"clientpos", clientpos},
      {"getfocus", getfocus},
      // {"setzoom", setzoom},      
      {"show", show},
      {"hide", hide},
      {"enablepointerevents", enablepointerevents},
      {"disablepointerevents", disablepointerevents},
//      {"tostring", tostring},
      {"parent", parent},
      {"boundrect", boundrect},
      {"canvas", canvas},
      {"setsize", setsize},
      //{"intersect", intersect},  
      {"fls", fls},
//      {"setclip", setclip},
      { NULL, NULL }
    };
    luaL_setfuncs(L, methods, 0);
    return 0;
  }
  
  template <class T>
  int LuaItemBind<T>::create(lua_State* L) {    
    int n = lua_gettop(L);  // Number of arguments
    T* item = 0;
    canvas::Group* group = 0;
    if (n==1 || (n==2 && lua_isnil(L, 2))) {
      item = new T(L);
    } else if (n==2) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind<>::meta);
      item = new T(L, group);
    }
    LuaHelper::new_userdata(L, meta.c_str(), item);
    if (group) {
      LuaHelper::register_userdata(L, item);
    }
    return 1;
  }

  template <class T>
  int LuaItemBind<T>::fls(lua_State* L) {
    const int n=lua_gettop(L);
    if (n==1) {
      LuaHelper::call(L, meta.c_str(), &T::FLS);
    } else
    if (n==5) {
      T* item = LuaHelper::check<T>(L, 1, meta);
      double x = luaL_checknumber(L, 2);
      double y = luaL_checknumber(L, 3);
      double width = luaL_checknumber(L, 4);
      double height = luaL_checknumber(L, 5);
      mfc::Region rgn(x, y, width, height);
      item->FLS(rgn);
    } else {
      luaL_error(L, "Wrong number of arguments.");
    }
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::draw(lua_State* L) { return 0; }

  template <class T>
  int LuaItemBind<T>::setpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta.c_str(), &T::SetXY);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::pos(lua_State* L) {
    return LuaHelper::get2number2<T,double>(L, meta.c_str(), &T::pos);
  }

  template <class T>
  int LuaItemBind<T>::clientpos(lua_State* L) {
    return LuaHelper::get2number2<T,double>(L, meta.c_str(), &T::clientpos);
  }

  template <class T>
  int LuaItemBind<T>::gc(lua_State* L) {
    return LuaHelper::delete_userdata<T>(L, meta.c_str());
  }

  template <class T>
  int LuaItemBind<T>::setsize(lua_State* L) {
    LuaHelper::callstrict2(L, meta.c_str(), &T::SetSize);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::canvas(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    T* item = LuaHelper::check<T>(L, 1, meta.c_str());
    canvas::Canvas* canvas = item->canvas();
    LuaHelper::find_weakuserdata(L, canvas);
    return 1;
  }

  template <class T>
  int LuaItemBind<T>::show(lua_State* L) {
    LuaHelper::call(L, meta, &T::Show);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::hide(lua_State* L) {
    LuaHelper::call(L, meta, &T::Hide);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::enablepointerevents(lua_State* L) {
    LuaHelper::call(L, meta, &T::EnablePointerEvents);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::disablepointerevents(lua_State* L) {
    LuaHelper::call(L, meta, &T::DisablePointerEvents);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaItemBind<T>::boundrect(lua_State* L) {
    return LuaHelper::get4numbers(L, meta, &T::GetBoundRect);    
  }

  template <class T>
  int LuaItemBind<T>::parent(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    T* item = LuaHelper::check<T>(L, 1, meta);
    canvas::Group* group = item->parent();
    LuaHelper::find_userdata<>(L, group);
    return 1;
  }

  template <class T>
  int LuaItemBind<T>::getfocus(lua_State* L) {
    T* item = LuaHelper::check<T>(L, 1, meta);
    item->GetFocus();
    return LuaHelper::chaining(L);
  } 
   
  template class LuaItemBind<LuaItem>;

  ///////////////////////////////////////////////////////////////////////////////
  // LuaGroupBind
  ///////////////////////////////////////////////////////////////////////////////
    
  template <class T>
  canvas::Item* LuaGroupBind<T>::test(lua_State* L, int index) {
    canvas::Item* item = 0;
    item = LuaHelper::test<LuaRect>(L, index, LuaRectBind<>::meta);
    if (!item) {
      item = LuaHelper::test<LuaGroup>(L, index, meta);
      if (!item) {
        item = LuaHelper::test<LuaText>(L, index, LuaTextBind<>::meta);
        if (!item) {
          item = LuaHelper::test<LuaPic>(L, index, LuaPicBind<>::meta);
          if (!item) {
            item = LuaHelper::test<LuaLine>(L, index, LuaLineBind<>::meta);
            if (!item) {
              item = LuaHelper::test<LuaItem>(L, index, LuaItemBind<LuaItem>::meta);            
              if (!item) {
                item = LuaHelper::test<LuaButton>(L, index, LuaButtonBind<LuaButton>::meta);
                if (!item) {
                  item = LuaHelper::test<LuaEdit>(L, index, LuaButtonBind<LuaEdit>::meta);
                }              
              }
            }
          }
        }
      }
    }
    return item;
  }

  template <class T>
  int LuaGroupBind<T>::removeall(lua_State* L) {
    canvas::Group* group = LuaHelper::call(L, meta, &T::RemoveAll);
    T::iterator it = group->begin();
    for ( ; it != group->end(); ++it) {
      canvas::Item* item = *it;
      LuaHelper::unregister_userdata<>(L, item);
      lua_pop(L, 2);
    }
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaGroupBind<T>::remove(lua_State* L) {
    try {
      T* group = LuaHelper::check<T>(L, 1, meta);
      canvas::Item* item = test(L, 2);
      if (item) {
        group->Remove(item);
        LuaHelper::unregister_userdata(L, item);
      } else {
        luaL_error(L, "Argument is no canvas item.");
      }
    } catch(std::exception &e) {
      luaL_error(L, e.what());
    }
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaGroupBind<T>::add(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, item");
    if (err!=0) return err;
    T* group = LuaHelper::check<T>(L, 1, meta);
    canvas::Item* item = test(L, 2);
    if (item) {
      try {
        group->Add(item);
        LuaHelper::register_userdata(L, item);
       } catch(std::exception &e) {
        luaL_error(L, e.what());
      }
    } else {
      luaL_error(L, "Argument is no canvas item.");
    }
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaGroupBind<T>::setzoom(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Group::setzoom);
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaGroupBind<T>::intersect(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 3, "self, self, x1, y1 [,x2, y2]");
    if (err!=0) return err;
    T* item = LuaHelper::check<T>(L, 1, meta);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    /*canvas::Item* res = item->intersect(x, y);
    if (res) {
      LuaHelper::find_userdata(L, res);
    } else {
      lua_pushnil(L);
    }*/
    return 1;
  }

  template <class T>
  int LuaGroupBind<T>::getitems(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    T* group = LuaHelper::check<T>(L, 1, meta);
    lua_newtable(L); // create item table
    T::iterator it = group->begin();
    for (; it!=group->end(); ++it) {
      LuaHelper::find_userdata(L, *it);
      lua_rawseti(L, 2, lua_rawlen(L, 2)+1); // items[#items+1] = newitem
    }
    lua_pushvalue(L, 2);
    return 1;
  }

  template <class T>
  int LuaGroupBind<T>::gc(lua_State* L) {
    T** group = (T**) luaL_checkudata(L, -1, meta.c_str());
    LuaGroup::iterator it = (*group)->begin();
    for ( ; it != (*group)->end(); ++it) {
      LuaHelper::unregister_userdata(L, *it);
    }
    return LuaHelper::delete_userdata<T>(L, meta);
  }

  template <class T>
  int LuaGroupBind<T>::setzorder(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 3, "self, item, z");
    if (err!=0) return err;
    T* group = LuaHelper::check<T>(L, 1, meta);
    canvas::Item* item = test(L, 2);
    if (item) {    
      int z  = luaL_checknumber(L, 3);
      group->set_zorder(item, z);
    }    
    return LuaHelper::chaining(L);
  }

  template <class T>
  int LuaGroupBind<T>::zorder(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 2, "self, item");
    if (err!=0) return err;
    T* group = LuaHelper::check<T>(L, 1, meta);
    canvas::Item* item = test(L, 2);
    if (item) {    
      int z  = group->zorder(item);
      lua_pushnumber(L, z);
    } else {
      return luaL_error(L, "Item not in group");
    }
    return 1;
  }

  template <class T>
  int LuaGroupBind<T>::setclip(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 5, "Wrong number of arguments.");
    if (err!=0) return err;
    T* item = LuaHelper::check<T>(L, 1, meta);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    double width = luaL_checknumber(L, 4);
    double height = luaL_checknumber(L, 5);
    item->SetClip(x, y, width, height);
    return LuaHelper::chaining(L);
  }

  template class LuaGroupBind<LuaGroup>;
             
  ///////////////////////////////////////////////////////////////////////////////
  // LuaGraphicsBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaGraphicsBind::meta = "psygraphicsmeta";

  int LuaGraphicsBind::open(lua_State *L) {
    static const luaL_Reg funcs[] = {
      { NULL, NULL }
    };
    static const luaL_Reg methods[] = {
      {"translate", translate},
      {"setcolor", setcolor},
      {"color", color},
      {"drawline", drawline},
      {"drawrect", drawrect},
      {"drawoval", drawoval},
      {"drawroundrect", drawroundrect},
      {"fillrect", fillrect},
      {"fillroundrect", fillroundrect},
      {"filloval", filloval},
      {"copyarea", copyarea},
      {"drawstring", drawstring},
      {"setfont", setfont},
      {"font", font},
      {"drawpolygon", drawpolygon},
      {"fillpolygon", fillpolygon},
      {"drawpolyline", drawpolyline},
      {"drawimage", drawimage},
      { "__gc", gc },
      { NULL, NULL }
    };
    luaL_newmetatable(L, meta);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, methods, 0);
    luaL_newlib(L, funcs);
    return 1;
  }

  int LuaGraphicsBind::translate(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 3, "self, x, y");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    g->Translate(x, y);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::setcolor(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, color");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    ARGB color = static_cast<ARGB>(luaL_checknumber(L, 2));
    g->SetColor(color);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::color(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    lua_pushnumber(L, g->color());
    return 1;
  }

  int LuaGraphicsBind::drawline(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, x2, y2");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    g->DrawLine(x1, y1, x2, y2);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawstring(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 4, "self, str, x, y");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    const char* str = luaL_checkstring(L, 2);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);
    g->DrawString(str, x, y);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawrect(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, x2, y2");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    g->DrawRect(x1, y1, x2, y2);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawoval(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, width, height");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    g->DrawOval(x1, y1, x2, y2);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawroundrect(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 7, "self, x1, y1, x2, y2, arcwidth, archeight");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double x3 = luaL_checknumber(L, 6);
    double y3 = luaL_checknumber(L, 7);
    g->DrawRoundRect(x1, y1, x2, y2, x3, y3);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::fillrect(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, x2, y2");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    g->FillRect(x1, y1, x2, y2);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::fillroundrect(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 7, "self, x1, y1, x2, y2, arcwidth, archeight");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double x3 = luaL_checknumber(L, 6);
    double y3 = luaL_checknumber(L, 7);
    g->FillRoundRect(x1, y1, x2, y2, x3, y3);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::copyarea(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 7, "self, x1, y1, width, height, dx, dy");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double x3 = luaL_checknumber(L, 6);
    double y3 = luaL_checknumber(L, 7);
    g->CopyArea(x1, y1, x2, y2, x3, y3);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::filloval(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, width, height");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    g->FillOval(x1, y1, x2, y2);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::setfont(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, font");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "name");
    lua_getfield(L, 2, "height");
    const char *name = luaL_checkstring(L, -2);
    int height = luaL_checknumber(L, -1);
    Font font;
    font.name = name;
    font.height = height;
    g->SetFont(font);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::font(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Graphics* gr = *(Graphics **)luaL_checkudata(L, 1, meta);
    Font fnt = gr->font();
    lua_newtable(L);
    lua_pushstring(L, fnt.name.c_str());
    lua_setfield(L, 2, "name");
    lua_pushnumber(L, fnt.height);
    lua_setfield(L, 2, "height");
    return 1;
  }

  int LuaGraphicsBind::gc(lua_State* L) {
    // Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    // delete g;
    // create and delete done by canvas
    return 0;
  }

  int LuaGraphicsBind::drawpolygon(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    luaL_checktype(L, 2, LUA_TTABLE);
    size_t n1 = lua_rawlen(L, 2);
    size_t n2 = lua_rawlen(L, 2);
    if (n1!=n2) return luaL_error(L, "Length of x and y points have to be the same size.");
    Points points;
    for (int i=1; i <=n1; ++i) {
      lua_rawgeti(L, 2, i);
      lua_rawgeti(L, 3, i);
      double x = luaL_checknumber(L, -2);
      double y = luaL_checknumber(L, -1);
      points.push_back(std::pair<int, int>(x, y));
      lua_pop(L, 2);
    }
    g->DrawPolygon(points);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawpolyline(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    luaL_checktype(L, 2, LUA_TTABLE);
    size_t n1 = lua_rawlen(L, 2);
    size_t n2 = lua_rawlen(L, 2);
    if (n1!=n2) return luaL_error(L, "Length of x and y points have to be the same size.");
    Points points;
    for (int i=1; i <=n1; ++i) {
      lua_rawgeti(L, 2, i);
      lua_rawgeti(L, 3, i);
      double x = luaL_checknumber(L, -2);
      double y = luaL_checknumber(L, -1);
      points.push_back(std::pair<int, int>(x, y));
      lua_pop(L, 2);
    }
    g->DrawPolyline(points);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::fillpolygon(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
    if (err!=0) return err;
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    luaL_checktype(L, 2, LUA_TTABLE);
    size_t n1 = lua_rawlen(L, 2);
    size_t n2 = lua_rawlen(L, 2);
    if (n1!=n2) return luaL_error(L, "Length of x and y points have to be the same size.");
    Points points;
    for (int i=1; i <=n1; ++i) {
      lua_rawgeti(L, 2, i);
      lua_rawgeti(L, 3, i);
      double x = luaL_checknumber(L, -2);
      double y = luaL_checknumber(L, -1);
      points.push_back(std::pair<int, int>(x, y));
      lua_pop(L, 2);
    }
    g->FillPolygon(points);
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawimage(lua_State* L) {
    int n = lua_gettop(L);
    if (n!=4 and n!=6 and n!=8) return luaL_error(L, "Wrong number of arguments.");
    Graphics* g = *(Graphics **)luaL_checkudata(L, 1, meta);
    Image* img = LuaHelper::check<Image>(L, 2, LuaImageBind::meta);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);
    if (n==4) {
      g->DrawImage(img, x, y);
    } else
    if (n==6) {
      double width = luaL_checknumber(L, 5);
      double height = luaL_checknumber(L, 6);
      g->DrawImage(img, x, y, width, height);
    } else
    if (n==8) {
      double width = luaL_checknumber(L, 5);
      double height = luaL_checknumber(L, 6);
      double xsrc = luaL_checknumber(L, 7);
      double ysrc = luaL_checknumber(L, 8);
      g->DrawImage(img, x, y, width, height, xsrc, ysrc);
    }
    return LuaHelper::chaining(L);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaItem+Bind
  ///////////////////////////////////////////////////////////////////////////////

  bool LuaItem::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, this); }

  void LuaItem::Draw(Graphics* g, Region& repaint_rgn, canvas::Canvas* widget) {
      LuaHelper::find_userdata<>(L, this);
      lua_getfield(L, -1, "draw");
      lua_pushvalue(L, -2);
      lua_remove(L, -3);
      Graphics** gr = (Graphics **)lua_newuserdata(L, sizeof(Graphics *));
      *gr = g;
      luaL_setmetatable(L, LuaGraphicsBind::meta);
      Region ** ud = (Region **)lua_newuserdata(L, sizeof(Region *));
      *ud = &repaint_rgn;
      luaL_setmetatable(L, LuaRegionBind::meta);
      int status = lua_pcall(L, 3, 0, 0);
      if (status) {
        const char* msg = lua_tostring(L, -1);
        throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));
      }
    }

  const Region& LuaItem::region() const {
    if (update_) {
      rgn_.SetRect(0, 0, w_+1, h_ +1);
      update_ = false;
    }
    return rgn_;
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // LuaImageBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaImageBind::meta = "psyimagebind";

  int LuaImageBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"load", load},
      {"settransparent", settransparent},
      {"size", size},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaImageBind::create(lua_State* L) {
    Image* img = new mfc::Image(&PsycleGlobal::conf().macParam().dial);
    LuaHelper::new_userdata<>(L, meta, img);
    return 1;
  }

  int LuaImageBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<Image>(L, meta);
  }

  int LuaImageBind::load(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, filename");
    if (err!=0) return err;
    Image* img = LuaHelper::check<Image>(L, 1, meta);
    const char* str = luaL_checkstring(L, 2);
    try {
      img->Load(str);
    } catch(std::exception &e) {
      return luaL_error(L, e.what());
    }
    return LuaHelper::chaining(L);
  }

  int LuaImageBind::size(lua_State* L) {
    return LuaHelper::get2numbers(L, LuaImageBind::meta, &Image::width, &Image::height);
  }

  int LuaImageBind::settransparent(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, color");
    if (err!=0) return err;
    Image* img = LuaHelper::check<Image>(L, 1, meta);
    int argb = luaL_checknumber(L, 2);
    img->SetTransparent(true, argb);
    return LuaHelper::chaining(L);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaRegion+Bind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaRegionBind::meta = "psyregionbind";

  int LuaRegionBind::open(lua_State *L) {
    static const luaL_Reg funcs[] = {
      {"new", create},
      { "__gc", gc },
      { NULL, NULL }
    };
    static const luaL_Reg methods[] = {
      {"boundrect", boundrect},
      { NULL, NULL }
    };
    luaL_newmetatable(L, meta);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, methods, 0);
    luaL_newlib(L, funcs);
    return 1;
  }

  int LuaRegionBind::create(lua_State* L) {
    int n = lua_gettop(L);
    Region ** ud = (Region **)lua_newuserdata(L, sizeof(Region *));
    luaL_setmetatable(L, meta);
    *ud = new mfc::Region();
    return 1;
  }

  int LuaRegionBind::gc(lua_State* L) {
    Region* rgn= *(Region **)luaL_checkudata(L, 1, meta);
    delete rgn;
    return 0;
  }

  int LuaRegionBind::boundrect(lua_State* L) {
    Region* rgn= *(Region **)luaL_checkudata(L, 1, meta);
    double x, y, width, height;
    rgn->GetBoundRect(x, y, width, height);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, width);
    lua_pushnumber(L, height);
    return 4;
  }
 
      
} // namespace host
} // namespace psycle