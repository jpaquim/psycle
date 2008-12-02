#pragma once

#include "Canvas.hpp"
#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MachineView : public TestCanvas::Canvas
		{
		public:
			MachineView(CWnd* parent, class Song* song);
			~MachineView();

			void Rebuild();
			void SetSolo(Machine* mac);

		private:
			void InitSkin();
			void BuildWires();
			
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

