#pragma once
#include <psycle/host/detail/project.hpp>
#include "Machine.hpp"
#include "Canvas.hpp"

struct lua_State;
struct luaL_Reg;

namespace psycle { namespace host {

struct menuitem { 	
    menuitem() {
#if !defined WINAMP_PLUGIN
      menu = 0;
#endif
      mid = -1; check = false; }
    std::string id; std::string label; 
#if !defined WINAMP_PLUGIN
    CMenu* menu;
#endif
    int mid; bool check;
  };

  struct LuaMenuBind {  
    static int open(lua_State *L);
    static const char* meta;
  private:
    static int create(lua_State *L);
    static int id(lua_State *L);
    static int label(lua_State *L);
    static int gc(lua_State* L);
    static int check(lua_State* L);
    static int uncheck(lua_State* L);
    static int checked(lua_State* L);
    static int addlistener(lua_State* L);
    static int notify(lua_State* L);
  };

  struct LuaCanvasBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int gc(lua_State* L);
    static int root(lua_State* L);
    static int setcolor(lua_State* L);
  };

  struct LuaGroupBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setxy(lua_State *L);
    static int gc(lua_State* L);
  };
  
  struct LuaRect : public canvas::Rect {
    LuaRect(lua_State* L) : Rect(), L_(L) {}
    LuaRect(lua_State* L, canvas::Group* parent, double x1, double y1, double x2, double y2 ) :
      canvas::Rect(parent, x1, y1, x2, y2), L_(L) {}

    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L_;
  };

  struct LuaRectBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setpos(lua_State *L);
    static int setcolor(lua_State* L);
    static int gc(lua_State* L);
  };

  struct LuaLineBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setcolor(lua_State* L);
    static int gc(lua_State* L);
  };

  struct LuaTextBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int settext(lua_State* L);
    static int setcolor(lua_State* L);
    static int gc(lua_State* L);
  };

  } // namespace
} // namespace