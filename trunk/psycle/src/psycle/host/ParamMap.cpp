///\file
///\brief implementation file for psycle::host::CParamList.
#pragma once
#include <psycle/host/detail/project.private.hpp>
#include "ParamMap.hpp"
#include "PsycleGlobal.hpp"
#include "PsycleConfig.hpp"
#include "Machine.hpp"

namespace psycle {
namespace host {

ParamMap::ParamMap(Machine* machine) 
    : machine_(machine),
      top_client_group_(new ui::Group()),
      list_view_(new ui::ListView()),
      cbx_box_(new ui::ComboBox()),
      machine_param_end_txt_(new ui::canvas::Text()),
      root_node_(new ui::Node()),
      background_(ui::canvas::OrnamentFactory::Instance().CreateFill(0x232323)),
      background1_(ui::canvas::OrnamentFactory::Instance().CreateFill(0x555555)),
      background2_(ui::canvas::OrnamentFactory::Instance().CreateFill(0x292929)) {
  set_title("Parameter map");
  ui::canvas::Canvas::Ptr view_(new ui::canvas::Canvas());
  view_->SetSave(false);  
  ui::canvas::LineBorder* border = new ui::canvas::LineBorder(0x737373);
  border->set_border_radius(ui::canvas::BorderRadius(5, 5, 5, 5));
  border_.reset(border);
  view_->set_ornament(background_);
  set_view(view_);   
  view_->set_aligner(ui::Aligner::Ptr(new ui::canvas::DefaultAligner()));      
  FillListView();
  AddListView();  
  InitLayout();    
  AddMachineParamSelect(top_client_group_);      
  FillComboBox();
  UpdateMachineParamEndText();
}

void ParamMap::FillListView() {  
  for (int i = 0; i < 256; ++i) {    
    std::stringstream str;
    str << i << " ->";
    ui::Node::Ptr col1_node(new ui::Node(str.str()));                
    std::stringstream str1;    
    str1 << machine_->translate_param(i) << " [" << param_name(machine_->translate_param(i)) << "]";
    ui::Node::Ptr col2_node(new ui::Node(str1.str()));
    col1_node->AddNode(col2_node);
    root_node_->AddNode(col1_node);
  }
  if (list_view_) {
    list_view_->UpdateList();
  }
}

void ParamMap::FillComboBox() {
  for (int i = 0; i != machine_->GetNumParams(); ++i) {    
    std::stringstream str;
    str << i << " [" << param_name(i) << "]";
    cbx_box_->add_item(str.str());
  }
  cbx_box_->set_item_index(0);
}

void ParamMap::OnReplaceButtonClick(ui::Button&) {
  int top_index = list_view_->top_index();
  std::vector<ui::Node::Ptr> nodes = list_view_->selected_nodes();
  std::vector<ui::Node::Ptr>::iterator it = nodes.begin();
  for (int i = 0; it != nodes.end(); ++i, ++it) {
    if (i + cbx_box_->item_index() < machine_->GetNumParams()) {      
      int virtual_index = (*it)->imp(*list_view_->imp())->pos();
      (*it)->clear();            
      std::stringstream str;
      str << i << " [" << param_name(i + cbx_box_->item_index()) << "]";
      ui::Node::Ptr col2_node(new ui::Node(str.str()));
      (*it)->AddNode(col2_node);      
      machine_->set_virtual_param_index(virtual_index, i + cbx_box_->item_index());
    } else {
      break;
    }    
  }  
  list_view_->UpdateList();
  list_view_->EnsureVisible(255);
  list_view_->EnsureVisible(top_index);
}

void ParamMap::OnListViewChange(ui::ListView&, const ui::Node::Ptr&) {
  UpdateMachineParamEndText();
}

void ParamMap::OnComboBoxSelect(ui::ComboBox&) {
  UpdateMachineParamEndText();
}

void ParamMap::UpdateMachineParamEndText() {
  int end = cbx_box_->item_index() + list_view_->selected_nodes().size() - 1;
  if (end >= 0 && end < machine_->GetNumParams()) {         
    machine_param_end_txt_->set_text(param_name(end));
  }
}

std::string ParamMap::param_name(int index) const {
  if (index < machine_->GetNumParams()) {
    char s[1024];
    machine_->GetParamName(index, s);
    return s;
  } 
  return "";  
}

void ParamMap::InitLayout() {
  Window::Ptr client_group(new ui::Group());  
  view()->Add(client_group);
  client_group->set_align(ui::ALCLIENT);    
  client_group->set_aligner(ui::Aligner::Ptr(new ui::canvas::DefaultAligner()));
  client_group->set_ornament(background1_);
    
  client_group->Add(top_client_group_);
  top_client_group_->set_auto_size(false, true);
  top_client_group_->set_margin(ui::Rect(ui::Point(5, 5), ui::Point(0, 5)));
  top_client_group_->set_align(ui::ALTOP);
  
  top_client_group_->set_aligner(ui::Aligner::Ptr(new ui::canvas::DefaultAligner()));
  top_client_group_->set_ornament(border_);

  ui::Group::Ptr help_group(new ui::Group());
  client_group->Add(help_group);
  help_group->set_aligner(ui::Aligner::Ptr(new ui::canvas::DefaultAligner()));
  help_group->set_auto_size(false, false);
  help_group->set_pos(ui::Rect(ui::Point(), ui::Dimension(0, 200)));
  help_group->set_margin(ui::Rect(ui::Point(5, 5), ui::Point(0, 5)));
  help_group->set_align(ui::ALTOP);
  help_group->set_ornament(border_);

  CreateTitleRow(help_group, "Help");
  
  ui::canvas::Text::Ptr txt1(new ui::canvas::Text("Up/Down Cursor Multiselect a range."));
  txt1->set_align(ui::ALTOP);
  txt1->set_color(0xCDCDCD);
  txt1->set_margin(ui::Rect(ui::Point(5, 5), ui::Point(0, 5)));
  help_group->Add(txt1);
}

void ParamMap::AddListView() {  
  view()->Add(list_view_);
  list_view_->set_background_color(0x444444);
  list_view_->set_text_color(0xCDCDCD);
  list_view_->set_align(ui::ALLEFT);
  list_view_->ViewReport();
  list_view_->EnableRowSelect();
  list_view_->AddColumn("Virtual", 50);
  list_view_->AddColumn("Machine Parameter", 200);  
  list_view_->set_root_node(root_node_);
  list_view_->UpdateList();
  list_view_->change.connect(boost::bind(&ParamMap::OnListViewChange, this, _1, _2));
}

ui::Group::Ptr ParamMap::CreateRow(const ui::Group::Ptr& parent) {
  ui::Group::Ptr header_group(new ui::Group());
  header_group->set_aligner(ui::Aligner::Ptr(new ui::canvas::DefaultAligner()));
  header_group->set_auto_size(false, true);
  header_group->set_align(ui::ALTOP);
  header_group->set_margin(ui::Rect(ui::Point(5, 5), ui::Point(0, 5)));  
  parent->Add(header_group);  
  return header_group;
}

ui::Group::Ptr ParamMap::CreateTitleRow(const ui::Group::Ptr& parent, const std::string& header_text) {
  ui::Group::Ptr header_group = CreateRow(parent);
  header_group->set_ornament(background2_);  
  ui::canvas::Text::Ptr txt(new ui::canvas::Text(header_text));
  header_group->Add(txt);
  txt->set_color(0xB4B4B4);
  txt->set_auto_size(true, true);
  txt->set_margin(ui::Rect(ui::Point(5, 5), ui::Point(0, 5)));
  txt->set_align(ui::ALLEFT);
  return header_group;
}

void ParamMap::AddMachineParamSelect(const ui::Group::Ptr& parent) {
  CreateTitleRow(parent, "ASSIGN Virtual Parameter with Machine Parameter");  
  
  ui::Group::Ptr cbx_group = CreateRow(parent);  
  cbx_group->set_margin(ui::Rect(ui::Point(5, 0), ui::Point(0, 5)));  
  ui::canvas::Text::Ptr txt1(new ui::canvas::Text("FROM"));
  txt1->set_alignment(ui::ALLEFT);
  txt1->set_color(0xCDCDCD);
  cbx_group->Add(txt1);
  txt1->set_auto_size(false, true);
  txt1->set_align(ui::ALLEFT);
  txt1->set_pos(ui::Rect(ui::Point(), ui::Dimension(50, 0)));    
  cbx_group->Add(cbx_box_);
  cbx_box_->set_auto_size(false, false);
  cbx_box_->set_align(ui::ALLEFT);
  cbx_box_->set_pos(ui::Rect(ui::Point(0, 0), ui::Dimension(150, 20)));  
  cbx_box_->select.connect(boost::bind(&ParamMap::OnComboBoxSelect, this, _1));

  ui::Group::Ptr end_group = CreateRow(parent);
  end_group->set_margin(ui::Rect(ui::Point(5, 5), ui::Point(0, 10)));  
  ui::canvas::Text::Ptr txt2(new ui::canvas::Text("TO     "));
  end_group->Add(txt2);
  txt2->set_auto_size(false, true);
  txt2->set_color(0xCDCDCD);
  txt2->set_alignment(ui::ALLEFT);
  txt2->set_align(ui::ALLEFT);
  txt2->set_pos(ui::Rect(ui::Point(), ui::Dimension(50, 0)));      
  end_group->Add(machine_param_end_txt_);    
  machine_param_end_txt_->set_align(ui::ALLEFT); 
  machine_param_end_txt_->set_color(0x18C3E7);
    
  ui::Group::Ptr btn_group = CreateRow(parent);
  btn_group->set_margin(ui::Rect(ui::Point(5, 0), ui::Point(0, 5)));  
    
  ui::canvas::Button::Ptr replace_btn(new ui::canvas::Button());
  btn_group->Add(replace_btn);
  replace_btn->set_align(ui::ALLEFT);
  replace_btn->set_pos(ui::Rect(ui::Point(0, 0), ui::Dimension(200, 20)));  
  replace_btn->set_text("replace");    
  replace_btn->click.connect(boost::bind(&ParamMap::OnReplaceButtonClick, this, _1));
}

}   // namespace
}   // namespace
