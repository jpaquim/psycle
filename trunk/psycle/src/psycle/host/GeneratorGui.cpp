#include "GeneratorGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"
#include "MainFrm.hpp"

namespace psycle {
	namespace host {

		GeneratorGui::GeneratorGui(class MachineView* view,
							       class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0),
			  pixbuf_(this),
			  mute_pixbuf_(this),
			  pan_pixbuf_(this),
			  vu_bg_pixbuf_(this),
			  vu_peak_pixbuf_(this),
			  vu_led_pixbuf_(this),
			  text_(this)
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
			UpdateText();
		}

		GeneratorGui::~GeneratorGui()
		{		
		}

		void GeneratorGui::UpdateVU() 
		{
			MachineGui::UpdateVU();
			int vol = mac()->_volumeDisplay;
			int max = mac()->_volumeMaxDisplay;
			vol *= MachineCoords_.dGeneratorVu.width;
			vol /= 96;
			max *= MachineCoords_.dGeneratorVu.width;
			max /= 96;
			vu_bg_pixbuf_.SetXY(vol+MachineCoords_.dGeneratorVu.x,
								MachineCoords_.dGeneratorVu.y);
			vu_bg_pixbuf_.SetSize(MachineCoords_.dGeneratorVu.width-vol,
								  MachineCoords_.sGeneratorVu0.height);
			vu_bg_pixbuf_.SetSource(MachineCoords_.sGenerator.x+MachineCoords_.dGeneratorVu.x+vol,
									MachineCoords_.sGenerator.y+MachineCoords_.dGeneratorVu.y);
			vu_bg_pixbuf_.QueueDraw();
			vu_peak_pixbuf_.SetXY(max + MachineCoords_.dGeneratorVu.x, 
								  MachineCoords_.dGeneratorVu.y);
			vu_peak_pixbuf_.SetSize(MachineCoords_.sGeneratorVuPeak.width, 
									MachineCoords_.sGeneratorVuPeak.height);
			vu_peak_pixbuf_.SetSource(MachineCoords_.sGeneratorVuPeak.x, 
									  MachineCoords_.sGeneratorVuPeak.y);
			vu_peak_pixbuf_.QueueDraw();
			vu_led_pixbuf_.SetXY(MachineCoords_.dGeneratorVu.x, 
								 MachineCoords_.dGeneratorVu.y);
			vu_led_pixbuf_.SetSize(vol,
								   MachineCoords_.sGeneratorVu0.height);
			vu_led_pixbuf_.SetSource(MachineCoords_.sGeneratorVu0.x, 
			 					     MachineCoords_.sGeneratorVu0.y);
			vu_led_pixbuf_.QueueDraw();
		}

		void GeneratorGui::UpdateText()
		{
			char name[sizeof(mac()->_editName)+6+3];
			sprintf(name,"%.2X:%s",mac()->_macIndex, mac()->_editName);
			text_.SetText(name);
		}

		void GeneratorGui::UpdatePan()
		{
			int panning = mac()->_panning*MachineCoords_.dGeneratorPan.width;
			panning /= 128;
			pan_pixbuf_.SetXY(panning+MachineCoords_.dGeneratorPan.x, 
		 	  			      MachineCoords_.dGeneratorPan.y);
		}

		void GeneratorGui::SetSkin(const SMachineCoords& MachineCoords,
					 			   CBitmap* machineskin,
								   CBitmap* machineskinmask,
								   CBitmap* machinebkg,
								   HBITMAP hbmMachineSkin,
								   HBITMAP hbmMachineBkg,
								   HBITMAP hbmMachineDial,
								   const CFont& font,
								   COLORREF font_color)
		{
			machineskin_ = machineskin;
			machineskinmask_ = machineskinmask;
			machinebkg_ = machinebkg;
			hbmMachineSkin_ = hbmMachineSkin;
			hbmMachineBkg_ = hbmMachineBkg;
			hbmMachineDial = hbmMachineDial;
			MachineCoords_ = MachineCoords;

			pixbuf_.SetImage(machineskin);
			mute_pixbuf_.SetImage(machineskin);
			solo_pixbuf_.SetImage(machineskin);
			vu_bg_pixbuf_.SetImage(machineskin);
			vu_peak_pixbuf_.SetImage(machineskin);
			vu_led_pixbuf_.SetImage(machineskin);
			pixbuf_.SetSize(MachineCoords.sGenerator.width, 
							MachineCoords.sGenerator.height);
							pixbuf_.SetSource(MachineCoords.sGenerator.x, 
							MachineCoords.sGenerator.y);
			mute_pixbuf_.SetXY(MachineCoords.dGeneratorMute.x, 
							   MachineCoords.dGeneratorMute.y);
			mute_pixbuf_.SetSize(MachineCoords.sGeneratorMute.width, 
							     MachineCoords.sGeneratorMute.height);
			mute_pixbuf_.SetSource(MachineCoords.sGeneratorMute.x, 
								   MachineCoords.sGeneratorMute.y);
			mute_pixbuf_.SetVisible(false);
			solo_pixbuf_.SetXY(MachineCoords.dGeneratorSolo.x,
							   MachineCoords.dGeneratorSolo.y);
			solo_pixbuf_.SetSize(MachineCoords.sGeneratorSolo.width, 
								 MachineCoords.sGeneratorSolo.height);
			solo_pixbuf_.SetSource(MachineCoords.sGeneratorSolo.x, 
								   MachineCoords.sGeneratorSolo.y);
			solo_pixbuf_.SetVisible(false);
			UpdatePan();
			pan_pixbuf_.SetSize(MachineCoords.sGeneratorPan.width, 
								MachineCoords.sGeneratorPan.height);
			pan_pixbuf_.SetSource(MachineCoords.sGeneratorPan.x, 
								  MachineCoords.sGeneratorPan.y);
			text_.SetXY(MachineCoords.dGeneratorName.x,
					    MachineCoords.dGeneratorName.y);			
			text_.SetFont(font);
			text_.SetColor(GetRValue(font_color) / 255.0,
						   GetGValue(font_color) / 255.0,
						   GetBValue(font_color) / 255.0,
						   1.0);
		}

		void GeneratorGui::SetSelected(bool on)
		{
			if ( on && !IsSelected() ) {
				view()->song()->seqBus = view()->song()->FindBusFromIndex(mac()->_macIndex);
				view()->main()->UpdateComboGen();
				view()->child_view()->Invalidate(1);
			}
			int size = 5;
			TestCanvas::Line::Points pts;
			pts.push_back(std::pair<double,double>(-size, -size));
			pts.push_back(std::pair<double,double>(-size, size));
			sel_line_left_top_1.SetPoints(pts);
			sel_line_left_top_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-size, -size));
			pts.push_back(std::pair<double,double>(size, -size));
			sel_line_left_top_2.SetPoints(pts);
			sel_line_left_top_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(pixbuf_.width()-size, -size));
			pts.push_back(std::pair<double,double>(pixbuf_.width()+size, -size));
			sel_line_right_top_1.SetPoints(pts);
			sel_line_right_top_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(pixbuf_.width()+size, -size));
			pts.push_back(std::pair<double,double>(pixbuf_.width()+size, +size));
			sel_line_right_top_2.SetPoints(pts);
			sel_line_right_top_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-size, pixbuf_.height() - size));
			pts.push_back(std::pair<double,double>(-size, pixbuf_.height() + size));
			sel_line_left_bottom_1.SetPoints(pts);
			sel_line_left_bottom_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-size, pixbuf_.height() + size));
			pts.push_back(std::pair<double,double>(+size, pixbuf_.height() + size));
			sel_line_left_bottom_2.SetPoints(pts);
			sel_line_left_bottom_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(pixbuf_.width() + size, pixbuf_.height() - size));
			pts.push_back(std::pair<double,double>(pixbuf_.width() + size, pixbuf_.height() + size));
			sel_line_right_bottom_1.SetPoints(pts);
			sel_line_right_bottom_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(pixbuf_.width() - size, pixbuf_.height() + size));
			pts.push_back(std::pair<double,double>(pixbuf_.width() + size, pixbuf_.height() + size));
			sel_line_right_bottom_2.SetPoints(pts);
			sel_line_right_bottom_2.SetVisible(on);			
		}

		bool GeneratorGui::IsSelected() const
		{
			return sel_line_left_top_1.visible();
		}

		bool GeneratorGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {
				view()->SelectMachine(this);
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			MachineGui::OnEvent(ev);
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
