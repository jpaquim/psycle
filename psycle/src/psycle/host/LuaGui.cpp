// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "LuaGui.hpp"

#include "PsycleConfig.hpp"
#include "LuaPlugin.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

namespace psycle {
namespace host {

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
  LuaHelper::new_shared_userdata<>(L, meta, menubar);
  return 1;
}

int LuaMenuBarBind::add(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, menu");
  if (err!=0) return err;
  boost::shared_ptr<MenuBar> menubar = LuaHelper::check_sptr<MenuBar>(L, 1, meta);
  boost::shared_ptr<Menu> menu = LuaHelper::check_sptr<Menu>(L, 2, LuaMenuBind::meta);
  menubar->add(menu.get());
  LuaHelper::register_userdata<>(L, menu.get());
  return LuaHelper::chaining(L);
}

int LuaMenuBarBind::gc(lua_State* L) {
//    return LuaHelper::delete_userdata<MenuBar>(L, meta);
  return 0;
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
  LuaHelper::new_shared_userdata<>(L, meta, menu);
  return 1;
}

int LuaMenuBind::remove(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, menu or menuitem");
  if (err!=0) return err;
  boost::shared_ptr<Menu> menu = LuaHelper::check_sptr<Menu>(L, 1, meta);
  boost::shared_ptr<MenuItem> item;
  item = LuaHelper::test_sptr<MenuItem>(L, 2, LuaMenuItemBind::meta);
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
    menu->remove(item.get());
    LuaHelper::unregister_userdata(L, item.get());
  }
  return LuaHelper::chaining(L);
}

int LuaMenuBind::add(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, menu or menuitem");
  if (err!=0) return err;
  boost::shared_ptr<Menu> menu = LuaHelper::check_sptr<Menu>(L, 1, meta);
  boost::shared_ptr<MenuItem> item;
  item = LuaHelper::test_sptr<MenuItem>(L, 2, LuaMenuItemBind::meta);
  if (!item) {
    boost::shared_ptr<Menu> new_menu= LuaHelper::test_sptr<Menu>(L, 2, LuaMenuBind::meta);
    if (new_menu) {
      menu->add(new_menu.get());
      LuaHelper::register_userdata(L, new_menu.get());
    } else {
      return luaL_error(L, "Argument is no menu or menuitem.");
    }
  } else {
    menu->add(item.get());
    LuaHelper::register_userdata(L, item.get());
  }
  return LuaHelper::chaining(L);
}

int LuaMenuBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<Menu>(L, meta);
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
  LuaHelper::new_shared_userdata(L, meta, item);
  lua_newtable(L);
  lua_setfield(L, -2, "listener_");
  return 1;
}

int LuaMenuItemBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<MenuItem>(L, meta);
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
// PsycleCmdDefBind
/////////////////////////////////////////////////////////////////////////////

int LuaCmdDefBind::open(lua_State* L) {   
  static const luaL_Reg funcs[] = {
    {"keytocmd", keytocmd},           
    {NULL, NULL}
  };
  luaL_newlib(L, funcs);
  static const char* const e[] = {
		"TRANSPOSECHANNELINC", "TRANSPOSECHANNELDEC", "TRANSPOSECHANNELINC12", 
    "TRANSPOSECHANNELDEC12", "TRANSPOSEBLOCKINC", "TRANSPOSEBLOCKDEC",
    "TRANSPOSEBLOCKINC12", "TRANSPOSEBLOCKDEC12", "PATTERNCUT",
    "PATTERNCOPY", "PATTERNPASTE", "ROWINSERT", "ROWDELETE", "ROWCLEAR",
    "BLOCKSTART", "BLOCKEND", "BLOCKUNMARK", "BLOCKDOUBLE", "BLOCKHALVE",
    "BLOCKCUT", "BLOCKCOPY", "BLOCKPASTE", "BLOCKMIX", "BLOCKINTERPOLATE",
    "BLOCKSETMACHINE", "BLOCKSETINSTR", "SELECTALL", "SELECTCOL",
    "EDITQUANTIZEDEC", "EDITQUANTIZEINC", "PATTERNMIXPASTE",
    "PATTERNTRACKMUTE", "KEYSTOPANY", "PATTERNDELETE", "BLOCKDELETE",
    "PATTERNTRACKSOLO", "PATTERNTRACKRECORD", "SELECTBAR"
	};
  {
    size_t size = sizeof(e)/sizeof(e[0]);    
    LuaHelper::buildenum(L, e, size, CS_EDT_START);      
  }
  {
    static const char* const e[] = {"NULL", "NOTE", "EDITOR", "IMMEDIATE"};
    size_t size = sizeof(e)/sizeof(e[0]);
    LuaHelper::buildenum(L, e, size, 0);
  }
  return 1;
};

int LuaCmdDefBind::keytocmd(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "keycode");
  char c = luaL_checknumber(L, -1);
  lua_pop(L, 1);
  UINT nFlags = 0;
  CmdDef cmd = PsycleGlobal::inputHandler().KeyToCmd(c, nFlags);
  if (cmd.GetID() == -1) {
    // try again with extended key (bit 8)
    cmd = PsycleGlobal::inputHandler().KeyToCmd(c, nFlags | 256);
  }
  lua_createtable(L, 0, 3);
  lua_pushnumber(L, cmd.GetNote());
  lua_setfield(L, 2, "note");
  lua_pushnumber(L, cmd.GetID());
  lua_setfield(L, 2, "id");
  lua_pushnumber(L, cmd.GetType());
  lua_setfield(L, 2, "type");
  return 1;
}
  
/////////////////////////////////////////////////////////////////////////////
// PsycleActions + Lua Bind
/////////////////////////////////////////////////////////////////////////////

void LuaActionListener::OnNotify(ActionType action) {
  LuaGlobal::proxy(L)->lock();
  LuaHelper::find_userdata<>(L, this);
  lua_getfield(L, -1, "onnotify");
  lua_pushvalue(L, -2);
  lua_remove(L, -3);
  lua_pushnumber(L, action);
  int status = lua_pcall(L, 2, 0, 0);
  if (status) {
    LuaGlobal::proxy(L)->unlock();
    const char* msg = lua_tostring(L, -1);
    throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));
  }
  LuaGlobal::proxy(L)->unlock();
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
  LuaHelper::new_shared_userdata<>(L, meta, listener);
  LuaHelper::register_userdata<>(L, listener);
  return 1;
}

int LuaActionListenerBind::gc(lua_State* L) {
  typedef boost::shared_ptr<LuaActionListener> T;
  T listener = *(T *)luaL_checkudata(L, 1, meta);
  PsycleGlobal::actionHandler().RemoveListener(listener.get());
  return LuaHelper::delete_shared_userdata<LuaActionListener>(L, meta);
}

/////////////////////////////////////////////////////////////////////////////
// PsycleFileOpenBind
/////////////////////////////////////////////////////////////////////////////

struct LuaFileDialog { 
  LuaFileDialog(bool isopen) : isopen(isopen) { }
  std::string fname; bool isopen;
};

const char* LuaFileOpenBind::meta = "psyfileopenmeta";

int LuaFileOpenBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"show", show},
    {"filename", filename},
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods,  gc);
  return 1;
}

int LuaFileOpenBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;       
  LuaHelper::new_shared_userdata<>(L, meta, new LuaFileDialog(true));
  return 1;
}

int LuaFileOpenBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaFileDialog>(L, meta);
}

int LuaFileOpenBind::show(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  char szFilters[]= "Text Files (*.NC)|*.NC|Text Files (*.lua)|*.lua|All Files (*.*)|*.*||";
  CFileDialog dlg(luadlg->isopen, "lua", "*.lua",
              OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());
  INT_PTR result = dlg.DoModal();
  std::string pathname(dlg.GetPathName().GetString());
  luadlg->fname = pathname;
  try {
    LuaImport import(L, luadlg.get(), 0);  
    if (import.open(result == IDOK ? "onok" : "oncancel")) {
      import << pathname;
      import.pcall(0);
    }
  } catch (std::exception& e) {
    return luaL_error(L, e.what());
  }
  lua_pushboolean(L, result == IDOK);
  return 1;
}

int LuaFileOpenBind::filename(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  lua_pushstring(L, luadlg->fname.c_str());
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
// PsycleFileSaveBind
/////////////////////////////////////////////////////////////////////////////

const char* LuaFileSaveBind::meta = "psyfilesavemeta";

int LuaFileSaveBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"show", show},
    {"filename", filename},
    { NULL, NULL }
  };
  LuaHelper::open(L, meta, methods, gc);
  return 1;
}

int LuaFileSaveBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;     
  LuaHelper::new_shared_userdata<>(L, meta, new LuaFileDialog(false));  
  return 1;
}

int LuaFileSaveBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaFileDialog>(L, meta);
}

int LuaFileSaveBind::show(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  char szFilters[]= "Text Files (*.NC)|*.NC|Lua Files (*.lua)|*.lua|All Files (*.*)|*.*||";
  CFileDialog dlg(luadlg->isopen, "lua", "*.lua",
              OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());
  INT_PTR result = dlg.DoModal();
  std::string pathname(dlg.GetPathName().GetString());
  luadlg->fname = pathname;
  try {
    LuaImport import(L, luadlg.get(), 0);  
    if (import.open(result == IDOK ? "onok" : "oncancel")) {
      import << pathname;
      import.pcall(0);
    }
  } catch (std::exception& e) {
    return luaL_error(L, e.what());
  }
  lua_pushboolean(L, result == IDOK);
  return 1;
}

int LuaFileSaveBind::filename(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  lua_pushstring(L, luadlg->fname.c_str());
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
// LuaFrameWnd+Bind
/////////////////////////////////////////////////////////////////////////////

int LuaFrameWnd::OnFrameClose() {    
  LuaHelper::find_weakuserdata<>(L, this);
  bool has_event_method = false;
  if (!lua_isnil(L, -1)) {
    lua_getfield(L, -1, "onclose");
  }
  has_event_method = !lua_isnil(L, -1);
  if (has_event_method) {
    lua_pushvalue(L, -2);
    lua_remove(L, -3);                  
    int status = lua_pcall(L, 1, 0, 0);
    if (status) {
      const char* msg = lua_tostring(L, -1);
      throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));        
    }      
  } else {
    lua_pop(L, 2);
  }    
  lua_gc(L, LUA_GCCOLLECT, 0);    
  return 0;
}

std::string LuaFrameWndBind::meta = LuaFrameWnd::type();

bool LuaFrameWndBind::mfcclosing = false;

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
  //boost::shared_ptr<LuaFrameWnd> frame(new LuaFrameWnd(L));
  LuaFrameWnd* frame = new LuaFrameWnd(L);
  frame->Create(NULL,"PsycleLuaPlugin", WS_OVERLAPPEDWINDOW,
    CRect(120, 100, 700, 480), ::AfxGetMainWnd());
  LuaHelper::new_shared_userdata(L, meta, frame);
  //LuaHelper::new_shared_userdata<boost::shared_ptr<LuaFrameWnd>, LuaFrameWnd>(L, meta, frame);
  LuaHelper::register_weakuserdata(L, frame);
  return 1;
}

int LuaFrameWndBind::gc(lua_State* L) {
  if (!LuaFrameWndBind::mfcclosing) {    
    LuaFrameWnd::Ptr wnd = *(LuaFrameWnd::Ptr*) luaL_checkudata(L, 1, meta.c_str());    
    canvas::Canvas* c = wnd->canvas().lock().get();
    if (c) {
      LuaHelper::unregister_userdata(L, c);
    }
    wnd->DestroyWindow();    
  }
  return 0;
}

int LuaFrameWndBind::show(lua_State* L) {
  // boost::shared_ptr<LuaFrameWnd> wnd;
  // wnd = LuaHelper::check_sptr<boost::shared_ptr<LuaFrameWnd> >(L, 1, meta);
  LuaFrameWnd::Ptr wnd = LuaHelper::check_sptr<LuaFrameWnd>(L, 1, meta);
  wnd->SetWindowPos(NULL, 200, 200, 600, 480,  SWP_NOZORDER | SWP_SHOWWINDOW);
  return LuaHelper::chaining(L);
}

int LuaFrameWndBind::hide(lua_State* L) {
  LuaFrameWnd::Ptr wnd = LuaHelper::check_sptr<LuaFrameWnd>(L, 1, meta);
  wnd->ShowWindow(SW_HIDE);
  return LuaHelper::chaining(L);
}

int LuaFrameWndBind::setcanvas(lua_State* L) {
  LuaFrameWnd::Ptr wnd = LuaHelper::check_sptr<LuaFrameWnd>(L, 1, meta);
  canvas::Canvas::WeakPtr old_canvas = wnd->canvas();
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

///////////////////////////////////////////////////////////////////////////////
// LuaCanvasBind
///////////////////////////////////////////////////////////////////////////////
template<class T>
canvas::Item::Ptr CallEvents(lua_State* L, canvas::Event* ev, T that, bool is_canvas = false) {  
  int n1 = lua_gettop(L);
  if (is_canvas) {
    LuaHelper::find_weakuserdata<>(L, that.get());
  } else {
    LuaHelper::find_userdata<>(L, that.get());
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
      default: return that;
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
  if (ev->type() != canvas::Event::ONTIMER) {
    lua_gc(L, LUA_GCCOLLECT, 0);
  }
  return has_event_method ? that : 0;
}

canvas::Item::WeakPtr LuaItem::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, shared_from_this()); }
canvas::Item::WeakPtr LuaGroup::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, shared_from_this()); }
canvas::Item::WeakPtr LuaRect::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, shared_from_this()); }
canvas::Item::WeakPtr LuaLine::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, shared_from_this()); }
canvas::Item::WeakPtr LuaText::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, shared_from_this()); }
canvas::Item::WeakPtr LuaPic::OnEvent(canvas::Event* ev) { return CallEvents(L, ev, shared_from_this()); }  

/////////////////////////////////////////////////////////////////////////////
// LuaCanvas+Bind
/////////////////////////////////////////////////////////////////////////////

LuaCanvas::LuaCanvas(lua_State* L) 
    : ui::canvas::Canvas(),
      lua_in_(L, this, LuaGlobal::proxy(L)),
      L(L) {     
}

canvas::Item::WeakPtr LuaCanvas::OnEvent(canvas::Event* ev) {
  LuaGlobal::proxy(L)->lock();
  canvas::Item::WeakPtr item = canvas::Canvas::OnEvent(ev);  
  CallEvents(L, ev, shared_from_this(), true);
  LuaGlobal::proxy(L)->unlock();
  return item;
}


template <class T>
int LuaCanvasBind<T>::open(lua_State *L) {  
   LuaHelper::openex<B>(L, meta, setmethods, gc);
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

template <class T>
int LuaCanvasBind<T>::setcursor(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "Wrong number of arguments.");
  if (err!=0) return err;
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
  CursorStyle style = (CursorStyle) (int) luaL_checknumber(L, 2);
  canvas->SetCursor(style);
  return LuaHelper::chaining(L);
}

template <class T>
int LuaCanvasBind<T>::showscrollbar(lua_State* L) {
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas->show_scrollbar = true;
  return LuaHelper::chaining(L);
}

template <class T>
int LuaCanvasBind<T>::setscrollinfo(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, nposh, nposv");
  if (err!=0) return err;
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
  luaL_checktype(L, 2, LUA_TTABLE);
  lua_getfield(L, 2, "nposh");
  lua_getfield(L, 2, "nposv");
  canvas->nposh = luaL_checknumber(L, -2);
  canvas->nposv = luaL_checknumber(L, -1);
  canvas->show_scrollbar = true;
  return LuaHelper::chaining(L);
}
  
template class LuaCanvasBind<LuaCanvas>;


///////////////////////////////////////////////////////////////////////////////
// LuaItemBind
///////////////////////////////////////////////////////////////////////////////
  

template <class T>
int LuaItemBind<T>::create(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  return 1;
}


template <class T>
int LuaItemBind<T>::gc(lua_State* L) {
  typedef boost::shared_ptr<T> SPtr;
  SPtr item = *(SPtr*) luaL_checkudata(L, 1, meta.c_str());
  std::vector<canvas::Item::Ptr> subitems = item->SubItems();
  T::iterator it = subitems.begin();
  for ( ; it != subitems.end(); ++it) {
    canvas::Item::Ptr subitem = *it;
    LuaHelper::unregister_userdata<>(L, subitem.get());
  }    
  return LuaHelper::delete_shared_userdata<T>(L, meta.c_str());
}

template <class T>
int LuaItemBind<T>::fls(lua_State* L) {
  const int n=lua_gettop(L);
  if (n==1) {
    LuaHelper::bind(L, meta, &T::FLS);
  } else
  if (n==2) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    LuaRegion* rgn = *(LuaRegion **)luaL_checkudata(L, 2, LuaRegionBind::meta);
    item->FLS(*rgn->get());            
  } else
  if (n==5) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
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
int LuaItemBind<T>::canvas(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas::Canvas* canvas = item->root();
  LuaHelper::find_weakuserdata(L, canvas);
  return 1;
}

template <class T>
int LuaItemBind<T>::parent(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas::Item::WeakPtr group = item->parent();
  LuaHelper::find_userdata<>(L, group.lock().get());
  return 1;
}


template class LuaItemBind<LuaItem>;

///////////////////////////////////////////////////////////////////////////////
// LuaGroupBind
///////////////////////////////////////////////////////////////////////////////

template <class T>
canvas::Item::Ptr LuaGroupBind<T>::test(lua_State* L, int index) {
  canvas::Item::Ptr item;
  item = LuaHelper::test_sptr<LuaRect>(L, index, LuaRectBind<>::meta);
  if (!item) {
    item = LuaHelper::test_sptr<LuaCanvas>(L, index, LuaCanvasBind<>::meta);         
    if (!item) {
      item = LuaHelper::test_sptr<LuaGroup>(L, index, LuaGroupBind<>::meta);
      if (!item) {
        item = LuaHelper::test_sptr<LuaText>(L, index, LuaTextBind<>::meta);
        if (!item) {
          item = LuaHelper::test_sptr<LuaPic>(L, index, LuaPicBind<>::meta);
          if (!item) {
            item = LuaHelper::test_sptr<LuaLine>(L, index, LuaLineBind<>::meta);
            if (!item) {
              item = LuaHelper::test_sptr<LuaItem>(L, index, LuaItemBind<LuaItem>::meta);
              if (!item) {
                item = LuaHelper::test_sptr<LuaButton>(L, index, LuaButtonBind<LuaButton>::meta);
                if (!item) {
                  item = LuaHelper::test_sptr<LuaEdit>(L, index, LuaButtonBind<LuaEdit>::meta);                    
                  if (!item) {
                    item = LuaHelper::test_sptr<LuaScrollBar>(L, index, LuaScrollBarBind<LuaScrollBar>::meta);                
                    if (!item) {
                        item = LuaHelper::test_sptr<LuaScintilla>(L, index, LuaScintillaBind<LuaScintilla>::meta);                        
                    }
                  }
                }
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
  canvas::Item::Ptr group = LuaHelper::check_sptr<T>(L, 1, meta);
  std::vector<canvas::Item::Ptr> subitems = group->SubItems();
  T::iterator it = subitems.begin();
  for ( ; it != subitems.end(); ++it) {
    canvas::Item::Ptr subitem = *it;
    LuaHelper::unregister_userdata<>(L, subitem.get());
  }  
  group->RemoveAll();  
  return LuaHelper::chaining(L);
}

template <class T>
int LuaGroupBind<T>::remove(lua_State* L) {  
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);    
  canvas::Item::Ptr item = test(L, 2);
  if (item) {
    std::vector<canvas::Item::Ptr> subitems = item->SubItems();
    T::iterator it = subitems.begin();
    for ( ; it != subitems.end(); ++it) {
      canvas::Item::Ptr subitem = *it;
      LuaHelper::unregister_userdata<>(L, subitem.get());    
    }
    LuaHelper::unregister_userdata<>(L, item.get());
    group->Remove(item);      
  } else {
    luaL_error(L, "Argument is no canvas item.");
  }  
  return LuaHelper::chaining(L);
}

template <class T>
int LuaGroupBind<T>::add(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, item");
  if (err!=0) return err;
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas::Item::Ptr item = test(L, 2);
  if (item) {
    try {
      group->Add(item);
      LuaHelper::register_userdata(L, item.get());
    } catch(std::exception &e) {
      luaL_error(L, e.what());
    }
  } else {
    luaL_error(L, "Argument is no canvas item.");
  }
  return LuaHelper::chaining(L);
}

template <class T>
int LuaGroupBind<T>::intersect(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 3, "self, self, x1, y1 [,x2, y2]");
  if (err!=0) return err;
  // T* item = LuaHelper::check<T>(L, 1, meta);
  // double x = luaL_checknumber(L, 2);
  // double y = luaL_checknumber(L, 3);
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
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);
  lua_newtable(L); // create item table
  T::iterator it = group->begin();
  for (; it!=group->end(); ++it) {
    LuaHelper::find_userdata(L, (*it).get());
    lua_rawseti(L, 2, lua_rawlen(L, 2)+1); // items[#items+1] = newitem
  }
  lua_pushvalue(L, 2);
  return 1;
}

template <class T>
int LuaGroupBind<T>::setzorder(lua_State* L) {
/*  int err = LuaHelper::check_argnum(L, 3, "self, item, z");
  if (err!=0) return err;
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas::Item* item = test(L, 2);
  if (item) {
    int z  = luaL_checknumber(L, 3);
    group->set_zorder(item, z - 1); // transform from lua onebased to zerobased
  }*/
  return LuaHelper::chaining(L);
}

template <class T>
int LuaGroupBind<T>::zorder(lua_State* L) {  
  int err = LuaHelper::check_argnum(L, 2, "self, item");
  if (err!=0) return err;
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas::Item::Ptr item = test(L, 2);
  if (item) {
    int z  = group->zorder(item);
    lua_pushnumber(L, z + 1); // transform from zerobased to lua onebased
  } else {
    return luaL_error(L, "Item not in group");
  }  
  return 1;
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
  boost::shared_ptr<Image> img = LuaHelper::check_sptr<Image>(L, 2, LuaImageBind::meta);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  if (n==4) {
    g->DrawImage(img.get(), x, y);
  } else
  if (n==6) {
    double width = luaL_checknumber(L, 5);
    double height = luaL_checknumber(L, 6);
    g->DrawImage(img.get(), x, y, width, height);
  } else
  if (n==8) {
    double width = luaL_checknumber(L, 5);
    double height = luaL_checknumber(L, 6);
    double xsrc = luaL_checknumber(L, 7);
    double ysrc = luaL_checknumber(L, 8);
    g->DrawImage(img.get(), x, y, width, height, xsrc, ysrc);
  }
  return LuaHelper::chaining(L);
}

///////////////////////////////////////////////////////////////////////////////
// LuaItem+Bind
///////////////////////////////////////////////////////////////////////////////

LuaItem::LuaItem(lua_State* L) 
  : ui::canvas::Item(),
    lua_in_(L, this, LuaGlobal::proxy(L)),       
    L(L) {
}

void LuaItem::Draw(Graphics* g, Region& draw_rgn) {         
  if (lua_in_.open("draw")) {
    Graphics** gr = (Graphics **)lua_newuserdata(L, sizeof(Graphics *));
    *gr = g;
    luaL_setmetatable(L, LuaGraphicsBind::meta);
    lua_in_.pcall(0);
    lua_in_.close();
  }      
}

void LuaItem::onupdateregion() {
  if (lua_in_.open("onupdateregion")) {
    LuaRegion** rgn = (LuaRegion **)lua_newuserdata(L, sizeof(LuaRegion *));
    *rgn = new LuaRegion(rgn_.get(), true);
    luaL_setmetatable(L, LuaRegionBind::meta);
    lua_in_.pcall(0);
    lua_in_.close();
  }
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
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaImageBind::create(lua_State* L) {
  Image* img = new mfc::Image(&PsycleGlobal::conf().macParam().dial);
  LuaHelper::new_shared_userdata<>(L, meta, img);
  return 1;
}

int LuaImageBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<Image>(L, meta);
}

int LuaImageBind::load(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, filename");
  if (err!=0) return err;
  Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
  const char* str = luaL_checkstring(L, 2);
  try {
    img->Load(str);
  } catch(std::exception &e) {
    return luaL_error(L, e.what());
  }
  return LuaHelper::chaining(L);
}

int LuaImageBind::size(lua_State* L) {
  LuaHelper::bind(L, meta, &Image::Size);
  return 2;
}

int LuaImageBind::settransparent(lua_State* L) {  
  int err = LuaHelper::check_argnum(L, 2, "self, color");
  if (err!=0) return err;
  boost::shared_ptr<Image> img = LuaHelper::check_sptr<Image>(L, 1, meta);
  int argb = luaL_checknumber(L, 2);
  img->SetTransparent(true, argb);
  return LuaHelper::chaining(L);
}


void LuaScintilla::OnFirstModified() {   
  LuaImport in(L, this, LuaGlobal::proxy(L));
  if (in.open("onfirstmodified")) {
    in.pcall(0);            
  }
}

void LuaButton::OnClick() {
  try {
    LuaImport in(L, this, LuaGlobal::proxy(L));
    if (in.open("onclick")) {
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      AfxMessageBox(e.what());    
  }
}

void LuaScrollBar::OnScroll(int pos) {  
  LuaImport in(L, this, LuaGlobal::proxy(L));
  if (in.open("onscroll")) {
    in << pos << pcall(0);
  }  
}

///////////////////////////////////////////////////////////////////////////////
// LuaRegion+Bind
///////////////////////////////////////////////////////////////////////////////

const char* LuaRegionBind::meta = "psyregionbind";

int LuaRegionBind::open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"new", create},      
    { NULL, NULL }
  };
  static const luaL_Reg methods[] = {
    {"boundrect", boundrect},
    {"setrect", setrect},
    {"combine", combine},
    {"offset", offset},
    { "__gc", gc },
    { NULL, NULL }
  };
  luaL_newmetatable(L, meta);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, methods, 0);    
  luaL_newlib(L, funcs);
  lua_pushnumber(L, RGN_OR);
  lua_setfield(L, -2, "OR");
  lua_pushnumber(L, RGN_AND);
  lua_setfield(L, -2, "AND");
  lua_pushnumber(L, RGN_XOR);
  lua_setfield(L, -2, "XOR");
  lua_pushnumber(L, RGN_DIFF);
  lua_setfield(L, -2, "DIFF");
  lua_pushnumber(L, RGN_COPY);
  lua_setfield(L, -2, "COPY");
  return 1;
}

int LuaRegionBind::create(lua_State* L) {
  LuaRegion ** ud = (LuaRegion **)lua_newuserdata(L, sizeof(LuaRegion *));
  luaL_setmetatable(L, meta);
  *ud = new LuaRegion(new mfc::Region(), false);
  return 1;
}

int LuaRegionBind::setrect(lua_State *L) {
  LuaRegion ** ud = (LuaRegion **)luaL_checkudata(L, 1, meta);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  (*ud)->get()->SetRect(x, y, w, h);
  return LuaHelper::chaining(L);
}

int LuaRegionBind::gc(lua_State* L) {
  LuaRegion* rgn= *(LuaRegion **)luaL_checkudata(L, 1, meta);   
  delete rgn;
  return 0;
}

int LuaRegionBind::boundrect(lua_State* L) {
  LuaRegion* rgn = *(LuaRegion **)luaL_checkudata(L, 1, meta);
  double x, y, width, height;
  rgn->get()->BoundRect(x, y, width, height);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 4;
}

int LuaRegionBind::combine(lua_State *L) {
  LuaRegion* rgn = *(LuaRegion **)luaL_checkudata(L, 1, meta);
  LuaRegion* rgn1 = *(LuaRegion **)luaL_checkudata(L, 2, meta);
  int combine_mode = luaL_checknumber(L, 3);
  int result = rgn->get()->Combine(*rgn1->get(), combine_mode);    
  lua_pushinteger(L, result);
  return 1;
}

int LuaRegionBind::offset(lua_State *L) {  
  LuaRegion* rgn = *(LuaRegion **)luaL_checkudata(L, 1, meta);
  int dx = luaL_checknumber(L, 2);
  int dy = luaL_checknumber(L, 3);
  rgn->get()->Offset(dx, dy);
  return LuaHelper::chaining(L);
}

} // namespace host
} // namespace psycle