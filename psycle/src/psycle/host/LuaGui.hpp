// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Machine.hpp"
#include "Canvas.hpp"
#include "Dyn_dialog.h"
#include "InputHandler.hpp"

struct lua_State;
struct luaL_Reg;

namespace psycle { namespace host {

   class LuaMenu;
   class LuaMachine;

   class LuaMenuBar {
    public:
     LuaMenuBar() : update_(false) {}
     void add(LuaMenu* menu) { items.push_back(menu); }
     void append(LuaMenu* menu);
     void remove(CMenu* menu, int pos);
     bool needsupdate() const { return update_; }
     void setupdate(bool update) { update_ = update; }
    private:
     std::vector<LuaMenu*> items;
     bool update_;
   };

   class LuaMenuItem {
    public:
     LuaMenuItem() : menu_(0), id_(-1), check_(false) {}
     void set_id(int id) { id_ = id; }
     int id() const { return id_; }
     void set_label(const std::string& label);
     const std::string& label() const { return label_; }
     void check();      
     void uncheck();      
     bool checked() const { return check_; } 
     void set_menu(LuaMenu* menu) { menu_ = menu; }
     static int id_counter;
     static std::map<std::uint16_t, LuaMenuItem*> menuItemIdMap;
    private:            
      LuaMenu* menu_;
      int id_;
      bool check_;      
      std::string label_;            
  };

  class LuaMenu {   
     public:
       LuaMenu() : parent_(0), pos_(-1), bar_(0), owner_(true) {
          cmenu_ = new CMenu();
       }
       LuaMenu(CMenu* menu) : cmenu_(menu), parent_(0), pos_(-1), bar_(0), owner_(false) {}
       ~LuaMenu() {
         if (owner_) delete cmenu_;
       }
       void set_label(const std::string& label);
       const std::string& label() const { return label_; }       
       CMenu* cmenu() { return cmenu_; }
       void setcmenu(CMenu* menu) { cmenu_ = menu; }
       void set_parent(LuaMenu* parent) { parent_ = parent; }
       LuaMenu* parent() { return parent_; }
       int pos() const { return pos_; }
       void set_pos(int pos) { pos_ = pos; }
       void setbar(LuaMenuBar* bar) { bar_ = bar; }
       LuaMenuBar* bar() { return parent() ? parent()->bar() : bar_; }
       void add(LuaMenu* newmenu);
       void add(LuaMenuItem* item);
       void addseparator();
       void remove(LuaMenuItem* item);
     private:
       std::vector<LuaMenuItem*> items;
       std::string label_;
       CMenu* cmenu_;       
       LuaMenu* parent_;
       LuaMenuBar* bar_;
       bool owner_;       
       int pos_;
   };
   
  struct LuaMenuBarBind {  
    static int open(lua_State *L);
    static const char* meta;
  private:
    static int create(lua_State *L);
    static int add(lua_State *L);    
    static int gc(lua_State* L);    
  };

  struct LuaMenuBind {  
    static int open(lua_State *L);
    static const char* meta;
  private:
    static int create(lua_State *L);
    static int add(lua_State *L);
    static int remove(lua_State *L);
    static int addseparator(lua_State *L);
    static int setlabel(lua_State *L);
    static int gc(lua_State* L);    
  };

  struct LuaMenuItemBind {  
    static int open(lua_State *L);
    static const char* meta;
  private:
    static int create(lua_State *L);
    static int id(lua_State *L);
    static int setlabel(lua_State *L);
    static int label(lua_State *L);
    static int gc(lua_State* L);
    static int check(lua_State* L);
    static int uncheck(lua_State* L);
    static int checked(lua_State* L);
    static int addlistener(lua_State* L);
    static int notify(lua_State* L);
  };
    
  class LuaActionListener : public ActionListener {
  public:
    LuaActionListener(lua_State* state) : ActionListener(), L(state), mac_(0) {}    
    virtual void OnNotify(ActionType action);
    void setmac(LuaMachine* mac) { mac_ = mac; }
  private:
    lua_State* L;
    LuaMachine* mac_;
  };
  
  struct LuaActionListenerBind {
    static const char* meta;
    static int open(lua_State *L);    
    static int create(lua_State *L);    
  private:    
    static int gc(lua_State* L);
  };    

  struct LuaDialog : public Dynamic_dialog {
    LuaDialog(lua_State* state, const char* title = 0, int width = 100,
              int height = 100, int position_x = 0,
              int position_y = 0) :
              Dynamic_dialog(title, width, height, position_x, position_y), L(state) {}    
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    LuaMachine* mac;
   private:
     lua_State* L;
  };

  struct LuaDialogBind {
    static int open(lua_State *L);
    static const char* meta;    
   private:
    static int create(lua_State *L);
    static int gc(lua_State* L);
    static int show(lua_State* L);
    static int hide(lua_State* L);
    static int addtext(lua_State* L);
    static int addedit(lua_State* L);
    static int addbutton(lua_State* L);
    static int editvalue(lua_State* L);
    static int settext(lua_State* L);
  };

  struct LuaCanvas : public canvas::Canvas {
    LuaCanvas(lua_State* state) : canvas::Canvas(), L(state), show_scrollbar(false) {}    
    virtual canvas::Item* OnEvent(canvas::Event* ev);
    bool show_scrollbar;
   private:
     lua_State* L;     
  };

  struct LuaCanvasBind {
    static int open(lua_State *L);
    static const char* meta;    
   private:
    static int create(lua_State *L);
    static int gc(lua_State* L);
    static int root(lua_State* L);
    static int size(lua_State* L);
    static int setpreferredsize(lua_State* L);
    static int preferredsize(lua_State* L);
    static int setcolor(lua_State* L);    
    static int color(lua_State* L);
    static int generate(lua_State* L);
    static int setcapture(lua_State* L);
    static int releasecapture(lua_State* L);
    static int hidecursor(lua_State* L);
    static int showcursor(lua_State* L);
    static int setcursorpos(lua_State* L);
    static int showscrollbar(lua_State* L);
  };

  struct LuaGroup : public canvas::Group {
    LuaGroup(lua_State* state) : canvas::Group(), L(state) {}
    LuaGroup(lua_State* state, canvas::Group* parent) :
      canvas::Group(parent, 0, 0), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaGroupBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setpos(lua_State *L);    
    static int pos(lua_State *L);
    static int clientpos(lua_State* L);
    static int setzoom(lua_State* L);
    static int getfocus(lua_State *L);
    static int getitems(lua_State* L);    
    static int remove(lua_State* L);
    static int removeall(lua_State* L);
    static int add(lua_State* L);
    static int show(lua_State* L);
    static int hide(lua_State* L);    
    static int enablepointerevents(lua_State* L);
    static int disablepointerevents(lua_State* L);
    static int gc(lua_State* L);
    static int parent(lua_State *L);
    static int tostring(lua_State* L);
    static int setzorder(lua_State* L);
    static int zorder(lua_State* L);
    static int bounds(lua_State* L);   
    static int intersect(lua_State* L);
    static int intersectrect(lua_State* L);
    static int canvas(lua_State* L);    
    static canvas::Item* test(lua_State* L, int index);
    static int fls(lua_State *L);
  };
  
  struct LuaRect : public canvas::Rect {
    LuaRect(lua_State* state) : Rect(), L(state) {}
    LuaRect(lua_State* state, canvas::Group* parent, double x1, double y1, double x2, double y2) :
      canvas::Rect(parent, x1, y1, x2, y2), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);    
   private:
     lua_State* L;
  };

  struct LuaRectBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);    
    // fill
    static int setcolor(lua_State* L);    
    static int color(lua_State* L);
    // stroke
    static int setstrokecolor(lua_State* L);    
    static int strokecolor(lua_State* L);

    static int setpos(lua_State *L);
    static int pos(lua_State *L);
    static int clientpos(lua_State* L);    
    static int parent(lua_State *L);    
    static int gc(lua_State* L);
    static int tostring(lua_State* L);    
    static int setzorder(lua_State* L);
    static int zorder(lua_State* L);
    static int setborder(lua_State* L);
    static int border(lua_State* L);    
    static int getfocus(lua_State *L);
  };

  struct LuaLine : public canvas::Line {
    LuaLine(lua_State* state) : Line(), L(state) {}
    LuaLine(lua_State* state, canvas::Group* parent) :
      canvas::Line(parent), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaLineBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setcolor(lua_State* L);    
    static int color(lua_State* L);
    static int setpoints(lua_State* L);
    static int setpoint(lua_State* L);
    static int points(lua_State* L);
    static int parent(lua_State *L);
    static int gc(lua_State* L);
    static int setpos(lua_State *L);
    static int pos(lua_State *L);
    static int enablepointerevents(lua_State* L);
    static int disablepointerevents(lua_State* L);
  };

  struct LuaText : public canvas::Text {
    LuaText(lua_State* state) : canvas::Text(), L(state) {}
    LuaText(lua_State* state, canvas::Group* parent) :
      canvas::Text(parent), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaTextBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setpos(lua_State *L);
    static int settext(lua_State* L);
    static int text(lua_State* L);
    static int setcolor(lua_State* L);    
    static int color(lua_State* L);
    static int pos(lua_State *L);
    static int gc(lua_State* L);
    static int parent(lua_State *L);    
    static int tostring(lua_State* L);
  };

  /*
  struct LuaPixBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int parent(lua_State *L);        
    static int setsource(lua_State* L);
    static int setsize(lua_State* L);
    static int setpixmap(lua_State* L);
    static int settransparent(lua_State* L);
    static int load(lua_State* L);
    static int gc(lua_State* L);
    static int pos(lua_State *L);
    static int setpos(lua_State *L);
    static int tostring(lua_State* L);
  };
  */

  struct LuaGraphicsBind {
    static int open(lua_State *L);
    static int translate(lua_State *L);
    static int setcolor(lua_State* L);
    static int color(lua_State* L);
    static int drawline(lua_State *L);
    static int drawrect(lua_State *L);
    static int drawroundrect(lua_State *L);
    static int drawoval(lua_State* L);
    static int fillrect(lua_State *L);
    static int fillroundrect(lua_State *L);
    static int filloval(lua_State* L);
    static int copyarea(lua_State* L);
    static int drawstring(lua_State* L);
    static int setfont(lua_State* L);
    static int font(lua_State* L);
    static int drawpolygon(lua_State* L);
    static int fillpolygon(lua_State* L);
    static const char* meta;
   private:    
    static int gc(lua_State* L);
  };

  class LuaItem : public canvas::Item {
  public:
    LuaItem(lua_State* state) : canvas::Item(), L(state) { Init(); }
    LuaItem(lua_State* state, canvas::Group* parent) :
        canvas::Item(parent), L(state) { 
      Init();
    }
    virtual bool OnEvent(canvas::Event* ev);
    virtual void Draw(canvas::Graphics* g, const CRgn& repaint_region,
      canvas::Canvas* widget);
    const CRgn& region() const;    
    // till regions will be bind
    void SetSize(double w, double h) { STR(); w_ = w; h_ = h; FLS(); }
   private:
     void Init() { w_ = h_ = 0; }
     lua_State* L;
     int w_, h_;
  };

  struct LuaItemBind {
    static int open(lua_State *L);
    static int create(lua_State *L);
    static int draw(lua_State* L);
    static int pos(lua_State *L);
    static int setpos(lua_State *L);    
    static int setsize(lua_State* L); // till there's a rgn bind
    static int fls(lua_State *L);
    static const char* meta;    
   private:    
    static int gc(lua_State* L);
  };
  
  } // namespace
} // namespace