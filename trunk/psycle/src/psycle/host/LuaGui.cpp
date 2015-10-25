// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "LuaGui.hpp"

#include "PsycleConfig.hpp"
#include "LuaHelper.hpp"
#include "LuaHost.hpp"
#include "LuaPlugin.hpp"

#include <lua.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>


namespace psycle { namespace host {

  ///////////////////////////////////////////////////////////////////////////////
  // Menu + Binds
  ///////////////////////////////////////////////////////////////////////////////


  void LuaMenuBar::append(LuaMenu* menu) {
    int pos = menu->cmenu()->GetMenuItemCount();
    std::vector<LuaMenu*>::iterator it = items.begin();
    for ( ; it != items.end(); ++it) {
      LuaMenu* m = *it;
      menu->cmenu()->AppendMenu(MF_POPUP, (UINT_PTR)m->cmenu()->m_hMenu, m->label().c_str());
      m->set_parent(menu);
      m->set_pos(pos++); 
    }
    menu->setbar(this);
  }

  void LuaMenuBar::remove(CMenu* menu, int pos) {
     std::vector<LuaMenu*>::iterator it = items.begin();
     for ( ; it != items.end(); ++it) {          
       menu->RemoveMenu(pos++, MF_BYPOSITION);
     } 
  }

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
    LuaMenuBar* menubar = new LuaMenuBar();            
    LuaHelper::new_userdata<>(L, meta, menubar);       
    return 1;
  }

  int LuaMenuBarBind::add(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, menu");
    if (err!=0) return err;    
    LuaMenuBar* menubar = LuaHelper::check<LuaMenuBar>(L, 1, meta);
    LuaMenu* menu = LuaHelper::check<LuaMenu>(L, 2, LuaMenuBind::meta);
    menubar->add(menu);
    LuaHelper::register_userdata<>(L, menu);
    return LuaHelper::chaining(L);
  }
  
  int LuaMenuBarBind::gc(lua_State* L) {    
    return LuaHelper::delete_userdata<LuaMenuBar>(L, meta);
  }


  void LuaMenu::set_label(const std::string& label) {
    label_ = label; 
    if (parent()) {
      parent()->cmenu()->ModifyMenu(pos_, MF_BYPOSITION, 0, label.c_str());
      LuaMenuBar* b = bar();     
      b->setupdate(b!=0);
    }
  }
      
  void LuaMenu::add(LuaMenu* newmenu) {           
    cmenu_->AppendMenu(MF_POPUP | MF_ENABLED, (UINT_PTR)newmenu->cmenu()->m_hMenu, newmenu->label().c_str());
    newmenu->set_parent(this);
    newmenu->set_pos(cmenu_->GetMenuItemCount()-1);
  }
  
  void LuaMenu::add(LuaMenuItem* item) {
    items.push_back(item);
    item->set_menu(this);
    const int id = ID_DYNAMIC_MENUS_START+item->id_counter;
    item->set_id(id);     
    LuaMenuItem::menuItemIdMap[item->id()] = item;
    cmenu_->AppendMenu(MF_STRING, id, item->label().c_str());
    if (item->checked()) {
		  cmenu_->CheckMenuItem(id, MF_CHECKED | MF_BYCOMMAND);
    }      
  }
  
  void LuaMenu::addseparator() {         
    cmenu_->AppendMenu(MF_SEPARATOR, 0, "-");
  }

  void LuaMenu::remove(LuaMenuItem* item) {
    std::vector<LuaMenuItem*>::iterator it;     
    it = std::find(items.begin(), items.end(), item);
    if (it != items.end()) {
      items.erase(it);
    }   
    cmenu_->RemoveMenu(item->id(), MF_BYCOMMAND);
  }

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
    LuaMenu* menu = new LuaMenu();    
    menu->cmenu()->CreatePopupMenu();    
    const char* label = luaL_checkstring(L, 2);
    menu->set_label(label);
    LuaHelper::new_userdata<>(L, meta, menu);    
    return 1;
  }

  int LuaMenuBind::remove(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 2, "self, menu or menuitem");
    if (err!=0) return err;
    LuaMenu* menu = LuaHelper::check<LuaMenu>(L, 1, meta);
    LuaMenuItem* item = 0;    
    item = LuaHelper::test<LuaMenuItem>(L, 2, LuaMenuItemBind::meta);
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
      LuaHelper::unregister_userdata<>(L, item);        
    }    
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::add(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, menu or menuitem");
    if (err!=0) return err;
    LuaMenu* menu = LuaHelper::check<LuaMenu>(L, 1, meta);
    LuaMenuItem* item = 0;    
    item = LuaHelper::test<LuaMenuItem>(L, 2, LuaMenuItemBind::meta);
    if (!item) {
      LuaMenu* new_menu= LuaHelper::test<LuaMenu>(L, 2, LuaMenuBind::meta);
      if (new_menu) {    
        menu->add(new_menu);
        LuaHelper::register_userdata<>(L, new_menu);
      } else {
        return luaL_error(L, "Argument is no menu or menuitem.");
      }
    } else {               
      menu->add(item);           
      LuaHelper::register_userdata<>(L, item);
    }    
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::addseparator(lua_State* L) {    
    LuaHelper::call<LuaMenu>(L, meta, &LuaMenu::addseparator);
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::setlabel(lua_State* L) {
    LuaHelper::callstrictstr<LuaMenu>(L, meta, &LuaMenu::set_label);    
    return LuaHelper::chaining(L);
  }

  int LuaMenuBind::gc(lua_State* L) {    
    return LuaHelper::delete_userdata<LuaMenu>(L, meta);
  }
  
  void LuaMenuItem::set_label(const std::string& label) { 
    label_ = label; 
    if (menu_) {
       menu_->cmenu()->ModifyMenu(id(), MF_BYCOMMAND, id(), label.c_str());
    }
  }  

  void LuaMenuItem::check() {
    check_ = true;
    if (menu_) {
      menu_->cmenu()->CheckMenuItem(id(), MF_CHECKED | MF_BYCOMMAND);
    }
  }
  
  void LuaMenuItem::uncheck() { 
    check_ = false;
    if (menu_) {
       menu_->cmenu()->CheckMenuItem(id(), MF_UNCHECKED | MF_BYCOMMAND);
    }
  }
  
  const char* LuaMenuItemBind::meta = "psymenuitemmeta";
  int LuaMenuItem::id_counter = 0;
  std::map<std::uint16_t, LuaMenuItem*> LuaMenuItem::menuItemIdMap;

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
    LuaMenuItem* item = new LuaMenuItem();
    item->set_id(item->id_counter);
    item->id_counter++;
    item->set_label(label);
    LuaHelper::new_userdata<>(L, meta, item);
    lua_newtable(L);
    lua_setfield(L, -2, "listener_");    
    return 1;
  }

  int LuaMenuItemBind::gc(lua_State* L) {	
    return LuaHelper::delete_userdata<LuaMenuItem>(L, meta);
  }

  int LuaMenuItemBind::setlabel(lua_State* L) {
    LuaHelper::callstrictstr<LuaMenuItem>(L, meta, &LuaMenuItem::set_label);    
    return LuaHelper::chaining(L);
  }

  int LuaMenuItemBind::label(lua_State* L) {
    return LuaHelper::getstring<LuaMenuItem>(L, meta, &LuaMenuItem::label);
  }

  int LuaMenuItemBind::checked(lua_State* L) {	
    return LuaHelper::getbool<LuaMenuItem>(L, meta, &LuaMenuItem::checked);
  }

  int LuaMenuItemBind::id(lua_State* L) {	
    return LuaHelper::getnumber<LuaMenuItem, int>(L, meta, &LuaMenuItem::id);
  }

  int LuaMenuItemBind::check(lua_State* L) {	
    LuaHelper::call<LuaMenuItem>(L, meta, &LuaMenuItem::check);
    return LuaHelper::chaining(L);
  }

  int LuaMenuItemBind::uncheck(lua_State* L) {	
    LuaHelper::call<LuaMenuItem>(L, meta, &LuaMenuItem::uncheck);    
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
  // LuaDialogBind
  /////////////////////////////////////////////////////////////////////////////
  const char* LuaDialogBind::meta = "psydialogmeta";

  template<class T>
  bool CallDialogEvents(lua_State* L, T* that,WPARAM wParam, LPARAM lParam) {         
    int n1 = lua_gettop(L);                  
    LuaHelper::find_weakuserdata<>(L, that);    
    bool has_event_method = false;
    bool is_key = false;
    if (!lua_isnil(L, -1)) { 
      lua_getfield(L, -1, "onevent");               
      has_event_method = !lua_isnil(L, -1);
      if (has_event_method) {
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_newtable(L); // build event table
        WORD wControlID = LOWORD(wParam);
	    //WORD wMessageID = HIWORD(wParam);
        lua_pushnumber(L, wControlID);
        lua_setfield(L, -2, "id");
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


  BOOL LuaDialog::OnCommand(WPARAM wParam, LPARAM lParam) {
	  //wParam
	  //The low-order word of wParam identifies the command ID of the menu item, control, or accelerator.
	  //The high-order word of wParam specifies the notification message if the message is from a control.
	  //If the message is from an accelerator, the high-order word is 1.
	  //If the message is from a menu, the high-order word is 0.

	  //lParam
	  //Identifies the control that sends the message if the message is from a control. Otherwise, lParam is 0.	  
    mac->lock();
    CallDialogEvents<>(L, this, wParam, lParam); 
    mac->unlock();
	  return CDialog::OnCommand(wParam, lParam);
  }

  int LuaDialogBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},                  
      {"addedit", addedit},
      {"value", editvalue},
      {"addbutton", addbutton},
      {"addtext", addtext},
      {"settext", settext},
      {"show", show},
      {"hide", hide},      
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods,  gc);    
  }

  int LuaDialogBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    //if (n != 2) {
    //      return luaL_error(L, "Got %d arguments expected (self, name)", n); 
    //}    
    const char* name = luaL_checkstring(L, 2);
    int width = n > 2 ? luaL_checknumber(L, 3) : 200;
    int height = n > 3 ? luaL_checknumber(L, 4) : 100;
    int x = n > 4 ? luaL_checknumber(L, 5) : 0;
    int y = n > 5 ? luaL_checknumber(L, 6) : 0;
    LuaDialog* dlg = new LuaDialog(L, name, width, height, x, y);
    //dlg->Set_dialog_style(
      //WS_CAPTION | WS_VISIBLE | WS_DLGFRAME | WS_POPUP |
      //DS_SETFONT | WS_SYSMENU);    
    // dlg->ShowWindow(SW_SHOW);

    LuaHelper::get_proxy(L);
    int n2 = lua_gettop(L);
    LuaMachine* mac = LuaHelper::check<LuaMachine>(L, n2, LuaMachineBind::meta);
    dlg->mac = mac;
    LuaHelper::new_userdata<>(L, meta, dlg);   
    LuaHelper::register_weakuserdata<>(L, dlg);
    return 1;
  }

  int LuaDialogBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<Dynamic_dialog>(L, meta);
  }

  int LuaDialogBind::addedit(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     const char* name = n > 1 ? luaL_checkstring(L, 2) : "";     
     int left = n > 2 ? luaL_checknumber(L, 3) : 0;
     int top = n > 3 ? luaL_checknumber(L, 4) : 0;
     int width = n > 4 ? luaL_checknumber(L, 5) : 100;
     int height = n > 5 ? luaL_checknumber(L, 6) : 20;
     int max_len = n > 6 ? luaL_checknumber(L, 7) : 20;
     int id = dlg->Add_string_edit_control(name, left, top, width, height, max_len, 0); 
     lua_pushnumber(L, id);
     return 1;     
  }

  int LuaDialogBind::editvalue(lua_State* L) {
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     dlg->UpdateData();
     int id = luaL_checknumber(L, 2);
     CString* str = dlg->get_edit_string(id);
     if (str) {
       CT2A ascii(*str);
       lua_pushstring(L, ascii);
     } else {
       lua_pushnil(L);
     }
     return 1;
  }
  
  int LuaDialogBind::settext(lua_State* L) {    
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     int id = luaL_checknumber(L, 2);
     const char* newstr = luaL_checkstring(L, 3);
     dlg->set_text(id, newstr);     
     return 0;
  }

  int LuaDialogBind::addbutton(lua_State* L) {
     int n = lua_gettop(L);  // Number of arguments
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     const char* name = luaL_checkstring(L, 2);
     int left = n > 2 ? luaL_checknumber(L, 3) : 0;
     int top = n > 3 ? luaL_checknumber(L, 4) : 0;
     int width = n > 4 ? luaL_checknumber(L, 5) : 100;
     int height = n > 5 ? luaL_checknumber(L, 6) : 20;
     int id = dlg->Add_pushbutton(name, dlg->Assign_id(0),  left, top, width, height);
     //int id = dlg->Add_OK_button(left, top, width, height);
     lua_pushnumber(L, id);
     return 1;
  }

  int LuaDialogBind::addtext(lua_State* L) {
     int n = lua_gettop(L);  // Number of arguments
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     const char* name = luaL_checkstring(L, 2);
     int left = n > 2 ? luaL_checknumber(L, 3) : 0;
     int top = n > 3 ? luaL_checknumber(L, 4) : 0;
     int width = n > 4 ? luaL_checknumber(L, 5) : 100;
     int height = n > 5 ? luaL_checknumber(L, 6) : 20;
     int id = dlg->Add_static_text(left, top, width, height, name);
     lua_pushnumber(L, id);
     return 1;
  }

  int LuaDialogBind::show(lua_State* L) {
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     BOOL ret = dlg->DoModal();
     dlg->UpdateData();
     dlg->ShowWindow(SW_SHOW);     
     return 0;
  }

  int LuaDialogBind::hide(lua_State* L) {
     Dynamic_dialog* dlg = LuaHelper::check<Dynamic_dialog>(L, 1, meta);
     dlg->ShowWindow(SW_HIDE);
     dlg->UpdateWindow();
     return 0;
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
        default: return true;
      }        
      has_event_method = !lua_isnil(L, -1);
      if (has_event_method) {
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_newtable(L); // build event table           
        lua_pushnumber(L, ev->shift());
        lua_setfield(L, -2, "shift");
        lua_pushboolean(L, MK_SHIFT & ev->shift());
        lua_setfield(L, -2, "shiftkey");
        lua_pushboolean(L, MK_CONTROL & ev->shift());
        lua_setfield(L, -2, "ctrlkey");
        lua_pushboolean(L, MK_ALT & ev->shift());
        lua_setfield(L, -2, "altkey");
        if (!is_key) {
          lua_pushnumber(L, ev->x());
          lua_setfield(L, -2, "clientx");
          lua_pushnumber(L, ev->y());          
          lua_setfield(L, -2, "clienty");
          lua_pushnumber(L, ev->x()-that->zoomabsx());
          lua_setfield(L, -2, "x");
          lua_pushnumber(L, ev->y()-that->zoomabsy());
          lua_setfield(L, -2, "y");
          lua_pushnumber(L, ev->button());
          lua_setfield(L, -2, "button");        
        } else {
          // todo key events
          lua_pushnumber(L, ev->button());
          lua_setfield(L, -2, "keycode");
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

  /////////////////////////////////////////////////////////////////////////////
  // LuaCanvas+Bind
  /////////////////////////////////////////////////////////////////////////////

  const char* LuaCanvasBind::meta = "psycanvasmeta";

  canvas::Item* LuaCanvas::OnEvent(canvas::Event* ev) {    
    canvas::Item* item = canvas::Canvas::OnEvent(ev);
    CallEvents<>(L, ev, this, true);    
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
      {"showcursor", showcursor},
      {"hidecursor", hidecursor},
      {"setcursorpos", setcursorpos},
      {"showscrollbar", showscrollbar},
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods,  gc);    
  }

  int LuaCanvasBind::create(lua_State* L) {	
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;     
    canvas::Canvas* canvas = new LuaCanvas(L);     
    canvas->SetSave(true);
    LuaHelper::new_userdata<>(L, meta, canvas);
    luaL_requiref(L, "psycle.ui.canvas.group", LuaGroupBind::open, 0);
    LuaHelper::new_userdata<>(L, LuaGroupBind::meta, canvas->root(), 3);    
    LuaHelper::register_userdata<>(L, canvas->root());
    lua_pushvalue(L, 2);
    LuaHelper::register_weakuserdata<>(L, canvas);
    return 1;
  }

  int LuaCanvasBind::gc(lua_State* L) {
    LuaCanvas** canvas = (LuaCanvas**) luaL_checkudata(L, -1, meta);            
    LuaHelper::unregister_userdata<>(L, (*canvas)->root());     
    return LuaHelper::delete_userdata<LuaCanvas>(L, meta);
  }

  int LuaCanvasBind::root(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err; 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);    
    LuaHelper::find_userdata<>(L, canvas->root());    
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
    return LuaHelper::getnumber<LuaCanvas,canvas::ARGB>(L, meta, &LuaCanvas::color);    
  }

  int LuaCanvasBind::setpreferredsize(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::Canvas::setpreferredsize);
    return LuaHelper::chaining(L);    
  }

  int LuaCanvasBind::setcapture(lua_State* L) {    
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->SetCapture();
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::releasecapture(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->ReleaseCapture();		
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::showcursor(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->ShowCursor();
    return LuaHelper::chaining(L);
  }

  int LuaCanvasBind::hidecursor(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->HideCursor();
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

  ///////////////////////////////////////////////////////////////////////////////
  // LuaGroupBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaGroupBind::meta = "psygroupmeta";

  bool LuaGroup::OnEvent(canvas::Event* ev) {    
    return CallEvents<>(L, ev, this);
  }

  int LuaGroupBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"setpos", setpos},
      {"pos", pos},
      {"clientpos", clientpos},
      {"getfocus", getfocus},
      {"setzoom", setzoom},
      {"items", getitems},      
      {"remove", remove},
      {"removeall", removeall},
      {"add", add},
      {"show", show},
      {"hide", hide},
      {"enablepointerevents", enablepointerevents},
      {"disablepointerevents", disablepointerevents},
      {"tostring", tostring},
      {"parent", parent},
      {"setzorder", setzorder},
      {"zorder", zorder},
      {"bounds", bounds},      
      {"canvas", canvas},
      {"intersect", intersect},
      {"intersectrect", intersectrect},
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaGroupBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments         
    LuaGroup* newgroup = 0;
    canvas::Group* parent = 0;
    if (n==1 || (n==2 && lua_isnil(L, 2))) {
      newgroup = new LuaGroup(L);
    } else if (n==2) {     
      parent = LuaHelper::check<canvas::Group>(L, 2, meta);
      newgroup = new LuaGroup(L, parent);      
    }    
    LuaHelper::new_userdata<>(L, meta, newgroup);
    if (parent) {
       LuaHelper::register_userdata<>(L, newgroup);
    }
    return 1;
  }

  int LuaGroupBind::canvas(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;  
    canvas::Group* group = LuaHelper::check<canvas::Group>(L, 1, meta);
    canvas::Canvas* canvas = group->canvas();    
    LuaHelper::find_weakuserdata(L, canvas);
    return 1;
  }

  canvas::Item* LuaGroupBind::test(lua_State* L, int index) {
    canvas::Item* item = 0;
    item = LuaHelper::test<LuaRect>(L, index, LuaRectBind::meta);
    if (!item) {
      item = LuaHelper::test<LuaGroup>(L, index, LuaGroupBind::meta);
      if (!item) {
        item = LuaHelper::test<LuaText>(L, index, LuaTextBind::meta);
        //if (!item) {
          //item = LuaHelper::test<canvas::PixBuf>(L, index, LuaPixBind::meta);
          if (!item) {
            item = LuaHelper::test<canvas::Line>(L, index, LuaLineBind::meta);
          }
        //}
      }
    }
    return item;
  }
  
  int LuaGroupBind::removeall(lua_State* L) {
    LuaGroup* group = LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::RemoveAll);
    LuaGroup::iterator it = group->begin();
    for ( ; it != group->end(); ++it) {
      canvas::Item* item = *it;
      LuaHelper::unregister_userdata<>(L, item);
      lua_pop(L, 2);
    }
    return LuaHelper::chaining(L);    
  }

  int LuaGroupBind::remove(lua_State* L) {
    try {
      canvas::Group* group = LuaHelper::check<canvas::Group>(L, 1, meta);
      canvas::Item* item = test(L, 2);
      if (item) {
        group->Remove(item);
        LuaHelper::unregister_userdata<>(L, item);
      } else {
        luaL_error(L, "Argument is no canvas item.");
      }
    } catch(std::exception &e) {
      luaL_error(L, e.what());
    }    
    return LuaHelper::chaining(L);
  }

  int LuaGroupBind::add(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;       
    canvas::Group* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    canvas::Item* item = test(L, 2);    
    if (item) {
      try {                      
        group->Add(item);
        LuaHelper::register_userdata<>(L, item);
       } catch(std::exception &e) {
        luaL_error(L, e.what());
      }
    } else {
      luaL_error(L, "Argument is no canvas item.");
    }
    return LuaHelper::chaining(L);
  }

  int LuaGroupBind::show(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::show);
    return LuaHelper::chaining(L);    
  }

  int LuaGroupBind::hide(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::hide);
    return LuaHelper::chaining(L); 
  }

  int LuaGroupBind::enablepointerevents(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::enablepointerevents);
    return LuaHelper::chaining(L); 
  }

  int LuaGroupBind::disablepointerevents(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::disablepointerevents);
    return LuaHelper::chaining(L);     
  }

  int LuaGroupBind::setpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::Group::SetXY);    
    return LuaHelper::chaining(L); 
  }

  int LuaGroupBind::pos(lua_State* L) {    
    return LuaHelper::get2number2<LuaGroup,double>(L, meta, &LuaGroup::pos);    
  }

  int LuaGroupBind::setzoom(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Group::setzoom);    
    return LuaHelper::chaining(L); 
  }

  int LuaGroupBind::bounds(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;     
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    double x1, y1, x2, y2;
    group->GetBounds(x1, y1, x2, y2);
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);
    lua_pushnumber(L, x2);
    lua_pushnumber(L, y2);
    return 4;
  }

  int LuaGroupBind::clientpos(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;    
    canvas::Group* item = LuaHelper::check<canvas::Group>(L, 1, meta);
    double x1, y1;
    x1 = item->zoomabsx();
    y1 = item->zoomabsy();
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);    
    return 2;
  }

  int LuaGroupBind::intersect(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 3, "self, self, x1, y1 [,x2, y2]");
    if (err!=0) return err;
    canvas::Group* item = LuaHelper::check<canvas::Group>(L, 1, meta);    
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

  int LuaGroupBind::intersectrect(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1 ,x2, y2");
    if (err!=0) return err;
    canvas::Group* item = LuaHelper::check<canvas::Group>(L, 1, meta);    
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    std::vector<canvas::Item*> res;
    item->intersect(res, x1, y1, x2, y2);
    lua_newtable(L);    
    for (int i = 0; i < res.size(); ++i) {
      LuaHelper::find_userdata(L, res[i]);
      lua_rawseti(L, -2, i+1);      
    } 
    return 1;
  }

  int LuaGroupBind::getitems(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err; 
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);    
    lua_newtable(L); // create item table
    LuaGroup::iterator it = group->begin();
    for (; it!=group->end(); ++it) {
      LuaHelper::find_userdata<>(L, *it);      
      lua_rawseti(L, 2, lua_rawlen(L, 2)+1); // items[#items+1] = newitem      
    }
    lua_pushvalue(L, 2);
    return 1;
  }
  
  int LuaGroupBind::gc(lua_State* L) {
    LuaGroup** group = (LuaGroup**) luaL_checkudata(L, -1, meta);
    LuaGroup::iterator it = (*group)->begin();
    for ( ; it != (*group)->end(); ++it) {      
      LuaHelper::unregister_userdata<>(L, *it);      
    }    
    return LuaHelper::delete_userdata<LuaGroup>(L, meta);   
  }

  int LuaGroupBind::getfocus(lua_State* L) {
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    group->GetFocus();
    return LuaHelper::chaining(L);
  }

  int LuaGroupBind::parent(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err; 
    canvas::Group* g = LuaHelper::check<canvas::Group>(L, 1, meta);        
    canvas::Group* group = g->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  int LuaGroupBind::tostring(lua_State* L) {
    LuaGroup* rect = LuaHelper::check<LuaGroup>(L, 1, meta);
    lua_pushstring(L, "group");
    return 1;
  }

  int LuaGroupBind::setzorder(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Item::set_zorder);    
    return LuaHelper::chaining(L);
  }

  int LuaGroupBind::zorder(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);
    if (item->parent()) {
      int z  = item->parent()->zorder(item);
      lua_pushnumber(L, z);
    } else {
      lua_pushnumber(L, -1);
    }    
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaRectBind
  ///////////////////////////////////////////////////////////////////////////////

  bool LuaRect::OnEvent(canvas::Event* ev) {    
    return CallEvents<>(L, ev, this);
  }

  const char* LuaRectBind::meta = "psyrectmeta";

  int LuaRectBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"setcolor", setcolor},      
      {"color", color},
      {"setstrokecolor", setstrokecolor},      
      {"strokecolor", color},            
      {"setpos", setpos},
      {"pos", pos},
      {"clientpos", clientpos},
      {"parent", parent},      
      {"tostring", tostring},  
      {"setzorder", setzorder},
      {"zorder", zorder},
      {"setborder", setborder},
      {"border", border},      
      {"getfocus", getfocus},
      {NULL, NULL}
    };
   return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaRectBind::create(lua_State* L) {    
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2 && n!=1) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }  
    LuaRect* rect = 0;
    LuaGroup* group = 0;
    if (n==2) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }
    rect = new LuaRect(L, group, 10, 10, 100, 100);
    rect->SetFillColor(0x0C0C0C);    
    LuaHelper::new_userdata<>(L, meta, rect);        
    LuaHelper::register_userdata<>(L, rect); 
    return 1;
  }

  int LuaRectBind::setpos(lua_State* L) {  
    const int n = lua_gettop(L);
    if (n==3) {
      canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
      rect->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));    
      return LuaHelper::chaining(L); 
    } else
    if (n==5) {
      LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
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

  int LuaRectBind::setborder(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 3, "self, x1, y1");
    if (err!=0) return err;
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);    
    rect->SetBorder(x1, y1);
    return LuaHelper::chaining(L); 
  }

  int LuaRectBind::border(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    double x1, y1;
    rect->border(x1, y1);
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);    
    return 2;
  }
  
  int LuaRectBind::pos(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    lua_pushnumber(L, rect->x());
    lua_pushnumber(L, rect->y());
    lua_pushnumber(L, rect->width());
    lua_pushnumber(L, rect->height());
    return 4;
  }

  int LuaRectBind::clientpos(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err; 
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    double x1, y1;
    x1 = rect->zoomabsx();
    y1 = rect->zoomabsy();    
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);    
    lua_pushnumber(L, rect->width());
    lua_pushnumber(L, rect->height()); 
    return 4;
  }

  int LuaRectBind::setzorder(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 2, "self, zorder");
    if (err!=0) return err;
    int zorder = luaL_checknumber(L, 2);
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    if (rect->parent()) {
      rect->parent()->set_zorder(rect, zorder);
    }
    return LuaHelper::chaining(L); 
  }

  int LuaRectBind::zorder(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err; 
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    if (rect->parent()) {
      int z  = rect->parent()->zorder(rect);
      lua_pushnumber(L, z);
    } else {
      lua_pushnumber(L, -1);
    }    
    return 1;
  }
 
  int LuaRectBind::parent(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);        
    canvas::Group* group = rect->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  int LuaRectBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<LuaRect>(L, meta);
  }

  int LuaRectBind::setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Rect::SetFillColor);
    return LuaHelper::chaining(L);      
  }
  
  int LuaRectBind::color(lua_State* L) {
    return LuaHelper::getnumber<LuaRect, canvas::ARGB>(L, meta, &LuaRect::fillcolor);    
  }

  int LuaRectBind::setstrokecolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Rect::SetStrokeColor);
    return LuaHelper::chaining(L);      
  }
  
  int LuaRectBind::strokecolor(lua_State* L) {
    return LuaHelper::getnumber<LuaRect, canvas::ARGB>(L, meta, &LuaRect::strokecolor);    
  }
  
  int LuaRectBind::tostring(lua_State* L) {
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    lua_pushstring(L, "rect");
    return 1;
  }

  int LuaRectBind::getfocus(lua_State* L) {
    LuaRect* item = LuaHelper::check<LuaRect>(L, 1, meta);
    item->GetFocus();
    return LuaHelper::chaining(L);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaLineBind
  ///////////////////////////////////////////////////////////////////////////////

  bool LuaLine::OnEvent(canvas::Event* ev) {    
    return CallEvents<>(L, ev, this);
  }

  const char* LuaLineBind::meta = "psylinemeta";

  int LuaLineBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"parent", parent},
      {"setcolor", setcolor},      
      {"color", color},
      {"setpoints", setpoints},
      {"points", points},
      {"setpoint", setpoint},
      {"setpos", setpos},
      {"pos", pos},
      {"enablepointerevents", enablepointerevents},
      {"disablepointerevents", disablepointerevents},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaLineBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2 && n!=1) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaGroup* group = 0;
    if (n==2) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }    
    LuaLine* line = new LuaLine(L, group);    
    canvas::Line::Points pts;
    pts.push_back(std::pair<double, double>(10, 10));
    pts.push_back(std::pair<double, double>(300, 300));    
    line->SetPoints(pts);
    line->SetColor(0x0C0C0C);    
    LuaHelper::new_userdata<>(L, meta, line);
    LuaHelper::register_userdata<>(L, line);
    return 1;
  }

  int LuaLineBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<LuaLine>(L, meta);
  }

  int LuaLineBind::setpoints(lua_State* L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);
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

  int LuaLineBind::enablepointerevents(lua_State* L) {
    LuaHelper::call<LuaLine>(L, meta, &LuaLine::enablepointerevents);
    return LuaHelper::chaining(L); 
  }

  int LuaLineBind::disablepointerevents(lua_State* L) {
    LuaHelper::call<LuaLine>(L, meta, &LuaLine::disablepointerevents);
    return LuaHelper::chaining(L);     
  }
  
  int LuaLineBind::setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Line::SetColor);
    return LuaHelper::chaining(L);      
  }
  
  int LuaLineBind::color(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    LuaLine* item = LuaHelper::check<LuaLine>(L, 1, meta);
    canvas::ARGB color = item->color();    
    lua_pushnumber(L, color);       
    return 1;
  }

  int LuaLineBind::setpoint(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 4, "self, idx, x, y");
    if (err!=0) return err;
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);
    double idx = luaL_checknumber(L, 2);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);    
    canvas::Line::Point pt(x, y);
    line->SetPoint(idx-1,  pt);
    return LuaHelper::chaining(L); 
  }
  
  int LuaLineBind::parent(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;    
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);        
    canvas::Group* group = item->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  int LuaLineBind::setpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::Line::SetXY);    
    return LuaHelper::chaining(L);    
  }

  int LuaLineBind::pos(lua_State* L) {    
    return LuaHelper::get2number2<LuaLine,double>(L, meta, &LuaLine::pos);
  }

  int LuaLineBind::points(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);
    LuaLine::Points pts = line->points();
    lua_newtable(L);
    LuaLine::Points::iterator it = pts.begin();
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

  ///////////////////////////////////////////////////////////////////////////////
  // LuaTextBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaTextBind::meta = "psytextmeta";

  bool LuaText::OnEvent(canvas::Event* ev) {    
    return CallEvents<>(L, ev, this);
  }

  int LuaTextBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"settext", settext},
      {"text", text},
      {"setcolor", setcolor},      
      {"color", color},
      {"setpos", setpos},
      {"pos", pos},
      {"tostring", tostring},
      {"parent", parent},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }   

  int LuaTextBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    if (n!=1 && n!=2) {
      return luaL_error(L, "Too many arguments.", n); 
    }
    LuaGroup* group = 0;
    if ((n==2 && !lua_isnil(L, 2))) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }     
    canvas::Text* text = new LuaText(L, group);    
    text->SetColor(0x0C0C0C);    
    LuaHelper::new_userdata<>(L, meta, text);
    if (group) {
      LuaHelper::register_userdata<>(L, text);
    }
    return 1;
  }

  int LuaTextBind::setpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::Text::SetXY);    
    return LuaHelper::chaining(L);    
  }

  int LuaTextBind::pos(lua_State* L) {    
    return LuaHelper::get2number2<LuaText,double>(L, meta, &LuaText::pos);
  }

  int LuaTextBind::settext(lua_State* L) {
    LuaHelper::callstrictstr<canvas::Text>(L, meta, &canvas::Text::SetText);    
    return LuaHelper::chaining(L);
  }

  int LuaTextBind::text(lua_State* L) {
    return LuaHelper::getstring<canvas::Text>(L, meta, &canvas::Text::text);    
  }

  int LuaTextBind::setcolor(lua_State* L) {
    LuaHelper::callstrict1(L, meta, &canvas::Text::SetColor);
    return LuaHelper::chaining(L);      
  }

  int LuaTextBind::color(lua_State* L) {    
    return LuaHelper::getnumber<LuaText, canvas::ARGB>(L, meta, &LuaText::color);
  }

  int LuaTextBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<LuaText>(L, meta);
  }

  int LuaTextBind::tostring(lua_State* L) {
    LuaText* txt = LuaHelper::check<LuaText>(L, 1, meta);
    lua_pushstring(L, "text");
    return 1;
  }

  int LuaTextBind::parent(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    canvas::Text* txt = LuaHelper::check<canvas::Text>(L, 1, meta);        
    canvas::Group* group = txt->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaPixBind
  ///////////////////////////////////////////////////////////////////////////////

  /*
  const char* LuaPixBind::meta = "psypixmeta";

  int LuaPixBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},            
      {"pos", pos},
      {"setpos", setpos},
      {"setsource", setsource},
      {"setsize", setsize},
      {"setpix", setpixmap},
      {"settransparent", settransparent},
      {"load", load},
      {"tostring", tostring},
      {"parent", parent},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaPixBind::create(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1 and n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaGroup* group = 0;
    if (n==2) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }
    canvas::PixBuf* pix = new canvas::PixBuf(group);    
    LuaHelper::new_userdata<>(L, meta, pix);
    LuaHelper::register_userdata<>(L, pix);
    return 1;
  }
  
  int LuaPixBind::setsource(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::PixBuf::SetSource);    
    return LuaHelper::chaining(L);    
  }

  int LuaPixBind::settransparent(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 4, "self, r, g, b");
    if (err!=0) return err;
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);    
    pix->SetTransparent(true, r, g, b);
    return LuaHelper::chaining(L); 
  }

  int LuaPixBind::setsize(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &canvas::PixBuf::SetSize);    
    return LuaHelper::chaining(L); 
  }

  int LuaPixBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<canvas::PixBuf>(L, meta);
  }

  int LuaPixBind::setpixmap(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, skin");
    if (err!=0) return err;
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);        
    int skin = luaL_checknumber(L, 2);
    pix->SetImage(skin);    
    return LuaHelper::chaining(L); 
  }

  int LuaPixBind::load(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 2, "self, filename");
    if (err!=0) return err;
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);   
    const char* str = luaL_checkstring(L, 2);
    pix->Load(str);    
    return LuaHelper::chaining(L); 
  }

  int LuaPixBind::pos(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    lua_pushnumber(L, pix->x());
    lua_pushnumber(L, pix->y());
    lua_pushnumber(L, pix->x()+pix->width());
    lua_pushnumber(L, pix->y()+pix->height());
    return 4;
  }

  int LuaPixBind::setpos(lua_State* L) {        
    const int n = lua_gettop(L);
    if (n==3) {
      canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
      LuaHelper::callstrict2(L, meta, &canvas::PixBuf::SetXY);
    } else
    if (n==5) {
      canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
      double x1 = luaL_checknumber(L, 2);
      double y1 = luaL_checknumber(L, 3);
      double x2 = luaL_checknumber(L, 4);
      double y2 = luaL_checknumber(L, 5);                 
      pix->SetXY(x1, y1);
      pix->SetSize(x2-x1, y2-y1);
    } else {
       return luaL_error(L, "Wrong number of arguments.");
    }
    return LuaHelper::chaining(L);
  }

  int LuaPixBind::tostring(lua_State* L) {
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);
    lua_pushstring(L, "pix");
    return 1;
  }

  int LuaPixBind::parent(lua_State* L) {    
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;   
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);        
    canvas::Group* group = item->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }
  */

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
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);    
    gr->Translate(x, y);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::setcolor(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 2, "self, color");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);    
    canvas::ARGB color = static_cast<canvas::ARGB>(luaL_checknumber(L, 2));
    gr->SetColor(color);
    return LuaHelper::chaining(L);    
  }

  int LuaGraphicsBind::color(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);    
    lua_pushnumber(L, gr->color());    
    return 1;
  }

  int LuaGraphicsBind::drawline(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, x2, y2");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    gr->DrawLine(x1, y1, x2, y2);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawstring(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 4, "self, str, x, y");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    const char* str = luaL_checkstring(L, 2);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);    
    gr->DrawString(str, x, y);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawrect(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, x2, y2");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    gr->DrawRect(x1, y1, x2, y2);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawoval(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, width, height");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    gr->DrawOval(x1, y1, x2, y2);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::drawroundrect(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 7, "self, x1, y1, x2, y2, arcwidth, archeight");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double x3 = luaL_checknumber(L, 6);
    double y3 = luaL_checknumber(L, 7);
    gr->DrawRoundRect(x1, y1, x2, y2, x3, y3);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::fillrect(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, x2, y2");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    gr->FillRect(x1, y1, x2, y2);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::fillroundrect(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 7, "self, x1, y1, x2, y2, arcwidth, archeight");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double x3 = luaL_checknumber(L, 6);
    double y3 = luaL_checknumber(L, 7);
    gr->FillRoundRect(x1, y1, x2, y2, x3, y3);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::copyarea(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 7, "self, x1, y1, width, height, dx, dy");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double x3 = luaL_checknumber(L, 6);
    double y3 = luaL_checknumber(L, 7);
    gr->CopyArea(x1, y1, x2, y2, x3, y3);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::filloval(lua_State* L) { 
    int err = LuaHelper::check_argnum(L, 5, "self, x1, y1, width, height");
    if (err!=0) return err;
    canvas::Graphics* gr = *(canvas::Graphics **)luaL_checkudata(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    gr->FillOval(x1, y1, x2, y2);    
    return LuaHelper::chaining(L);
  }

  int LuaGraphicsBind::gc(lua_State* L) {
    return 0; //return LuaHelper::delete_userdata<canvas::Graphics>(L, meta);    
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaItem+Bind
  ///////////////////////////////////////////////////////////////////////////////
  bool LuaItem::OnEvent(canvas::Event* ev) {    
    return CallEvents<>(L, ev, this);
  }

  void LuaItem::Draw(canvas::Graphics* g, const CRgn& repaint_rgn, canvas::Canvas* widget) {            
      LuaHelper::find_userdata<>(L, this);
      lua_getfield(L, -1, "draw");
      lua_pushvalue(L, -2);
      lua_remove(L, -3);
      canvas::Graphics** gr = (canvas::Graphics **)lua_newuserdata(L, sizeof(canvas::Graphics *));
      *gr = g;
      luaL_setmetatable(L, LuaGraphicsBind::meta);                  
      int status = lua_pcall(L, 2, 0, 0);      
      if (status) {
        const char* msg = lua_tostring(L, -1);
        throw psycle::host::exceptions::library_error::runtime_error(std::string(msg));
      }      
    }

  const CRgn& LuaItem::region() const {
      if (update_) {
        rgn_.DeleteObject();        
        rgn_.CreateRectRgn(0, 0, w_, h_);
        update_ = false;
      }
      return rgn_;
    }

  const char* LuaItemBind::meta = "psyitemmeta";

  int LuaItemBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"draw", draw},
      {"setpos", setpos},      
      {"pos", pos},
      {"setsize", setsize},
      {"fls", fls},
      { NULL, NULL }
    };
    return LuaHelper::open(L, meta, methods,  gc);
  }

  int LuaItemBind::create(lua_State* L) {    
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2 && n!=1) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }  
    LuaItem* item = 0;
    LuaGroup* group = 0;
    if (n==2) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }
    item = new LuaItem(L, group);    
    LuaHelper::new_userdata<>(L, meta, item);        
    LuaHelper::register_userdata<>(L, item); 
    return 1;
  }

  int LuaItemBind::fls(lua_State* L) { 
    LuaHelper::call(L, meta, &LuaItem::FLS);    
    return LuaHelper::chaining(L);
  }

  int LuaItemBind::draw(lua_State* L) { return 0; }

  int LuaItemBind::setpos(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &LuaItem::SetXY);    
    return LuaHelper::chaining(L); 
  }

  int LuaItemBind::pos(lua_State* L) {    
    return LuaHelper::get2number2<LuaItem,double>(L, meta, &LuaItem::pos);    
  }

  int LuaItemBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<canvas::Item>(L, meta);    
  }

  int LuaItemBind::setsize(lua_State* L) {
    LuaHelper::callstrict2(L, meta, &LuaItem::SetSize);    
    return LuaHelper::chaining(L); 
  }

}}
