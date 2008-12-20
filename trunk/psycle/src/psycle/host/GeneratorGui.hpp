#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class GeneratorGui : public MachineGui {
		public:
			GeneratorGui(class MachineView* view,
					  class Machine* mac);
			~GeneratorGui();

			virtual bool OnEvent(TestCanvas::Event* ev);
			virtual void BeforeDeleteDlg();
			virtual void SetSelected(bool on);
			virtual bool IsSelected() const;
			virtual void UpdateVU();

			virtual void SetSkin(const SMachineCoords&	MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color);
			virtual void SetMute(bool mute);
			virtual void SetSolo(bool mute);

		private:
			void ShowDialog();
			void UpdateText();
			void UpdatePan();
			bool TestMute(double x, double y);
			bool TestPan(double x, double y);
			void DoPanDragging(double x, double y);
			bool TestSolo(double x, double y);

			bool pan_dragging_;
			class CFrameMachine* dialog_;
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
			CBitmap* machineskin_;
			CBitmap* machineskinmask_;
			CBitmap* machinebkg_;
			HBITMAP hbmMachineSkin_;
			HBITMAP hbmMachineBkg_;	
			HBITMAP hbmMachineDial_;	
			SMachineCoords	MachineCoords_;
		};
	}
}