#include "GeneratorGui.hpp"

#ifdef use_psycore
#include <psycle/core/song.h>
#endif

#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "MainFrm.hpp"

namespace psycle {
	namespace host {

		GeneratorGui::GeneratorGui(class MachineView* view,
#ifdef use_psycore
								   class psy::core::Machine* mac)
#else
							       class Machine* mac)
#endif

			: MachineGui(view, mac),
			  dialog_(0),
			  pixbuf_(this),
			  mute_pixbuf_(this),
			  solo_pixbuf_(this),
			  pan_pixbuf_(this),
			  vu_bg_pixbuf_(this),
			  vu_peak_pixbuf_(this),
			  vu_led_pixbuf_(this),
			  text_(this),
			  pan_dragging_(false)
		{			
			UpdateText();
		}

		GeneratorGui::~GeneratorGui()
		{		
			if ( dialog_ )
				dialog_->DestroyWindow();
		}

		void GeneratorGui::UpdateVU(CDC* devc) 
		{
			MachineGui::UpdateVU(devc);
			int vol = mac()->_volumeDisplay;
			int max = mac()->_volumeMaxDisplay;
			vol *= MachineCoords_.dGeneratorVu.width;
			vol /= 96;
			max *= MachineCoords_.dGeneratorVu.width;
			max /= 96;
			if (vol > 0) {
				if (MachineCoords_.sGeneratorVu0.width) {
					vol /= MachineCoords_.sGeneratorVu0.width;// restrict to leds
					vol *= MachineCoords_.sGeneratorVu0.width;
				}
			} else {
				vol = 0;
			}
			vu_bg_pixbuf_.SetXY(vol+MachineCoords_.dGeneratorVu.x,
								MachineCoords_.dGeneratorVu.y);
			vu_bg_pixbuf_.SetSize(MachineCoords_.dGeneratorVu.width-vol,
								  MachineCoords_.sGeneratorVu0.height);
			vu_bg_pixbuf_.SetSource(MachineCoords_.sGenerator.x+MachineCoords_.dGeneratorVu.x+vol,
									MachineCoords_.sGenerator.y+MachineCoords_.dGeneratorVu.y);
			CRgn dummy;
			XFORM rXform;
			devc->GetWorldTransform(&rXform);
			XFORM rXform_new = rXform;
			rXform_new.eDx = x();
			rXform_new.eDy = y();
			devc->SetGraphicsMode(GM_ADVANCED);
			devc->SetWorldTransform(&rXform_new);					
			vu_bg_pixbuf_.Draw(devc,dummy,view());
			vu_bg_pixbuf_.Draw(devc,dummy,view());
			if (max > 0) {
				vu_peak_pixbuf_.SetXY(max + MachineCoords_.dGeneratorVu.x, 
									  MachineCoords_.dGeneratorVu.y);
				vu_peak_pixbuf_.SetSize(MachineCoords_.sGeneratorVuPeak.width, 
										MachineCoords_.sGeneratorVuPeak.height);
				vu_peak_pixbuf_.SetSource(MachineCoords_.sGeneratorVuPeak.x, 
										  MachineCoords_.sGeneratorVuPeak.y);
				vu_peak_pixbuf_.Draw(devc,dummy,view());
			}
			if ( vol > 0 ) {
				vu_led_pixbuf_.SetXY(MachineCoords_.dGeneratorVu.x, 
									 MachineCoords_.dGeneratorVu.y);
				vu_led_pixbuf_.SetSize(vol,
									   MachineCoords_.sGeneratorVu0.height);
				vu_led_pixbuf_.SetSource(MachineCoords_.sGeneratorVu0.x, 
			 						     MachineCoords_.sGeneratorVu0.y);
				vu_led_pixbuf_.Draw(devc,dummy,view());
			}
			devc->SetGraphicsMode(GM_ADVANCED);
			devc->SetWorldTransform(&rXform);
		}

		void GeneratorGui::UpdateText()
		{
			std::ostringstream str;
			str << std::hex << std::setfill('0') << std::setw(2) << mac()->id() << ":" << mac()->GetEditName();		
			text_.SetText(str.str());
		}

		void GeneratorGui::UpdatePan()
		{
			int panning = mac()->Pan() * MachineCoords_.dGeneratorPan.width;
			panning /= 128;
			pan_pixbuf_.SetXY(panning + MachineCoords_.dGeneratorPan.x, 
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
			MachineCoords_ = MachineCoords;

			pixbuf_.SetImage(machineskin);
			pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			pixbuf_.SetMask(machineskinmask);
			mute_pixbuf_.SetImage(machineskin);
			mute_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			mute_pixbuf_.SetMask(machineskinmask);
			solo_pixbuf_.SetImage(machineskin);
			solo_pixbuf_.SetTransparent(MachineCoords.bHasTransparency);
			solo_pixbuf_.SetMask(machineskinmask);
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

		
		bool GeneratorGui::TestPan(double x, double y)
		{				
			int panning = mac()->Pan()*MachineCoords_.dGeneratorPan.width;
			panning /= 128;
			if (InRect(x,
				       y,
					   MachineCoords_.dGeneratorPan.x + panning,
					   MachineCoords_.dGeneratorPan.y,
					   MachineCoords_.dGeneratorPan.x +
					   panning +
					   MachineCoords_.sGeneratorPan.width,
					   MachineCoords_.dGeneratorPan.y +
					   MachineCoords_.sGeneratorPan.height)) {
				pan_dragging_ = true;
				return true;
			}
			return false;
		}

		void GeneratorGui::DoPanDragging(double x, double y)
		{
			int newpan = (x  - MachineCoords_.dGeneratorPan.x - (MachineCoords_.sGeneratorPan.width/2))*128;
			if (MachineCoords_.dGeneratorPan.width) {
				newpan /= MachineCoords_.dGeneratorPan.width;
				mac()->SetPan(newpan);
				newpan= mac()->Pan();
				UpdatePan();
				QueueDraw();				
				std::ostringstream str;
				if (newpan != 64) {
					str << mac()->GetEditName() << " Pan: "
					<< (int) ( 100.0f - ((float)newpan*0.78125f)) << "% Left / " 
					<< (int) ((float)newpan*0.78125f) << "% Right";
				} else {
					str << mac()->GetEditName() << " Pan: Center";
				}
				view()->WriteStatusBar(str.str());			
			}
		}

		bool GeneratorGui::TestMute(double x, double y)
		{			
			if(InRect(x,
					  y,
					  MachineCoords_.dGeneratorMute.x,
					  MachineCoords_.dGeneratorMute.y,
					  MachineCoords_.dGeneratorMute.x + 
					  MachineCoords_.sGeneratorMute.width,
					  MachineCoords_.dGeneratorMute.y + 
					  MachineCoords_.sGeneratorMute.height)) {
			  SetMute(!mac()->_mute);
			  QueueDraw();
			  return true;
			} else {
			  return false;
			}
		}

		bool GeneratorGui::TestSolo(double x, double y)
		{
			if(InRect(x,
					  y,
					  MachineCoords_.dGeneratorSolo.x,
					  MachineCoords_.dGeneratorSolo.y,
					  MachineCoords_.dGeneratorSolo.x + 
					  MachineCoords_.sGeneratorSolo.width,
					  MachineCoords_.dGeneratorSolo.y + 
					  MachineCoords_.sGeneratorSolo.height)) {
			  view()->SetSolo(mac());			  
			  return true;
			} else {
				return false;
			}
		}

		void GeneratorGui::SetMute(bool mute)
		{			
			mac()->_mute = mute;
			if (mac()->_mute) {
				mac()->_volumeCounter=0.0f;
				mac()->_volumeDisplay=0;
				if (view()->song()->machineSoloed == mac()->id()) {
					view()->song()->machineSoloed = -1;
				}
			}
			mute_pixbuf_.SetVisible(mute);			
		}

		void GeneratorGui::SetSolo(bool on)
		{			
			solo_pixbuf_.SetVisible(on);
		}

		bool GeneratorGui::OnEvent(PsycleCanvas::Event* ev)
		{
			if ( ev->type == PsycleCanvas::Event::BUTTON_PRESS ) {
				view()->SelectMachine(this);
				if ( !TestSolo(ev->x, ev->y) )
					if ( !TestMute(ev->x, ev->y) )
						TestPan(ev->x, ev->y);
			} else
			if ( ev->type == PsycleCanvas::Event::MOTION_NOTIFY ) {
				if ( pan_dragging_ ) {
					DoPanDragging(ev->x, ev->y);
					return true;
				}
			} else
			if ( ev->type == PsycleCanvas::Event::BUTTON_RELEASE ) {
				pan_dragging_ = false;
			}
			return MachineGui::OnEvent(ev);
		}

		void GeneratorGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void GeneratorGui::ShowDialog(double x, double y)
		{
			if ( !dialog_ ) {				
				dialog_ = new CFrameMachine(this);
				dialog_->Init(x, y);				
			}
		}

		int GeneratorGui::preferred_width() const
		{
			return pixbuf_.width();
		}

		int GeneratorGui::preferred_height() const
		{
			 return pixbuf_.height();
		}

	}  // namespace host
}  // namespace psycle
