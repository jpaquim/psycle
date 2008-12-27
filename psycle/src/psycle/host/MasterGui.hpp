#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MasterGui : public MachineGui {
		public:
			MasterGui(class MachineView* view,
					  class Machine* mac);
			~MasterGui();

			virtual bool OnEvent(TestCanvas::Event* ev);
			virtual void SetSkin(const SMachineCoords& MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color);

		private:
			void ShowDialog(double x, double y);

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
			// skin
			CBitmap* machineskin_;
			CBitmap* machineskinmask_;
			CBitmap* machinebkg_;
			HBITMAP hbmMachineSkin_;
			HBITMAP hbmMachineBkg_;	
			HBITMAP hbmMachineDial_;	
			SMachineCoords MachineCoords_;
		};
	}
}