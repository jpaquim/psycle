///\file
///\brief interface file for psycle::host::MachineGui.
#pragma once

#include "canvas.hpp"

namespace psycle {
	namespace host {
		

		class SSkinSource
		{
		public:
			int x;
			int y;
			int width;
			int height;
		};
		
		class SSkinDest
		{
		public:
			int x;
			int y;
		};

		class SMachineCoords
		{
		public:
			SSkinSource sMaster;
			SSkinSource sGenerator;
			SSkinSource sGeneratorVu0;
			SSkinSource sGeneratorVuPeak;
			SSkinSource sGeneratorPan;
			SSkinSource sGeneratorMute;
			SSkinSource sGeneratorSolo;
			SSkinSource sEffect;
			SSkinSource sEffectVu0;
			SSkinSource sEffectVuPeak;
			SSkinSource sEffectPan;
			SSkinSource sEffectMute;
			SSkinSource sEffectBypass;
			SSkinSource dGeneratorVu;
			SSkinSource dGeneratorPan;
			SSkinDest dGeneratorMute;
			SSkinDest dGeneratorSolo;
			SSkinDest dGeneratorName;
			SSkinSource dEffectVu;
			SSkinSource dEffectPan;
			SSkinDest dEffectMute;
			SSkinDest dEffectBypass;
			SSkinDest dEffectName;
			BOOL bHasTransparency;
			COLORREF cTransparency;
		};

		class MachineGui : public TestCanvas::Group {
		public:
			MachineGui(class MachineView* view,
			           class Machine* mac);
			~MachineGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			void SetSkin(const SMachineCoords&	MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& generator_font,
						 COLORREF generator_font_color,
						 const CFont& effect_font,
						 COLORREF effect_font_color);

			void SetMute(bool mute);
			void SetSolo(bool mute);
			void UpdateVU();

			void AttachWire(class WireGui* gui, int point);

		private:		
			void dragging_start(double x, double y);
			void dragging(double x, double y);
			void dragging_stop();
			bool InRect(double x,
						double y,
						double x1,
						double y1,
						double x2,
						double y2) const;
			bool TestMute(double x, double y);
			bool TestSolo(double x, double y);
			void OnMove();
			void UpdatePan();
			void UpdateText();
			
			Machine* mac_;
			//TestCanvas::Rect rect_;
			TestCanvas::PixBuf pixbuf_;
			TestCanvas::PixBuf mute_pixbuf_;
			TestCanvas::PixBuf solo_pixbuf_;
			TestCanvas::PixBuf pan_pixbuf_;
			TestCanvas::PixBuf vu_bg_pixbuf_;
			TestCanvas::PixBuf vu_peak_pixbuf_;
			TestCanvas::PixBuf vu_led_pixbuf_;
			TestCanvas::Text text_;
			double dragging_x_;
			double dragging_y_;
			bool dragging_;
			std::vector< std::pair<WireGui*, int> > wire_uis_;
			
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

