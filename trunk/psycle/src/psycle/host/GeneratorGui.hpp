#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class GeneratorGui : public MachineGui {
		public:
			GeneratorGui(class MachineView* view,
					  class Machine* mac);
			~GeneratorGui();

			virtual void SetSkin(const SMachineCoords&	MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color);
			virtual void UpdateVU();

			virtual void BeforeDeleteDlg();
			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void SetSelected(bool on);
			virtual bool IsSelected() const;
			void SetMute(bool mute);
			void SetSolo(bool mute);
			void SetBypass(bool mute){}

		protected:
			virtual void ShowDialog(double x, double y);
		private:
			void UpdateText();
			void UpdatePan();
			bool TestMute(double x, double y);
			bool TestSolo(double x, double y);
			bool TestPan(double x, double y);
			void DoPanDragging(double x, double y);

			class CFrameMachine* dialog_;
			bool pan_dragging_;
			
			TestCanvas::Line sel_line_left_top_1;
			TestCanvas::Line sel_line_left_top_2;
			TestCanvas::Line sel_line_right_top_1;
			TestCanvas::Line sel_line_right_top_2;
			TestCanvas::Line sel_line_left_bottom_1;
			TestCanvas::Line sel_line_left_bottom_2;
			TestCanvas::Line sel_line_right_bottom_1;
			TestCanvas::Line sel_line_right_bottom_2;
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