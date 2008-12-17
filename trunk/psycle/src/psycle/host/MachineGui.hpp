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

			virtual void SetSkin(const SMachineCoords&	MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color);

			void SetMute(bool mute);
			void SetSolo(bool mute);
			virtual void UpdateVU();

			void AttachWire(class WireGui* gui, int point);
			void DetachWire(class WireGui* gui);

			MachineView* view();
			Machine* mac() { return mac_; };

			virtual void BeforeDeleteDlg();

			virtual void SetSelected(bool on);
			virtual bool IsSelected() const;

		protected:
			// helper
			bool InRect(double x,
						double y,
						double x1,
						double y1,
						double x2,
						double y2) const;
		private:		
			void dragging_start(double x, double y);
			void dragging(double x, double y);
			void dragging_stop();			
			//bool TestMute(double x, double y);
			//bool TestSolo(double x, double y);
			void OnMove();
			void UpdatePan();
			
			
			Machine* mac_;
			MachineView* view_;
						
			double dragging_x_;
			double dragging_y_;
			bool dragging_;
			std::vector< std::pair<WireGui*, int> > wire_uis_;
						
		};
		

	}  // namespace host
}  // namespace psycle
