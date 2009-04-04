///\file
///\brief interface file for psycle::host::MachineGui.
#pragma once
#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psy {
	namespace core {
		class Machine;
	}
}
using namespace psy::core;
#endif

#include "canvas.hpp"

namespace psycle {
	namespace host {
		
		class MachineView;
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		class Machine;
#endif

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

		class MachineGui : public PsycleCanvas::Group {
		public:
			MachineGui(MachineView* view, Machine* mac);
			virtual ~MachineGui();

			virtual void AttachWire(class WireGui* gui);
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
			virtual void ShowDialog(double x, double y) = 0;
			virtual void UpdateVU(CDC* devc);
			virtual void UpdateText();								
			virtual void BeforeDeleteDlg();
			virtual bool OnEvent(PsycleCanvas::Event* ev);
			virtual void SetSelected(bool on);
			virtual bool IsSelected();			

			MachineView* view() { return view_; }
			Machine* mac() { return mac_; }

			virtual int preferred_width() const;
			virtual int preferred_height() const;
						
		protected:
			// helper
			bool InRect(double x,
						double y,
						double x1,
						double y1,
						double x2,
						double y2) const;
			
		private:		
			void StartDragging(double x, double y);
			void DoDragging(double x, double y);
			void StopDragging();						
			void OnMove();
			
			Machine* mac_;
			MachineView* view_;
						
			double dragging_x_;
			double dragging_y_;
			bool new_con_;
			bool dragging_;
			std::vector<WireGui*> wire_uis_;

			// selection border
			PsycleCanvas::Line sel_line_left_top_1;
			PsycleCanvas::Line sel_line_left_top_2;
			PsycleCanvas::Line sel_line_right_top_1;
			PsycleCanvas::Line sel_line_right_top_2;
			PsycleCanvas::Line sel_line_left_bottom_1;
			PsycleCanvas::Line sel_line_left_bottom_2;
			PsycleCanvas::Line sel_line_right_bottom_1;
			PsycleCanvas::Line sel_line_right_bottom_2;
		};		

	}  // namespace host
}  // namespace psycle
