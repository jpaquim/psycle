#include "LuaGui.hpp"

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

  const char* LuaCanvasBind::meta = "psycanvasmeta";

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
    canvas::Canvas* canvas = new canvas::Canvas();    
    LuaHelper::new_userdata<>(L, meta, canvas);          
    return 1;
  }

  int LuaCanvasBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<LuaCanvasBind>(L, meta);
  }

  int LuaCanvasBind::root(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
    } 
    canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, 1, meta);
    LuaHelper::new_userdata<>(L, LuaGroupBind::meta, canvas->root());
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

  int LuaGroupBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"setxy", setxy},
      { NULL, NULL }
    };
    luaL_newmetatable(L, meta);
    lua_pushcclosure(L, gc, 0);
    lua_setfield(L,-2, "__gc");
    luaL_newlib(L, methods);  
    return 1;
  }

  int LuaGroupBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    // if (n != 2) {
    //  return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    // }        
    canvas::Group* p = LuaHelper::check<canvas::Group>(L, 2, meta);
    canvas::Group* group = new canvas::Group(p, 0, 0);
    LuaHelper::new_userdata<>(L, meta, group);          
    return 1;
  }

  int LuaGroupBind::setxy(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L, "Got %d arguments expected 3 (self, x, y)", n); 
    }    
    canvas::Group* group = LuaHelper::check<canvas::Group>(L, 1, meta);
    group->SetXY(luaL_checknumber(L, 2), luaL_checknumber(L, 3));    
    return 1;
  }  

  int LuaGroupBind::gc(lua_State* L) {
    // return LuaHelper::delete_userdata<LuaRectBind>(L, meta);
    return 0;
  }

  bool LuaRect::OnEvent(canvas::Event* ev) {
    // do some stuff    
    LuaHelper::find_userdata<>(L_, this);
    lua_getfield(L_, -1, "onbuttonleft");
    if (!lua_isnil(L_, -1)) {
      lua_pushvalue(L_, -2);
      lua_pcall(L_, 1, 0, 0);
    }
    lua_pop(L_, 2);    
    return true;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaRectBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaRectBind::meta = "psyrectmeta";

  int LuaRectBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setpos", setpos},
      {"setcolor", setcolor},      
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
    canvas::Group* group = LuaHelper::check<canvas::Group>(L, 2, LuaGroupBind::meta);
    LuaRect* rect = new LuaRect(L, group, 10, 10, 100, 100);
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
    return 1;
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

  int LuaRectBind::gc(lua_State* L) {
    // return LuaHelper::delete_userdata<LuaRectBind>(L, meta);
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaLineBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaLineBind::meta = "psylinemeta";

  int LuaLineBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setcolor", setcolor},
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
    canvas::Group* group = LuaHelper::check<canvas::Group>(L, 2, LuaGroupBind::meta);
    canvas::Line* line = new canvas::Line(group);
    canvas::Line::Points pts;
    pts.push_back(std::pair<double, double>(10,10));
    pts.push_back(std::pair<double, double>(30,30));    
    line->SetPoints(pts);
    line->SetColor(100,100,100,0);    
    LuaHelper::new_userdata<>(L, meta, line);
    return 1;
  }

  int LuaLineBind::setcolor(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 4) {
      return luaL_error(L, "Got %d arguments expected 1 (self, r, g ,b)", n); 
    } 
    canvas::Line* line = LuaHelper::check<canvas::Line>(L, 1, meta);
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

  ///////////////////////////////////////////////////////////////////////////////
  // LuaLineBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaTextBind::meta = "psytextmeta";

  int LuaTextBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},      
      {"settext", settext},
      {"setcolor", setcolor},
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
    return 1;
  }  

  int LuaTextBind::create(lua_State* L) {	
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, group)", n); 
    }    
    canvas::Group* group = LuaHelper::check<canvas::Group>(L, 2, LuaGroupBind::meta);
    canvas::Text* text = new canvas::Text(group, "Hello World");   
    text->SetColor(100, 100, 100, 0);    
    LuaHelper::new_userdata<>(L, meta, text);
    return 1;
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
  
}}