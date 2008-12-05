#include "MachineView.hpp"
#include "Global.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "NativeGui.hpp"
#include "Machine.hpp"
#include "WireGui.hpp"
#include "MainFrm.hpp"
#include "NewMachine.hpp"
#include "Mastergui.hpp"

namespace psycle {
	namespace host {

		MachineView::MachineView(CChildView* parent, Song* song)
			: TestCanvas::Canvas(parent),
			  parent_(parent),
			  song_(song)
		{
			set_bg_color(Global::pConfig->mv_colour);
			InitSkin();
		}

		MachineView::~MachineView()
		{
		}

		void MachineView::SetSolo(Machine* tmac) {
		}

		void MachineView::UpdateVUs()
		{
			std::map<Machine*, MachineGui*>::iterator it = gui_map_.begin();
			for ( ; it != gui_map_.end(); ++it ) {
				(*it).second->UpdateVU();
			}
		}

		void MachineView::OnEvent(TestCanvas::Event* ev)
		{
			TestCanvas::Canvas::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				if ( !root()->intersect(ev->x, ev->y) ) {
					ShowNewMachineDlg();
				}
			}
		}

		void MachineView::ShowNewMachineDlg()
		{
			CNewMachine dlg;
			if ((dlg.DoModal() == IDOK) && (dlg.Outputmachine >= 0)) {
				// AddMacViewUndo();
				int fb,xs,ys;
				if (dlg.selectedMode == modegen)  {
					fb = Global::_pSong->GetFreeBus();
					xs = MachineCoords.sGenerator.width;
					ys = MachineCoords.sGenerator.height;
				}	else {
					fb = Global::_pSong->GetFreeFxBus();
					xs = MachineCoords.sEffect.width;
					ys = MachineCoords.sEffect.height;
				}
				if ( fb == -1) {
					// MessageBox("Machine Creation Failed","Error!",MB_OK);
				}
				int CW = 1024;
				int CH = 768;
				int x = (rand())%(CW-xs);
				int y = (rand())%(CH-ys);
				bool created=false;
				if (Global::_pSong->_pMachine[fb] ) {
					created = Global::_pSong->ReplaceMachine(Global::_pSong->_pMachine[fb],(MachineType)dlg.Outputmachine, x, y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
				}
				else  {
					created = Global::_pSong->CreateMachine((MachineType)dlg.Outputmachine, x, y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
				}
				if (!created) {
					// MessageBox("Machine Creation Failed","Error!",MB_OK);
				} else {
					Machine* mac = song_->_pMachine[fb];					
					CreateMachineGui(mac);
				}
			}
		}

		void MachineView::Rebuild()
		{
			root()->Clear();
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
				if (song_->_pMachine[idx]) {
					CreateMachineGui(song_->_pMachine[idx]);
				}
			}
			BuildWires();
		}

		void MachineView::CreateMachineGui(Machine* mac) {
			assert(mac);
			MachineGui* gui;
			switch ( mac->_mode ) {
				case MACHMODE_MASTER:
					gui = new MasterGui(this, mac);
				break;
				default:
					gui = new MachineGui(this, mac);
			}
			gui_map_[mac] = gui;
			gui->SetSkin(MachineCoords,
						 &machineskin,
						 &machineskinmask,
						 &machinebkg,
						 hbmMachineSkin,
						 hbmMachineBkg,
						 hbmMachineDial,
						 Global::pConfig->generatorFont,
						 Global::pConfig->mv_generator_fontcolour,
						 Global::pConfig->effectFont,
						 Global::pConfig->mv_effect_fontcolour);
			if ( mac->_mute )
				gui->SetMute(true);
			else if (song_->machineSoloed == mac->_macIndex)
				gui->SetSolo(true);
			gui->set_manage(true);
		}

		void MachineView::BuildWires()
		{
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
				Machine* mac = (song_->_pMachine[idx]);
				if (mac) {
					for (int w=0; w<MAX_CONNECTIONS; w++) {
						if (mac->_connection[w]) {
							Machine* pout = song_->_pMachine[mac->_outputMachines[w]];
							if (pout)
							{
								std::map<Machine*, MachineGui*>::iterator fromIt = 
									gui_map_.find(mac);
								std::map<Machine*, MachineGui*>::iterator toIt = 
									gui_map_.find(pout);
								WireGui* wireUi = new WireGui(this);
								wireUi->set_manage(true);
								fromIt->second->AttachWire(wireUi,0);
								toIt->second->AttachWire(wireUi,1);
								wireUi->setGuiConnectors(fromIt->second,
														 toIt->second,
														 0);
								root()->Add(wireUi);
								wireUi->UpdatePosition();
							}
						}
					}
				}
			}
		}

		void MachineView::OnNewConnection(MachineGui* sender)
		{
			WireGui* line = new WireGui(this);
			line->SetStart(sender);
			double x1, y1, x2, y2;
			sender->GetBounds( x1, y1, x2, y2);
			double midW = (x2 - x1) / 2;
			double midH = (y2 - y1) / 2;
			TestCanvas::Group* fromParent = sender->parent();
			double x3, y3, x4, y4;
			fromParent->GetBounds(x3, y3, x4, y4);
			double x = x1+x3;
			double y = y1+y3;
			TestCanvas::Line::Points points(2);
			points[0] = std::pair<double, double>(x + midW, y + midH);
			points[1] = std::pair<double, double>(x + midW, y + midH);
			line->SetPoints(points);
			line->dragging_start(1);
		}

		void MachineView::OnRewireEnd(WireGui* sender,
									  double x,
									  double y,
									  int picker)
		{
			MachineGui* connect_to_gui = 0;
			std::map<Machine*,MachineGui*>::iterator it = gui_map_.begin();
			for ( ; it != gui_map_.end(); ++it ) {
				MachineGui* gui = (*it).second;
				if ( gui->intersect(x, y) ) {
					connect_to_gui = gui;
					break;
				}
			}
			WireUp(sender, connect_to_gui, x, y, picker);
		}

		void MachineView::WireUp(WireGui* sender,
								 MachineGui* connect_to_gui,
								 double x,
								 double y,
								 int picker)
		{
			MachineGui* connect_from_gui = sender->start();
			Machine* tmac = connect_from_gui->mac();
			Machine* dmac = connect_to_gui->mac();
			int dsttype=0;
			if (song_->InsertConnection(tmac, dmac,0,dsttype)== -1) {
				//MessageBox("Couldn't connect the selected machines!","Error!", MB_ICONERROR);				
			} else {
				connect_from_gui->AttachWire(sender,0);
				connect_to_gui->AttachWire(sender,1);
				sender->setGuiConnectors(connect_from_gui, connect_to_gui, 0);
				sender->set_manage(true);
				sender->UpdatePosition();
			}			
		}

		void MachineView::InitSkin()
		{
			std::string szOld;
			CNativeGui::uiSetting().LoadMachineDial();
			if (!Global::pConfig->machine_skin.empty())
			{
				szOld = Global::pConfig->machine_skin;
				#if defined PSYCLE__CONFIGURATION__SKIN__UGLY_DEFAULT
					MachineCoords.sMaster.x = 0;
					MachineCoords.sMaster.y = 0;
					MachineCoords.sMaster.width = 148;
					MachineCoords.sMaster.height = 48;

					MachineCoords.sGenerator.x = 0;
					MachineCoords.sGenerator.y = 48;
					MachineCoords.sGenerator.width = 148;
					MachineCoords.sGenerator.height = 48;
					MachineCoords.sGeneratorVu0.x = 0;
					MachineCoords.sGeneratorVu0.y = 144;
					MachineCoords.sGeneratorVu0.width = 6;
					MachineCoords.sGeneratorVu0.height = 5;
					MachineCoords.sGeneratorVuPeak.x = 96;
					MachineCoords.sGeneratorVuPeak.y = 144;
					MachineCoords.sGeneratorVuPeak.width = 6;
					MachineCoords.sGeneratorVuPeak.height = 5;
					MachineCoords.sGeneratorPan.x = 21;
					MachineCoords.sGeneratorPan.y = 149;
					MachineCoords.sGeneratorPan.width = 24;
					MachineCoords.sGeneratorPan.height = 9;
					MachineCoords.sGeneratorMute.x = 7;
					MachineCoords.sGeneratorMute.y = 149;
					MachineCoords.sGeneratorMute.width = 7;
					MachineCoords.sGeneratorMute.height = 7;
					MachineCoords.sGeneratorSolo.x = 14;
					MachineCoords.sGeneratorSolo.y = 149;
					MachineCoords.sGeneratorSolo.width = 7;
					MachineCoords.sGeneratorSolo.height = 7;

					MachineCoords.sEffect.x = 0;
					MachineCoords.sEffect.y = 96;
					MachineCoords.sEffect.width = 148;
					MachineCoords.sEffect.height = 48;
					MachineCoords.sEffectVu0.x = 0;
					MachineCoords.sEffectVu0.y = 144;
					MachineCoords.sEffectVu0.width = 6;
					MachineCoords.sEffectVu0.height = 5;
					MachineCoords.sEffectVuPeak.x = 96;
					MachineCoords.sEffectVuPeak.y = 144;
					MachineCoords.sEffectVuPeak.width = 6;
					MachineCoords.sEffectVuPeak.height = 5;
					MachineCoords.sEffectPan.x = 21;
					MachineCoords.sEffectPan.y = 149;
					MachineCoords.sEffectPan.width = 24;
					MachineCoords.sEffectPan.height = 9;
					MachineCoords.sEffectMute.x = 7;
					MachineCoords.sEffectMute.y = 149;
					MachineCoords.sEffectMute.width = 7;
					MachineCoords.sEffectMute.height = 7;
					MachineCoords.sEffectBypass.x = 0;
					MachineCoords.sEffectBypass.y = 149;
					MachineCoords.sEffectBypass.width = 7;
					MachineCoords.sEffectBypass.height = 12;

					MachineCoords.dGeneratorVu.x = 8;
					MachineCoords.dGeneratorVu.y = 3;
					MachineCoords.dGeneratorVu.width = 96;
					MachineCoords.dGeneratorVu.height = 0;
					MachineCoords.dGeneratorPan.x = 3;
					MachineCoords.dGeneratorPan.y = 35;
					MachineCoords.dGeneratorPan.width = 117;
					MachineCoords.dGeneratorPan.height = 0;
					MachineCoords.dGeneratorMute.x = 137;
					MachineCoords.dGeneratorMute.y = 4;
					MachineCoords.dGeneratorSolo.x = 137;
					MachineCoords.dGeneratorSolo.y = 17;
					MachineCoords.dGeneratorName.x = 10;
					MachineCoords.dGeneratorName.y = 12;

					MachineCoords.dEffectVu.x = 8;
					MachineCoords.dEffectVu.y = 3;
					MachineCoords.dEffectVu.width = 96;
					MachineCoords.dEffectVu.height = 0;
					MachineCoords.dEffectPan.x = 3;
					MachineCoords.dEffectPan.y = 35;
					MachineCoords.dEffectPan.width = 117;
					MachineCoords.dEffectPan.height = 0;
					MachineCoords.dEffectMute.x = 137;
					MachineCoords.dEffectMute.y = 4;
					MachineCoords.dEffectBypass.x = 137;
					MachineCoords.dEffectBypass.y = 15;
					MachineCoords.dEffectName.x = 10;
					MachineCoords.dEffectName.y = 12;
					MachineCoords.bHasTransparency = FALSE;
				#else
					MachineCoords.sMaster.x = 0;
					MachineCoords.sMaster.y = 0;
					MachineCoords.sMaster.width = 148;
					MachineCoords.sMaster.height = 47;//48;

					MachineCoords.sGenerator.x = 0;
					MachineCoords.sGenerator.y = 47;//48;
					MachineCoords.sGenerator.width = 148;
					MachineCoords.sGenerator.height = 47;//48;
					MachineCoords.sGeneratorVu0.x = 0;
					MachineCoords.sGeneratorVu0.y = 141;//144;
					MachineCoords.sGeneratorVu0.width = 7;//6;
					MachineCoords.sGeneratorVu0.height = 4;//5;
					MachineCoords.sGeneratorVuPeak.x = 128;//96;
					MachineCoords.sGeneratorVuPeak.y = 141;//144;
					MachineCoords.sGeneratorVuPeak.width = 2;//6;
					MachineCoords.sGeneratorVuPeak.height = 4;//5;
					MachineCoords.sGeneratorPan.x = 45;//102;
					MachineCoords.sGeneratorPan.y = 145;//144;
					MachineCoords.sGeneratorPan.width = 16;//24;
					MachineCoords.sGeneratorPan.height = 5;//9;
					MachineCoords.sGeneratorMute.x = 0;//133;
					MachineCoords.sGeneratorMute.y = 145;//144;
					MachineCoords.sGeneratorMute.width = 15;//7;
					MachineCoords.sGeneratorMute.height = 14;//7;
					MachineCoords.sGeneratorSolo.x = 15;//140;
					MachineCoords.sGeneratorSolo.y = 145;//144;
					MachineCoords.sGeneratorSolo.width = 15;//7;
					MachineCoords.sGeneratorSolo.height = 14;//7;

					MachineCoords.sEffect.x = 0;
					MachineCoords.sEffect.y = 94;//96;
					MachineCoords.sEffect.width = 148;
					MachineCoords.sEffect.height = 47;//48;
					MachineCoords.sEffectVu0.x = 0;
					MachineCoords.sEffectVu0.y = 141;//144;
					MachineCoords.sEffectVu0.width = 7;//6;
					MachineCoords.sEffectVu0.height = 4;//5;
					MachineCoords.sEffectVuPeak.x = 128;//96;
					MachineCoords.sEffectVuPeak.y = 141;//144;
					MachineCoords.sEffectVuPeak.width = 2;//6;
					MachineCoords.sEffectVuPeak.height = 4;//5;
					MachineCoords.sEffectPan.x = 45;//102;
					MachineCoords.sEffectPan.y = 145;//144;
					MachineCoords.sEffectPan.width = 16;//24;
					MachineCoords.sEffectPan.height = 5;//9;
					MachineCoords.sEffectMute.x = 0;//133;
					MachineCoords.sEffectMute.y = 145;//144;
					MachineCoords.sEffectMute.width = 15;//7;
					MachineCoords.sEffectMute.height = 14;//7;
					MachineCoords.sEffectBypass.x = 30;//126;
					MachineCoords.sEffectBypass.y = 145;//144;
					MachineCoords.sEffectBypass.width = 15;//7;
					MachineCoords.sEffectBypass.height = 14;//13;

					MachineCoords.dGeneratorVu.x = 10;//8;
					MachineCoords.dGeneratorVu.y = 35;//3;
					MachineCoords.dGeneratorVu.width = 130;//96;
					MachineCoords.dGeneratorVu.height = 0;
					MachineCoords.dGeneratorPan.x = 39;//3;
					MachineCoords.dGeneratorPan.y = 26;//35;
					MachineCoords.dGeneratorPan.width = 91;//117;
					MachineCoords.dGeneratorPan.height = 0;
					MachineCoords.dGeneratorMute.x = 11;//137;
					MachineCoords.dGeneratorMute.y = 5;//4;
					MachineCoords.dGeneratorSolo.x = 26;//137;
					MachineCoords.dGeneratorSolo.y = 5;//17;
					MachineCoords.dGeneratorName.x = 49;//10;
					MachineCoords.dGeneratorName.y = 7;//12;

					MachineCoords.dEffectVu.x = 10;//8;
					MachineCoords.dEffectVu.y = 35;//3;
					MachineCoords.dEffectVu.width = 130;//96;
					MachineCoords.dEffectVu.height = 0;
					MachineCoords.dEffectPan.x = 39;//3;
					MachineCoords.dEffectPan.y = 26;//35;
					MachineCoords.dEffectPan.width = 91;//117;
					MachineCoords.dEffectPan.height = 0;
					MachineCoords.dEffectMute.x = 11;//137;
					MachineCoords.dEffectMute.y = 5;//4;
					MachineCoords.dEffectBypass.x = 26;//137;
					MachineCoords.dEffectBypass.y = 5;//15;
					MachineCoords.dEffectName.x = 49;//10;
					MachineCoords.dEffectName.y = 7;//12;
					MachineCoords.bHasTransparency = FALSE;
				#endif
				machineskin.DeleteObject();
				DeleteObject(hbmMachineSkin);
				machineskinmask.DeleteObject();
				machineskin.LoadBitmap(IDB_MACHINE_SKIN);
			}
		}

	}
}
