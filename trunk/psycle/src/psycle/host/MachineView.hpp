#pragma once

#include "Canvas.hpp"
#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MachineView : public TestCanvas::Canvas
		{
		public:
			MachineView(class CChildView* parent, class CMainFrame* main, class Song* song);
			~MachineView();

			void Rebuild();
			void SetSolo(Machine* mac);			
			void UpdateVUs();			
			void ShowNewMachineDlg();
			void SelectMachine(MachineGui* gui);

			virtual void OnEvent(TestCanvas::Event* ev);

			CChildView* child_view() { return parent_; }
			CMainFrame* main();
			Song* song() { return song_; }

			void OnNewConnection(MachineGui* sender);
			void OnRewireEnd(WireGui* sender,
							 double x,
							 double y,
							 int picker);

			void LockVu();
			void UnlockVu();

			void SetDeleteMachineGui(MachineGui* gui) {
				del_machine_ = gui;
			}

			void InitSkin();

		private:			
			void BuildWires();
			void WireUp(WireGui* sender,
						MachineGui* connect_to_gui,
						double x,
						double y,
						int picker);
			void CreateMachineGui(Machine* mac);
			void RaiseMachinesToTop();
			void UpdateSoloMuteBypass();
			void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
			void LoadMachineBackground();
			void FindMachineSkin(CString findDir, CString findName, BOOL *result);
			
			CChildView* parent_;
			CMainFrame* main_;
			std::map<Machine*, MachineGui*> gui_map_;
			Song* song_;
			WireGui* del_line_;
			MachineGui* del_machine_;
			bool is_locked_;
			CBitmap machineskin;
			CBitmap machineskinmask;
			CBitmap machinebkg;
			HBITMAP hbmMachineSkin;
			HBITMAP hbmMachineBkg;	
			HBITMAP hbmMachineDial;	
			SMachineCoords	MachineCoords;
			int bkgx;
			int bkgy;
		};
	}
}

