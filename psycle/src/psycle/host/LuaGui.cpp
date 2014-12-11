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
    LuaHelper::find_userdata<>(L, that);    
    switch (ev->type()) {
      case canvas::Event::BUTTON_PRESS : lua_getfield(L, -1, "onmousedown"); break;
      case canvas::Event::BUTTON_RELEASE : lua_getfield(L, -1, "onmouseup"); break;
      case canvas::Event::BUTTON_2PRESS : lua_getfield(L, -1, "ondblclick"); break;
      case canvas::Event::MOTION_NOTIFY : lua_getfield(L, -1, "onmousemove"); break;      
      default: return true;
    }
    bool has_event_method = !lua_isnil(L, -1);
    if (has_event_method) {
      lua_pushvalue(L, -2);
      lua_pushnumber(L, ev->x());
      lua_pushnumber(L, ev->y());
      lua_pushnumber(L, ev->button());
      lua_pushnumber(L, ev->shift());
      int status = lua_pcall(L, 5, 0, 0);
      if (status) {
        const char* msg =lua_tostring(L, -1);
        std::ostringstream s; s << msg << std::endl;
        throw psycle::host::exceptions::library_error::runtime_error(s.str());	
      }
    }
    lua_pop(L, 2);    
    return has_event_method;
  }

  /////////////////////////////////////////////////////////////////////////////
  // LuaCanvas+Bind
  /////////////////////////////////////////////////////////////////////////////

  const char* LuaCanvasBind::meta = "psycanvasmeta";

  canvas::Item* LuaCanvas::OnEvent(canvas::Event* ev) {
    canvas::Item* item = canvas::Canvas::OnEvent(ev);
    if (!item) {
      CallEvents<>(L, ev, this);
    }
    return item;
  }

  int LuaCanvasBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"root", root},
      {"setcolor", setcolor},      
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
    LuaHelper::new_userdata<>(L, LuaGroupBind::meta, canvas->root());
    LuaHelper::register_userdata<>(L, canvas->root());
    LuaHelper::new_userdata<>(L, meta, canvas);          
    LuaHelper::register_userdata<>(L, canvas);
    return 1;
  }

  int LuaCanvasBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<canvas::Canvas>(L, meta);
  }

  int LuaCanvasBind::root(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    LuaHelper::find_userdata<>(L, canvas->root());        
    return 1;
  }

  int LuaCanvasBind::setcolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, r, g ,b)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);
    canvas->set_bg_color(RGB(r, g, b));
    lua_pushvalue(L, 1); // chaining
    return 1;
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
      {"getxy", getxy},
      {"getfocus", getfocus},
      {"items", getitems},
      {"remove", remove},
      {"show", show},
      {"hide", hide},
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
    if (n==1) {
      newgroup = new LuaGroup(L);
    } else
    if (n==2) {
      canvas::Group* p = LuaHelper::check<canvas::Group>(L, 2, meta);
      newgroup = new LuaGroup(L, p);
    } 
    newgroup->set_manage(true);
    LuaHelper::new_userdata<>(L, meta, newgroup);
    LuaHelper::register_userdata<>(L, newgroup);
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
      LuaHelper::unregister_userdata<>(L, group);
    }
    return 0;
  }

  int LuaGroupBind::show(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::show);
    return 0;
  }

  int LuaGroupBind::hide(lua_State* L) {
    LuaHelper::call<LuaGroup>(L, meta, &LuaGroup::hide);
    return 0;
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

  int LuaGroupBind::getxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);        
    lua_pushnumber(L, group->x());
    lua_pushnumber(L, group->y());    
    return 2;
  }

  int LuaGroupBind::getitems(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
    LuaGroup::iterator it = group->begin();
    lua_newtable(L); // // create item table    
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
    if (group->parent() == 0) {
      delete group;
    }
    return 0;
  }
  
  int LuaGroupBind::getfocus(lua_State* L) {
     LuaGroup* group = LuaHelper::check<LuaGroup>(L, 1, meta);
     group->GetFocus();
     return 0;
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
      {"setoutlinecolor", setoutlinecolor},
      {"setxy", setxy},
      {"getxy", getxy},
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
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    LuaRect* rect = new LuaRect(L, group, 10, 10, 100, 100);
    rect->set_manage(true);
    LuaHelper::new_userdata<>(L, meta, rect);    
    rect->SetColor(100,100,100,0);
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

  int LuaRectBind::getxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    canvas::Rect* rect = LuaHelper::check<canvas::Rect>(L, 1, meta);        
    lua_pushnumber(L, rect->x1());
    lua_pushnumber(L, rect->y1());    
    return 2;
  }

  int LuaRectBind::setcolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, r, g ,b)", n); 
    } 
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);
    rect->SetColor(r, g, b, 0);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaRectBind::setoutlinecolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, r, g ,b)", n); 
    } 
    LuaRect* rect = LuaHelper::check<LuaRect>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);
    rect->SetOutlineColor(r, g, b, 0);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaRectBind::gc(lua_State* L) {
    // return LuaHelper::delete_userdata<LuaRectBind>(L, meta);
    return 0;
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
      {"setcolor", setcolor},
      {"setpoints", setpoints},      
      {"setxy", setxy},
      {"getxy", getxy},
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
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    LuaLine* line = new LuaLine(L, group);
    line->set_manage(true);
    canvas::Line::Points pts;
    pts.push_back(std::pair<double, double>(10,10));
    pts.push_back(std::pair<double, double>(300,300));    
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

  int LuaLineBind::gc(lua_State* L) {
    // return LuaHelper::delete_userdata<LuaRectBind>(L, meta);
    return 0;
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

  int LuaLineBind::getxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaLine* line = LuaHelper::check<LuaLine>(L, 1, meta);        
    lua_pushnumber(L, line->x());
    lua_pushnumber(L, line->y());    
    return 2;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaLineBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaTextBind::meta = "psytextmeta";

  bool LuaText::OnEvent(canvas::Event* ev) {    
    return CallEvents<>(L, ev, this);
  }

  int LuaTextBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"settext", settext},
      {"setcolor", setcolor},
      {"setxy", setxy},
      {"getxy", getxy},
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
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
    canvas::Text* text = new LuaText(L, group);
    text->SetText("Hello World");   
    text->set_manage(true);
    text->SetColor(100, 100, 100, 0);    
    LuaHelper::new_userdata<>(L, meta, text); 
    LuaHelper::register_userdata<>(L, text);
    return 1;
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

  int LuaTextBind::getxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 3 (self)", n); 
    }    
    LuaText* text = LuaHelper::check<LuaText>(L, 1, meta);        
    lua_pushnumber(L, text->x());
    lua_pushnumber(L, text->y());    
    return 2;
  }

  int LuaTextBind::setcolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, r, g ,b)", n); 
    } 
    canvas::Text* text = LuaHelper::check<canvas::Text>(L, 1, meta);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);
    text->SetColor(r, g, b, 0);
    lua_pushvalue(L, 1); // chaining
    return 1;
  }

  int LuaTextBind::gc(lua_State* L) {
    // return LuaHelper::delete_userdata<LuaRectBind>(L, meta);
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaLineBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaPixBind::meta = "psypixmeta";

  int LuaPixBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"setxy", setxy},
      {"setsource", setsource},
      {"setsize", setsize},
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
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    LuaGroup* group = LuaHelper::check<LuaGroup>(L, 2, LuaGroupBind::meta);
   
    canvas::PixBuf* pix = new canvas::PixBuf(group);
    // pix->SetTransparent(true, 68, 68, 68);
    pix->SetSource(32,0);
    pix->SetSize(32,32);
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
    // return LuaHelper::delete_userdata<LuaRectBind>(L, meta);
    return 0;
  }
  
}}