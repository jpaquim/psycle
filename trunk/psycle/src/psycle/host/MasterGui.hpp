#pragma once
#include "Psycle.hpp"
#include "MachineGui.hpp"

namespace psycle { namespace host {

class MasterDlg;

class MasterGui : public MachineGui {
	public:
		MasterGui(MachineView* view, Machine* mac);
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
		bool OnEvent(canvas::Event* ev);

	private:
		class CMasterDlg* dialog_;
		//skin elements
		canvas::PixBuf pixbuf_;
		canvas::PixBuf mute_pixbuf_;
		canvas::PixBuf solo_pixbuf_;
		canvas::PixBuf pan_pixbuf_;
		canvas::PixBuf vu_bg_pixbuf_;
		canvas::PixBuf vu_peak_pixbuf_;
		canvas::PixBuf vu_led_pixbuf_;
		canvas::Text text_;			
		SMachineCoords MachineCoords_;
};

}}
