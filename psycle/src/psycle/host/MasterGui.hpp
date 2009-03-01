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
			void UpdateVU(CDC* devc);

		protected:
			virtual void ShowDialog(double x, double y);
			bool OnEvent(PsycleCanvas::Event* ev);

		private:
			class CMasterDlg* dialog_;
			//skin elements
			PsycleCanvas::PixBuf pixbuf_;
			PsycleCanvas::PixBuf mute_pixbuf_;
			PsycleCanvas::PixBuf solo_pixbuf_;
			PsycleCanvas::PixBuf pan_pixbuf_;
			PsycleCanvas::PixBuf vu_bg_pixbuf_;
			PsycleCanvas::PixBuf vu_peak_pixbuf_;
			PsycleCanvas::PixBuf vu_led_pixbuf_;
			PsycleCanvas::Text text_;			
			SMachineCoords MachineCoords_;
		};
	}
}