// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

#include <boost/shared_ptr.hpp>

namespace psycle {
namespace host {
namespace ui {


class Menu2 {
 public:
};

class MenuBar2 : public Menu2 {
 public:
};

class MenuItem2 {
 public:
};


class Menu;

class MenuBar {
 public:
  typedef boost::shared_ptr<MenuBar> Ptr;
  MenuBar() {}
  virtual ~MenuBar() = 0;
  virtual void add(Menu* menu) = 0;
  virtual void append(Menu* menu) = 0;
  virtual void remove(CMenu* menu, int pos) = 0;
  virtual bool needsupdate() const = 0;
  virtual void setupdate(bool update) = 0;
  //virtual void setcmenu(CMenu* menu) = 0;
};

inline MenuBar::~MenuBar() {}

class MenuItem {
 public:
  static int id_counter;
  static std::map<int, ui::MenuItem*> menuItemIdMap;
  MenuItem() {}
  virtual ~MenuItem() = 0;
  virtual void set_id(int id) = 0;
  virtual int id() const = 0;
  virtual void set_label(const std::string& label) = 0;
  virtual const std::string& label() const = 0;
  virtual void check() = 0;
  virtual void uncheck() = 0;
  virtual bool checked() const = 0;
  //virtual void set_root(Menu* menu) = 0;
};

inline MenuItem::~MenuItem() {}

class Menu { 
 public:
  Menu() {}  
  virtual ~Menu() = 0; 
  virtual void set_label(const std::string& label) = 0;
  virtual const std::string& label() const = 0; 
  virtual CMenu* cmenu() = 0;
  virtual void setcmenu(CMenu* menu) = 0;
  virtual void set_parent(Menu* parent) = 0;
  virtual Menu* parent() = 0;
  virtual int pos() const = 0;
  virtual void set_pos(int pos) = 0;
  virtual void setbar(MenuBar* bar) = 0;
  virtual MenuBar* bar() = 0;
  virtual void add(Menu* newmenu) = 0;
  virtual void add(MenuItem* item) = 0;
  virtual void addseparator() = 0;
  virtual void remove(MenuItem* item) = 0;
};

inline Menu::~Menu() {}

class Menu;




namespace mfc 
{

class MenuBar : public ui::MenuBar {
 public:
  MenuBar() : update_(false) { }
  ~MenuBar() { }
  void add(ui::Menu* menu) { items.push_back(menu); }
  void append(ui::Menu* menu);
  void remove(CMenu* menu, int pos);
  bool needsupdate() const { return update_; }
  void setupdate(bool update) { update_ = update; }
  /*void setroot(CMenu* menu) {

  }*/
 private:
  std::vector<ui::Menu*> items;
  bool update_;
};

class MenuItem : public ui::MenuItem {
 public:
  MenuItem() : menu_(0), id_(-1), check_(false) {}
  ~MenuItem() { }
  void set_id(int id) { id_ = id; }
  int id() const { return id_; }
  void set_label(const std::string& label);
  const std::string& label() const { return label_; }
  void check();
  void uncheck();
  bool checked() const { return check_; }
  void set_menu(ui::Menu* menu) { menu_ = menu; }  
 private:
  ui::Menu* menu_;
  int id_;
  bool check_;
  std::string label_;
};

class Menu : public ui::Menu {
 public:
  Menu() : parent_(0), pos_(-1), bar_(0), owner_(true) {
    cmenu_ = new CMenu();
  }
  Menu(CMenu* menu) : cmenu_(menu), parent_(0), pos_(-1), bar_(0), owner_(false) {}
  ~Menu() {
    if (owner_) delete cmenu_;
  }
  void set_label(const std::string& label);
  const std::string& label() const { return label_; }
  CMenu* cmenu() { return cmenu_; }
  void setcmenu(CMenu* menu) { cmenu_ = menu; }
  void set_parent(ui::Menu* parent) { parent_ = parent; }
  ui::Menu* parent() { return parent_; }
  int pos() const { return pos_; }
  void set_pos(int pos) { pos_ = pos; }
  void setbar(ui::MenuBar* bar) { bar_ = bar; }
  ui::MenuBar* bar() { return parent() ? parent()->bar() : bar_; }
  void add(ui::Menu* newmenu);
  void add(ui::MenuItem* item);
  void addseparator();
  void remove(ui::MenuItem* item);

 private:
  std::vector<ui::MenuItem*> items;
  std::string label_;
  CMenu* cmenu_;
  ui::Menu* parent_;
  ui::MenuBar* bar_;
  bool owner_;
  int pos_;
};




} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle

