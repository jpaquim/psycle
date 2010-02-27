#pragma once
#include "MachineGui.hpp"
#include "Psycle.hpp"

namespace psycle { namespace host {

class EffectGui : public MachineGui {
	public:
		EffectGui(MachineView* view, psycle::core::Machine* mac);
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
		virtual bool OnEvent(canvas::Event* ev);			

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
		canvas::PixBuf pixbuf_;
		canvas::PixBuf mute_pixbuf_;
		canvas::PixBuf bypass_pixbuf_;
		canvas::PixBuf pan_pixbuf_;
		canvas::PixBuf vu_bg_pixbuf_;
		canvas::PixBuf vu_peak_pixbuf_;
		canvas::PixBuf vu_led_pixbuf_;
		canvas::Text text_;
		SMachineCoords	MachineCoords_;
};

}}
