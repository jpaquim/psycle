#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class EffectGui : public MachineGui {
		public:
			EffectGui(class MachineView* view,
					  class Machine* mac);
			~EffectGui();

			virtual void SetSkin(const SMachineCoords&	MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color);

			virtual void BeforeDeleteDlg();
			virtual bool OnEvent(TestCanvas::Event* ev);
			virtual void SetSelected(bool on) {}
			virtual bool IsSelected() const{return false;}

			virtual void UpdateVU();
			virtual void SetBypass(bool on);
			virtual void SetMute(bool on);
			virtual void SetSolo(bool on){}
		protected:
			virtual void ShowDialog(double x, double y);
		private:
			void UpdateText();
			void UpdatePan();			
			bool TestPan(double x, double y);
			bool InMute(double x, double y);
			bool InBypass(double x, double y);
			void DoPanDragging(double x, double y);

			bool pan_dragging_;
			class CFrameMachine* dialog_;
			//skin elements
			TestCanvas::PixBuf pixbuf_;
			TestCanvas::PixBuf mute_pixbuf_;
			TestCanvas::PixBuf bypass_pixbuf_;
			TestCanvas::PixBuf pan_pixbuf_;
			TestCanvas::PixBuf vu_bg_pixbuf_;
			TestCanvas::PixBuf vu_peak_pixbuf_;
			TestCanvas::PixBuf vu_led_pixbuf_;
			TestCanvas::Text text_;
			// skin
/*			CBitmap* machineskin_;
			CBitmap* machineskinmask_;
			CBitmap* machinebkg_;
			HBITMAP hbmMachineSkin_;
			HBITMAP hbmMachineBkg_;	
			HBITMAP hbmMachineDial_;
*/
			SMachineCoords	MachineCoords_;
		};
	}
}