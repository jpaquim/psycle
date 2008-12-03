#pragma once

#include "Canvas.hpp"
#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MachineView : public TestCanvas::Canvas
		{
		public:
			MachineView(class CChildView* parent, class Song* song);
			~MachineView();

			void Rebuild();
			void SetSolo(Machine* mac);
			void UpdateVUs();
			void ShowNewMachineDlg();

			virtual void OnEvent(TestCanvas::Event* ev);

			CChildView* child_view() { return parent_; }
			Song* song() { return song_; }

		private:
			void InitSkin();
			void BuildWires();
			void CreateMachineGui(Machine* mac);
			
			CChildView* parent_;
			std::map<Machine*, MachineGui*> gui_map_;
			Song* song_;			
			CBitmap machineskin;
			CBitmap machineskinmask;
			CBitmap machinebkg;
			HBITMAP hbmMachineSkin;
			HBITMAP hbmMachineBkg;	
			HBITMAP hbmMachineDial;	
			SMachineCoords	MachineCoords;
		};
	}
}

