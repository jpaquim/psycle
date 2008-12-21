#include "EffectGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		EffectGui::EffectGui(class MachineView* view,
							 class Machine* mac)
			: MachineGui(view, mac),
			  pan_dragging_(false),
			  dialog_(0),
			  pixbuf_(this),
			  mute_pixbuf_(this),
			  bypass_pixbuf_(this),
			  pan_pixbuf_(this),
			  vu_bg_pixbuf_(this),
			  vu_peak_pixbuf_(this),
			  vu_led_pixbuf_(this),
			  text_(this)
		{
			UpdateText();
		}

		EffectGui::~EffectGui()
		{
		}

		bool EffectGui::TestPan(double x, double y)
		{				
			int panning = mac()->_panning*MachineCoords_.dEffectPan.width;
			panning /= 128;
			if (InRect(x,
				       y,
					   MachineCoords_.dEffectPan.x + panning,
					   MachineCoords_.dEffectPan.y,
					   MachineCoords_.dEffectPan.x +
					   panning +
					   MachineCoords_.sEffectPan.width,
					   MachineCoords_.dEffectPan.y +
					   MachineCoords_.sEffectPan.height)) {
				pan_dragging_ = true;
				return true;
			}
			return false;
		}

		bool EffectGui::InBypass(double x, double y)
		{
			return (InRect(x,
						   y,
						   MachineCoords_.dEffectBypass.x,
						   MachineCoords_.dEffectBypass.y,
					       MachineCoords_.dEffectBypass.x + 
					       MachineCoords_.sEffectBypass.width,
					       MachineCoords_.dEffectBypass.y + 
					       MachineCoords_.sEffectBypass.height));
		}

		void EffectGui::SetBypass(bool on)
		{
			mac()->Bypass(on);
			if (mac()->Bypass())
			{
				mac()->_volumeCounter=0.0f;
				mac()->_volumeDisplay=0;
			}			
			bypass_pixbuf_.SetVisible(on);
		}

		void EffectGui::SetMute(bool on)
		{
			mac()->_mute = on;
			if (mac()->_mute) {
				mac()->_volumeCounter=0.0f;
				mac()->_volumeDisplay=0;
				if (view()->song()->machineSoloed == mac()->_macIndex) {
					view()->song()->machineSoloed = -1;
				}									
			}
			mute_pixbuf_.SetVisible(on);
		}

		bool EffectGui::InMute(double x, double y)
		{			
			return (InRect(x,
						   y,
						   MachineCoords_.dEffectMute.x,
						   MachineCoords_.dEffectMute.y,
						   MachineCoords_.dEffectMute.x + 
					       MachineCoords_.sEffectMute.width,
					       MachineCoords_.dEffectMute.y + 
					       MachineCoords_.sEffectMute.height));
		}

		void EffectGui::DoPanDragging(double x, double y)
		{
			int newpan = (x  - MachineCoords_.dGeneratorPan.x - (MachineCoords_.sGeneratorPan.width/2))*128;
			if (MachineCoords_.dGeneratorPan.width) {
				newpan /= MachineCoords_.dGeneratorPan.width;
				mac()->SetPan(newpan);
				UpdatePan();
				QueueDraw();
			}
		}


		void EffectGui::UpdateVU() 
		{
			MachineGui::UpdateVU();
			int vol = mac()->_volumeDisplay;
			int max = mac()->_volumeMaxDisplay;
			vol *= MachineCoords_.dEffectVu.width;
			vol /= 96;
			max *= MachineCoords_.dEffectVu.width;
			max /= 96;
			vu_bg_pixbuf_.SetXY(vol+MachineCoords_.dEffectVu.x,
								MachineCoords_.dEffectVu.y);
			vu_bg_pixbuf_.SetSize(MachineCoords_.dEffectVu.width-vol,
								  MachineCoords_.sEffectVu0.height);
			vu_bg_pixbuf_.SetSource(MachineCoords_.sEffect.x+MachineCoords_.dGeneratorVu.x+vol,
									MachineCoords_.sEffect.y+MachineCoords_.dGeneratorVu.y);
			vu_bg_pixbuf_.QueueDraw();
			vu_peak_pixbuf_.SetXY(max + MachineCoords_.dEffectVu.x, 
								  MachineCoords_.dEffectVu.y);
			vu_peak_pixbuf_.SetSize(MachineCoords_.sEffectVuPeak.width, 
									MachineCoords_.sEffectVuPeak.height);
			vu_peak_pixbuf_.SetSource(MachineCoords_.sEffectVuPeak.x, 
									  MachineCoords_.sEffectVuPeak.y);
			vu_peak_pixbuf_.QueueDraw();
			vu_led_pixbuf_.SetXY(MachineCoords_.dEffectVu.x, 
								 MachineCoords_.dEffectVu.y);
			vu_led_pixbuf_.SetSize(vol,
								   MachineCoords_.sEffectVu0.height);
			vu_led_pixbuf_.SetSource(MachineCoords_.sEffectVu0.x, 
			 					     MachineCoords_.sEffectVu0.y);
			vu_led_pixbuf_.QueueDraw();
		}

		void EffectGui::UpdateText()
		{
			char name[sizeof(mac()->_editName)+6+3];
			sprintf(name,"%.2X:%s",mac()->_macIndex, mac()->_editName);
			text_.SetText(name);
		}

		void EffectGui::UpdatePan()
		{
			int panning = mac()->_panning*MachineCoords_.dEffectPan.width;
			panning /= 128;
			pan_pixbuf_.SetXY(panning+MachineCoords_.dEffectPan.x, 
							  MachineCoords_.dEffectPan.y);
		}

		void EffectGui::SetSkin(const SMachineCoords&	MachineCoords,
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
			pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			pixbuf_.SetMask(machineskinmask);
			mute_pixbuf_.SetImage(machineskin);
			mute_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			mute_pixbuf_.SetMask(machineskinmask);
			bypass_pixbuf_.SetImage(machineskin);
			bypass_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			bypass_pixbuf_.SetMask(machineskinmask);
			vu_bg_pixbuf_.SetImage(machineskin);
			vu_bg_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			vu_bg_pixbuf_.SetMask(machineskinmask);
			vu_peak_pixbuf_.SetImage(machineskin);
			vu_peak_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			vu_peak_pixbuf_.SetMask(machineskinmask);
			vu_led_pixbuf_.SetImage(machineskin);
			vu_led_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			vu_led_pixbuf_.SetMask(machineskinmask);
			pan_pixbuf_.SetImage(machineskin);
			pan_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			pan_pixbuf_.SetMask(machineskinmask);
			pixbuf_.SetSize(MachineCoords.sEffect.width, 
							MachineCoords.sEffect.height);
							pixbuf_.SetSource(MachineCoords.sEffect.x, 
							MachineCoords.sEffect.y);
			UpdatePan();
			pan_pixbuf_.SetSize(MachineCoords.sEffectPan.width, 
								MachineCoords.sEffectPan.height);
			pan_pixbuf_.SetSource(MachineCoords.sEffectPan.x, 
								  MachineCoords.sEffectPan.y);
			bypass_pixbuf_.SetXY(MachineCoords.dEffectBypass.x,
								 MachineCoords.dEffectBypass.y);
			bypass_pixbuf_.SetSize(MachineCoords.sEffectBypass.width, 
								   MachineCoords.sEffectBypass.height);
			bypass_pixbuf_.SetSource(MachineCoords.sEffectBypass.x, 
									 MachineCoords.sEffectBypass.y);
			bypass_pixbuf_.SetVisible(false);
			mute_pixbuf_.SetXY(MachineCoords.dEffectMute.x, 
							   MachineCoords.dEffectMute.y);
			mute_pixbuf_.SetSize(MachineCoords.sEffectMute.width, 
							     MachineCoords.sEffectMute.height);
			mute_pixbuf_.SetSource(MachineCoords.sEffectMute.x, 
								   MachineCoords.sEffectMute.y);
			mute_pixbuf_.SetVisible(false);
			text_.SetXY(MachineCoords.dEffectName.x,
					    MachineCoords.dEffectName.y);
			text_.SetFont(font);
			text_.SetColor(GetRValue(font_color) / 255.0,
						   GetGValue(font_color) / 255.0,
						   GetBValue(font_color) / 255.0,
						   1.0);
		}

		bool EffectGui::OnEvent(TestCanvas::Event* ev)
		{		
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS) {
				if (InMute(ev->x, ev->y)) {					
					return true;
				} else if (InBypass(ev->x, ev->y)) {
					return true;
				}
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {				
				//if ( !TestMute(ev->x, ev->y) )
				if (InMute(ev->x, ev->y)) {
				  SetMute(!mac()->_mute);
				  QueueDraw();
				} else
				if (InBypass(ev->x, ev->y)) {
					SetBypass(!mac()->Bypass());
					QueueDraw();
					return true;
				}
				else
					TestPan(ev->x, ev->y);
			} else
			if ( ev->type == TestCanvas::Event::MOTION_NOTIFY ) {
				if ( pan_dragging_ ) {
					DoPanDragging(ev->x, ev->y);
					return true;
				}
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_RELEASE ) {
				pan_dragging_ = false;
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return MachineGui::OnEvent(ev);
		}

		void EffectGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void EffectGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CFrameMachine(mac()->_macIndex, this);
				//CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
