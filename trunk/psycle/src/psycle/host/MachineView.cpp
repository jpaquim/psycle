#include "MachineView.hpp"
#include "Global.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "NativeGui.hpp"
#include "Machine.hpp"
#include "WireGui.hpp"
#include "MainFrm.hpp"

namespace psycle {
	namespace host {

		MachineView::MachineView(CWnd* parent, Song* song)
			: TestCanvas::Canvas(parent),
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

		void MachineView::Rebuild()
		{
			root()->Clear();
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
				if (song_->_pMachine[idx]) {
					MachineGui* gui = new MachineGui(
					                           this,
											   song_->_pMachine[idx]);
					gui_map_[song_->_pMachine[idx]] = gui;
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
					if ( song_->_pMachine[idx]->_mute )
						gui->SetMute(true);
					else if (song_->machineSoloed == idx)
						gui->SetSolo(true);
					gui->set_manage(true);
				}
			}
			BuildWires();
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
								WireGui* wireUi = new WireGui();
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
