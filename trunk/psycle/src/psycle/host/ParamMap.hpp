///\file
///\brief interface file for psycle::host::CParamMap
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "CanvasItems.hpp"

namespace psycle {
namespace host {

class Machine;

class ParamMapSkin {
 public:
  ParamMapSkin();
  
  ui::ARGB background_color;  
  ui::ARGB list_view_background_color;
  ui::ARGB title_background_color;
  ui::ARGB font_color;
  ui::ARGB title_font_color;
  ui::ARGB range_end_color;
  ui::Ornament::Ptr background;  
  ui::Ornament::Ptr title_background;
  ui::Font font;
  ui::Font title_font;

 private:
  static ui::Font CastCFont(CFont* font);
};

class ParamMap : public ui::Frame {
 public:
	ParamMap(Machine* machine);	
  
  virtual void OnClose() { 
    delete this;  
  }

  void UpdateNew(int par,int value);

 private:
  void InitLayout();  
  void AddListView();
  void AddHelpGroup(const ui::Window::Ptr& parent);
  void FillListView();
  void FillComboBox();  
  void AddMachineParamSelect(const ui::Group::Ptr& parent);
  void UpdateMachineParamEndText();
  void OnReplaceButtonClick(ui::Button&);
  void OnListViewChange(ui::ListView&, const ui::Node::Ptr&);
  void OnComboBoxSelect(ui::ComboBox&);  
  std::string param_name(int index) const;
  ui::Group::Ptr CreateRow(const ui::Window::Ptr& parent);
  ui::Group::Ptr CreateTitleRow(const ui::Window::Ptr& parent, const std::string& header_text);
  void ReplaceSelection();

  ui::Node::Ptr root_node_;
  Machine* machine_;
  ui::Group::Ptr top_client_group_;
  ui::ListView::Ptr list_view_;
  ui::ComboBox::Ptr cbx_box_;
  ui::CheckBox::Ptr allow_auto_learn_chk_box_;
  ui::canvas::Text::Ptr machine_param_end_txt_;  
  boost::shared_ptr<ui::Ornament> border_;
  boost::shared_ptr<ui::Ornament> text_border_;

  ParamMapSkin skin_;
};

}   // namespace
}   // namespace
