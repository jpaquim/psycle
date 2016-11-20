// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

// #include "stdafx.h"
#include "LuaGui.hpp"
#include "LuaHost.hpp"
#include "MfcUi.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

namespace psycle {
namespace host {

const char* LuaMenuBarBind::meta = "psymenubarmeta";

ui::MultiType PsycleStock::value(int stock_key) const {
  using namespace ui;
  PsycleConfig::PatternView* pv_cfg = &PsycleGlobal::conf().patView();
  PsycleConfig::MachineParam* mv_cfg = &PsycleGlobal::conf().macParam();  
  ARGB result(0xFF000000);
  using namespace stock;
  using namespace color;  
  switch (stock_key) {
    case PVBACKGROUND: result = pv_cfg->background; break;
    case PVROW: result = pv_cfg->row; break;    
    case PVROW4BEAT: result = pv_cfg->row4beat; break;
    case PVROWBEAT: result = pv_cfg->rowbeat; break;
    case PVSEPARATOR: result = pv_cfg->separator; break;
    case PVFONT: result = pv_cfg->font; break;
    case PVCURSOR: result = pv_cfg->cursor; break;
    case PVSELECTION: result = pv_cfg->selection; break;
    case PVPLAYBAR: result = pv_cfg->playbar; break;    
    case MVFONTBOTTOMCOLOR : result = mv_cfg->fontBottomColor; break;
		case MVFONTHTOPCOLOR : result = mv_cfg->fonthTopColor; break;
		case MVFONTHBOTTOMCOLOR : result = mv_cfg->fonthBottomColor; break;
		case MVFONTTITLECOLOR : result = mv_cfg->fonttitleColor; break;
		case MVTOPCOLOR : result = mv_cfg->topColor; break;
		case MVBOTTOMCOLOR : result = mv_cfg->bottomColor; break;
		case MVHTOPCOLOR : result = mv_cfg->hTopColor; break;
		case MVHBOTTOMCOLOR : result = mv_cfg->hBottomColor; break;
		case MVTITLECOLOR : result = mv_cfg->titleColor; break;
    default:
    break;
  }    
  return ToARGB(result);  
}

std::string PsycleStock::key_tostring(int stock_key) const {  
  std::string result;
  using namespace stock;
  using namespace color;  
  switch (stock_key) {
    case PVBACKGROUND: result = "stock.color.PVBACKGROUND"; break;
    case PVROW: result = "stock.color.PVROW"; break;
    case PVROW4BEAT: result = "stock.color.PVROW4BEAT"; break;
    case PVROWBEAT: result = "stock.color.PVROWBEAT"; break;
    case PVSEPARATOR: result = "stock.color.PVSEPARATOR"; break;
    case PVFONT: result = "stock.color.PVFONT"; break;
    case PVCURSOR: result = "stock.color.PVCURSOR"; break;
    case PVSELECTION: result = "stock.color.PVSELECTION"; break;
    case PVPLAYBAR: result = "stock.color.PVPLAYBAR"; break;
    case MVFONTBOTTOMCOLOR : result = "stock.color.MVFONTBOTTOMCOLOR"; break;
		case MVFONTHTOPCOLOR : result = "stock.color.MVFONTHTOPCOLOR"; break;
		case MVFONTHBOTTOMCOLOR : result = "stock.color.MVFONTHBOTTOMCOLOR"; break;
		case MVFONTTITLECOLOR : result = "stock.color.MVFONTTITLECOLOR"; break;
		case MVTOPCOLOR : result = "stock.color.MVTOPCOLOR"; break;
		case MVBOTTOMCOLOR : result = "stock.color.MVBOTTOMCOLOR"; break;
		case MVHTOPCOLOR : result = "stock.color.MVHTOPCOLOR"; break;
		case MVHBOTTOMCOLOR : result = "stock.color.MVHBOTTOMCOLOR"; break;
		case MVTITLECOLOR : result = "stock.color.MVTITLECOLOR"; break;
    default:
    break;
  }
  return result;
}

LockIF* locker(lua_State *L) {
  return LuaGlobal::proxy(L);  
}

int LuaMenuBarBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"update", update},
    {"invalidate", invalidate},
    {"setrootnode", setrootnode},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaMenuBarBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) {
    return err;
  }
  LuaHelper::new_shared_userdata<>(L, meta, new LuaMenuBar(L));
  return 1;
}

int LuaMenuBarBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaMenuBar>(L, meta);  
}

int LuaMenuBarBind::setrootnode(lua_State* L) {
  using namespace ui;
  MenuBar::Ptr menu_bar = LuaHelper::check_sptr<LuaMenuBar>(L, 1, meta);
  Node::Ptr node = LuaHelper::check_sptr<Node>(L, 2, 
                                               LuaNodeBind::meta.c_str());
  menu_bar->set_root_node(node);  
  return LuaHelper::chaining(L);
}

const char* LuaPopupMenuBind::meta = "psypopupmenumeta";

int LuaPopupMenuBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"update", update},
    {"invalidate", invalidate},
    {"setrootnode", setrootnode},
    {"track", track},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaPopupMenuBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) {
    return err;
  }
  LuaHelper::new_shared_userdata<>(L, meta, new LuaPopupMenu(L));
  return 1;
}

int LuaPopupMenuBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaPopupMenu>(L, meta);  
}

int LuaPopupMenuBind::setrootnode(lua_State* L) {
  using namespace ui;
  PopupMenu::Ptr popup_menu = LuaHelper::check_sptr<LuaPopupMenu>(L, 1, meta);
  Node::Ptr node = LuaHelper::check_sptr<Node>(L, 2,
                                               LuaNodeBind::meta.c_str());
  popup_menu->set_root_node(node);  
  return LuaHelper::chaining(L);
}

int LuaPopupMenuBind::track(lua_State* L) {
  using namespace ui;
  PopupMenu::Ptr popup_menu = LuaHelper::check_sptr<LuaPopupMenu>(L, 1, meta);
  Point::Ptr point = LuaHelper::check_sptr<Point>(L, 2, LuaPointBind::meta);
  popup_menu->Track(*point.get());    
  return LuaHelper::chaining(L);
}

/////////////////////////////////////////////////////////////////////////////
// PsycleFileOpenBind
/////////////////////////////////////////////////////////////////////////////

const char* LuaFileOpenBind::meta = "psyfileopenmeta";

int LuaFileOpenBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"show", show},
    {"filename", filename},
    {"setfolder", setfolder},
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods,  gc);
  return 1;
}

int LuaFileOpenBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) {
    return err;
  }
  LuaHelper::new_shared_userdata<>(L, meta, new LuaFileDialog(true));
  return 1;
}

int LuaFileOpenBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaFileDialog>(L, meta);
}

int LuaFileOpenBind::show(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg = 
      LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  char szFilters[]= 
      "Text Files (*.NC)|*.NC|Text Files (*.lua)|*.lua|All Files (*.*)|*.*||";
  CFileDialog dlg(luadlg->is_open_dlg(), "lua", "*.lua",
              OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());
  if (luadlg->folder() != "") {
    dlg.m_ofn.lpstrInitialDir  = luadlg->folder().c_str();
  }
  INT_PTR result = dlg.DoModal();  
  luadlg->set_filename(dlg.GetPathName().GetString());
  try {
    LuaImport import(L, luadlg.get(), 0);  
    if (import.open(result == IDOK ? "onok" : "oncancel")) {
      import << luadlg->filename() << pcall(0);
    }
  } catch (std::exception& e) {
    return luaL_error(L, e.what());
  }
  lua_pushboolean(L, result == IDOK);
  return 1;
}

int LuaFileOpenBind::filename(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg 
      = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  lua_pushstring(L, luadlg->filename().c_str());
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
    {"setfolder", setfolder},
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);
  return 1;
}

int LuaFileSaveBind::create(lua_State* L) {  
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) {
    return err;
  }  
  LuaHelper::new_shared_userdata<>(L, meta, new LuaFileDialog(false));
  return 1;
}

int LuaFileSaveBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaFileDialog>(L, meta);
}

int LuaFileSaveBind::show(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg
      = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  char szFilters[]= "Text Files (*.NC)|*.NC|Lua Files (*.lua)|*.lua|All Files (*.*)|*.*||";
  CFileDialog dlg(luadlg->is_open_dlg(), "lua", "*.lua",
              OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());
  if (luadlg->folder() != "") {
    dlg.m_ofn.lpstrInitialDir  = luadlg->folder().c_str();
  }
  INT_PTR result = dlg.DoModal();
  luadlg->set_filename(dlg.GetPathName().GetString());
  try {
    LuaImport import(L, luadlg.get(), 0);  
    if (import.open(result == IDOK ? "onok" : "oncancel")) {
      import << luadlg->filename() << pcall(0);
    }
  } catch (std::exception& e) {
    return luaL_error(L, e.what());
  }
  lua_pushboolean(L, result == IDOK);
  return 1;
}

int LuaFileSaveBind::filename(lua_State* L) {
  boost::shared_ptr<LuaFileDialog> luadlg
      = LuaHelper::check_sptr<LuaFileDialog>(L, 1, meta);
  lua_pushstring(L, luadlg->filename().c_str());
  return 1;
}

// LuaFileObserver + Bind
void LuaFileObserver::OnCreateFile(const std::string& path) { 
  struct {
    std::string path;
    lua_State* L;
    LuaFileObserver* that;
    void operator()() const {
      try {
        LuaImport in(L, that, locker(L));
        if (in.open("oncreatefile")) {
          lua_pushstring(L, path.c_str());
          in << pcall(0);      
        }
      } catch (std::exception& e) {
         ui::alert(e.what());		
      }
    }
   } f;   
  f.L = L;
  f.that = this;
  f.path = path;
  LuaGlobal::InvokeLater(L, f);  
}

void LuaFileObserver::OnDeleteFile(const std::string& path) {
  struct {
    std::string path;
    lua_State* L;
    LuaFileObserver* that;
    void operator()() const {
      try {
        LuaImport in(L, that, locker(L));
        if (in.open("ondeletefile")) {
          lua_pushstring(L, path.c_str());
          in << pcall(0);      
        }
      } catch (std::exception& e) {
         ui::alert(e.what());		
      }
    }
   } f;   
  f.L = L;
  f.that = this;
  f.path = path;
  LuaGlobal::InvokeLater(L, f);
}

void LuaFileObserver::OnChangeFile(const std::string& path) {   
  struct {
    std::string path;
    lua_State* L;
    LuaFileObserver* that;
    void operator()() const {
      try {
        LuaImport in(L, that, locker(L));
        if (in.open("onchangefile")) {
          lua_pushstring(L, path.c_str());
          in << pcall(0);      
        }
      } catch (std::exception& e) {
         ui::alert(e.what());		
      }
    }
   } f;   
  f.L = L;
  f.that = this;
  f.path = path;
  LuaGlobal::InvokeLater(L, f);
}

const char* LuaFileObserverBind::meta = "psyfileobservermeta";

int LuaFileObserverBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"setdirectory", setdirectory},
    {"directory", directory},
    {"startwatching", startwatching},
    {"stopwatching", stopwatching},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaFileObserverBind::create(lua_State* L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 1) {
    return luaL_error(L, "Got %d arguments expected 1 (self)", n);
  }  
  LuaHelper::new_shared_userdata<>(L, meta, new LuaFileObserver(L), 1, true);
  return 1;
}

int LuaFileObserverBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<LuaFileObserver>(L, meta);
  return 0;
}

int LuaFileObserverBind::setdirectory(lua_State* L) { 
  LUAEXPORTM(L, meta, &LuaFileObserver::SetDirectory);
}

int LuaFileObserverBind::directory(lua_State* L) { 
  LUAEXPORTM(L, meta, &LuaFileObserver::directory);
}

int LuaFileObserverBind::startwatching(lua_State* L) { 
  LUAEXPORTM(L, meta, &LuaFileObserver::StartWatching);
}

int LuaFileObserverBind::stopwatching(lua_State* L) { 
  LUAEXPORTM(L, meta, &LuaFileObserver::StopWatching);
}

// LuaSystemsBind
std::string LuaSystemsBind::meta = "psyuisystemsbind";

int LuaSystemsBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"setstyleclass", setstyleclass},    
    {"changewindowtype", changewindowtype},
    {"updatewindows", updatewindows},
    {"updatewindow", updatewindow},
    {NULL, NULL}
  };
  static const char* const e[] = {
    "WINDOW", "GROUP", "FRAME", "CANVAS", "POPUPFRAME",
    "RECTANGLEBOX", "HEADERGROUP", "LINE", "PIC",
    "TEXT", "EDIT", "BUTTON", "COMBOBOX", "CHECKBOX",
    "RADIOBUTTON", "GROUPBOX", "LISTVIEW", "TREEVIEW",
    "SCROLLBAR", "SCROLLBOX", "SCINTILLA"
  };    
  LuaHelper::open(L, meta, methods,  gc);
  lua_newtable(L);
  LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]), 1);
  lua_setfield(L, -2, "windowtypes");
  return 1;
}

int LuaSystemsBind::create(lua_State* L) {  
  LuaHelper::new_shared_userdata<>(L, meta,
                                   LuaGlobal::proxy(L)->systems(), 1, true);
  return 1;
}

int LuaSystemsBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Systems>(L, meta);
}

int LuaSystemsBind::changewindowtype(lua_State* L) {
  using namespace ui;
  Properties properties;
  Systems::Ptr systems = LuaHelper::check_sptr<Systems>(L, 1, meta);
  WindowTypes::Type window_type =
      static_cast<WindowTypes::Type>(luaL_checkinteger(L, 2));
  Window::Ptr item = LuaGroupBind<>::test(L, 3);
  systems->ChangeWindowType(window_type, item.get());
  return LuaHelper::chaining(L);
}

int LuaSystemsBind::updatewindow(lua_State* L) {
  using namespace ui;
  Properties properties;
  Systems::Ptr systems = LuaHelper::check_sptr<Systems>(L, 1, meta);
  WindowTypes::Type window_type =
      static_cast<WindowTypes::Type>(luaL_checkinteger(L, 2));
  Window::Ptr item = LuaGroupBind<>::test(L, 3);
  systems->UpdateWindow(window_type, item.get());
  return LuaHelper::chaining(L);
}

int LuaSystemsBind::setstyleclass(lua_State* L) {
  using namespace ui;
  Properties properties;
  Systems::Ptr systems = LuaHelper::check_sptr<Systems>(L, 1, meta);
  WindowTypes::Type window_type =
      static_cast<WindowTypes::Type>(luaL_checkinteger(L, 2));
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    const char *key = lua_tostring(L, -2);
    int type = lua_type(L, -1);
    if (type == LUA_TTABLE) {
      lua_getfield(L, -1, "value_");        
      ARGB value = lua_tonumber(L, -1);
      lua_pop(L, 1);
      lua_getfield(L, -1, "stockkey_");
      int stock_key(-1);
      if (!lua_isnil(L, -1)) {
        stock_key = luaL_checkinteger(L, -1);
      }
      lua_pop(L, 1); 
      PsycleStock stock;
      Property p(stock);
      p.set_value(value);
      p.set_stock_key(stock_key);
      properties.elements[key] = p;
    }
    lua_pop(L, 1);
  }  
  systems->set_class_properties(window_type, properties);
  return LuaHelper::chaining(L);
}

// LuaSystemMetricsBind
int  LuaSystemMetrics::open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"screensize", screensize},    
    {NULL, NULL}
  };
  luaL_newlib(L, funcs);
  return 1;
}

int LuaSystemMetrics::screensize(lua_State* L) {
  using namespace ui;
  LuaHelper::requirenew<LuaDimensionBind>(
      L, "psycle.ui.dimension", 
      new Dimension(Systems::instance().metrics().screen_dimension()));
  return 1;
}

//  LuaCenterShowStrategyBind
const char* LuaFrameAlignerBind::meta = "psyframealignermeta";

int LuaFrameAlignerBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},        
    {"sizetoscreen", sizetoscreen},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaFrameAlignerBind::create(lua_State* L) {
  using namespace ui;  
  AlignStyle::Type alignment(AlignStyle::ALCENTER);
  int n = lua_gettop(L);  
  if (n == 2) {
    alignment = static_cast<AlignStyle::Type>(luaL_checkinteger(L, 2));
  } else 
  if (n != 1) {
    luaL_error(L, "FrameAligner create. Wrong number of arguments.");
  }    
  LuaHelper::new_shared_userdata<>(L, meta, new FrameAligner(alignment));
  return 1;
}

int LuaFrameAlignerBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::FrameAligner>(L, meta);
}

// LuaFrame + Bind
void LuaFrame::OnClose() {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onclose")) {      
      in.pcall(0);
      in.close();
      LuaHelper::collect_full_garbage(L);
    } 
  } catch (std::exception& e) {
    ui::alert(e.what());		
  }        
}

void LuaFrame::OnShow() {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onshow")) {      
      in << pcall(0);      
    } 
  } catch (std::exception& e) {
    ui::alert(e.what());   
  }  
}

void LuaFrame::OnContextPopup(ui::Event& ev, const ui::Point& mouse_point,
                              const ui::Node::Ptr& node) {
  using namespace ui;
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("oncontextpopup")) {      
      LuaHelper::requirenew<LuaEventBind>(L, "psycle.ui.canvas.event", &ev,
                                          true);
      LuaHelper::requirenew<LuaPointBind>(L, "psycle.ui.point",
                                          new Point(mouse_point));
      in.pcall(0);
    } 
  } catch (std::exception& e) {
    alert(e.what());   
  }
}

// LuaPopupFrame + Bind
void LuaPopupFrame::OnClose() {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onclose")) {      
      in.pcall(0);
      in.close();
      LuaHelper::collect_full_garbage(L);
    } 
  } catch (std::exception& e) {
    ui::alert(e.what());   
  }  
}

void LuaPopupFrame::OnShow() {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onshow")) {      
      in.pcall(0);      
    } 
  } catch (std::exception& e) {
    ui::alert(e.what());   
  }  
}

// LuaCanvasBind
template <class T>
int LuaCanvasBind<T>::create(lua_State* L) { 
  using namespace ui; 
  Group::Ptr group = LuaItemBind<>::testgroup(L);  
  Window::Ptr item = LuaHelper::new_shared_userdata(L, meta, new T(L));  
  item->set_aligner(boost::shared_ptr<Aligner>(new DefaultAligner()));  
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }
  return 1;
}

template <class T>
bool CanvasItem<T>::SendEvent(lua_State* L,
                              const::std::string method,
                              ui::Event& ev, 
                              ui::Window& item) {
  LuaImport in(L, &item, locker(L));
  bool has_method = in.open(method);
  if (has_method) {
    ev.StopWorkParent();
    ui::Event* base_ev =  new ui::Event();    
    LuaHelper::requirenew<LuaEventBind>(L, "psycle.ui.canvas.event", base_ev);
    in.pcall(0);
    ev = *base_ev;    
  }
  LuaHelper::collect_full_garbage(L);
  return has_method;
}

template <class T>
bool CanvasItem<T>::SendKeyEvent(lua_State* L,
                                 const::std::string method,
                                 ui::KeyEvent& ev, 
                                 ui::Window& item) {
  LuaImport in(L, &item, locker(L));
  bool has_method = in.open(method);
  if (has_method) {
    ev.StopWorkParent();    
    LuaHelper::requirenew<LuaKeyEventBind>(L, "psycle.ui.canvas.keyevent", &ev, true);
    in.pcall(0);
    LuaHelper::collect_full_garbage(L);
  }  
  return has_method;
}

template <class T>
bool CanvasItem<T>::SendMouseEvent(lua_State* L,
                                   const::std::string method,
                                   ui::MouseEvent& ev, 
                                   ui::Window& item) {  
	LuaImport in(L, &item, locker(L));
	bool has_method = in.open(method);
	if (has_method) {
		ev.StopWorkParent();
		LuaHelper::requirenew<LuaMouseEventBind>(L, "psycle.ui.canvas.mouseevent", &ev, true);
		in.pcall(0);
    LuaHelper::collect_full_garbage(L);
    in.close();
	}  	
	return has_method;
}

template <class T>
void CanvasItem<T>::OnSize(const ui::Dimension& dimension) {
  T::OnSize(dimension);  
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onsize")) {
      in << dimension.width() << dimension.height() << pcall(0);
    }
  } catch(std::exception& e) {
    ui::alert(e.what());
  }
}

template <class T>
void CanvasItem<T>::set_properties(const ui::Properties& properties) {
  using namespace ui;  
  T::set_properties(properties);  
  LuaImport in(L, this, locker(L));
  try {
    if (in.open("setproperties")) {
      LuaHelper::new_lua_module(L, "psycle.orderedtable");      
      for (Properties::Container::const_iterator it = properties.elements.begin(); it != properties.elements.end(); ++it) {    
        const Property& p = it->second;
        LuaHelper::new_lua_module(L, "property");        
        MultiType value = p.value();
        if (value.which() == 0) {
          lua_pushnumber(L, boost::get<ARGB>(p.value()));
        } else {
          luaL_error(L, "Wrong Property type.");
        }
        lua_setfield(L, -2, "value_");
        if (p.stock_key() != -1) {
          lua_pushinteger(L, p.stock_key());
          lua_setfield(L, -2, "stock_key_");
        }
        lua_setfield(L, -2, it->first.c_str());
      }
      in << pcall(0);
    }
  } catch(std::exception& e) {
    alert(e.what());
  }  
}


template<class T>
ui::Dimension CanvasItem<T>::OnCalcAutoDimension() const {
	ui::Dimension result;	
	LuaImport in(L, (void*) this, locker(L));
  try {
    if (in.open("oncalcautodimension")) {			
			in.pcall(2);
			result.set(luaL_checknumber(L, -1), luaL_checknumber(L, -2));
		} else {
			result = T::OnCalcAutoDimension();
    }
	} catch(std::exception& e) {
    ui::alert(e.what());
  }
	return result;
}

// LuaTreeView
void LuaTreeView::OnChange(const ui::Node::Ptr& node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onchange")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaTreeView::OnRightClick(const ui::Node::Ptr& node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onrightclick")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaTreeView::OnEditing(const ui::Node::Ptr& node, const std::string& text) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onediting")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in << text << pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaTreeView::OnEdited(const ui::Node::Ptr& node, const std::string& text) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onedited")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in << text << pcall(0);      
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaTreeView::OnContextPopup(ui::Event& ev, const ui::Point& mouse_point, const ui::Node::Ptr& node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("oncontextpopup")) {      
      LuaHelper::requirenew<LuaEventBind>(L, "psycle.ui.canvas.event", &ev, true);
      LuaHelper::setfield(L, "x", mouse_point.x());
      LuaHelper::setfield(L, "y", mouse_point.y());
      if (node.get()) {
        LuaHelper::find_weakuserdata(L, node.get());
      } else {
        lua_pushnil(L);
      }
      lua_setfield(L, -2, "node");
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

// LuaListView
void LuaListView::OnChange(const ui::Node::Ptr& node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onchange")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaListView::OnRightClick(const ui::Node::Ptr& node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onrightclick")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaListView::OnEditing(const ui::Node::Ptr& node, const std::string& text) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onediting")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in << text << pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaListView::OnEdited(const ui::Node::Ptr& node, const std::string& text) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onedited")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in << text << pcall(0);      
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

// LuaNodeBind
std::string LuaNodeBind::meta = ui::Node::type();

// LuaMenuContainer
void LuaMenuBar::OnMenuItemClick(boost::shared_ptr<ui::Node> node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onclick")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

// LuaPopupMenu
void LuaPopupMenu::OnMenuItemClick(boost::shared_ptr<ui::Node> node) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onclick")) {
      LuaHelper::find_weakuserdata<>(L, node.get());
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

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
  LuaImport in(L, this, locker(L));
  try {
    if (in.open("onsize")) {
      in << w << h << pcall(0);
    }
  } catch(std::exception& e) {
    ui::alert(e.what());
  }
}

bool LuaItem::transparent() const {
  bool result(false);
  LuaImport in(L, const_cast<LuaItem*>(this), locker(L));
  try {
    if (in.open("transparent")) {
      in << pcall(1) >> result;
    } else {
      result = CanvasItem<ui::Window>::transparent();
    }
  } catch(std::exception& e) {
    ui::alert(e.what());
  }
  return result;
}


template <class T>
boost::shared_ptr<ui::Group> LuaItemBind<T>::testgroup(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
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
  }
  return group;
}
  
template <class T>
int LuaItemBind<T>::create(lua_State* L) {  
  ui::Group::Ptr group = testgroup(L);    
  ui::Window::Ptr item = LuaHelper::new_shared_userdata(L, meta.c_str(), LuaGlobal::proxy(L)->systems()->Create(T::window_type()));  

  if (item->debug_text() == "text") {
     LuaHelper::find_weakuserdata(L, item.get());
     if (lua_isnil(L, -1)) {
       assert(0);
     } else {
       lua_pop(L, 1);
     }
  }

  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  LuaGlobal::proxy(L)->systems()->UpdateWindow(T::window_type(), item.get());  
  return 1;
}

template <class T>
int LuaItemBind<T>::gc(lua_State* L) {
  typedef boost::shared_ptr<T> SPtr;
  SPtr item = *(SPtr*) luaL_checkudata(L, 1, meta.c_str());
  if (!item->empty()) {
    ui::Window::Container subitems = item->SubItems();
    T::iterator it = subitems.begin();
    for (; it != subitems.end(); ++it) {      
      LuaHelper::unregister_userdata<>(L, (*it).get());          
    }
  }
  return LuaHelper::delete_shared_userdata<T>(L, meta.c_str());
}

template <class T>
int LuaItemBind<T>::setalign(lua_State* L) {  
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  window->set_align(static_cast<ui::AlignStyle::Type>(luaL_checkinteger(L, 2)));
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::align(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  lua_pushinteger(L, static_cast<int>(window->align()));
  return 1;
}

template <class T>
int LuaItemBind<T>::setcursorposition(lua_State* L) {
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);  
  window->SetCursorPosition(ui::Point(luaL_checknumber(L, 2),
                                      luaL_checknumber(L, 3)));
  return LuaHelper::chaining(L);
} 

template <class T>
int LuaItemBind<T>::setpadding(lua_State* L) {
  using namespace ui;
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
  BoxSpace::Ptr space = LuaHelper::check_sptr<BoxSpace>(L, 2, LuaBoxSpaceBind::meta);
  item->set_padding(*space);    
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::padding(lua_State* L) {
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);    
  LuaHelper::requirenew<LuaBoxSpaceBind>(L, "psycle.ui.boxspace", new ui::BoxSpace(item->padding()));
  return 1;
}

template <class T>
int LuaItemBind<T>::setmargin(lua_State* L) {
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
  boost::shared_ptr<ui::BoxSpace> space = LuaHelper::check_sptr<ui::BoxSpace>(L, 2, LuaBoxSpaceBind::meta);
  item->set_margin(*space);    
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::margin(lua_State* L) {
  boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);    
  LuaHelper::requirenew<LuaBoxSpaceBind>(L, "psycle.ui.boxspace", new ui::BoxSpace(item->margin()));
  return 1;
}

template <class T>
int LuaItemBind<T>::setcursor(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "Wrong number of arguments.");
  if (err!=0) return err;
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  CursorStyle::Type style = (CursorStyle::Type) (int) luaL_checknumber(L, 2);
  window->SetCursor(style);
  return LuaHelper::chaining(L);
}

template <class T>
int LuaItemBind<T>::fls(lua_State* L) {
  const int n=lua_gettop(L);
  if (n==1) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
    item->FLS();        
  } else
  if (n==2) {
    boost::shared_ptr<T> item = LuaHelper::check_sptr<T>(L, 1, meta);
	ui::Region::Ptr rgn = LuaHelper::check_sptr<ui::Region>(L, 1, meta);    
    item->FLS(*rgn.get());            
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
  LuaHelper::find_weakuserdata<>(L, item->parent());
  return 1;
}

template <class T>
int LuaItemBind<T>::root(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err != 0) {
    return err;
  }
  boost::shared_ptr<T> window = LuaHelper::check_sptr<T>(L, 1, meta);
  Window* root = window->root();
  LuaHelper::find_weakuserdata<>(L, root);
  return 1;
}

template class LuaItemBind<LuaItem>;

// LuaGroupBind
template <class T>
int LuaGroupBind<T>::create(lua_State* L) {
  using namespace ui;
  int n = lua_gettop(L);  // Number of arguments  
  boost::shared_ptr<LuaGroup> group;
  if (n==2 && !lua_isnil(L, 2)) {
    group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaGroupBind<>::meta);
    if (!group) {			
		  group = LuaHelper::test_sptr<LuaGroup>(L, 2, LuaHeaderGroupBind<>::meta);
			if (!group) {
				group = LuaHelper::check_sptr<LuaGroup>(L, 2, LuaCanvasBind<>::meta);
			}			
    }    
  }
  boost::shared_ptr<T> item = LuaHelper::new_shared_userdata(L, meta.c_str(), new T(L));  
  item->set_aligner(boost::shared_ptr<Aligner>(new ui::DefaultAligner()));  
  if (group) {    
    group->Add(item);
    LuaHelper::register_userdata(L, item.get());
  }  
  return 1;
}

template <class T>
ui::Window::Ptr LuaGroupBind<T>::test(lua_State* L, int index) {  
  using namespace ui;
  static const char* metas[] = {    
    LuaRectangleBoxBind<>::meta.c_str(),
    LuaCanvasBind<>::meta.c_str(),
    LuaGroupBind<>::meta.c_str(),
	  LuaHeaderGroupBind<>::meta.c_str(),
    LuaTextBind<>::meta.c_str(),
	  LuaSplitterBind<>::meta.c_str(),
    LuaPicBind<>::meta.c_str(),
    LuaLineBind<>::meta.c_str(),
    LuaItemBind<LuaItem>::meta.c_str(),
    LuaButtonBind<LuaButton>::meta.c_str(),
	  LuaGroupBoxBind<LuaGroupBox>::meta.c_str(),
	  LuaRadioButtonBind<LuaRadioButton>::meta.c_str(),
    LuaEditBind<LuaEdit>::meta.c_str(),
    LuaScrollBarBind<LuaScrollBar>::meta.c_str(),
    LuaScintillaBind<LuaScintilla>::meta.c_str(),
    LuaComboBoxBind<LuaComboBox>::meta.c_str(),
    LuaTreeViewBind<LuaTreeView>::meta.c_str(),
    LuaListViewBind<LuaListView>::meta.c_str(),
    LuaScrollBoxBind<LuaScrollBox>::meta.c_str()
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
  using namespace ui;
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
  using namespace ui;
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
  using namespace ui;
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
    luaL_error(L, "Argument is no window.");
  }
  return LuaHelper::chaining(L);
}

template <class T>
int LuaGroupBind<T>::insert(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 3, "self, window, window");
  if (err != 0) {
    return err;
  }
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);
  Window::Ptr pos = test(L, 2);
  Window::Ptr window = test(L, 3);
  if (window) {
    try {
      Window::iterator  it = std::find(group->begin(), group->end(), pos);
      if (it != group->end() ) {
        group->Insert(it, window);        
      } else {
        group->Add(window);
      }      
      LuaHelper::register_userdata(L, window.get());
    } catch(std::exception &e) {
      luaL_error(L, e.what());
    }
  } else {
    luaL_error(L, "Argument is no window.");
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
int LuaGroupBind<T>::at(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "self, index");
  if (err!=0) {
    return err;
  }
  T::Ptr group = LuaHelper::check_sptr<T>(L, 1, meta);
  int index = static_cast<int>(luaL_checkinteger(L, 2));
  try {
    Window::Ptr item = group->at(index + 1);
    LuaHelper::find_userdata(L, (item).get());
  } catch (std::exception& e) {
    luaL_error(L, e.what());
  }  
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
  if (err != 0) {
    return err;
  }
  boost::shared_ptr<T> group = LuaHelper::check_sptr<T>(L, 1, meta);
  ui::Window::Ptr item = test(L, 2);
  if (item) {
    int z  = group->zorder(item);
    lua_pushnumber(L, z + 1); // transform from zerobased to lua onebased
  } else {
    return luaL_error(L, "Window not in group");
  }  
  return 1;
}

template class LuaGroupBind<LuaGroup>;
template class LuaHeaderGroupBind<LuaHeaderGroup>;

// LuaFontInfoBind
std::string LuaFontInfoBind::meta = "fontinfometa";

int LuaFontInfoBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {  
    {"new", create},    
    {"setfamily", setfamily},
    {"family", family},
    {"setsize", setsize},
    {"size", size},
    {"style", style},
    {"tostring", tostring},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods, gc);  
}

int LuaFontInfoBind::create(lua_State* L) {  
  using namespace ui;
  int n = lua_gettop(L);
    LuaHelper::new_shared_userdata<>(L, meta,  new FontInfo());
  if (n==1) {
    LuaHelper::new_shared_userdata<>(L, meta,  new FontInfo());
  } else 
  if (n==2) {
    const char* family_name = luaL_checkstring(L, 2);
    LuaHelper::new_shared_userdata<>(L, meta,  new FontInfo(family_name));
  } else
  if (n==3) {
    const char* family_name = luaL_checkstring(L, 2);
    int size  = luaL_checkinteger(L, 3);    
    LuaHelper::new_shared_userdata<>(L, meta,  new FontInfo(family_name, size));
  } else 
  if (n==4) {
    const char* family_name = luaL_checkstring(L, 2);
    int size  = luaL_checkinteger(L, 3);    
    int style  = luaL_checkinteger(L, 4);    
    FontInfo* font_info = new FontInfo(family_name, size);
    font_info->set_style(static_cast<FontStyle::Type>(style));
    LuaHelper::new_shared_userdata<>(L, meta,  font_info);
  } else {
    luaL_error(L, "Wrong Number of Arguments.");
  }   
  return 1;  
}

int LuaFontInfoBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<ui::FontInfo>(L, meta);  
  return 0;
}

int LuaFontInfoBind::tostring(lua_State* L) {  
  using namespace ui;
	FontInfo::Ptr font_info = LuaHelper::check_sptr<FontInfo>(L, 1, meta);
  lua_pushstring(L, font_info->tostring().c_str());
  return 1;
}

// LuaFontBind
const char* LuaFontBind::meta = "psyfontsmeta";

int LuaFontBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {  
    {"new", create},
    {"setfontinfo", setfontinfo},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);  
}

int LuaFontBind::create(lua_State* L) {
  ui::Font* font = ui::Systems::instance().CreateFont();  
  LuaHelper::new_shared_userdata<>(L, meta, font);
  return 1;  
}

int LuaFontBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<ui::Font>(L, meta);  
  return 0;
}

int LuaFontBind::setfontinfo(lua_State* L) {
  using namespace ui;
  Font::Ptr font = LuaHelper::check_sptr<Font>(L, 1, meta);
  FontInfo::Ptr font_info = LuaHelper::check_sptr<FontInfo>(L, 2, LuaFontInfoBind::meta);    
  font->set_font_info(*font_info.get());    
  return LuaHelper::chaining(L);
}

// LuaFontsBind
const char* LuaFontsBind::meta = "psyfontsmeta";

int LuaFontsBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {  
    {"new", create},
    {"fontlist", fontlist},
    {"importfont", importfont},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);  
}

int LuaFontsBind::create(lua_State* L) {
  ui::Fonts* fonts = ui::Systems::instance().CreateFonts();  
  LuaHelper::new_shared_userdata<>(L, meta, fonts);
  return 1;  
}

int LuaFontsBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<ui::Fonts>(L, meta);  
  return 0;
}

int LuaFontsBind::fontlist(lua_State* L) {
  using namespace ui;
  boost::shared_ptr<Fonts> fonts = LuaHelper::check_sptr<Fonts>(L, 1, meta);  
  lua_newtable(L);
  std::vector<std::string> list = fonts->font_list();
  std::vector<std::string>::iterator it = list.begin();
  for (int i = 0; it != list.end(); ++it, ++i) {    
    lua_pushstring(L, (*it).c_str());
    lua_rawseti(L, -2, i);
  }
  return 1;
}

// LuaGraphicsBind
const char* LuaGraphicsBind::meta = "psygraphicsmeta";

int LuaGraphicsBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {  
    {"new", create},
    {"translate", translate},
    {"setcolor", setcolor},
    {"color", color},
    {"drawline", drawline},
    {"drawcurve", drawcurve},
    {"drawarc", drawarc},    
    {"drawrect", drawrect},    
    {"drawoval", drawoval},
    {"drawcircle", drawcircle},
    {"drawroundrect", drawroundrect},
    {"fillrect", fillrect},
    {"fillroundrect", fillroundrect},
    {"filloval", filloval},
    {"fillcircle", fillcircle},
    {"copyarea", copyarea},
    {"drawstring", drawstring},
    {"setfont", setfont},
    {"font", font},
    {"textdimension", textdimension}, 
    {"drawpolygon", drawpolygon},
    {"fillpolygon", fillpolygon},
    {"drawpolyline", drawpolyline},
    {"drawimage", drawimage},		
    {"beginpath", beginpath},
    {"endpath", endpath},
    {"fillpath", fillpath},
    {"drawpath", drawpath},
    {"moveto", moveto},
    {"lineto", lineto},
    {"curveto", curveto},
    {"arcto", arcto},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);  
}

int LuaGraphicsBind::create(lua_State* L) {
  using namespace ui;
  Graphics* g = Systems::instance().CreateGraphics();  
  LuaHelper::new_shared_userdata<>(L, meta, g);
  return 1;  
}

int LuaGraphicsBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<ui::Graphics>(L, meta);  
  return 0;
}

int LuaGraphicsBind::drawstring(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 3, "");
  if (err != 0) {
    return err;
  }  
  Graphics::Ptr g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  const char* str = luaL_checkstring(L, 2);
  Point::Ptr pos = LuaHelper::check_sptr<Point>(L, 3, LuaPointBind::meta);
  g->DrawString(str, *pos.get());
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::setfont(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "self, font");
   if (err != 0) {
    return err;
  }
  Graphics::Ptr g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  Font::Ptr font = LuaHelper::check_sptr<Font>(L, 2, LuaFontBind::meta);  
  g->SetFont(*font.get());
  return LuaHelper::chaining(L);
}

int LuaGraphicsBind::font(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err != 0) {
    return err;
  }
  Graphics::Ptr g = LuaHelper::check_sptr<Graphics>(L, 1, meta);  
  LuaHelper::requirenew<LuaFontBind>(L, "psycle.ui.font", new Font(g->font()));
  return 1;
}

int LuaGraphicsBind::textdimension(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "");
  if (err != 0) {
    return err;
  }
  Graphics::Ptr g = LuaHelper::check_sptr<Graphics>(L, 1, meta);  
  const char* text = luaL_checkstring(L, 2);  
  LuaHelper::requirenew<LuaDimensionBind>(L, "psycle.ui.dimension",
      new Dimension(g->text_dimension(text)));
  return 1;
}

int LuaGraphicsBind::drawpolygon(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
  if (err !=0 ) {
    return err;
  }
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  luaL_checktype(L, 2, LUA_TTABLE);
  size_t n1 = lua_rawlen(L, 2);
  size_t n2 = lua_rawlen(L, 2);
  if (n1 != n2) {
    return luaL_error(L, "Length of x and y points have to be the same size.");
  }
  Points points;
  for (size_t i=1; i <= n1; ++i) {
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
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
  if (err!=0) {
    return err;
  }
  Graphics::Ptr g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  luaL_checktype(L, 2, LUA_TTABLE);
  size_t n1 = lua_rawlen(L, 2);
  size_t n2 = lua_rawlen(L, 2);
  if (n1!=n2) return luaL_error(L, "Length of x and y points have to be the same size.");
  Points points;
  for (size_t i=1; i <= n1; ++i) {
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
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 3, "self, xpoints, ypoints");
  if (err!=0) {
    return err;
  }
  boost::shared_ptr<Graphics> g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  luaL_checktype(L, 2, LUA_TTABLE);
  size_t n1 = lua_rawlen(L, 2);
  size_t n2 = lua_rawlen(L, 2);
	if (n1 != n2) {
		return luaL_error(L, "Length of x and y points have to be the same size.");
	}
  Points points;
  for (size_t i=1; i <=n1; ++i) {
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
  using namespace ui;
  int n = lua_gettop(L);
  if (n!=3 && n!=4) {
    return luaL_error(L, "Wrong number of arguments.");
  }
  Graphics::Ptr g = LuaHelper::check_sptr<Graphics>(L, 1, meta);
  Image::Ptr img = LuaHelper::check_sptr<Image>(L, 2, LuaImageBind::meta);  
  if (n==3) {
    Point::Ptr pos = LuaHelper::test_sptr<Point>(L, 3, LuaPointBind::meta);  
    if (pos.get()) {
      g->DrawImage(img.get(), *pos.get());
    } else {
      Rect::Ptr pos = LuaHelper::check_sptr<Rect>(L, 3, LuaUiRectBind::meta);
      g->DrawImage(img.get(), *pos.get());
    }
  } else
  if (n==4) {    
    Rect::Ptr pos = LuaHelper::check_sptr<Rect>(L, 3, LuaUiRectBind::meta);
    Point::Ptr src = LuaHelper::check_sptr<Point>(L, 4, LuaPointBind::meta);      
    g->DrawImage(img.get(), *pos.get(), *src.get());
  }
  return LuaHelper::chaining(L);
}

// LuaRun + Bind
void LuaRun::Run() {  
  LuaImport in(L, this, 0);  
  if (in.open("run")) {    
    in.pcall(0);
  }  
}

std::string LuaRunBind::meta = "psyrunbind";

int LuaRunBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaRunBind::create(lua_State* L) {  
  LuaHelper::new_shared_userdata(L, meta, new LuaRun(L));
  return 1;
}

int LuaRunBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaRun>(L, meta);
}

// LuaItem + Bind
void LuaItem::Draw(ui::Graphics* g, ui::Region& draw_rgn) {    
  LuaImport in(L, this, locker(L));  
  if (in.open("draw")) {        
    LuaHelper::requirenew<LuaGraphicsBind>(L, "psycle.ui.canvas.graphics", g, true);
	  LuaHelper::requirenew<LuaRegionBind>(L, "psycle.ui.region", &draw_rgn, true);
    in.pcall(0);    
    LuaHelper::collect_full_garbage(L);
  }
}

// LuaImageBind
const char* LuaImageBind::meta = "psyimagebind";

int LuaImageBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
	  {"reset", reset},
    {"load", load},
	  {"cut", cut},
	  {"save", save},
    {"settransparent", settransparent},
    {"size", size},
	  {"resize", resize},
	  {"rotate", rotate},
	  {"graphics", graphics},
	  {"setpixel", setpixel},
	  {"getpixel", getpixel},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaImageBind::create(lua_State* L) {  
  LuaHelper::new_shared_userdata<>(L, meta, ui::Systems::instance().CreateImage());
  return 1;
}

int LuaImageBind::gc(lua_State* L) {  
  return LuaHelper::delete_shared_userdata<ui::Image>(L, meta);
}

int LuaImageBind::graphics(lua_State * L) {
  using namespace ui;
	int err = LuaHelper::check_argnum(L, 1, "self");
  if (err != 0) {
    return err;
  }
	Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
	std::auto_ptr<Graphics> g = img->graphics();	
	LuaHelper::requirenew<LuaGraphicsBind>(L, "psycle.ui.canvas.graphics", g.get());	
	g.release();
	return 1;
}

int LuaImageBind::setpixel(lua_State* L) {
  using namespace ui;
	int err = LuaHelper::check_argnum(L, 4, "self, color");
	if (err != 0) {
    return err;
  }
	Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
	ARGB argb = static_cast<ARGB>(luaL_checknumber(L, 4));
	img->SetPixel(Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)), argb);
	return LuaHelper::chaining(L);
}

int LuaImageBind::getpixel(lua_State * L) {
  using namespace ui;
	int err = LuaHelper::check_argnum(L, 3, "self");
	if (err != 0) {
    return err;
  }
	Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
	ARGB color = img->GetPixel(ui::Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
	lua_pushnumber(L, static_cast<int>(color));
	return 1;
}

int LuaImageBind::save(lua_State * L) {
  using namespace ui;
	int n = lua_gettop(L);
	Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);  
	if (n == 1) {
		try {
			img->Save();
		}
		catch (std::exception& e) {
			std::stringstream str;
			str << "File Save Error: " << e.what();			
			luaL_error(L, str.str().c_str());
		}
	} else
  if (n == 2) {
		try {
			const char* filename = luaL_checkstring(L, 2);
			img->Save(filename);
		}
		catch (std::exception& e) {
			std::stringstream str;
			str << "File Save Error: " << e.what();
			luaL_error(L, str.str().c_str());			
		}
	}
	return LuaHelper::chaining(L);
}

int LuaImageBind::cut(lua_State * L) {
  using namespace ui;
	int err = LuaHelper::check_argnum(L, 5, "self, x, y, w, h");
  if (err!=0) {
    return err;
  }
  Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);  
	img->Cut(Rect(Point(luaL_checknumber(L, 2), luaL_checknumber(L, 3)),
		            Dimension(luaL_checknumber(L, 4), luaL_checknumber(L, 5))));
  return LuaHelper::chaining(L);	
}

int LuaImageBind::resize(lua_State * L) {
  using namespace ui;
	int err = LuaHelper::check_argnum(L, 3, "self, w, h");
	if (err != 0) {
    return err;
  }
	Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
	img->Resize(Dimension(luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
	return LuaHelper::chaining(L);
}

int LuaImageBind::rotate(lua_State * L) {
  using namespace ui;
	int err = LuaHelper::check_argnum(L, 2, "self, radians");
	if (err != 0) {
    return err;
  }
	Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
	img->Rotate(static_cast<float>(luaL_checknumber(L, 2)));
	return LuaHelper::chaining(L);
}

int LuaImageBind::settransparent(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "self, color");
  if (err!=0) {
    return err;
  }
  Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);
  ARGB argb = static_cast<ARGB>(luaL_checknumber(L, 2));
  img->SetTransparent(true, argb);
  return LuaHelper::chaining(L);
}

int LuaImageBind::reset(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 3, "self, x, y");
  if (err!=0) {
    return err;
  }
  Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);  
	img->Reset(Dimension(luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
  return LuaHelper::chaining(L);
}

int LuaImageBind::size(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err != 0) {
    return err;
  }
  Image::Ptr img = LuaHelper::check_sptr<Image>(L, 1, meta);  
  Dimension dim = img->dim();
  lua_pushnumber(L, dim.width());
  lua_pushnumber(L, dim.height());
  return 2;
}

std::string LuaImagesBind::meta = "psyimagesbind";

// PointBind
std::string LuaPointBind::meta = "psypointmeta";

int LuaPointBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {    
    {"new", create},
    {"setxy", setxy},
    {"setx", setx},
    {"x", x},
    {"sety", sety},
    {"y", y},
    {"offset", offset},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaPointBind::create(lua_State* L) {
  using namespace ui;  
  int n = lua_gettop(L);
  if (n == 1) {
    LuaHelper::new_shared_userdata<>(L, meta, new Point());
  } else
  if (n == 3) {
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    LuaHelper::new_shared_userdata<>(L, meta, new Point(x, y));
  } else {
    luaL_error(L, "Wrong number of arguments");
  }
  return 1;
}

int LuaPointBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Point>(L, meta);
}

// DimensionBind
std::string LuaDimensionBind::meta = "psydimensionmeta";

int LuaDimensionBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"set", set},
    {"set_width", setwidth},
    {"width", width},
    {"set_height", setheight},
    {"height", height},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaDimensionBind::create(lua_State* L) {
  int n = lua_gettop(L);
  if (n == 1) {
    LuaHelper::new_shared_userdata<>(L, meta, new ui::Dimension());
  } else
  if (n == 3) {
    double width = luaL_checknumber(L, 2);
    double height = luaL_checknumber(L, 3);
    LuaHelper::new_shared_userdata<>(L, meta, new ui::Dimension(width, height));
  } else {
    luaL_error(L, "Wrong number of arguments");
  }
  return 1;
}

int LuaDimensionBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Dimension>(L, meta);
}

// RectBind
std::string LuaUiRectBind::meta = "psyuirectmeta";

int LuaUiRectBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"left", left},
    {"top", top},
    {"right", right},
    {"bottom", bottom},
    {"width", width},
    {"height", height},
    {"topleft", topleft},
    {"bottomright", bottomright},
    {"intersect", intersect},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaUiRectBind::create(lua_State* L) {
  using namespace ui;
  int n = lua_gettop(L);
  if (n==1) {
    LuaHelper::new_shared_userdata<>(L, meta, new Rect());
  } else
  if (n==3) {    
    Point::Ptr point = LuaHelper::check_sptr<Point>(L, 2, LuaPointBind::meta);
    Dimension::Ptr dimension =
        LuaHelper::check_sptr<Dimension>(L, 3, LuaDimensionBind::meta);
    LuaHelper::new_shared_userdata<>(L, meta, new Rect(*point.get(), *dimension.get()));    
  } else {
    luaL_error(L, "Wrong number of arguments");
  }  
  return 1;
}

int LuaUiRectBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Rect>(L, meta);
}

int LuaUiRectBind::set(lua_State* L) {
  return LuaHelper::chaining(L);
}

int LuaUiRectBind::topleft(lua_State *L) {
  using namespace ui;
  Rect::Ptr rect = LuaHelper::check_sptr<Rect>(L, 1, LuaUiRectBind::meta);
  LuaHelper::requirenew<LuaPointBind>(L,
                                      "psycle.ui.point", 
																			 new Point(rect->top_left()));
  return 1;
}

int LuaUiRectBind::bottomright(lua_State *L) {
  using namespace ui;
  Rect::Ptr rect = LuaHelper::check_sptr<Rect>(L, 1, LuaUiRectBind::meta);
  LuaHelper::requirenew<LuaPointBind>(L,
                                      "psycle.ui.point", 
																			 new Point(rect->bottom_right()));
  return 1;
}

// BoxSpaceBind
std::string LuaBoxSpaceBind::meta = "psyuiboxspacemeta";

int LuaBoxSpaceBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"left", left},
    {"top", top},
    {"right", right},
    {"bottom", bottom},    
    {"set", set},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaBoxSpaceBind::create(lua_State* L) {
  int n = lua_gettop(L);
  if (n==1) {
    LuaHelper::new_shared_userdata<>(L, meta, new ui::BoxSpace());
  } else
  if (n==2) {
    double space = luaL_checknumber(L, 2);
    LuaHelper::new_shared_userdata<>(L, meta, new ui::BoxSpace(space));
  } else
  if (n==5) {    
    double top = luaL_checknumber(L, 2);
    double right = luaL_checknumber(L, 3);
    double bottom = luaL_checknumber(L, 4);
    double left = luaL_checknumber(L, 5);    
    LuaHelper::new_shared_userdata<>(L, meta, new ui::BoxSpace(top, right, bottom, left));
  } else {
    luaL_error(L, "Wrong number of arguments");
  }  
  return 1;
}

int LuaBoxSpaceBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::BoxSpace>(L, meta);
}

int LuaBoxSpaceBind::set(lua_State* L) {
  int n = lua_gettop(L);
  ui::BoxSpace::Ptr box_space = LuaHelper::check_sptr<ui::BoxSpace>(L, 1, meta);
  if (n == 2) {
    double space = luaL_checknumber(L, 2);
    box_space->set(space);    
  } else
  if (n == 5) {    
    double top = luaL_checknumber(L, 2);
    double right = luaL_checknumber(L, 3);
    double bottom = luaL_checknumber(L, 4);
    double left = luaL_checknumber(L, 5);    
    box_space->set(top, right, bottom, left);
  }
  return LuaHelper::chaining(L);
}

// LuaBorderRadiusBind
std::string LuaBorderRadiusBind::meta = "psyuiborderradiusemeta";

int LuaBorderRadiusBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaBorderRadiusBind::create(lua_State* L) {
  int n = lua_gettop(L);
  if (n==1) {
    LuaHelper::new_shared_userdata<>(L, meta, new ui::BorderRadius());
  } else
  if (n==2) {
    double radius = luaL_checknumber(L, 2);
    LuaHelper::new_shared_userdata<>(L, meta, new ui::BorderRadius(radius));
  } else {
    luaL_error(L, "Wrong number of arguments");
  }  
  return 1;
}

int LuaBorderRadiusBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::BorderRadius>(L, meta);
}

//LuaEventBind
const char* LuaEventBind::meta = "psyeventbind";

int LuaEventBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"preventdefault", preventdefault},
    {"isdefaultprevented", isdefaultprevented},
    {"stoppropagation", stoppropagation},
    {"ispropagationstopped", ispropagationstopped},    
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods,  gc);
}

int LuaEventBind::create(lua_State* L) {  
  LuaHelper::new_shared_userdata<>(L, meta, new ui::Event());
  return 1;
}

int LuaEventBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Event>(L, meta);
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
    {"stoppropagation", stoppropagation},
    {"ispropagationstopped", ispropagationstopped},
    {NULL, NULL}
  };  
  LuaHelper::open(L, meta, methods,  gc);
  static const char* const e[] = {
		"LBUTTON", "RBUTTON", "CANCEL", "MBUTTON", "XBUTTON1", "XBUTTON2",
    "UNDEFINED07", "BACK", "TAB", "RESERVED0A", "RESERVED11", "CLEAR",
    "RETURN", "UNDEFINED0E", "UNDEFINED0F", "SHIFT", "CONTROL", "ALT",
    "PAUSE", "CAPITAL", "KANA", "HANGUL", "UNDEFINED16", "FINAL", "HANJA",
    "UNDEFINED1A", "ESCAPE", "CONVERT", "NONCONVERT", "ACCEPT", "MODECHANGE",
    "SPACE", "PRIOR", "NEXT", "END", "HOME", "LEFT", "UP", "RIGHT", "DOWN",
    "SELECT", "PRINT", "EXECUTE", "SNAPSHOT", "INSERT", "DELETE", "HELP"
	};
  size_t size = sizeof(e)/sizeof(e[0]);    
  LuaHelper::buildenum(L, e, size, 1);  
  static const char* const func_keys[] = {
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",    
    "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F21", 
    "F22", "F23", "F24"    
	};  
  size = sizeof(func_keys)/sizeof(func_keys[0]);    
  LuaHelper::buildenum(L, func_keys, size, 112);         
  return 1;
}

int LuaKeyEventBind::create(lua_State* L) {
	int keycode = static_cast<int>(luaL_checkinteger(L, -1));  
  LuaHelper::new_shared_userdata<>(L, meta, new ui::KeyEvent(keycode, 0));
  return 1;
}

int LuaKeyEventBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::KeyEvent>(L, meta);
}

// LuaMouseEventBind
const char* LuaMouseEventBind::meta = "psymouseeventbind";

int LuaMouseEventBind::open(lua_State *L) {
	static const luaL_Reg methods[] = {
		{"new", create},
    {"button", button},		
    {"clientpos", clientpos},
		{"preventdefault", preventdefault},
		{"isdefaultprevented", isdefaultprevented},
		{"stoppropagation", stoppropagation},
		{"ispropagationstopped", ispropagationstopped},
		{NULL, NULL}
	};
	LuaHelper::open(L, meta, methods, gc);	
	return 1;
}

int LuaMouseEventBind::create(lua_State* L) {	
	LuaHelper::new_shared_userdata<>(L, meta, new ui::MouseEvent());
	return 1;
}

int LuaMouseEventBind::gc(lua_State* L) {
	return LuaHelper::delete_shared_userdata<ui::MouseEvent>(L, meta);
}

// OrnamentFactoryBind
std::string OrnamentFactoryBind::meta = "canvasembelissherfactory";

int OrnamentFactoryBind::open(lua_State *L) {
  static const luaL_Reg funcs[] = {
    {"new", create},      
    {NULL, NULL}
  };

  static const luaL_Reg methods[] = {   
    {"createlineborder", createlineborder},
    {"createwallpaper", createwallpaper},
    {"createfill", createfill},
    {"createcirclefill", createcirclefill},
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
  using namespace ui;
  OrnamentFactory** p = (OrnamentFactory**)lua_newuserdata(L, sizeof(OrnamentFactory*));
  luaL_setmetatable(L, meta.c_str());
  *p = &OrnamentFactory::Instance();  
  return 1;
}

int OrnamentFactoryBind::createlineborder(lua_State* L) { 
  using namespace ui; 
	luaL_checkudata(L, 1, meta.c_str());  
	int n = lua_gettop(L);
	if (n==2) {		
		LuaHelper::requirenew<LineBorderBind>(L,
																					"psycle.ui.canvas.lineborder", 
																					ui::OrnamentFactory::Instance()
																						.CreateLineBorder(
																							static_cast<ARGB>(luaL_checknumber(L, 2))));
  } else if (n == 3) {
    boost::shared_ptr<ui::BoxSpace> space = LuaHelper::check_sptr<ui::BoxSpace>(L, 3, LuaBoxSpaceBind::meta);
		LuaHelper::requirenew<LineBorderBind>(L,
																					"psycle.ui.canvas.lineborder", 
																					ui::OrnamentFactory::Instance().CreateLineBorder(
																						 static_cast<ARGB>(luaL_checknumber(L, 2)), *space.get()));
	} else {
		return luaL_error(L, "LineBorder, wrong number of arguments");
	}
  return 1;
}

int OrnamentFactoryBind::createwallpaper(lua_State* L) { 
  luaL_checkudata(L, 1, meta.c_str());
  LuaHelper::requirenew<WallpaperBind>(L,
                                       "psycle.ui.canvas.wallpaper", 
                                        ui::OrnamentFactory::Instance()
                                         .CreateWallpaper(
                                            LuaHelper::check_sptr<ui::Image>(
                                               L, 2, LuaImageBind::meta)));
  return 1;
}

int OrnamentFactoryBind::createfill(lua_State* L) {   
  using namespace ui;
  luaL_checkudata(L, 1, meta.c_str());
  LuaHelper::requirenew<FillBind>(L,
                                  "psycle.ui.canvas.fill",
                                     ui::OrnamentFactory::Instance()
                                       .CreateFill(static_cast<ARGB>(luaL_checknumber(L, 2))));
  return 1;
}

int OrnamentFactoryBind::createcirclefill(lua_State* L) {   
  using namespace ui;
  luaL_checkudata(L, 1, meta.c_str());
  LuaHelper::requirenew<FillBind>(L,
                                  "psycle.ui.canvas.circlefill",
                                     ui::OrnamentFactory::Instance()
                                       .CreateCircleFill(static_cast<ARGB>(luaL_checknumber(L, 2))));
  return 1;
}

int OrnamentFactoryBind::createboundfill(lua_State* L) {
  using namespace ui;
  luaL_checkudata(L, 1, meta.c_str());
  LuaHelper::requirenew<FillBind>(L,
                                  "psycle.ui.canvas.fill",
                                     ui::OrnamentFactory::Instance()
                                       .CreateBoundFill(
                                         static_cast<ARGB>(luaL_checknumber(L, 2))));
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
  using namespace ui;
  int n = lua_gettop(L);
  ARGB color(0xFFFFFFFF);
  if (n==2) {
    color = static_cast<ARGB>(luaL_checkinteger(L, 2));
  }  
  LineBorder* border = new LineBorder(color);
  LuaHelper::new_shared_userdata<>(L, meta, border);
  return 1;
}

int LineBorderBind::gc(lua_State* L) {
  using namespace ui;
  return LuaHelper::delete_shared_userdata<LineBorder>(L, meta);  
}

int LineBorderBind::setborderradius(lua_State* L) {
  using namespace ui;
  boost::shared_ptr<LineBorder> border = LuaHelper::check_sptr<LineBorder>(L, 1, meta);  
  boost::shared_ptr<ui::BorderRadius> radius = LuaHelper::check_sptr<ui::BorderRadius>(L, 2, LuaBorderRadiusBind::meta);  
  border->set_border_radius(*radius.get());  
  return LuaHelper::chaining(L);
} 

int LineBorderBind::setborderstyle(lua_State* L) {
  using namespace ui;
  boost::shared_ptr<LineBorder> border = LuaHelper::check_sptr<LineBorder>(L, 1, meta);  
  LineFormat left = static_cast<LineFormat>(luaL_checkinteger(L, 2));
  LineFormat top = static_cast<LineFormat>(luaL_checkinteger(L, 3));
  LineFormat right = static_cast<LineFormat>(luaL_checkinteger(L, 4));
  LineFormat bottom = static_cast<LineFormat>(luaL_checkinteger(L, 5));
  border->set_border_style(BorderStyle(left, top, right, bottom));  
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
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "self");
  if (err!=0) return err;
  ui::Image::Ptr image = LuaHelper::check_sptr<ui::Image>(L, 2, LuaImageBind::meta);
  Wallpaper* wallpaper = new Wallpaper(image);
  LuaHelper::new_shared_userdata<>(L, meta, wallpaper);
  return 1;
}

int WallpaperBind::gc(lua_State* L) {
  using namespace ui;
  return LuaHelper::delete_shared_userdata<ui::Wallpaper>(L, meta);
}

std::string CircleFillBind::meta = "canvascirclefill";

int CircleFillBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);   
  return 1;
}

int CircleFillBind::create(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "self");
  if (err!=0) return err;
  ARGB color = static_cast<ARGB>(luaL_checknumber(L, 2));
  CircleFill* fill = new CircleFill(color);
  LuaHelper::new_shared_userdata<>(L, meta, fill);
  return 1;
}

int CircleFillBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Fill>(L, meta);
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
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 2, "self");
  if (err!=0) return err;
  ARGB color = static_cast<ARGB>(luaL_checknumber(L, 2));
  Fill* fill = new Fill(color);
  LuaHelper::new_shared_userdata<>(L, meta, fill);
  return 1;
}

int FillBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<ui::Fill>(L, meta);
}

// LuaGameController + Bind
void LuaGameController::OnButtonDown(int button) {   
  LuaImport in(L, this, locker(L));
  if (in.open("onbuttondown")) {
    in << button << pcall(0);            
  }
}

void LuaGameController::OnButtonUp(int button) {   
  LuaImport in(L, this, locker(L));
  if (in.open("onbuttonup")) {
    in << button << pcall(0);            
  }
}

void LuaGameController::OnXAxis(int pos, int oldpos) {
  LuaImport in(L, this, locker(L));
  if (in.open("onxaxis")) {
    in << pos << oldpos << pcall(0);            
  }
}

void LuaGameController::OnYAxis(int pos, int oldpos) {
  LuaImport in(L, this, locker(L));
  if (in.open("onyaxis")) {
    in << pos << oldpos << pcall(0);            
  }
}

void LuaGameController::OnZAxis(int pos, int oldpos) {
  LuaImport in(L, this, locker(L));
  if (in.open("onzaxis")) {
    in << pos << oldpos << pcall(0);            
  }
}

std::string LuaGameControllerBind::meta = "gamecontroller";

int LuaGameControllerBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {"xposition", xposition},
    {"yposition", yposition},
    {"xzposition", zposition},
    {"buttons", buttons},  
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);   
  return 1;
}

int LuaGameControllerBind::create(lua_State* L) {
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;  
  LuaGameController* game_controller_info = new LuaGameController(L);
  LuaHelper::new_shared_userdata<>(L, meta, game_controller_info);
  return 1;
}

int LuaGameControllerBind::gc(lua_State* L) {
  return LuaHelper::delete_shared_userdata<LuaGameController>(L, meta);
}

int LuaGameControllerBind::buttons(lua_State* L) {
  boost::shared_ptr<LuaGameController> controller = LuaHelper::check_sptr<LuaGameController>(L, 1, meta);
  lua_pushinteger(L, controller->buttons().to_ulong());
  return 1;
}

// GameControllersBind
std::string LuaGameControllersBind::meta = "gamecontrollerbind";

int LuaGameControllersBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},
    {"controllers", controllers},
    {"update", update},
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);
  return 1;
}

int LuaGameControllersBind::create(lua_State* L) {
  using namespace ui;
  int err = LuaHelper::check_argnum(L, 1, "self");
  if (err!=0) return err;    
  GameControllers<LuaGameController>* controller = 
    new GameControllers<LuaGameController>();  
  LuaHelper::new_shared_userdata<>(L, meta, controller);
  lua_newtable(L);  
  ui::GameControllers<LuaGameController>::iterator it = controller->begin();
  for (; it != controller->end(); ++it) {
    (*it)->set_lua_state(L);
    LuaHelper::requirenew<LuaGameControllerBind>(L, "psycle.ui.gamecontroller",
                                                 (*it).get(), true);
    lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
  }
  lua_setfield(L, -2, "_controllers");
  return 1;
}

int LuaGameControllersBind::gc(lua_State* L) {
  using namespace ui;
  return LuaHelper::delete_shared_userdata<GameControllers<LuaGameController> >(L, meta);
}

int LuaGameControllersBind::controllers(lua_State* L) {
  using namespace ui;
  boost::shared_ptr<LuaGameController> controller = LuaHelper::check_sptr<LuaGameController>(L, 1, meta);
  lua_getfield(L, -1, "_controllers");
  return 1;
}

// LuaScintilla

void LuaScintilla::OnMarginClick(int line_pos) {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onmarginclick")) {
      in << line_pos << pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

std::string LuaLexerBind::meta = "psylexermeta";


void LuaButton::OnClick() {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onclick")) {
      in.pcall(0);
    } 
  } catch (std::exception& e) {
      ui::alert(e.what());   
  }
}

void LuaRadioButton::OnClick() {
	try {
		LuaImport in(L, this, locker(L));
		if (in.open("onclick")) {
			in.pcall(0);
		}
	}
	catch (std::exception& e) {
		ui::alert(e.what());
	}
}

void LuaGroupBox::OnClick() {
	try {
		LuaImport in(L, this, locker(L));
		if (in.open("onclick")) {
			in.pcall(0);
		}
	}
	catch (std::exception& e) {
		ui::alert(e.what());
	}
}

void LuaComboBox::OnSelect() {
  try {
    LuaImport in(L, this, locker(L));
    if (in.open("onselect")) {      
      in.pcall(0);
    } 
  } catch (std::exception& e) {
    ui::alert(e.what());   
  }
}

void LuaScrollBar::OnScroll(int pos) {  
  LuaImport in(L, this, locker(L));
  if (in.open("onscroll")) {
    in << pos << pcall(0);
  }  
}


// LuaRegion+Bind
const char* LuaRegionBind::meta = "psyregionbind";

int LuaRegionBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {
	  {"new", create},
    {"bounds", bounds},
    {"setrect", setrect},		
    {"combine", combine},
    {"offset", offset},    
    {NULL, NULL}
  };
  LuaHelper::open(L, meta, methods, gc);
  LuaHelper::setfield(L, "OR", RGN_OR);
  LuaHelper::setfield(L, "AND", RGN_AND);
  LuaHelper::setfield(L, "XOR", RGN_XOR);
  LuaHelper::setfield(L, "DIFF", RGN_DIFF);
  LuaHelper::setfield(L, "COPY", RGN_COPY);
  return 1;
}

int LuaRegionBind::create(lua_State* L) {
  LuaHelper::new_shared_userdata<>(L, meta, ui::Systems::instance().CreateRegion());  
  return 1;
}

int LuaRegionBind::gc(lua_State* L) {
  LuaHelper::delete_shared_userdata<ui::Region>(L, meta); 
  return 0;
}

// LuaAreaBind
const char* LuaAreaBind::meta = "psyareabind";

int LuaAreaBind::open(lua_State *L) {  
  static const luaL_Reg methods[] = {
		{"new", create},
    {"boundrect", boundrect},
    {"setrect", setrect},
		{"clear", clear},
    {"combine", combine},
    {"offset", offset},    
    {NULL, NULL}
  };
	LuaHelper::open(L, meta, methods, gc);
  LuaHelper::setfield(L, "OR", RGN_OR);
  LuaHelper::setfield(L, "AND", RGN_AND);
  LuaHelper::setfield(L, "XOR", RGN_XOR);
  LuaHelper::setfield(L, "DIFF", RGN_DIFF);
  LuaHelper::setfield(L, "COPY", RGN_COPY);
  return 1;
}

int LuaAreaBind::create(lua_State* L) {
	LuaHelper::new_shared_userdata<>(L, meta, new ui::Area());
	return 1;
}

int LuaAreaBind::gc(lua_State* L) {
	LuaHelper::delete_shared_userdata<ui::Area>(L, meta);
	return 0;
}

void LuaPic::Draw(ui::Graphics* g, ui::Region& draw_rgn) {
  Pic::Draw(g, draw_rgn);
  LuaImport in(L, this, locker(L));
  if (in.open("draw")) {
	LuaHelper::requirenew<LuaGraphicsBind>(L, "psycle.ui.graphics", g, true);
	LuaHelper::requirenew<LuaRegionBind>(L, "psycle.ui.region", &draw_rgn, true);
	in.pcall(0);			
  }	
}

ui::Region* LuaSystems::CreateRegion() { 
  return new ui::Region();
}

ui::Graphics* LuaSystems::CreateGraphics() {
  return new ui::Graphics();
}

ui::Graphics* LuaSystems::CreateGraphics(bool debug) {
  return new ui::Graphics(debug);
}

ui::Graphics* LuaSystems::CreateGraphics(void* dc) {
  return new ui::Graphics(dc);
}

ui::Image* LuaSystems::CreateImage() {
  return new ui::Image();
}

ui::Font* LuaSystems::CreateFont() {
  return new ui::Font();
}

ui::Fonts* LuaSystems::CreateFonts() {
  return new ui::mfc::Fonts();
}

ui::Window* LuaSystems::CreateWin() {
  return new LuaItem(L_);
}

ui::Frame* LuaSystems::CreateFrame() {
  return new LuaFrame(L_);
}

ui::Frame* LuaSystems::CreateMainFrame() {
  return new LuaFrame(L_);
}

ui::PopupFrame* LuaSystems::CreatePopupFrame() {
  return new LuaPopupFrame(L_);
}

ui::ComboBox* LuaSystems::CreateComboBox() {
  return new LuaComboBox(L_);
}

ui::Canvas* LuaSystems::CreateCanvas() {
  return new LuaCanvas(L_);
}

ui::Group* LuaSystems::CreateGroup() {
  return new LuaGroup(L_);
}

ui::RadioButton* LuaSystems::CreateRadioButton() {
  return new LuaRadioButton(L_);
}

ui::GroupBox* LuaSystems::CreateGroupBox() {
  return new LuaGroupBox(L_);
}

ui::RectangleBox* LuaSystems::CreateRectangleBox() {
  return new LuaRectangleBox(L_);
}

ui::HeaderGroup* LuaSystems::CreateHeaderGroup() {
  return new LuaHeaderGroup(L_);
}

ui::Edit* LuaSystems::CreateEdit() {
  return new LuaEdit(L_);
}

ui::Line* LuaSystems::CreateLine() {
  return new LuaLine(L_);
}

ui::Text* LuaSystems::CreateText() {
  return new LuaText(L_);
}

ui::Pic* LuaSystems::CreatePic() {
  return new LuaPic(L_);
}

ui::ScrollBox* LuaSystems::CreateScrollBox() {
  return new LuaScrollBox(L_);
}

ui::Scintilla* LuaSystems::CreateScintilla() {
  return new LuaScintilla(L_);
}

ui::Button* LuaSystems::CreateButton() {
  return new LuaButton(L_);
}

ui::CheckBox* LuaSystems::CreateCheckBox() {
  // return new LuaCheckbox(L_);
  assert(0);
  return 0;
}

ui::ScrollBar* LuaSystems::CreateScrollBar(ui::Orientation::Type orientation) {
  return new LuaScrollBar(L_, orientation);
}

ui::TreeView* LuaSystems::CreateTreeView() {
  return new LuaTreeView(L_);  
}

ui::ListView* LuaSystems::CreateListView() {
  return new LuaListView(L_);
}

ui::MenuContainer* LuaSystems::CreateMenuBar() {
  return new LuaMenuBar(L_);
}

ui::PopupMenu* LuaSystems::CreatePopupMenu() {
  return new LuaPopupMenu(L_);
}


} // namespace host
} // namespace psycle