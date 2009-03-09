#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class EffectGui : public MachineGui {
		public:
			EffectGui(class MachineView* view,
#ifdef use_psycore							 
					  class psy::core::Machine* mac);
#else
					  class Machine* mac);
#endif
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
			virtual bool OnEvent(PsycleCanvas::Event* ev);			

			virtual void UpdateVU(CDC* devc);
			virtual void UpdateText();
			virtual void SetBypass(bool on);
			virtual void SetMute(bool on);
			virtual void SetSolo(bool on){}
			virtual void UpdatePan();

			virtual int preferred_width() const;
			virtual int preferred_height() const;

		protected:
			virtual void ShowDialog(double x, double y);

		private:						
			bool TestPan(double x, double y);
			bool InMute(double x, double y);
			bool InBypass(double x, double y);
			void DoPanDragging(double x, double y);

			bool pan_dragging_;
			class CFrameMachine* dialog_;
			//skin elements
			PsycleCanvas::PixBuf pixbuf_;
			PsycleCanvas::PixBuf mute_pixbuf_;
			PsycleCanvas::PixBuf bypass_pixbuf_;
			PsycleCanvas::PixBuf pan_pixbuf_;
			PsycleCanvas::PixBuf vu_bg_pixbuf_;
			PsycleCanvas::PixBuf vu_peak_pixbuf_;
			PsycleCanvas::PixBuf vu_led_pixbuf_;
			PsycleCanvas::Text text_;
			SMachineCoords	MachineCoords_;
		};
	}
}