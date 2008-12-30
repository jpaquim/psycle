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
			bool bHasTransparency;
			COLORREF cTransparency;
		};

		class MachineGui : public TestCanvas::Group {
		public:
			MachineGui(class MachineView* view,
			           class Machine* mac);
			virtual ~MachineGui();

			virtual void AttachWire(class WireGui* gui, int point);
			virtual void DetachWire(class WireGui* gui);
			virtual void RemoveWires();
			virtual void SetSkin(const SMachineCoords&	MachineCoords,
						 CBitmap* machineskin,
						 CBitmap* machineskinmask,
						 CBitmap* machinebkg,
						 HBITMAP hbmMachineSkin,
						 HBITMAP hbmMachineBkg,
						 HBITMAP hbmMachineDial,
						 const CFont& font,
						 COLORREF font_color) = 0;

			virtual void SetMute(bool mute) = 0;
			virtual void SetSolo(bool mute) = 0;
			virtual void SetBypass(bool mute) = 0;
			virtual void UpdateVU();
			virtual void UpdateText();
						
			virtual void BeforeDeleteDlg();
			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void SetSelected(bool on) = 0;
			virtual bool IsSelected() const = 0;
			void DoMacPropDialog();

			MachineView* view() { return view_; }
			Machine* mac() { return mac_; };
			
			virtual void ShowDialog(double x, double y) = 0;

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
			void OnMove();
			
			Machine* mac_;
			MachineView* view_;
						
			double dragging_x_;
			double dragging_y_;
			bool new_con_;
			bool dragging_;
			std::vector< std::pair<WireGui*, int> > wire_uis_;
						
		};
		

	}  // namespace host
}  // namespace psycle
