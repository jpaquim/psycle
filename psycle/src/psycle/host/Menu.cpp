#include <psycle/host/detail/project.hpp>
#include "Menu.hpp"
#include "Psycle.hpp"


namespace psycle {
namespace host {
namespace ui {

int ui::MenuItem::id_counter = 0;
std::map<int, ui::MenuItem*> ui::MenuItem::menuItemIdMap;

namespace mfc {

 void MenuBar::append(ui::Menu* menu) {
  int pos = menu->cmenu()->GetMenuItemCount();
  std::vector<ui::Menu*>::iterator it = items.begin();
  for ( ; it != items.end(); ++it) {
    ui::Menu* m = *it;
    menu->cmenu()->AppendMenu(MF_POPUP, (UINT_PTR)m->cmenu()->m_hMenu, m->label().c_str());
    m->set_parent(menu);
    m->set_pos(pos++);
  }
  menu->setbar(this);
}

 void MenuBar::remove(CMenu* menu, int pos) {
    std::vector<ui::Menu*>::iterator it = items.begin();
    for ( ; it != items.end(); ++it) {
      menu->RemoveMenu(pos++, MF_BYPOSITION);
    }
}

// Menu
 void Menu::set_label(const std::string& label) {
  label_ = label;
  if (parent()) {
    parent()->cmenu()->ModifyMenu(pos_, MF_BYPOSITION, 0, label.c_str());
    ui::MenuBar* b = bar();
    b->setupdate(b!=0);
  }
}

 void Menu::add(ui::Menu* newmenu) {
  cmenu_->AppendMenu(MF_POPUP | MF_ENABLED, (UINT_PTR)newmenu->cmenu()->m_hMenu, newmenu->label().c_str());
  newmenu->set_parent(this);
  newmenu->set_pos(cmenu_->GetMenuItemCount()-1);
}

 void Menu::add(ui::MenuItem* item) {
  items.push_back(item);
  item->set_menu(this);
  const int id = ID_DYNAMIC_MENUS_START+item->id_counter;
  item->set_id(id);
  MenuItem::menuItemIdMap[item->id()] = item;
  cmenu_->AppendMenu(MF_STRING, id, item->label().c_str());
  if (item->checked()) {
		cmenu_->CheckMenuItem(id, MF_CHECKED | MF_BYCOMMAND);
  }
}

 void Menu::addseparator() {
  cmenu_->AppendMenu(MF_SEPARATOR, 0, "-");
}

 void Menu::remove(ui::MenuItem* item) {
  std::vector<ui::MenuItem*>::iterator it;
  it = std::find(items.begin(), items.end(), item);
  if (it != items.end()) {
    items.erase(it);
  }
  cmenu_->RemoveMenu(item->id(), MF_BYCOMMAND);
}

// menuitem  

 void MenuItem::set_label(const std::string& label) {
  label_ = label;
  if (menu_) {
      menu_->cmenu()->ModifyMenu(id(), MF_BYCOMMAND, id(), label.c_str());
  }
}

 void MenuItem::check() {
  check_ = true;
  if (menu_) {
    menu_->cmenu()->CheckMenuItem(id(), MF_CHECKED | MF_BYCOMMAND);
  }
}

 void MenuItem::uncheck() {
  check_ = false;
  if (menu_) {
      menu_->cmenu()->CheckMenuItem(id(), MF_UNCHECKED | MF_BYCOMMAND);
  }
}

} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle