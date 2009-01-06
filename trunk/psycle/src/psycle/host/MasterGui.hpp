#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MasterGui : public MachineGui {
		public:
			MasterGui(class MachineView* view,
					  class Machine* mac);
			~MasterGui();

			virtual void SetSkin(const SMachineCoords& MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color);

			virtual void SetSelected(bool on) {}
			virtual bool IsSelected() const {return false;}
			virtual void SetBypass(bool on){}
			virtual void SetMute(bool on){}
			virtual void SetSolo(bool on){}

			virtual int preferred_width() const;
			virtual int preferred_height() const;
			
		protected:
			virtual void ShowDialog(double x, double y);
			bool OnEvent(TestCanvas::Event* ev);

		private:
			class CMasterDlg* dialog_;
			//skin elements
			TestCanvas::PixBuf pixbuf_;
			TestCanvas::PixBuf mute_pixbuf_;
			TestCanvas::PixBuf solo_pixbuf_;
			TestCanvas::PixBuf pan_pixbuf_;
			TestCanvas::PixBuf vu_bg_pixbuf_;
			TestCanvas::PixBuf vu_peak_pixbuf_;
			TestCanvas::PixBuf vu_led_pixbuf_;
			TestCanvas::Text text_;			
			SMachineCoords MachineCoords_;
		};
	}
}