#include "GeneratorGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		GeneratorGui::GeneratorGui(class MachineView* view,
							       class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)			  
		{
			TestCanvas::Line::Points pts;
			pts.push_back(std::pair<double,double>(0, 0));
			pts.push_back(std::pair<double,double>(100, 100));
			sel_line_left_top_1.SetPoints(pts);
			sel_line_left_top_1.SetVisible(false);
			Add(&sel_line_left_top_1);
			sel_line_left_top_2.SetPoints(pts);
			sel_line_left_top_2.SetVisible(false);
			Add(&sel_line_left_top_2);
			sel_line_right_top_1.SetPoints(pts);
			sel_line_right_top_1.SetVisible(false);
			Add(&sel_line_right_top_1);
			sel_line_right_top_2.SetPoints(pts);
			sel_line_right_top_2.SetVisible(false);
			Add(&sel_line_right_top_2);
			sel_line_left_bottom_1.SetPoints(pts);
			sel_line_left_bottom_1.SetVisible(false);
			Add(&sel_line_left_bottom_1);
			sel_line_left_bottom_2.SetPoints(pts);
			sel_line_left_bottom_2.SetVisible(false);
			Add(&sel_line_left_bottom_2);
			sel_line_right_bottom_1.SetPoints(pts);
			sel_line_right_bottom_1.SetVisible(false);
			Add(&sel_line_right_bottom_1);
			sel_line_right_bottom_2.SetPoints(pts);
			sel_line_right_bottom_2.SetVisible(false);
			Add(&sel_line_right_bottom_2);
		}

		GeneratorGui::~GeneratorGui()
		{		
		}


		void GeneratorGui::SetSelected(bool on)
		{
			TestCanvas::Line::Points pts;
			pts.push_back(std::pair<double,double>(-10, -10));
			pts.push_back(std::pair<double,double>(-10, 10));
			sel_line_left_top_1.SetPoints(pts);
			sel_line_left_top_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-10, -10));
			pts.push_back(std::pair<double,double>(10, -10));
			sel_line_left_top_2.SetPoints(pts);
			sel_line_left_top_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferredWidth()-10, -10));
			pts.push_back(std::pair<double,double>(preferredWidth()+10, -10));
			sel_line_right_top_1.SetPoints(pts);
			sel_line_right_top_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferredWidth()+10, -10));
			pts.push_back(std::pair<double,double>(preferredWidth()+10, +10));
			sel_line_right_top_2.SetPoints(pts);
			sel_line_right_top_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-10, preferredHeight()-10));
			pts.push_back(std::pair<double,double>(-10, preferredHeight()+10));
			sel_line_left_bottom_1.SetPoints(pts);
			sel_line_left_bottom_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-10, preferredHeight()+10));
			pts.push_back(std::pair<double,double>(+10, preferredHeight()+10));
			sel_line_left_bottom_2.SetPoints(pts);
			sel_line_left_bottom_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferredWidth()+10, preferredHeight()-10));
			pts.push_back(std::pair<double,double>(preferredWidth()+10, preferredHeight()+10));
			sel_line_right_bottom_1.SetPoints(pts);
			sel_line_right_bottom_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferredWidth()-10, preferredHeight()+10));
			pts.push_back(std::pair<double,double>(preferredWidth()+10, preferredHeight()+10));
			sel_line_right_bottom_2.SetPoints(pts);
			sel_line_right_bottom_2.SetVisible(on);
		}

		bool GeneratorGui::IsSelected() const
		{
			return false;
		}

		bool GeneratorGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {
				view()->SelectMachine(this);
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void GeneratorGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void GeneratorGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CFrameMachine(mac()->_macIndex, this);
				//CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
