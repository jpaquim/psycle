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
  // MenuBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaMenuBind::meta = "psymenumeta";

  int LuaMenuBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create}, 
      {"label", label},
      {"id", id},
      {"check", check},
      {"uncheck", uncheck},
      {"checked", checked},
      {"addlistener", addlistener},
      {"notify", notify},
      { NULL, NULL }
    };
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
  }

  int LuaMenuBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, wave)", n); 
    }
    const char* label = luaL_checkstring(L, 2);
    menuitem* menu = new menuitem();
    menu->label = label;
    LuaHelper::new_userdata<>(L, meta, menu);  
    lua_newtable(L);
    lua_setfield(L, -2, "listener_");
    lua_getglobal(L, "require");
    lua_pushstring(L, "orderedtable");
    lua_pcall(L, 1, 1, 0);  
    lua_getfield(L, -1, "new");  
    lua_pcall(L, 0, 1, 0);    
    lua_setfield(L, 3, "menus");  
    lua_pop(L, 1);
    return 1;
  }

  int LuaMenuBind::gc(lua_State* L) {	
    return LuaHelper::delete_userdata<menuitem>(L, meta);
  }

  int LuaMenuBind::label(lua_State* L) {	
    int n = lua_gettop(L);
    if (n ==1) {
      menuitem* m = LuaHelper::check<menuitem>(L, 1, meta);       	   	   
      lua_pushstring(L, m->label.c_str());
    } else {
      luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }
    return 1;
  }

  int LuaMenuBind::id(lua_State* L) {	
    int n = lua_gettop(L);
    if (n ==1) {
      menuitem* m = LuaHelper::check<menuitem>(L, 1, meta);       	   	   
      lua_pushstring(L, m->id.c_str());
    } else {
      luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }
    return 1;
  }

  int LuaMenuBind::check(lua_State* L) {	
    int n = lua_gettop(L);
    if (n ==1) {
      menuitem* m = LuaHelper::check<menuitem>(L, 1, meta);       	   	   
      if (m->mid != -1) {
#if !defined WINAMP_PLUGIN
        m->menu->CheckMenuItem(m->mid, MF_CHECKED | MF_BYCOMMAND);		   
#endif
      } else {
        m->check = true;
      }
    } else {
      luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaMenuBind::uncheck(lua_State* L) {	
    int n = lua_gettop(L);
    if (n ==1) {
      menuitem* m = LuaHelper::check<menuitem>(L, 1, meta);       	   	   
      if (m->mid != -1) {	
#if !defined WINAMP_PLUGIN
        m->menu->CheckMenuItem(m->mid, MF_UNCHECKED | MF_BYCOMMAND);
#endif
      } else {
        m->check = false;
      }
    } else {
      luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }
    lua_pushvalue(L, 1); // chaining
    return 0;
  }

  int LuaMenuBind::checked(lua_State* L) {	
    int n = lua_gettop(L);
    if (n ==1) {
      menuitem* m = LuaHelper::check<menuitem>(L, 1, meta);       	   	   
      lua_pushstring(L, m->id.c_str());
    } else {
      luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaMenuBind::addlistener(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==2) {	   
      lua_getfield(L, 1, "listener_");
      lua_pushvalue(L, 2);
      lua_rawseti(L, -2, lua_rawlen(L, -2)+1); 
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
    }
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaMenuBind::notify(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==1) {
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
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
    }
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaCanvasBind
  ///////////////////////////////////////////////////////////////////////////////

  template<class T>
  bool CallEvents(lua_State* L, canvas::Event* ev, T* that) {     
    int n1 = lua_gettop(L);          
    LuaHelper::find_userdata<>(L, that);
    bool has_event_method = false;
    if (!lua_isnil(L, -1)) { 
      switch (ev->type()) {
      case canvas::Event::BUTTON_PRESS : 
        // ev->item()->canvas()->SetCapture();
        lua_getfield(L, -1, "onmousedown");
      break;
      case canvas::Event::BUTTON_RELEASE :
        lua_getfield(L, -1, "onmouseup");
        // ev->item()->canvas()->ReleaseCapture();
      break;
      case canvas::Event::BUTTON_2PRESS : lua_getfield(L, -1, "ondblclick"); break;
      case canvas::Event::MOTION_NOTIFY :
        OutputDebugString("mousemove0");
        lua_getfield(L, -1, "onmousemove");
      break;
      case canvas::Event::MOTION_OUT : lua_getfield(L, -1, "onmouseout"); break;
      case canvas::Event::KEY_DOWN : lua_getfield(L, -1, "onkeydown"); break;
      case canvas::Event::KEY_UP : lua_getfield(L, -1, "onkeyup"); break;
      default: return true;
      }        
      has_event_method = !lua_isnil(L, -1);
      if (has_event_method) {
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_newtable(L); // build event table        
        lua_pushnumber(L, ev->x());
        lua_setfield(L, -2, "clientx");
        lua_pushnumber(L, ev->y());
        lua_setfield(L, -2, "clienty");
        lua_pushnumber(L, ev->button());
        lua_setfield(L, -2, "button");
        lua_pushnumber(L, ev->shift());
        lua_setfield(L, -2, "shift");
        lua_pushboolean(L, MK_SHIFT & ev->shift());
        lua_setfield(L, -2, "shiftkey");
        lua_pushboolean(L, MK_CONTROL & ev->shift());
        lua_setfield(L, -2, "ctrlkey");
        lua_pushboolean(L, MK_ALT & ev->shift());
        lua_setfield(L, -2, "altkey");
        OutputDebugString("mousemove1");
        int status = lua_pcall(L, 2, 0, 0);      
        if (status) {
          const char* msg =lua_tostring(L, -1);
          std::ostringstream s; s << msg << std::endl;
          throw psycle::host::exceptions::library_error::runtime_error(s.str());	
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
    if (!item) {
      //      CallEvents<>(L, ev, this);
    }
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
      {NULL, NULL}
    };
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
  }

  int LuaCanvasBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Canvas* canvas = new LuaCanvas(L);     
    canvas->SetSave(true);
    LuaHelper::new_userdata<>(L, meta, canvas);
    lua_newtable(L);    
    lua_setfield(L, -2, "__userdata");
    // LuaHelper::new_userdata<>(L, LuaGroupBind::meta, canvas->root(), 2);
    // LuaHelper::register_userdata<>(L, canvas);
    LuaHelper::register_weakuserdata<>(L, canvas);    
    return 1;
  }

  int LuaCanvasBind::gc(lua_State* L) {
    LuaCanvas* ptr = *(LuaCanvas **)luaL_checkudata(L, 1, meta);
    lua_getglobal(L, "psycle");
    lua_getfield(L, -1, "userdata");
    /*lua_newtable(L); // metatable
    lua_pushstring(L, "kv");
    lua_setfield(L, -2, "__mode"); 
    lua_setmetatable(L, -2);*/    
    delete ptr;        
    return 0;
  }

  int LuaCanvasBind::root(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    luaL_requiref(L, "psycle.ui.canvas.group", LuaGroupBind::open, 0);
    LuaHelper::new_userdata<>(L, LuaGroupBind::meta, canvas->root(), 2);    
    return 1;
  }

  int LuaCanvasBind::size(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    lua_pushnumber(L, canvas->cw());
    lua_pushnumber(L, canvas->ch());
    return 2;
  }

  int LuaCanvasBind::preferredsize(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    double w, h;
    canvas->preferredsize(w, h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
  }

  int LuaCanvasBind::setcolor(lua_State* L) {
    LuaCanvas* canvas = LuaHelper::check<LuaCanvas>(L, 1, meta);
    int n = lua_gettop(L);  // Number of arguments
    if (n==2) {
      int skin = luaL_checknumber(L, 2);
      canvas->SetColor(skin);
    } else
      if (n==4 || n==5) {
        double r = luaL_checknumber(L, 2);
        double g = luaL_checknumber(L, 3);
        double b = luaL_checknumber(L, 4);
        double a = 1.0;
        if (n==5) {
          a = luaL_checknumber(L, 5);
        }
        canvas->SetColor(r, g, b, a);
      } else {
        return luaL_error(L, "Got %d arguments expected 2 or 4 or 5 (self, r, g ,b, a)", n); 
      }    
      lua_pushvalue(L, 1); // chaining
      return 1;
  }

  int LuaCanvasBind::color(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    LuaCanvas* canvas = LuaHelper::check<LuaCanvas>(L, 1, meta);
    double r, g, b, alpha;
    int skin;
    canvas->color(r, g, b, alpha, skin);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    lua_pushnumber(L, alpha);
    lua_pushnumber(L, skin);
    return 5;
  }

  int LuaCanvasBind::setpreferredsize(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, width, height)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    double width = luaL_checknumber(L, 2);
    double height = luaL_checknumber(L, 3);
    canvas->setpreferredsize(width, height);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaCanvasBind::setcapture(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->SetCapture();
    return 0;
  }

  int LuaCanvasBind::releasecapture(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->ReleaseCapture();		
    return 0;
  }

  int LuaCanvasBind::showcursor(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->ShowCursor();
    return 0;
  }

  int LuaCanvasBind::hidecursor(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    canvas->HideCursor();
    return 0;
  }

  int LuaCanvasBind::setcursorpos(lua_State* L) {
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    double x, y;
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    canvas->SetCursorPos(x, y);
    return 0;
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
      {"setxy", setxy},
      {"pos", pos},
      {"clientpos", clientpos},
      {"getfocus", getfocus},
      {"items", getitems},      
      {"remove", remove},
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
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
  }

  int LuaGroupBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    //if (n != 2) {
    //  return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    // }           
    LuaGroup* newgroup = 0;
    if (n==1 || (n==2 && lua_isnil(L, 2))) {
      newgroup = new LuaGroup(L);
    } else if (n==2) {     
      canvas::Group* p = LuaHelper::check<canvas::Group>(L, 2, meta);
      newgroup = new LuaGroup(L, p);      
    } 
    newgroup->set_manage(true);
    LuaHelper::new_userdata<>(L, meta, newgroup);
    LuaHelper::register_userdata<>(L, newgroup);
    return 1;
  }

  int LuaGroupBind::canvas(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    } 
    canvas::Group* group = LuaHelper::check<canvas::Group>(L, 1, meta);
    canvas::Canvas* canvas = group->canvas();    
    LuaHelper::find_weakuserdata(L, canvas);
    return 1;
  }

  int LuaGroupBind::remove(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }        
    canvas::Group* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    canvas::Group* parent = group->parent();
    if (parent) {
      parent->Erase(group);    
      // LuaHelper::unregister_userdata<>(L, group);
    }
    return 0;
  }

  int LuaGroupBind::add(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }        
    canvas::Group* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    canvas::Item* item = 0;
    item = LuaHelper::test<LuaRect>(L, 2, LuaRectBind::meta);
    if (!item) {
      item = LuaHelper::test<LuaGroup>(L, 2, LuaGroupBind::meta);
      if (!item) {
        item = LuaHelper::test<LuaText>(L, 2, LuaTextBind::meta);
        if (!item) {
          item = LuaHelper::test<canvas::PixBuf>(L, 2, LuaPixBind::meta);
          if (!item) {
            item = LuaHelper::test<canvas::Line>(L, 2, LuaLineBind::meta);
          }
        }
      }
    }
    if (item) {    
      group->Add(item);
      LuaHelper::register_userdata<>(L, item);
    } else {
      luaL_error(L, "Item not canvas compatible");
    }
    return 0;
  }

  int LuaGroupBind::show(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::show);
    lua_pushvalue(L, 1);
    return 1; // chaining
  }

  int LuaGroupBind::hide(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::hide);
    lua_pushvalue(L, 1);
    return 1; // chaining    
  }

  int LuaGroupBind::enablepointerevents(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::enablepointerevents);
    lua_pushvalue(L, 1);
    return 1; // chaining
  }

  int LuaGroupBind::disablepointerevents(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::disablepointerevents);
    lua_pushvalue(L, 1);
    return 1; // chaining    
  }

  int LuaGroupBind::setxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    group->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaGroupBind::pos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);        
    lua_pushnumber(L, group->x());
    lua_pushnumber(L, group->y());    
    return 2;
  }

  int LuaGroupBind::bounds(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
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
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Group* item = LuaHelper::check<canvas::Group>(L, 1, meta);
    double x1, y1;
    x1 = item->absx();
    y1 = item->absy();
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);    
    return 2;
  }

  int LuaGroupBind::intersect(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 or 5 (self, x1, y1 [,x2, y2])", n); 
    }    
    canvas::Group* item = LuaHelper::check<canvas::Group>(L, 1, meta);    
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    canvas::Item* res = item->intersect(x, y);    
    if (res) {
      LuaHelper::find_userdata(L, res);
    } else {
      lua_pushnil(L);
    }
    return 1;
  }

  int LuaGroupBind::intersectrect(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n!=5) {
      return luaL_error(L, "Got %d arguments expected 5 (self, x1, y1 ,x2, y2)", n); 
    }    
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
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    LuaGroup::iterator it = group->begin();
    lua_newtable(L); // create item table    
    for (int i= 0;it!=group->end(); ++it, ++i) {
      canvas::Item* item = *it;
      LuaHelper::find_userdata<>(L, *it);      
      lua_rawseti(L, 2, lua_rawlen(L, 2)+1); // items[#items+1] = newitem      
    }
    lua_pushvalue(L, 2);
    return 1;
  }
  
  int LuaGroupBind::gc(lua_State* L) {
    LuaGroup* group = *(LuaGroup **)luaL_checkudata(L, 1, meta);    
    if (!group->is_root() && !group->parent() && !group->canvas()) {
      delete group;      
    }
    return 0;
  }

  int LuaGroupBind::getfocus(lua_State* L) {
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    group->GetFocus();
    return 0;
  }

  int LuaGroupBind::parent(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
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
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, zorder)", n); 
    }    
    int zorder = luaL_checknumber(L, 2);
    canvas::Group* item = LuaHelper::check<canvas::Group>(L, 1, meta);
    if (item->parent()) {
      item->parent()->set_zorder(item, zorder);
    }
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaGroupBind::zorder(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
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
      {"setpos", setpos},
      {"setcolor", setcolor},
      {"color", color},
      {"skin", skin},
      {"setoutlinecolor", setoutlinecolor},
      {"outlinecolor", outlinecolor},
      {"setxy", setxy},
      {"pos", pos},
      {"clientpos", clientpos},
      {"parent", parent},
      {"remove", remove},
      {"tostring", tostring},  
      {"setzorder", setzorder},
      {"zorder", zorder},
      {"setborder", setborder},
      {"border", border},      
      {NULL, NULL}
    };
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
  }

  int LuaRectBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1 and n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaRect* rect = 0;
    LuaGroup* group = 0;
    if (n==2) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }
    rect = new LuaRect(L, group, 10, 10, 100, 100);
    rect->set_manage(true);
    LuaHelper::new_userdata<>(L, meta, rect);    
    rect->SetColor(100, 100, 100, 1);    
    LuaHelper::register_userdata<>(L, rect); 
    return 1;
  }

  int LuaRectBind::setpos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 5) {
      return luaL_error(L, "Got %d arguments expected 5 (self, x1, y1, x2, y2)", n); 
    }    
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    rect->SetPos(x1, y1, x2, y2);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaRectBind::setborder(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 5 (self, x1, y1, x2, y2)", n); 
    }    
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);    
    rect->SetBorder(x1, y1);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaRectBind::border(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    double x1, y1;
    rect->border(x1, y1);
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);    
    return 2;
  }

  int LuaRectBind::setxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    rect->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));    
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaRectBind::pos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    lua_pushnumber(L, rect->x1());
    lua_pushnumber(L, rect->y1());
    lua_pushnumber(L, rect->x2());
    lua_pushnumber(L, rect->y2());
    return 4;
  }

  int LuaRectBind::clientpos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    double x1, y1;
    x1 = rect->absx();
    y1 = rect->absy();
    double xoff = x1 - rect->x();
    double yoff = y1 - rect->y();
    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);    
    lua_pushnumber(L, rect->x2()+xoff);
    lua_pushnumber(L, rect->y2()+yoff); 
    return 4;
  }

  int LuaRectBind::setzorder(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, zorder)", n); 
    }    
    int zorder = luaL_checknumber(L, 2);
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    if (rect->parent()) {
      rect->parent()->set_zorder(rect, zorder);
    }
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaRectBind::zorder(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    if (rect->parent()) {
      int z  = rect->parent()->zorder(rect);
      lua_pushnumber(L, z);
    } else {
      lua_pushnumber(L, -1);
    }    
    return 1;
  }

  int LuaRectBind::color(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    double r, g, b, alpha;
    int skin;
    rect->color(r, g, b, alpha, skin);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    lua_pushnumber(L, alpha);
    lua_pushnumber(L, skin);
    return 5;
  }

  int LuaRectBind::outlinecolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);
    double r, g, b, alpha;
    int skin;
    rect->outlinecolor(r, g, b, alpha, skin);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    lua_pushnumber(L, alpha);
    lua_pushnumber(L, skin);
    return 5;
  }

  int LuaRectBind::skin(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }        
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);    
    lua_pushnumber(L, rect->skin());
    return 1;
  }

  int LuaRectBind::parent(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);        
    canvas::Group* group = rect->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  int LuaRectBind::remove(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }        
    canvas::Rect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    canvas::Group* parent = rect->parent();
    if (parent) {
      parent->Erase(rect);    
      LuaHelper::unregister_userdata<>(L, rect);
    }
    return 0;
  }

  int LuaRectBind::setcolor(lua_State* L) {
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    int n = lua_gettop(L);  // Number of arguments
    if (n==2) {
      int skin = luaL_checknumber(L, 2);
      rect->SetColor(skin);
    } else
      if (n==4 || n==5) {
        double r = luaL_checknumber(L, 2);
        double g = luaL_checknumber(L, 3);
        double b = luaL_checknumber(L, 4);
        double a = 1.0;
        if (n==5) {
          a = luaL_checknumber(L, 5);
        }
        rect->SetColor(r, g, b, a);
      } else {
        return luaL_error(L, "Got %d arguments expected 2 or 4 (self, r, g ,b)", n); 
      }    
      lua_pushvalue(L, 1); // chaining
      return 1;
  }

  int LuaRectBind::setoutlinecolor(lua_State* L) {
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    int n = lua_gettop(L);  // Number of arguments
    if (n==2) {
      int skin = luaL_checknumber(L, 2);
      rect->SetOutlineColor(skin);
    } else
      if (n==4 || n==5) {
        double r = luaL_checknumber(L, 2);
        double g = luaL_checknumber(L, 3);
        double b = luaL_checknumber(L, 4);
        double a = 1.0;
        if (n==5) {
          a = luaL_checknumber(L, 5);
        }
        rect->SetOutlineColor(r, g, b, a);
      } else {
        return luaL_error(L, "Got %d arguments expected 2 or 4 (self, r, g ,b)", n); 
      }    
      lua_pushvalue(L, 1); // chaining
      return 1;
  }

  int LuaRectBind::gc(lua_State* L) {
    LuaRect* item = *(LuaRect **)luaL_checkudata(L, 1, meta);    
    if (!item->canvas() && !item->parent()) {
      delete item;      
    }
    return 0;
  }

  int LuaRectBind::tostring(lua_State* L) {
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    lua_pushstring(L, "rect");
    return 1;
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
      {"setpoints", setpoints},
      {"points", points},
      {"setpoint", setpoint},
      {"setxy", setxy},
      {"pos", pos},
      { NULL, NULL }
    };
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
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
    line->set_manage(true);
    canvas::Line::Points pts;
    pts.push_back(std::pair<double, double>(10, 10));
    pts.push_back(std::pair<double, double>(300, 300));    
    line->SetPoints(pts);
    line->SetColor(100,100,100,0);    
    LuaHelper::new_userdata<>(L, meta, line);
    LuaHelper::register_userdata<>(L, line);
    return 1;
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
    lua_pushvalue(L, 1); // chaining
    return 0;
  }
  
  int LuaLineBind::setcolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, r, g ,b)", n); 
    } 
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);
    line->SetColor(r, g, b, 0);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaLineBind::setpoint(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, idx, x, y)", n); 
    } 
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);
    double idx = luaL_checknumber(L, 2);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);    
    canvas::Line::Point pt(x, y);
    line->SetPoint(idx-1,  pt);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaLineBind::gc(lua_State* L) {
    LuaLine* item = *(LuaLine **)luaL_checkudata(L, 1, meta);    
    if (!item->canvas() && !item->parent()) {
      delete item;      
    }
    return 0;
  }

  int LuaLineBind::parent(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);        
    canvas::Group* group = item->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  int LuaLineBind::setxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    }    
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);
    line->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaLineBind::pos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);        
    lua_pushnumber(L, line->x());
    lua_pushnumber(L, line->y());    
    return 2;
  }

  int LuaLineBind::points(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
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
      {"setxy", setxy},
      {"pos", pos},
      {"tostring", tostring},
      {"parent", parent},
      {"remove", remove},
      { NULL, NULL }
    };
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
  }

  int LuaTextBind::settext(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, text)", n); 
    }
    canvas::Text* text = LuaHelper::check<canvas::Text>(L, 1, meta);
    const char* str = luaL_checkstring(L, 2);    
    text->SetText(str);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }  

  int LuaTextBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    // if (n != 2) {
    //return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    //}
    LuaGroup* group = 0;
    if ((n==2 && !lua_isnil(L, 2))) {
      group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    }     
    canvas::Text* text = new LuaText(L, group);
    text->SetText("Hello World");   
    text->set_manage(true);
    text->SetColor(100, 100, 100, 0);    
    LuaHelper::new_userdata<>(L, meta, text); 
    LuaHelper::register_userdata<>(L, text);
    return 1;
  }

  int LuaTextBind::remove(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }        
    canvas::Text* txt = LuaHelper::check<LuaText>(L, 1, meta);
    canvas::Group* parent = txt->parent();
    if (parent) {
      parent->Erase(txt);    
      LuaHelper::unregister_userdata<>(L, txt);
    }
    return 0;
  }

  int LuaTextBind::setxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    }    
    canvas::Text* text = LuaHelper::check<canvas::Text>(L, 1, meta);
    text->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));    
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaTextBind::pos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaText* text = LuaHelper::check<LuaText>(L, 1, meta);        
    lua_pushnumber(L, text->x());
    lua_pushnumber(L, text->y());    
    return 2;
  }

  int LuaTextBind::text(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaText* txt = LuaHelper::check<LuaText>(L, 1, meta);        
    lua_pushstring(L, txt->text().c_str());       
    return 1;
  }

  int LuaTextBind::setcolor(lua_State* L) {
    LuaText* text = LuaHelper::check<LuaText>(L, 1, meta);
    int n = lua_gettop(L);  // Number of arguments
    if (n==2) {
      int skin = luaL_checknumber(L, 2);
      text->SetColor(skin);
    } else
      if (n==4) {
        double r = luaL_checknumber(L, 2);
        double g = luaL_checknumber(L, 3);
        double b = luaL_checknumber(L, 4);
        text->SetColor(r, g, b, 0);
      } else {
        return luaL_error(L, "Got %d arguments expected 2 or 4 (self, r, g ,b)", n); 
      }    
      lua_pushvalue(L, 1); // chaining
      return 1;
  }

  int LuaTextBind::color(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Text* text = LuaHelper::check<canvas::Text>(L, 1, meta);
    double r, g, b, alpha;
    int skin;
    text->color(r, g, b, alpha, skin);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    lua_pushnumber(L, alpha);
    lua_pushnumber(L, skin);
    return 5;
  }

  int LuaTextBind::gc(lua_State* L) {
    LuaText* item = *(LuaText **)luaL_checkudata(L, 1, meta);    
    if (!item->canvas() && !item->parent()) {
      delete item;      
    }
    return 0;
  }

  int LuaTextBind::tostring(lua_State* L) {
    LuaText* txt = LuaHelper::check<LuaText>(L, 1, meta);
    lua_pushstring(L, "text");
    return 1;
  }

  int LuaTextBind::parent(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Text* txt = LuaHelper::check<canvas::Text>(L, 1, meta);        
    canvas::Group* group = txt->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaPixBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaPixBind::meta = "psypixmeta";

  int LuaPixBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"setxy", setxy},
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
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
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
    pix->set_manage(true);    
    LuaHelper::new_userdata<>(L, meta, pix);
    LuaHelper::register_userdata<>(L, pix);
    return 1;
  }

  int LuaPixBind::setxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    }    
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    pix->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));    
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaPixBind::setsource(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    } 
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    double xsrc = luaL_checknumber(L, 2);
    double ysrc = luaL_checknumber(L, 3);    
    pix->SetSource(xsrc, ysrc);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaPixBind::settransparent(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 3 (self, r, g, b)", n); 
    } 
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);    
    pix->SetTransparent(true, r, g, b);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaPixBind::setsize(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, width, height)", n); 
    } 
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    double width = luaL_checknumber(L, 2);
    double height = luaL_checknumber(L, 3);    
    pix->SetSize(width, height);
    lua_pushvalue(L, 1); // chaining
    return 1;    
  }

  int LuaPixBind::gc(lua_State* L) {
    canvas::PixBuf* item = *(canvas::PixBuf **)luaL_checkudata(L, 1, meta);    
    if (!item->canvas() && !item->parent()) {
      delete item;      
    }
    return 0;    
  }

  int LuaPixBind::setpixmap(lua_State* L) {
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    int n = lua_gettop(L);  // Number of arguments
    if (n==2) {
      int skin = luaL_checknumber(L, 2);
      pix->SetImage(skin);
    } else {
      return luaL_error(L, "Got %d arguments expected 2 (self, skin)", n); 
    }    
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaPixBind::load(lua_State* L) {
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    int n = lua_gettop(L);  // Number of arguments
    if (n==2) {
      const char* str = luaL_checkstring(L, 2);
      pix->Load(str);
    } else {
      return luaL_error(L, "Got %d arguments expected 2 (self, filename)", n); 
    }    
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaPixBind::pos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    lua_pushnumber(L, pix->x());
    lua_pushnumber(L, pix->y());
    lua_pushnumber(L, pix->x()+pix->width());
    lua_pushnumber(L, pix->y()+pix->height());
    return 4;
  }

  int LuaPixBind::setpos(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 5) {
      return luaL_error(L, "Got %d arguments expected 5 (self, x1, y1, x2, y2)", n); 
    }    
    canvas::PixBuf* pix = LuaHelper::check<canvas::PixBuf>(L, 1, meta);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);                 
    pix->SetXY(x1, y1);
    pix->SetSize(x2-x1, y2-y1);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaPixBind::tostring(lua_State* L) {
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);
    lua_pushstring(L, "pix");
    return 1;
  }

  int LuaPixBind::parent(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    canvas::Item* item = LuaHelper::check<canvas::Item>(L, 1, meta);        
    canvas::Group* group = item->parent();
    LuaHelper::find_userdata<>(L, group);    
    return 1;
  }

}}
