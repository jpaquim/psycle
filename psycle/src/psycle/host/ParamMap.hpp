///\file
///\brief interface file for psycle::host::CParamMap
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "CanvasItems.hpp"

namespace psycle {
namespace host {

class Machine;

class ParamMap : public ui::Frame {
 public:
	ParamMap(Machine* machine);	
  
  virtual void OnClose() { delete this; }

 private:
  void InitLayout();  
  void AddListView();
  void FillListView();
  void FillComboBox();  
  void AddMachineParamSelect(const ui::Group::Ptr& parent);
  void UpdateMachineParamEndText();
  void OnReplaceButtonClick(ui::Button&);
  void OnListViewChange(ui::ListView&, const ui::Node::Ptr&);
  void OnComboBoxSelect(ui::ComboBox&);  
  std::string param_name(int index) const;
  ui::Group::Ptr CreateRow(const ui::Group::Ptr& parent);
  ui::Group::Ptr CreateTitleRow(const ui::Group::Ptr& parent, const std::string& header_text);

  ui::Node::Ptr root_node_;
  Machine* machine_;
  ui::Group::Ptr top_client_group_;
  ui::ListView::Ptr list_view_;
  ui::ComboBox::Ptr cbx_box_;
  ui::canvas::Text::Ptr machine_param_end_txt_;  
  boost::shared_ptr<ui::Ornament> background_;
  boost::shared_ptr<ui::Ornament> background1_;
  boost::shared_ptr<ui::Ornament> background2_;
  boost::shared_ptr<ui::Ornament> border_;
  boost::shared_ptr<ui::Ornament> text_border_;
};

}   // namespace
}   // namespace
