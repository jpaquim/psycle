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
			void UpdateVUs(CDC* devc);			
			// from_event has to be set to true, if you show the macprop dialog from inside
			// the machinegui OnEvent. In case of replace and delete this is important
			void ShowNewMachineDlg(double x , double y, Machine* mac, bool from_event);
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
			void OnWireRewire(WireGui* sender, int pick_point);
			void LockVu();
			void UnlockVu();
			// use this, if you want to delete a gui not called from 
			// the machinegui event, that will be deleted
			// (used e.g. in CGearRack)
			void DeleteMachineGui(Machine* mac);
			// use the next both, if you want to delete a machingui itself from
			// inside a machingui event (like used in macprop dialog)
			void SetDeleteMachineGui(MachineGui* gui, bool in_engine) {
				del_machine_ = gui;
				del_in_engine_ = in_engine;
			}
			void SetDeleteMachineGui(Machine* mac, bool in_engine);
			void InitSkin();
			MachineGui* CreateMachineGui(Machine* mac);
			void DoMacPropDialog(Machine* mac, bool from_event);
			void ShowDialog(Machine* mac, double x, double y);
			void UpdatePosition(Machine* mac);
			void WriteStatusBar(const std::string& text);

		private:			
			void BuildWires();
			void WireUp(WireGui* sender,
						MachineGui* connect_to_gui,
						double x,
						double y,
						int picker);
			bool RewireSrc(Machine* tmac, Machine* dmac);
			bool RewireDest(Machine* tmac, Machine* dmac);			
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
			WireGui* rewire_line_;
			MachineGui* del_machine_;
			bool is_locked_;
			bool del_in_engine_;
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

