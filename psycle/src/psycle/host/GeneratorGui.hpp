#pragma once
#include "Psycle.hpp"
#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class GeneratorGui : public MachineGui {
		public:
			GeneratorGui(MachineView* view, psycle::core::Machine* mac);
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
			virtual void UpdateVU(CDC* devc);
			virtual void UpdateText();

			virtual void BeforeDeleteDlg();
			virtual bool OnEvent(PsycleCanvas::Event* ev);

			void SetMute(bool mute);
			void SetSolo(bool mute);
			void SetBypass(bool mute){}

			virtual int preferred_width() const;
			virtual int preferred_height() const;

		protected:
			virtual void ShowDialog(double x, double y);
		private:			
			void UpdatePan();
			bool TestMute(double x, double y);
			bool TestSolo(double x, double y);
			bool TestPan(double x, double y);
			void DoPanDragging(double x, double y);

			class CFrameMachine* dialog_;
			bool pan_dragging_;
			
			
			//skin elements
			PsycleCanvas::PixBuf pixbuf_;
			PsycleCanvas::PixBuf mute_pixbuf_;
			PsycleCanvas::PixBuf solo_pixbuf_;
			PsycleCanvas::PixBuf pan_pixbuf_;
			PsycleCanvas::PixBuf vu_bg_pixbuf_;
			PsycleCanvas::PixBuf vu_peak_pixbuf_;
			PsycleCanvas::PixBuf vu_led_pixbuf_;
			PsycleCanvas::Text text_;
			// skin
			SMachineCoords	MachineCoords_;
		};
	}
}
