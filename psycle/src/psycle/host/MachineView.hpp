// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Canvas.hpp"
#include "MachineGui.hpp" //For SMachineCoords
#include "Psycle.hpp"

namespace psycle {
namespace core {
	class Song;
	class Machine;
}
namespace host {

class CChildView;
class CMainFrame;
class WireGui;

class MachineView : public canvas::Canvas {
	public:
		MachineView(class Project* project);
		~MachineView();

		void SetParent(CChildView* parent, CMainFrame* main) {
			parent_ = parent;
			main_ = main;
			canvas::Canvas::SetParent((CWnd*)parent_);
		}

		void Rebuild();
		void SetSolo(psycle::core::Machine* mac);
		void ShowNewMachineDlg(double x , double y, Machine* mac, bool from_event);
		// use this, if you want to delete a gui not called from 
		// the machinegui event, that will be deleted
		// (used e.g. in CGearRack)
		void DeleteMachineGui(psycle::core::Machine* mac);
		void SetDeleteMachineGui(psycle::core::Machine* mac, bool in_engine);
		MachineGui* CreateMachineGui(psycle::core::Machine* mac);
		// from_event has to be set to true, if you show the macprop dialog from inside
		// the machinegui OnEvent. In case of replace and delete this is important
		void DoMacPropDialog(psycle::core::Machine* mac, bool from_event);
		void ShowDialog(psycle::core::Machine* mac, double x, double y);
		void UpdatePosition(psycle::core::Machine* mac);
		void UpdateVUs(CDC* devc);			
		void SelectMachine(MachineGui* gui);
		virtual void OnEvent(canvas::Event* ev);
		CChildView* child_view() { return parent_; }
		CMainFrame* main() { return main_; }

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
		void CenterMaster();
		bool CheckUnsavedSong();
		psycle::core::Song* song();

		SMachineCoords	MachineCoords;			

	private:			
		void BuildWires();
		void WireUp(WireGui* sender,
					MachineGui* connect_to_gui,
					double x,
					double y,
					int picker);
		bool RewireSrc(psycle::core::Machine* tmac, psycle::core::Machine* dmac);
		bool RewireDest(psycle::core::Machine* tmac, psycle::core::Machine* dmac);			
		void RaiseMachinesToTop();
		void UpdateSoloMuteBypass();
		void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
		void LoadMachineBackground();
		void FindMachineSkin(CString findDir, CString findName, BOOL *result);
		
		CChildView* parent_;
		CMainFrame* main_;
		Project* project_;
		std::map<psycle::core::Machine*, MachineGui*> gui_map_;
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
		int bkgx;
		int bkgy;
};

}}
