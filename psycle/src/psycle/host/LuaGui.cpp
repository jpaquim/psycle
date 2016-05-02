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

const char* LuaMenuBarBind::meta = "psymenubarmeta";

int LuaMenuBarBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"update", update},
    {"setrootnode", setrootnode},
    { NULL, NULL }
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaMenuBarBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  ui::MenuBar* menubar = ui::Systems::instance().CreateMenuBar();
  LuaHelper::new_shared_userdata<>(L, meta, menubar);
  return 1;
}

int LuaMenuBarBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::MenuBar>(L, meta);  
}

int LuaMenuBarBind::setrootnode(lua_State* L) {
  boost::shared_ptr<ui::MenuBar> menu_bar = LuaHelper::check_sptr<ui::MenuBar>(L, 1, meta);
  using namespace ui::canvas;
  boost::shared_ptr<Node> node = 
    boost::dynamic_pointer_cast<Node>(LuaHelper::check_sptr<ui::canvas::Node>(L, 2, LuaTreeNodeBind::meta.c_str()));
  menu_bar->set_root_node(node);
  // menu_bar->Update();    
  return LuaHelper::chaining(L);
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
  LuaHelper::setfield(L, "note", cmd.GetNote());
  LuaHelper::setfield(L, "id", cmd.GetID());
  LuaHelper::setfield(L, "type", cmd.GetType());  
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
  LuaHelper::createwithstate<LuaActionListener>(L, meta, true);
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
// LuaSyetemMetricsBind
/////////////////////////////////////////////////////////////////////////////

int  LuaSystemMetrics::open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"screensize", screensize},    
    {NULL, NULL}
  };
  luaL_newlib(L, funcs);
  return 1;
}

int LuaSystemMetrics::screensize(lua_State* L) {
  lua_pushinteger(L, GetSystemMetrics(SM_CXFULLSCREEN));
  lua_pushinteger(L, GetSystemMetrics(SM_CYFULLSCREEN));
  return 2;
}


/////////////////////////////////////////////////////////////////////////////
// LuaFrameWnd+Bind
/////////////////////////////////////////////////////////////////////////////

void LuaFrameWnd::OnClose() {    
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
}

template <class T>
int LuaFrameItemBind<T>::create(lua_State* L) {
  //  int n = lua_gettop(L);  // Number of arguments    
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();
  LuaHelper::register_weakuserdata(L, item.get());  
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// LuaCanvasBind
///////////////////////////////////////////////////////////////////////////////
template <class T>
int LuaCanvasBind<T>::create(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  //item->set_imp(ui::ImpFactory::instance().CreateWindowCompositedImp());
  item->set_aligner(boost::shared_ptr<canvas::Aligner>(new ui::canvas::DefaultAligner()));
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}


template <class T>
bool CanvasItem<T>::SendEvent(lua_State* L,
                              const::std::string method,
                              ui::Event& ev, 
                              ui::Window& item) {
  LuaImport in(L, &item, LuaGlobal::proxy(L));
  bool has_method = in.open(method);
  if (has_method) {
    ev.StopWorkParent();
    canvas::Event* baseevent =  new canvas::Event();    
    luaL_requiref(L, "psycle.ui.canvas.event", LuaEventBind::open, 1);    
    LuaHelper::new_shared_userdata(L, LuaEventBind::meta, baseevent, lua_gettop(L)); 
    lua_remove(L, -2);
    in.pcall(0);
    ev = *baseevent;    
  }
  lua_gc(L, LUA_GCCOLLECT, 0);
  return has_method;
}

template <class T>
bool CanvasItem<T>::SendKeyEvent(lua_State* L,
                                 const::std::string method,
                                 ui::KeyEvent& ev, 
                                 ui::Window& item) {
  LuaImport in(L, &item, LuaGlobal::proxy(L));
  bool has_method = in.open(method);
  if (has_method) {
    ev.StopWorkParent();
    canvas::KeyEvent* key_event = 
      new canvas::KeyEvent(ev.keycode(), ev.flags());    
    luaL_requiref(L, "psycle.ui.canvas.keyevent", LuaKeyEventBind::open, 1);    
    LuaHelper::new_shared_userdata(L, LuaKeyEventBind::meta, key_event, lua_gettop(L)); 
    lua_remove(L, -2);
    in.pcall(0);
    ev = *key_event;    
  }
  lua_gc(L, LUA_GCCOLLECT, 0);
  return has_method;
}

template <class T>
bool CanvasItem<T>::SendMouseEvent(lua_State* L,
                                   const::std::string method,
                                   ui::MouseEvent& ev, 
                                   ui::Window& item) {  
  LuaImport in(L, &item, LuaGlobal::proxy(L));
  bool has_method = in.open(method);
  if (has_method) {
    ev.StopWorkParent();
    lua_newtable(L);
    LuaHelper::setfield(L, "shift", ev.shift());
    LuaHelper::setfield(L, "shiftkey", static_cast<int>(MK_SHIFT & ev.shift()));
    LuaHelper::setfield(L, "ctrlkey", static_cast<int>(MK_CONTROL & ev.shift()));
    LuaHelper::setfield(L, "altkey", static_cast<int>(MK_ALT & ev.shift()));
    LuaHelper::setfield(L, "clientx", ev.cx());
    LuaHelper::setfield(L, "clienty", ev.cy());
    LuaHelper::setfield(L, "x", ev.cx() - item.abs_pos().left());
    LuaHelper::setfield(L, "y", ev.cy() - item.abs_pos().top());
    LuaHelper::setfield(L, "button", ev.button());    
    in.pcall(0);    
  }
  lua_gc(L, LUA_GCCOLLECT, 0);
  return has_method;
}

template <class T>
void CanvasItem<T>::OnSize(double width, double height) {
  T::OnSize(width, height);  
  try {
    LuaImport in(L, this, LuaGlobal::proxy(L));
    if (in.open("onsize")) {
      in << width << height << pcall(0);
    }
  } catch(std::exception& e) {
    AfxMessageBox(e.what());
  }
}

template <class T>
bool CanvasItem<T>::OnUpdateArea() {
  LuaImport in(L, this, LuaGlobal::proxy(L));
  try {
    if (in.open("onupdatearea")) {
      ui::Area** rgn = (ui::Area **)lua_newuserdata(L, sizeof(ui::Area *));
      *rgn = area_.get();
      luaL_setmetatable(L, LuaAreaBind::meta);
      in.pcall(0);
      *rgn = 0;
    } else {
      T::OnUpdateArea();
    }
  } catch(std::exception& e) {
    AfxMessageBox(e.what());
  }
  return true;
}

/*template <class T>
void CanvasItem<T>::OnFocus() {
  T::OnFocus();
  try {
    LuaImport in(L, this, LuaGlobal::proxy(L));
    if (in.open("onfocus")) {
      in.pcall(0);
    }
  } catch(std::exception& e) {
    AfxMessageBox(e.what());
  }
}*/

template <class T>
void CanvasItem<T>::OnKillFocus() {
  T::OnKillFocus();
  try {
    LuaImport in(L, this, LuaGlobal::proxy(L));
    if (in.open("onkillfocus")) {
      in.pcall(0);
    }
  } catch(std::exception& e) {
    AfxMessageBox(e.what());
  }
}

// LuaTree
void LuaTree::OnClick(boost::shared_ptr<ui::Node> node) {
  try {
    LuaImport in(L, this, LuaGlobal::proxy(L));
    if (in.open("onclick")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      AfxMessageBox(e.what());    
  }
}

// LuaTreeNodeBind
std::string LuaTreeNodeBind::meta = "psytreenode"; // LuaTreeNode::type();

// LuaCanvas + Bind
template <class T>
int LuaCanvasBind<T>::open(lua_State *L) {  
  LuaHelper::openex(L, meta, setmethods, gc);
  static const char* const e[] = {
    "AUTO", "MOVE", "NO_DROP", "COL_RESIZE", "ALL_SCROLL", "POINTER",
    "NOT_ALLOWED", "ROW_RESIZE", "CROSSHAIR", "PROGRESS", "E_RESIZE",
    "NE_RESIZE", "DEFAULT", "TEXT", "N_RESIZE", "NW_RESIZE", "HELP",
    "VERTICAL_TEXT", "S_RESIZE", "SE_RESIZE", "INHERIT", "WAIT",
    "W_RESIZE", "SW_RESIZE"
  };
  // set cursor enum
  lua_newtable(L);
  LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]));
  lua_setfield(L, -2, "CURSOR");
  return 1;
}

template <class T>
int LuaCanvasBind<T>::setfocus(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "Wrong number of arguments.");
  if (err!=0) return err;
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
  Window::Ptr item = LuaGroupBind<>::test(L, 2);
  canvas->SetFocus(item);
  return LuaHelper::chaining(L);
}

template <class T>
int LuaCanvasBind<T>::showscrollbar(lua_State* L) {
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
//  canvas->show_scrollbar = true;
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
  // canvas->nposh = luaL_checknumber(L, -2);
  // canvas->nposv = luaL_checknumber(L, -1);
  // canvas->show_scrollbar = true;
  return LuaHelper::chaining(L);
}

template <class T>
int LuaCanvasBind<T>::invalidateontimer(lua_State* L) {     
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas->SetSave(true);
  return LuaHelper::chaining(L);
}

template <class T>
int LuaCanvasBind<T>::invalidatedirect(lua_State* L) {
  boost::shared_ptr<T> canvas = LuaHelper::check_sptr<T>(L, 1, meta);
  canvas->SetSave(false);
  return LuaHelper::chaining(L);
}
  
template class LuaCanvasBind<LuaCanvas>;

// LuaItemBind
void LuaItem::OnSize(double w, double h) {
  LuaImport in(L, this, LuaGlobal::proxy(L));
  try {
    if (in.open("onsize")) {
      in << w << h << pcall(0);
    }
  } catch(std::exception& e) {
    AfxMessageBox(e.what());
  }
}
  
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
  if (!item->imp()) {
    item->set_imp(ui::ImpFactory::instance().CreateWindowImp());    
  }
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
int LuaItemBind<T>::gc(lua_State* L) {
  typedef boost::shared_ptr<T> SPtr;
  SPtr item = *(SPtr*) luaL_checkudata(L, 1, meta.c_str());
  Window::Container subitems = item->SubItems();
  T::iterator it = subitems.begin();
  for ( ; it != subitems.end(); ++it) {
    canvas::Window::Ptr subitem = *it;
    LuaHelper::unregister_userdata<>(L, subitem.get());
  }       
  return LuaHelper::delete_shared_userdata<T>(L, meta.c_str());
}

template <class T>
int LuaItemBind<T>::setalign(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  window->set_align(static_cast<canvas::AlignStyle>(luaL_checkinteger(L, 2)));
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::align(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  lua_pushinteger(L, static_cast<int>(window->align()));
  return 1;
}

template <class T>
int LuaItemBind<T>::setmargin(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);  
  window->set_margin(Rect(ui::Point(luaL_checknumber(L, 2),
                                    luaL_checknumber(L, 3)),
                          ui::Point(luaL_checknumber(L, 4),
                                    luaL_checknumber(L, 5))));
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::margin(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  lua_pushinteger(L, window->margin().left());
  lua_pushinteger(L, window->margin().top());
  lua_pushinteger(L, window->margin().right());
  lua_pushinteger(L, window->margin().bottom());
  return 4;
}

template <class T>
int LuaItemBind<T>::setpadding(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  window->set_padding(Rect(ui::Point(luaL_checknumber(L, 2),
                                     luaL_checknumber(L, 3)),
                           ui::Point(luaL_checknumber(L, 4),
                                     luaL_checknumber(L, 5))));
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::padding(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  lua_pushinteger(L, window->padding().left());
  lua_pushinteger(L, window->padding().top());
  lua_pushinteger(L, window->padding().right());
  lua_pushinteger(L, window->padding().bottom());
  return 4;
}

template <class T>
int LuaItemBind<T>::setcursor(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "Wrong number of arguments.");
  if (err!=0) return err;
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  CursorStyle style = (CursorStyle) (int) luaL_checknumber(L, 2);
  window->SetCursor(style);
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::fls(lua_State* L) {
  const int n=lua_gettop(L);
  if (n==1) {
    LuaHelper::bind(L, meta, &T::FLS);
  } else
  if (n==2) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    ui::Region* rgn = *(ui::Region **)luaL_checkudata(L, 2, LuaRegionBind::meta);
    item->FLS(*rgn);            
  } else
  if (n==5) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    double width = luaL_checknumber(L, 4);
    double height = luaL_checknumber(L, 5);
    ui::Region rgn(ui::Rect(ui::Point(x, y), ui::Dimension(width, height)));
    item->FLS(rgn);
  } else {
    luaL_error(L, "Wrong number of arguments.");
  }
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::parent(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
  Window::WeakPtr group = item->parent();
  LuaHelper::find_weakuserdata<>(L, group.lock().get());
  return 1;
}

template <class T>
int LuaItemBind<T>::canvas(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  Window* root = window->root();
  LuaHelper::find_weakuserdata<>(L, root);
  return 1;
}

template class LuaItemBind<LuaItem>;

///////////////////////////////////////////////////////////////////////////////
// LuaGroupBind
///////////////////////////////////////////////////////////////////////////////

template <class T>
int LuaGroupBind<T>::create(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));  
  item->set_aligner(boost::shared_ptr<canvas::Aligner>(new ui::canvas::DefaultAligner()));
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
ui::Window::Ptr LuaGroupBind<T>::test(lua_State* L, int index) {  
  static const char* metas[] = {    
    LuaRectBind<>::meta.c_str(),
    LuaCanvasBind<>::meta.c_str(),
    LuaGroupBind<>::meta.c_str(),
    LuaTextBind<>::meta.c_str(),
    LuaPicBind<>::meta.c_str(),
    LuaLineBind<>::meta.c_str(),
    LuaItemBind<LuaItem>::meta.c_str(),
    LuaButtonBind<LuaButton>::meta.c_str(),
    LuaEditBind<LuaEdit>::meta.c_str(),
    LuaScrollBarBind<LuaScrollBar>::meta.c_str(),
    LuaScintillaBind<LuaScintilla>::meta.c_str(),
    LuaComboBoxBind<LuaComboBox>::meta.c_str(),
    LuaTreeBind<LuaTree>::meta.c_str(),
    LuaTableBind<LuaTable>::meta.c_str()
  };
  int size = sizeof(metas)/sizeof(metas[0]);
  Window::Ptr item;
  for (int i = 0; i < size; ++i) {
    item = LuaHelper::test_sptr<Window>(L, index, metas[i]);
    if (item) {
      break;
    }
  }  
  return item;
}

template <class T>
int LuaGroupBind<T>::removeall(lua_State* L) {  
  Window::Ptr group = LuaHelper::check_sptr<T>(L, 1, meta);
  std::vector<Window::Ptr> subitems = group->SubItems();
  T::iterator it = subitems.begin();
  for ( ; it != subitems.end(); ++it) {
    Window::Ptr subitem = *it;
    LuaHelper::unregister_userdata<>(L, subitem.get());
  }  
  group->RemoveAll();  
  return LuaHelper::chaining(L);
}

template <class T>
int LuaGroupBind<T>::remove(lua_State* L) {  
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);    
  Window::Ptr item = test(L, 2);
  if (item) {
    std::vector<Window::Ptr> subitems = item->SubItems();
    T::iterator it = subitems.begin();
    for ( ; it != subitems.end(); ++it) {
      Window::Ptr subitem = *it;
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
  Window::Ptr item = test(L, 2);
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
  /*Window* res = item->intersect(x, y);
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
  Window* item = test(L, 2);
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
  Window::Ptr item = test(L, 2);
  if (item) {
    int z  = group->zorder(item);
    lua_pushnumber(L, z + 1); // transform from zerobased to lua onebased
  } else {
    return luaL_error(L, "Item not in group");
  }  
  return 1;
}

template class LuaGroupBind<LuaGroup>;

template <class T>
int LuaComboBoxBind<T>::create(lua_State* L) {
  //  int n = lua_gettop(L);  // Number of arguments    
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();
  LuaHelper::register_weakuserdata(L, item.get());  
  return 1;
}

template <class T>
int LuaTreeBind<T>::create(lua_State* L) {  
  int n = lua_gettop(L);
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();  
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  // item->test1();
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
int LuaTableBind<T>::create(lua_State* L) {  
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();  
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
int LuaEditBind<T>::create(lua_State* L) {  
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
int LuaButtonBind<T>::create(lua_State* L) {  
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
int LuaScrollBarBind<T>::create(lua_State* L) {  
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n>=2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }       
  }
  ui::Orientation orientation = ui::VERT;
  if (n==3) {
     orientation = static_cast<ui::Orientation>(luaL_checkinteger(L, 3));
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp(orientation);
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

template <class T>
int LuaScintillaBind<T>::create(lua_State* L) {  
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {
      group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));
  item->MakeImp();
  LuaHelper::register_weakuserdata(L, item.get());
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaHelper::new_lua_module(L, "psycle.signal");  
  lua_setfield(L, -2, "keydown");
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// LuaGraphicsBind
///////////////////////////////////////////////////////////////////////////////

const char* LuaGraphicsBind::meta = "psygraphicsmeta";

int LuaGraphicsBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {  
    {"new", create},
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
    {"textsize", textsize}, 
    {"drawpolygon", drawpolygon},
    {"fillpolygon", fillpolygon},
    {"drawpolyline", drawpolyline},
    {"drawimage", drawimage},    
    { NULL, NULL }
  };
  return LuaHelper::open(L, meta, methods,  gc);  
}

int LuaGraphicsBind::create(lua_State* L) {
  ui::Graphics* g = ui::Systems::instance().CreateGraphics();  
  LuaHelper::new_shared_userdata<>(L, meta, g);
  return 1;  
}

int LuaGraphicsBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<Graphics>(L, meta);  
  return 0;
}

int LuaGraphicsBind::drawstring(lua_State* L) {
  LuaHelper::bind(L, meta, &Graphics::DrawString);  
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::drawrect(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 5, "");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);  
  double v1 = luaL_checknumber(L, 2);
  double v2 = luaL_checknumber(L, 2);
  double v3 = luaL_checknumber(L, 2);
  double v4 = luaL_checknumber(L, 2);
  g->DrawRect(ui::Rect(ui::Point(v1, v2), ui::Point(v1 + v3, v2 + v4)));
}

int LuaGraphicsBind::setfont(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "self, font");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  luaL_checktype(L, 2, LUA_TTABLE);
  lua_getfield(L, 2, "name");
  lua_getfield(L, 2, "height");
  const char *name = luaL_checkstring(L, -2);
  int height = luaL_checknumber(L, -1);
  std::auto_ptr<Font> font(ui::Systems::instance().CreateFont());
  FontInfo font_info;
  font_info.name = name;
  font_info.height = height;
  font->set_info(font_info);
  g->SetFont(*font);
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::font(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  FontInfo font_info = g->font().info();
  lua_newtable(L);
  LuaHelper::setfield(L, "name", font_info.name);
  LuaHelper::setfield(L, "height", font_info.height);
  return 1;
}

int LuaGraphicsBind::textsize(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 2, "");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);  
  const char* text = luaL_checkstring(L, 2);
  ui::Dimension text_dim = g->text_size(text);
  lua_pushnumber(L, text_dim.width());
  lua_pushnumber(L, text_dim.height());
  return 2;
}

int LuaGraphicsBind::drawpolygon(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
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
    points.push_back(Point(x, y));
    lua_pop(L, 2);
  }
  g->DrawPolygon(points);
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::drawpolyline(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
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
    points.push_back(Point(x, y));
    lua_pop(L, 2);
  }
  g->DrawPolyline(points);
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::fillpolygon(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
  if (err!=0) return err;
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
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
    points.push_back(Point(x, y));
    lua_pop(L, 2);
  }
  g->FillPolygon(points);
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::drawimage(lua_State* L) {
  int n = lua_gettop(L);
  if (n!=4 and n!=6 and n!=8) return luaL_error(L, "Wrong number of arguments.");
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
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

// LuaRun
void LuaRun::Run() {  
  LuaImport in(L, this, 0);  
  if (in.open("run")) {    
    in.pcall(0);    
    in.close();    
  }  
}

// LuaRunBind
std::string LuaRunBind::meta = "psyrunbind";

int LuaRunBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaRunBind::create(lua_State* L) {
  LuaRun* run = new LuaRun(L);  
  LuaHelper::new_shared_userdata<>(L, meta, run);
  return 1;
}

int LuaRunBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaRun>(L, meta);
}

// LuaItem+Bind
void LuaItem::Draw(Graphics* g, Region& draw_rgn) {    
  LuaImport in(L, this, LuaGlobal::proxy(L));  
  if (in.open("draw")) {        
    luaL_requiref(L, "psycle.ui.canvas.graphics", LuaGraphicsBind::open, true);
    int n = lua_gettop(L);
    LuaHelper::new_shared_userdata<>(L, LuaGraphicsBind::meta, g, n, true); 
    lua_remove(L, -2);
    in.pcall(0);    
    in.close();
    lua_gc(L, LUA_GCCOLLECT, 0);
  }  
}

// LuaImageBind
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
  ui::Image* img = ui::Systems::instance().CreateImage();
  //new mfc::Image(&PsycleGlobal::conf().macParam().dial);
  LuaHelper::new_shared_userdata<>(L, meta, img);
  return 1;
}

int LuaImageBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<Image>(L, meta);
}

int LuaImageBind::load(lua_State* L) {  
  try {
    LuaHelper::bind(L, meta, &Image::Load);
  } catch(std::exception &e) {
    return luaL_error(L, e.what());
  }
  return LuaHelper::chaining(L);
}

int LuaImageBind::settransparent(lua_State* L) {  
  int err = LuaHelper::check_argnum(L, 2, "self, color");
  if (err!=0) return err;
  boost::shared_ptr<Image> img = LuaHelper::check_sptr<Image>(L, 1, meta);
  int argb = luaL_checknumber(L, 2);
  img->SetTransparent(true, argb);
  return LuaHelper::chaining(L);
}

int LuaImageBind::size(lua_State* L) {  
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  boost::shared_ptr<Image> img = LuaHelper::check_sptr<Image>(L, 1, meta);  
  ui::Dimension dim = img->dim();
  lua_pushnumber(L, dim.width());
  lua_pushnumber(L, dim.height());
  return 2;
}


//LuaEventBind
const char* LuaEventBind::meta = "psyeventbind";

int LuaEventBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"preventdefault", preventdefault},
    {"isdefaultprevented", isdefaultprevented},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaEventBind::create(lua_State* L) {
  // int keycode = luaL_checkinteger(L, -1);  
  LuaHelper::new_shared_userdata<>(L, meta, new canvas::Event());
  return 1;
}

int LuaEventBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<canvas::Event>(L, meta);
}

// LuaKeyEventBind
const char* LuaKeyEventBind::meta = "psykeyeventbind";

int LuaKeyEventBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"keycode", keycode},
    {"shiftkey", shiftkey},
    {"ctrlkey", ctrlkey},
    {"preventdefault", preventdefault},
    {"isdefaultprevented", isdefaultprevented},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaKeyEventBind::create(lua_State* L) {
  int keycode = luaL_checkinteger(L, -1);
  canvas::KeyEvent* key_event = new canvas::KeyEvent(keycode, 0);
  LuaHelper::new_shared_userdata<>(L, meta, key_event);
  return 1;
}

int LuaKeyEventBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<canvas::KeyEvent>(L, meta);
}

// OrnamentFactoryBind

std::string OrnamentFactoryBind::meta = "canvasembelissherfactory";

int OrnamentFactoryBind::open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"new", create},      
    { NULL, NULL }
  };

  static const luaL_Reg methods[] = {   
    {"createlineborder", createlineborder},
    {"createwallpaper", createwallpaper},
    {"createfill", createfill},
    {"createboundfill", createboundfill},
    {NULL, NULL}
  };
  luaL_newmetatable(L, meta.c_str());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, methods, 0);    
  luaL_newlib(L, funcs); 
  return 1;
}

int OrnamentFactoryBind::create(lua_State* L) {  
  using namespace ui::canvas;
  OrnamentFactory** p = (OrnamentFactory**)lua_newuserdata(L, sizeof(OrnamentFactory*));
  luaL_setmetatable(L, meta.c_str());
  *p = &OrnamentFactory::Instance();  
  return 1;
}

int OrnamentFactoryBind::createlineborder(lua_State* L) { 
  using namespace ui::canvas;
  luaL_checkudata(L, 1, meta.c_str());
  int color = luaL_checkinteger(L, 2);
  Ornament* e = OrnamentFactory::Instance().CreateLineBorder(color);
  luaL_requiref(L, "psycle.ui.canvas.lineborder", &LineBorderBind::open, 1);
  LuaHelper::new_shared_userdata(L, LineBorderBind::meta.c_str(), e, 3);  
  return 1;
}

int OrnamentFactoryBind::createwallpaper(lua_State* L) { 
  using namespace ui::canvas;
  luaL_checkudata(L, 1, meta.c_str());
  ui::Image::Ptr image = LuaHelper::check_sptr<ui::Image>(L, 2, LuaImageBind::meta);  
  Ornament* e = OrnamentFactory::Instance().CreateWallpaper(image);
  luaL_requiref(L, "psycle.ui.canvas.wallpaper", &WallpaperBind::open, 1);
  LuaHelper::new_shared_userdata(L, WallpaperBind::meta.c_str(), e, 3);  
  return 1;
}

int OrnamentFactoryBind::createfill(lua_State* L) { 
  using namespace ui::canvas;
  luaL_checkudata(L, 1, meta.c_str());
  ARGB color = luaL_checknumber(L, 2);
  ui::Ornament* e = OrnamentFactory::Instance().CreateFill(color);
  luaL_requiref(L, "psycle.ui.canvas.fill", &WallpaperBind::open, 1);
  LuaHelper::new_shared_userdata(L, FillBind::meta.c_str(), e, 3);  
  return 1;
}

int OrnamentFactoryBind::createboundfill(lua_State* L) { 
  using namespace ui::canvas;
  luaL_checkudata(L, 1, meta.c_str());
  ARGB color = luaL_checknumber(L, 2);
  ui::Ornament* e = OrnamentFactory::Instance().CreateBoundFill(color);
  luaL_requiref(L, "psycle.ui.canvas.fill", &WallpaperBind::open, 1);
  LuaHelper::new_shared_userdata(L, FillBind::meta.c_str(), e, 3);  
  return 1;
}

std::string LineBorderBind::meta = "canvaslineborder";

int LineBorderBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"setborderradius", setborderradius},
    {"setborderstyle", setborderstyle},
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);   
  return 1;
}

int LineBorderBind::create(lua_State* L) {
  using namespace ui::canvas;
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;
  LineBorder* border = new LineBorder(0xFFFFFF);
  LuaHelper::new_shared_userdata<>(L, meta, border);
  return 1;
}

int LineBorderBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<canvas::LineBorder>(L, meta);  
}

int LineBorderBind::setborderradius(lua_State* L) {
  boost::shared_ptr<canvas::LineBorder> border = LuaHelper::check_sptr<canvas::LineBorder>(L, 1, meta);  
  double r1 = luaL_checknumber(L, 2);
  double r2 = luaL_checknumber(L, 3);
  double r3 = luaL_checknumber(L, 4);
  double r4 = luaL_checknumber(L, 5);
  border->set_border_radius(canvas::BorderRadius(r1, r2, r3, r4));  
  return LuaHelper::chaining(L);
}

int LineBorderBind::setborderstyle(lua_State* L) {
  boost::shared_ptr<canvas::LineBorder> border = LuaHelper::check_sptr<canvas::LineBorder>(L, 1, meta);  
  canvas::LineFormat left = static_cast<canvas::LineFormat>(luaL_checkinteger(L, 2));
  canvas::LineFormat top = static_cast<canvas::LineFormat>(luaL_checkinteger(L, 3));
  canvas::LineFormat right = static_cast<canvas::LineFormat>(luaL_checkinteger(L, 4));
  canvas::LineFormat bottom = static_cast<canvas::LineFormat>(luaL_checkinteger(L, 5));
  border->set_border_style(canvas::BorderStyle(left, top, right, bottom));  
  return LuaHelper::chaining(L);
}

std::string WallpaperBind::meta = "canvaswallpaper";

int WallpaperBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);   
  return 1;
}

int WallpaperBind::create(lua_State* L) {
  using namespace ui::canvas;
  int err = LuaHelper::check_argnum(L, 2, "self");
  if (err!=0) return err;
  ui::Image::Ptr image = LuaHelper::check_sptr<ui::Image>(L, 2, LuaImageBind::meta);
  Wallpaper* wallpaper = new Wallpaper(image);
  LuaHelper::new_shared_userdata<>(L, meta, wallpaper);
  return 1;
}

int WallpaperBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::canvas::Wallpaper>(L, meta);
}

std::string FillBind::meta = "canvasfill";

int FillBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);   
  return 1;
}

int FillBind::create(lua_State* L) {
  using namespace ui::canvas;
  int err = LuaHelper::check_argnum(L, 2, "self");
  if (err!=0) return err;
  ARGB color = luaL_checknumber(L, 2);
  Fill* fill = new Fill(color);
  LuaHelper::new_shared_userdata<>(L, meta, fill);
  return 1;
}

int FillBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::canvas::Fill>(L, meta);
}

// LuaScintilla

std::string LuaLexerBind::meta = "psylexermeta";

void LuaScintilla::OnFirstModified() {   
  LuaImport in(L, this, LuaGlobal::proxy(L));
  if (in.open("onfirstmodified")) {
    in.pcall(0);            
  }
}

template <class T>
int LuaTextBind<T>::setfont(lua_State* L) {
  boost::shared_ptr<LuaText> text = LuaHelper::check_sptr<LuaText>(L, 1, meta);    
  luaL_checktype(L, 2, LUA_TTABLE);
  lua_getfield(L, 2, "name");
  lua_getfield(L, 2, "height");
  const char *name = luaL_checkstring(L, -2);
  int height = luaL_checknumber(L, -1);
  std::auto_ptr<Font> font(ui::Systems::instance().CreateFont());
  FontInfo font_info;
  font_info.name = name;
  font_info.height = height;
  font->set_info(font_info);
  text->set_font(*font);
  return LuaHelper::chaining(L);
}

template <class T>
int LuaTextBind<T>::setalignment(lua_State* L) {
  boost::shared_ptr<LuaText> text = LuaHelper::check_sptr<LuaText>(L, 1, meta);      
  text->set_alignment(static_cast<canvas::AlignStyle>(luaL_checkinteger(L, 2)));
  return LuaHelper::chaining(L);
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
  LuaHelper::setfield(L, "OR", RGN_OR);
  LuaHelper::setfield(L, "AND", RGN_AND);
  LuaHelper::setfield(L, "XOR", RGN_XOR);
  LuaHelper::setfield(L, "DIFF", RGN_DIFF);
  LuaHelper::setfield(L, "COPY", RGN_COPY);
  return 1;
}

int LuaRegionBind::create(lua_State* L) {
  ui::Region ** ud = (ui::Region **)lua_newuserdata(L, sizeof(ui::Region *));
  luaL_setmetatable(L, meta);
  *ud = ui::Systems::instance().CreateRegion();
  return 1;
}

int LuaRegionBind::gc(lua_State* L) {
  ui::Region* rgn= *(ui::Region **)luaL_checkudata(L, 1, meta);
  delete rgn;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// LuaAreaBind
///////////////////////////////////////////////////////////////////////////////

const char* LuaAreaBind::meta = "psyareabind";

int LuaAreaBind::open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"new", create},      
    { NULL, NULL }
  };
  static const luaL_Reg methods[] = {
    {"boundrect", boundrect},
//    {"setrect", setrect},
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
  LuaHelper::setfield(L, "OR", RGN_OR);
  LuaHelper::setfield(L, "AND", RGN_AND);
  LuaHelper::setfield(L, "XOR", RGN_XOR);
  LuaHelper::setfield(L, "DIFF", RGN_DIFF);
  LuaHelper::setfield(L, "COPY", RGN_COPY);
  return 1;
}

int LuaAreaBind::create(lua_State* L) {
  ui::Area ** ud = (ui::Area **)lua_newuserdata(L, sizeof(ui::Area *));
  luaL_setmetatable(L, meta);
  *ud = new ui::Area();
  return 1;
}

int LuaAreaBind::gc(lua_State* L) {
  ui::Area* rgn= *(ui::Area **)luaL_checkudata(L, 1, meta);
  delete rgn;
  return 0;
}

} // namespace host
} // namespace psycle