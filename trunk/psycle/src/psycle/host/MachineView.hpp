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

			void OnNewConnection(MachineGui* sender);
			void OnRewireEnd(WireGui* sender,
							 double x,
							 double y,
							 int picker);

		private:
			void InitSkin();
			void BuildWires();
			void WireUp(WireGui* sender,
						MachineGui* connect_to_gui,
						double x,
						double y,
						int picker);
			void CreateMachineGui(Machine* mac);
			
			CChildView* parent_;
			std::map<Machine*, MachineGui*> gui_map_;
			Song* song_;
			WireGui* del_line_;
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

