#pragma once

#include "Canvas.hpp"
#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MachineView : public PsycleCanvas::Canvas
		{
		public:
#ifdef use_psycore
			MachineView(class CChildView* parent, class CMainFrame* main);
			void SetSong(class psy::core::Song* song);
#else
			MachineView(class CChildView* parent, class CMainFrame* main, class Song* song);
#endif
			~MachineView();

			void Rebuild();
#ifdef use_psycore
			void SetSolo(psy::core::Machine* mac);
			void ShowNewMachineDlg(double x , double y, psy::core::Machine* mac, bool from_event);
			// use this, if you want to delete a gui not called from 
			// the machinegui event, that will be deleted
			// (used e.g. in CGearRack)
			void DeleteMachineGui(psy::core::Machine* mac);
			void SetDeleteMachineGui(psy::core::Machine* mac, bool in_engine);
			MachineGui* CreateMachineGui(psy::core::Machine* mac);
			// from_event has to be set to true, if you show the macprop dialog from inside
			// the machinegui OnEvent. In case of replace and delete this is important
			void DoMacPropDialog(psy::core::Machine* mac, bool from_event);
			void ShowDialog(psy::core::Machine* mac, double x, double y);
			void UpdatePosition(psy::core::Machine* mac);
#else
			void SetSolo(Machine* mac);
			void ShowNewMachineDlg(double x , double y, Machine* mac, bool from_event);
			// use this, if you want to delete a gui not called from 
			// the machinegui event, that will be deleted
			// (used e.g. in CGearRack)
			void DeleteMachineGui(Machine* mac);
			void SetDeleteMachineGui(Machine* mac, bool in_engine);
			MachineGui* CreateMachineGui(Machine* mac);
			// from_event has to be set to true, if you show the macprop dialog from inside
			// the machinegui OnEvent. In case of replace and delete this is important
			void DoMacPropDialog(Machine* mac, bool from_event);
			void ShowDialog(Machine* mac, double x, double y);
			void UpdatePosition(Machine* mac);
#endif
			void UpdateVUs(CDC* devc);			
			void SelectMachine(MachineGui* gui);
			virtual void OnEvent(PsycleCanvas::Event* ev);
			CChildView* child_view() { return parent_; }
			CMainFrame* main();
#ifdef use_psycore
			psy::core::Song* song() { return song_; }
#else
			Song* song() { return song_; }
#endif

			void OnNewConnection(MachineGui* sender);
			void OnRewireEnd(WireGui* sender,
							 double x,
							 double y,
							 int picker);
			void OnWireRewire(WireGui* sender, int pick_point);
			void LockVu();
			void UnlockVu();

			// use the next both, if you want to delete a machingui itself from
			// inside a machingui event (like used in macprop dialog)
			void SetDeleteMachineGui(MachineGui* gui, bool in_engine) {
				del_machine_ = gui;
				del_in_engine_ = in_engine;
			}
			void InitSkin();
			void WriteStatusBar(const std::string& text);
			void AddMacViewUndo(); // place holder
			void CenterMaster();
			bool CheckUnsavedSong();

		private:			
			void BuildWires();
			void WireUp(WireGui* sender,
						MachineGui* connect_to_gui,
						double x,
						double y,
						int picker);

#ifdef use_psycore
			bool RewireSrc(psy::core::Machine* tmac, psy::core::Machine* dmac);
			bool RewireDest(psy::core::Machine* tmac, psy::core::Machine* dmac);
#else
			bool RewireSrc(Machine* tmac, Machine* dmac);
			bool RewireDest(Machine* tmac, Machine* dmac);			
#endif
			void RaiseMachinesToTop();
			void UpdateSoloMuteBypass();
			void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
			void LoadMachineBackground();
			void FindMachineSkin(CString findDir, CString findName, BOOL *result);
			
			CChildView* parent_;
			CMainFrame* main_;
#ifdef use_psycore		
			psy::core::Song* song_;
			std::map<psy::core::Machine*, MachineGui*> gui_map_;
#else
			Song* song_;
			std::map<Machine*, MachineGui*> gui_map_;
#endif

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
		public:
			SMachineCoords	MachineCoords;
		private:
			int bkgx;
			int bkgy;

		};
	}
}

