#include "MachineView.hpp"
#include "Configuration.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/machinefactory.h>
#include <psycle/core/plugininfo.h>
#include <psycle/core/player.h>
#include <psycle/core/file.h>
using namespace psy::core;
#else
#include "Song.hpp"
#include "Machine.hpp"
#endif

#include "MainFrm.hpp"
#include "NewMachine.hpp"
#include "NativeGui.hpp"
#include "WireGui.hpp"
#include "Mastergui.hpp"
#include "EffectGui.hpp"
#include "GeneratorGui.hpp"
#include "DummyGui.hpp"
#include "MixerGui.hpp"
#include "VstGenGui.hpp"
#include "VstFxGui.hpp"
#include "RecorderGui.hpp"
#include "SamplerGui.hpp"
#include "XmSamplerGui.hpp"
#include "MacProp.hpp"

#ifdef _MSC_VER
#undef min
#undef max
#endif

namespace psycle {
	namespace host {

		namespace {
			COLORREF inline rgb(int r, int g, int b) {
				return RGB(
					std::max(0, std::min(r, 255)),
					std::max(0, std::min(g, 255)),
					std::max(0, std::min(b, 255)));
			}
		}

#if PSYCLE__CONFIGURATION__USE_PSYCORE		
		MachineView::MachineView(CChildView* parent, CMainFrame* main)
#else
		MachineView::MachineView(CChildView* parent, CMainFrame* main, Song* song)
#endif
			: PsycleCanvas::Canvas(parent),
			  parent_(parent),
			  main_(main),
#if PSYCLE__CONFIGURATION__USE_PSYCORE		
			  song_(0),
#else
			  song_(song),
#endif
			  del_line_(0),
			  rewire_line_(0),
			  del_machine_(0),
			  is_locked_(false),
			  del_in_engine_(false)  			
		{
			// set_bg_color(Global::pConfig->mv_colour);
			// InitSkin();
		}

#if PSYCLE__CONFIGURATION__USE_PSYCORE
		void MachineView::SetSong(class psy::core::Song* song)
		{
			song_ = song;
		}
#endif

		MachineView::~MachineView()
		{
			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			machineskinmask.DeleteObject();
			machinebkg.DeleteObject();
			DeleteObject(hbmMachineBkg);
		}

		CMainFrame* MachineView::main()
		{
			return main_;
		}

		void MachineView::WriteStatusBar(const std::string& text)
		{
			main()->StatusBarText(text.c_str());
		}


		void MachineView::DeleteMachineGui(Machine* mac)
		{
			std::map<Machine*, MachineGui*>::iterator it;
			it = gui_map_.find(mac);
			assert(it != gui_map_.end());
			MachineGui* del_machine_ = it->second;
			gui_map_.erase(it);
			del_machine_->RemoveWires();
			del_machine_->set_manage(false);
			delete del_machine_;			
			parent_->Invalidate();
		}

		void MachineView::SetDeleteMachineGui(Machine* mac, bool in_engine)
		{
			std::map<Machine*, MachineGui*>::iterator it;
			it = gui_map_.find(mac);
			assert(it != gui_map_.end());
			del_machine_ = it->second;
			del_in_engine_ = in_engine;
		}

		void MachineView::DoMacPropDialog(Machine* mac, bool from_event)
		{
			std::map<Machine*, MachineGui*>::iterator it;
			int propMac = mac->id();
			it = gui_map_.find(mac);
			assert(it != gui_map_.end());
			MachineGui* gui = it->second;

			CMacProp dlg(gui);			
			dlg.pMachine = mac;
			dlg.pSong = song();
			dlg.thisMac = propMac;
			if(dlg.DoModal() == IDOK)
			{
				dlg.pMachine->SetEditName(dlg.txt);
				main()->StatusBarText(dlg.txt);
				main()->UpdateEnvInfo();
				main()->UpdateComboGen();
				if (main()->pGearRackDialog)
				{
					main()->RedrawGearRackList();
				}	
				gui->UpdateText();
			}
			if (dlg.deleted) {
				if ( from_event) {
					SetDeleteMachineGui(gui, true);				
				} else {
					int mac_prop = gui->mac()->id();
					DeleteMachineGui(gui->mac());
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					song()->DeleteMachine(song()->machine(mac_prop));
#else
					song()->DestroyMachine(mac_prop);
#endif
					main()->UpdateEnvInfo();
					main()->UpdateComboGen();
					if (main()->pGearRackDialog) {
						main()->RedrawGearRackList();
					}								
				}
			} else if (dlg.replaced) {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				int index = mac->id();
				ShowNewMachineDlg(mac->GetPosX(), mac->GetPosY(), mac, from_event);
				strcpy(dlg.txt, song()->machine(index)->GetEditName().c_str());
#else
				int index = mac->id();
				ShowNewMachineDlg(mac->_x, mac->_y, mac, from_event);
				strcpy(dlg.txt, song()->machine(index)->_editName);
#endif
			}
			child_view()->Invalidate(1);
		}

		void MachineView::ShowDialog(Machine* mac, double x, double y)
		{
			std::map<Machine*, MachineGui*>::iterator it;
			it = gui_map_.find(mac);
			assert(it != gui_map_.end());
			it->second->ShowDialog(x, y);
		}

		void MachineView::SetSolo(Machine* tmac)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
///\todo

/*			int smac = tmac->id();
			if (song()->machineSoloed == smac ){
				song()->machineSoloed = -1;
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if (song()->machine(i)) {
						if (( song()->machine(i)->_mode == MACHMODE_GENERATOR ))
						{
							song()->machine(i)->_mute = false;
						}
					}
				}
			} else {
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if ( song()->machine(i) )
					{
						if (( song()->machine(i)->_mode == MACHMODE_GENERATOR ) && (i != smac))
						{
							song()->machine(i)->_mute = true;
							song()->machine(i)->_volumeCounter=0.0f;
							song()->machine(i)->_volumeDisplay=0;
						}
					}
				}
				tmac->_mute = false;
				song()->machineSoloed = smac;
			}
*/

#else
			int smac = tmac->id();
			if (song()->machineSoloed == smac ){
				song()->machineSoloed = -1;
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if (song()->machine(i)) {
						if (( song()->machine(i)->_mode == MACHMODE_GENERATOR ))
						{
							song()->machine(i)->_mute = false;
						}
					}
				}
			} else {
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if ( song()->machine(i) )
					{
						if (( song()->machine(i)->_mode == MACHMODE_GENERATOR ) && (i != smac))
						{
							song()->machine(i)->_mute = true;
							song()->machine(i)->_volumeCounter=0.0f;
							song()->machine(i)->_volumeDisplay=0;
						}
					}
				}
				tmac->_mute = false;
				song()->machineSoloed = smac;
			}

#endif
			UpdateSoloMuteBypass();
			parent_->Invalidate();
		}

		void MachineView::UpdateSoloMuteBypass()
		{
			std::map<Machine*, MachineGui*>::iterator it = gui_map_.begin();
			for ( ; it != gui_map_.end(); ++it ) {
				MachineGui* mac_gui = (*it).second;
				mac_gui->SetMute(mac_gui->mac()->_mute);
				mac_gui->SetSolo(mac_gui->mac()->id() == song()->machineSoloed);
				mac_gui->SetBypass(mac_gui->mac()->Bypass());
			}
		}

		void MachineView::UpdateVUs(CDC* devc)
		{
		  if (!is_locked_) {
		    SetSave(true);
			std::map<Machine*, MachineGui*>::iterator it = gui_map_.begin();
			  for ( ; it != gui_map_.end(); ++it ) {
				(*it).second->UpdateVU(devc);
			  }
			}
		    Flush();
			SetSave(false);
		}

		void MachineView::LockVu()
		{
			is_locked_ = true;
		}

		void MachineView::UnlockVu()
		{
			is_locked_ = false;
		}

		void MachineView::OnEvent(PsycleCanvas::Event* ev)
		{
			PsycleCanvas::Canvas::OnEvent(ev);
			if ( ev->type == PsycleCanvas::Event::BUTTON_2PRESS ) {
				if ( !root()->intersect(ev->x, ev->y) ) {
					ShowNewMachineDlg(ev->x, ev->y, 0, false);
				}
			}
			if ( del_line_ ) {
				delete del_line_;
				del_line_ = 0;
				parent_->Invalidate();
			}
			if ( del_machine_) {
				std::map<Machine*, MachineGui*>::iterator it;
				it = gui_map_.find(del_machine_->mac());
				assert(it != gui_map_.end());
				gui_map_.erase(it);
				int prop_mac = del_machine_->mac()->id();
				del_machine_->RemoveWires();
				del_machine_->set_manage(false);
				delete del_machine_;
				del_machine_ = 0;
				if (del_in_engine_) {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					song()->DeleteMachine(song()->machine(prop_mac));
#else
					song()->DestroyMachine(prop_mac);
#endif
					main()->UpdateEnvInfo();
					main()->UpdateComboGen();				
					if (main()->pGearRackDialog) {
						main()->RedrawGearRackList();
					}						
				}
				parent_->Invalidate();
			}
		}

		void MachineView::ShowNewMachineDlg(double x, double y, Machine* mac, bool from_event)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			CNewMachine dlg;

			if(mac)
			{
				dlg.selectedRole = (mac->IsGenerator() ? MachineRole::GENERATOR : MachineRole::EFFECT);
			}
			if (dlg.DoModal() == IDOK && dlg.outputMachine != MachineKey::invalid() ) {
				if (!mac) {
					if (dlg.selectedRole == MachineRole::GENERATOR)  {
						x -= (MachineCoords.sGenerator.width/2);
						y -= (MachineCoords.sGenerator.height/2);
					}	else {
						x -= (MachineCoords.sEffect.width/2);
						y -= (MachineCoords.sEffect.height/2);
					}
//					child_view()->AddMacViewUndo(); maybe todo
					Machine* newMac = MachineFactory::getInstance().CreateMachine(dlg.outputMachine);
					if (newMac) {
						newMac->SetPosX(x);
						newMac->SetPosY(y);
						song()->AddMachine(newMac);
						if (newMac->id() != -1) {
							CreateMachineGui(newMac);
						} else {
							delete newMac;
							child_view()->MessageBox("Couldn't add the machine to the song the machine","Machine Creation Failed",MB_OK);
						}
					} else {
						child_view()->MessageBox("Couldn't create the machine","Machine Creation Failed",MB_OK);
					}
				} else if ((mac->id() >= MAX_BUSES && dlg.selectedRole != MachineRole::GENERATOR)
						|| (mac->id() < MAX_BUSES && dlg.selectedRole == MachineRole::GENERATOR))
				{
//					child_view()->AddMacViewUndo(); maybe todo
					Machine* newMac = MachineFactory::getInstance().CreateMachine(dlg.outputMachine, mac->id());
					// delete machine if it already exists
					if (newMac) {
//						child_view()->AddMacViewUndo(); maybe todo
						newMac->SetPosX(mac->GetPosX());
						newMac->SetPosY(mac->GetPosY());

						if (!from_event) DeleteMachineGui(mac);
						else SetDeleteMachineGui(mac, false);

						song()->ReplaceMachine(newMac, mac->id());
						CreateMachineGui(newMac);
					}
					else {
						child_view()->MessageBox("Couldn't create the machine","Machine Creation Failed",MB_OK);
					}
				}
				else {
					child_view()->MessageBox("Wrong Class of Machine!", "Machine Creation Failed");
				}
			}
#else
			CNewMachine dlg;
			if(mac)
			{
				if (mac->id() < MAX_BUSES)
				{
					dlg.selectedRole = MachineRole::GENERATOR;
				}
				else
				{
					dlg.selectedRole = MachineRole::EFFECT;
				}
			}
			if ((dlg.DoModal() == IDOK) && (dlg.Outputmachine >= 0)) {
				// AddMacViewUndo();
				int fb,xs,ys;
				if (!mac) {					
					if (dlg.selectedRole == MachineRole::GENERATOR)  {
						fb = song()->GetFreeBus();
						xs = MachineCoords.sGenerator.width;
						ys = MachineCoords.sGenerator.height;
					}	else {
						fb = song()->GetFreeFxBus();
						xs = MachineCoords.sEffect.width;
						ys = MachineCoords.sEffect.height;
					}
				} else {
					if (mac->id() >= MAX_BUSES && dlg.selectedRole != MachineRole::GENERATOR)
					{
//						child_view()->AddMacViewUndo(); maybe todo
						fb = mac->id();
						xs = MachineCoords.sEffect.width;
						ys = MachineCoords.sEffect.height;
						// delete machine if it already exists
						if (song()->machine(fb))
						{
							x = song()->machine(fb)->_x;
							y = song()->machine(fb)->_y;
							if (!from_event)
								DeleteMachineGui(song()->machine(fb));
							else 
								SetDeleteMachineGui(song()->machine(fb), false);
						}
					}
					else if (mac->id() < MAX_BUSES && dlg.selectedRole == MachineRole::GENERATOR)
					{
//						child_view()->AddMacViewUndo(); maybe todo
						fb = mac->id();
						xs = MachineCoords.sGenerator.width;
						ys = MachineCoords.sGenerator.height;
						// delete machine if it already exists
						if (song()->machine(fb))
						{
							x = song()->machine(fb)->_x;
							y = song()->machine(fb)->_y;
							if (!from_event)
								DeleteMachineGui(song()->machine(fb));
							else 
								SetDeleteMachineGui(song()->machine(fb), false);
						}
					}
					else
					{
						child_view()->MessageBox("Wrong Class of Machine!");
						return;
					}
				}
				// random position
				if ((x < 0) || (y < 0))
				{
					bool bCovered = TRUE;
					while (bCovered)
					{
						x = (rand())%(cw()-xs);
						y = (rand())%(ch()-ys);
						bCovered = FALSE;
						for (int i=0; i < MAX_MACHINES; i++)
						{
							if (song()->machine(i))
							{
								if ((abs(song()->machine(i)->_x - x) < 32) &&
									(abs(song()->machine(i)->_y - y) < 32))
								{
									bCovered = TRUE;
									i = MAX_MACHINES;
								}
							}
						}
					}
				}
				if ( fb == -1) {
					child_view()->MessageBox("Machine Creation Failed","Error!",MB_OK);
				}
				x -= xs/2;
				y -= ys/2;
				bool created=false;
				if (song()->machine(fb) ) {
					created = song()->ReplaceMachine(song()->machine(fb),(MachineType)dlg.Outputmachine, (int)x, (int)y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
				}
				else  {
					created = song()->CreateMachine((MachineType)dlg.Outputmachine, (int)x, (int)y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
				}
				if (!created) {
					child_view()->MessageBox("Machine Creation Failed","Error!",MB_OK);
				} else {
					CreateMachineGui(song()->machine(fb));
				}
			}
#endif
		}

		void MachineView::Rebuild()
		{			
			LockVu();
			root()->Clear();
			gui_map_.clear();
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
				if (song()->machine(idx)) {
					MachineGui* gui = CreateMachineGui(song()->machine(idx));
					assert(gui);
					double x = gui->x();
					double y = gui->y();
					double x1, y1, x2, y2;
					gui->GetBounds(x1, y1, x2, y2);
					x = std::min(x, cw()- (x2-x1));
					y = std::min(y, ch()- (y2-y1));
					gui->SetXY(x,y);
				}
			}
			BuildWires();
			UpdateSoloMuteBypass();
			UnlockVu();
		}

		void MachineView::SelectMachine(MachineGui* gui)
		{
			std::map<Machine*, MachineGui*>::iterator it = gui_map_.begin();
			for ( ; it != gui_map_.end(); ++it ) {
			   (*it).second->SetSelected(gui == (*it).second);
			}			
		}

		void MachineView::UpdatePosition(Machine* mac)
		{
			assert(mac);
			std::map<Machine*, MachineGui*>::iterator it;
			it = gui_map_.find(mac);
			assert(it != gui_map_.end());
			MachineGui* gui = it->second;
			gui->SetXY(mac->GetPosX(), mac->GetPosY());
		}

		MachineGui* MachineView::CreateMachineGui(Machine* mac)
		{
			assert(mac);
			MachineGui* gui;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			if( mac->getMachineKey() == MachineKey::master()) {
				gui = new MasterGui(this, mac);
			} else if( mac->getMachineKey() == MachineKey::sampler() ) {
				gui = new SamplerGui(this, mac);
			} else if( mac->getMachineKey() == MachineKey::sampulse() ) {
				gui = new XmSamplerGui(this, mac);
			} else if( mac->getMachineKey() == MachineKey::audioinput() ) {
				gui = new RecorderGui(this, mac);
			} else if( mac->getMachineKey() == MachineKey::mixer() ) {
				gui = new MixerGui(this, mac);
			} else if( mac->getMachineKey() == MachineKey::duplicator() ) {
				gui = new GeneratorGui(this, mac);
			} else if( mac->getMachineKey() == MachineKey::lfo() ) {
				gui = new GeneratorGui(this, mac);
			}
			else if (mac->getMachineKey() == MachineKey::dummy() ) {
				if (mac->IsGenerator())
					gui = new DummyGenGui(this, mac);
				else
					gui = new DummyEffectGui(this, mac);
			}
			else if (mac->getMachineKey().host() == Hosts::NATIVE ) {
				if (mac->IsGenerator())
					gui = new GeneratorGui(this, mac);
				else 
					gui = new EffectGui(this, mac);
			}
			else if (mac->getMachineKey().host() == Hosts::VST ) {
				if (mac->IsGenerator())
					gui = new VstGenGui(this, mac);
				else 
					gui = new VstFxGui(this, mac);
			
			}

			gui_map_[mac] = gui;
			int xs, ys;
			if (mac->IsGenerator()) {
				xs = MachineCoords.sGenerator.width;
				ys = MachineCoords.sGenerator.height;
				gui->SetSkin(MachineCoords,
							 &machineskin,
							 &machineskinmask,
							 &machinebkg,
							 hbmMachineSkin,
							 hbmMachineBkg,
							 hbmMachineDial,
							 Global::pConfig->generatorFont,
							 Global::pConfig->mv_generator_fontcolour);
			} else {
				xs = MachineCoords.sEffect.width;
				ys = MachineCoords.sEffect.height;
				gui->SetSkin(MachineCoords,
							 &machineskin,
							 &machineskinmask,
							 &machinebkg,
				 			 hbmMachineSkin,
							 hbmMachineBkg,
							 hbmMachineDial,
							 Global::pConfig->effectFont,
							 Global::pConfig->mv_effect_fontcolour);
			}
			// random position
			if ((mac->GetPosX() < 0) || (mac->GetPosX() < 0))
			{
				bool bCovered = TRUE;
				int x, y;
				while (bCovered)
				{
					x = (rand())%(cw()-xs);
					y = (rand())%(ch()-ys);
					bCovered = FALSE;
					for (int i=0; i < MAX_MACHINES; i++)
					{
						if (song()->machine(i))
						{
							if ((abs(song()->machine(i)->GetPosX() - x) < 32) &&
								(abs(song()->machine(i)->GetPosY() - y) < 32))
							{
								bCovered = TRUE;
								break;
							}
						}
					}
				}
				mac->SetPosX(x);
				mac->SetPosY(y);
			}
			if ( mac->_mute )
				gui->SetMute(true);
			else if (song_->machineSoloed == mac->id())
				gui->SetSolo(true);
#else
			switch ( mac->_type ) {
				case MACH_MASTER:
  					gui = new MasterGui(this, mac);
				break;
				case MACH_PLUGIN:
					if ( mac->_mode == MACHMODE_GENERATOR)
						gui = new GeneratorGui(this, mac);
					else if ( mac->_mode == MACHMODE_FX)
						gui = new EffectGui(this, mac);
					else
						throw std::runtime_error("unhandled machine mode");
				break;
				case MACH_DUPLICATOR:
					gui = new GeneratorGui(this, mac);
				break;
				case MACH_MIXER:
					gui = new MixerGui(this, mac);
				break;
				case MACH_VST:
				gui = new VstGenGui(this, mac);
				break;
				case MACH_VSTFX:
					gui = new VstFxGui(this, mac);
				break;
				case MACH_RECORDER:
					gui = new RecorderGui(this, mac);
				break;
				case MACH_SAMPLER:
					gui = new SamplerGui(this, mac);
				break;
				case MACH_XMSAMPLER:
					gui = new XmSamplerGui(this, mac);
				break;
				case MACH_DUMMY: //fallback.
				default:
					if ( mac->_mode == MACHMODE_GENERATOR)
						gui = new DummyGenGui(this, mac);
					else //if ( mac->_mode == MACHMODE_FX)
						gui = new DummyEffectGui(this, mac);
			}
			gui_map_[mac] = gui;
			if ( mac->_mode == MACHMODE_GENERATOR ) {
				gui->SetSkin(MachineCoords,
							 &machineskin,
							 &machineskinmask,
							 &machinebkg,
							hbmMachineSkin,
							hbmMachineBkg,
							hbmMachineDial,
							Global::pConfig->generatorFont,
							Global::pConfig->mv_generator_fontcolour);
			} else if ( mac->_mode == MACHMODE_FX) {
				gui->SetSkin(MachineCoords,
							 &machineskin,
							 &machineskinmask,
							 &machinebkg,
				 			 hbmMachineSkin,
							 hbmMachineBkg,
							 hbmMachineDial,
							 Global::pConfig->effectFont,
							 Global::pConfig->mv_effect_fontcolour);
			} else {
				gui->SetSkin(MachineCoords,
							 &machineskin,
							 &machineskinmask,
							 &machinebkg,
		  					 hbmMachineSkin,
							 hbmMachineBkg,
							 hbmMachineDial,
							 Global::pConfig->generatorFont,
							 Global::pConfig->mv_generator_fontcolour);
			}
			if ( mac->_mute )
				gui->SetMute(true);
			else if (song_->machineSoloed == mac->id())
				gui->SetSolo(true);
#endif
			gui->set_manage(true);
			return gui;
		}

		void MachineView::BuildWires()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
			psy::core::Machine* mac = (song()->machine(idx));
			if (mac) {
					for (int w=0; w<MAX_CONNECTIONS; w++) {
						if (mac->_connection[w]) {
							psy::core::Machine* pout = song()->machine(mac->_outputMachines[w]);
							if (pout)
							{
								std::map<psy::core::Machine*, MachineGui*>::iterator fromIt = 
									gui_map_.find(mac);
								std::map<psy::core::Machine*, MachineGui*>::iterator toIt = 
									gui_map_.find(pout);
								WireGui* wireUi = new WireGui(this);
								wireUi->set_wires(w, pout->FindInputWire(mac->id()));
								root()->Insert(root()->begin(), wireUi);
								wireUi->set_manage(true);
								fromIt->second->AttachWire(wireUi);
								toIt->second->AttachWire(wireUi);
								wireUi->SetGuiConnectors(fromIt->second,
														 toIt->second,
														 0);
								wireUi->UpdatePosition();
							}
						}
					}
				}
			}

#else
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
				Machine* mac = (song_->machine(idx));
				if (mac) {
					for (int w=0; w<MAX_CONNECTIONS; w++) {
						if (mac->_connection[w]) {
							Machine* pout = song_->machine(mac->_outputMachines[w]);
							if (pout)
							{
								std::map<Machine*, MachineGui*>::iterator fromIt = 
									gui_map_.find(mac);
								std::map<Machine*, MachineGui*>::iterator toIt = 
									gui_map_.find(pout);
								WireGui* wireUi = new WireGui(this);
								wireUi->set_wires(w, pout->FindInputWire(mac->id()));
								root()->Insert(root()->begin(), wireUi);
								wireUi->set_manage(true);
								fromIt->second->AttachWire(wireUi);
								toIt->second->AttachWire(wireUi);
								wireUi->SetGuiConnectors(fromIt->second,
														 toIt->second,
														 0);
								wireUi->UpdatePosition();
							}
						}
					}
				}
			}
#endif
		}

		void MachineView::OnNewConnection(MachineGui* sender)
		{
			WireGui* line = new WireGui(this);
			root()->Add(line);
			line->SetStart(sender);
			double x1, y1, x2, y2;
			sender->GetBounds( x1, y1, x2, y2);
			double midW = (x2 - x1) / 2;
			double midH = (y2 - y1) / 2;
			PsycleCanvas::Group* fromParent = sender->parent();
			double x3, y3, x4, y4;
			fromParent->GetBounds(x3, y3, x4, y4);
			double x = x1+x3;
			double y = y1+y3;
			PsycleCanvas::Line::Points points(2);
			points[0] = std::pair<double, double>(x + midW, y + midH);
			points[1] = std::pair<double, double>(x + midW, y + midH);
			line->SetPoints(points);
			if (rewire_line_) {		
				if (sender == rewire_line_->fromGUI())
					line->StartDragging(0);
				else
					line->StartDragging(1);
			} else {
				line->StartDragging(1);
			}
		}

		void MachineView::OnWireRewire(WireGui* sender, int pick_point)
		{
			rewire_line_ = sender;
			if (pick_point == 0) {
				OnNewConnection(sender->fromGUI());
			} else {
				OnNewConnection(sender->toGUI());
			}
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
			RaiseMachinesToTop();
		}

		void MachineView::RaiseMachinesToTop()
		{
			std::map<Machine*,MachineGui*>::iterator it = gui_map_.begin();
			for ( ; it != gui_map_.end(); ++it ) {
				MachineGui* gui = (*it).second;
				root()->RaiseToTop(gui);
			}
		}

		void MachineView::WireUp(WireGui* sender,
								 MachineGui* connect_to_gui,
								 double x,
								 double y,
								 int picker)
		{
			if ( connect_to_gui ) {
				MachineGui* connect_from_gui = sender->start();
				Machine* tmac = connect_from_gui->mac();
				Machine* dmac = connect_to_gui->mac();
				if (!rewire_line_) {				   
				   int dsttype=0;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				   if (song()->InsertConnection(*tmac, *dmac,0,dsttype)== -1) {
#else
				   if (song()->InsertConnection(tmac, dmac,0,dsttype)== -1) {
#endif
				   	  del_line_ = sender;
					  child_view()->MessageBox("Couldn't connect the selected machines!","Error!", MB_ICONERROR);				
				   } else {
					  // a new connection has been inserted
   					  sender->set_wires(tmac->FindOutputWire(dmac->id()),
										dmac->FindInputWire(tmac->id()));
				  	  connect_from_gui->AttachWire(sender);
					  connect_to_gui->AttachWire(sender);
					  sender->SetGuiConnectors(connect_from_gui, connect_to_gui, 0);
					  sender->set_manage(true);
					  sender->UpdatePosition();					  
				   }
				} else {
					bool rewired = false;
					if (picker == 0) {						
						rewired = RewireDest(tmac, dmac);
					} else 
					if (picker == 1) {					
						rewired = RewireSrc(tmac, dmac);						
					}					
					if ( rewired) {
						del_line_ = rewire_line_;
						del_line_->set_manage(false);						
						connect_from_gui->AttachWire(sender);
						connect_to_gui->AttachWire(sender);
						if (picker == 0) {				  			
							sender->SetGuiConnectors(connect_from_gui, connect_to_gui, 0);
							sender->set_wires(tmac->FindOutputWire(dmac->id()),
											  dmac->FindInputWire(tmac->id()));
						} else {
							sender->SetGuiConnectors(connect_to_gui, connect_from_gui, 0);
							sender->set_wires(dmac->FindOutputWire(tmac->id()),
											  tmac->FindInputWire(dmac->id()));
						}						
						sender->set_manage(true);
						sender->UpdatePosition();	
					} else {
						del_line_ = sender;
					}
					rewire_line_ = 0;
				}				
			} else {
				del_line_ = sender; // set wire for deletion
				parent_->Invalidate();
			}
		}

#if PSYCLE__CONFIGURATION__USE_PSYCORE
		bool MachineView::RewireSrc(Machine* tmac, Machine* dmac)
		{
			///\todo: hardcoded. This needs to be extended with multi-io.
			psy::core::InPort::id_type portin=psy::core::InPort::id_type(0);
			psy::core::OutPort::id_type portout=psy::core::OutPort::id_type(0);
			if(!song()->ChangeWireSourceMac(*dmac, *tmac, portout, psy::core::Wire::id_type(rewire_line_->wiredest()), portin))	{
				child_view()->MessageBox("Wire move could not be completed!","Error!", MB_ICONERROR);
				return false;
			}
			return true;
		}
#else
		bool MachineView::RewireSrc(Machine* tmac, Machine* dmac)
		{			
			int srctype=0;
			///\todo: for multi-io.
			//if ( tmac->GetOutputSlotTypes() > 1 ) ask user and get index
			if (!song()->ChangeWireSourceMac(dmac,tmac,dmac->GetFreeOutputWire(srctype),rewire_line_->wiredest()))
			{
				child_view()->MessageBox("Wire move could not be completed!","Error!", MB_ICONERROR);
				return false;
			}
			return true;
		}
#endif

#if PSYCLE__CONFIGURATION__USE_PSYCORE
		bool MachineView::RewireDest(Machine* tmac, Machine* dmac)
		{
			///\todo: hardcoded for the Mixer machine. This needs to be extended with multi-io.
			psy::core::InPort::id_type portin=psy::core::InPort::id_type(0);
			psy::core::OutPort::id_type portout=psy::core::OutPort::id_type(0);
			if ( dmac->GetInPorts() > 1 ) {
				if (child_view()->MessageBox("Should I connect this to a send/return input?","Mixer Connection",MB_YESNO) == IDYES ) {
					portin=psy::core::InPort::id_type(1);
				}
			}
			if(!song()->ChangeWireDestMac(*tmac, *dmac, portout, psy::core::Wire::id_type(rewire_line_->wiresrc()), portin))	{
				child_view()->MessageBox("Wire move could not be completed!","Error!", MB_ICONERROR);
				return false;
			}
			return true;
		}
#else
		bool MachineView::RewireDest(Machine* tmac, Machine* dmac)
		{
			// rewire dest;
			int w(-1);
			//FIXME: tmac?
			if ( tmac->_mode== MACHMODE_FX && dmac->GetInputSlotTypes() > 1 )
			{
				if (child_view()->MessageBox("Should I connect this to a send/return input?","Mixer Connection",MB_YESNO) == IDYES ) {
					w = dmac->GetFreeInputWire(1);
				}
				else { 
					w = dmac->GetFreeInputWire(0);
				}
			}
			else {
					w = dmac->GetFreeInputWire(0);
			}
			if (!song()->ChangeWireDestMac(tmac,dmac,rewire_line_->wiresrc(), w)) {
				child_view()->MessageBox("Wire move could not be completed!","Error!", MB_ICONERROR);
				return false;
			}
			return true;
		}
#endif

		void MachineView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
		{
			BITMAP bm;
			// Get the dimensions of the source bitmap
			pBmpSource->GetObject(sizeof(BITMAP), &bm);
			// Create the mask bitmap
			pBmpMask->DeleteObject();
			pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
			// We will need two DCs to work with. One to hold the Image
			// (the source), and one to hold the mask (destination).
			// When blitting onto a monochrome bitmap from a color, pixels
			// in the source color bitmap that are equal to the background
			// color are blitted as white. All the remaining pixels are
			// blitted as black.
			CDC hdcSrc, hdcDst;
			hdcSrc.CreateCompatibleDC(NULL);
			hdcDst.CreateCompatibleDC(NULL);
			// Load the bitmaps into memory DC
			CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
			CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
			// Change the background to trans color
			hdcSrc.SetBkColor(clrTrans);
			// This call sets up the mask bitmap.
			hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
			// Now, we need to paint onto the original image, making
			// sure that the "transparent" area is set to black. What
			// we do is AND the monochrome image onto the color Image
			// first. When blitting from mono to color, the monochrome
			// pixel is first transformed as follows:
			// if  1 (black) it is mapped to the color set by SetTextColor().
			// if  0 (white) is is mapped to the color set by SetBkColor().
			// Only then is the raster operation performed.
			hdcSrc.SetTextColor(RGB(255,255,255));
			hdcSrc.SetBkColor(RGB(0,0,0));
			hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
			// Clean up by deselecting any objects, and delete the
			// DC's.
			hdcSrc.SelectObject(hbmSrcT);
			hdcDst.SelectObject(hbmDstT);
			hdcSrc.DeleteDC();
			hdcDst.DeleteDC();
		}

		void MachineView::LoadMachineBackground()
		{
			machinebkg.DeleteObject();
			if ( hbmMachineBkg) DeleteObject(hbmMachineBkg);
			if (Global::pConfig->bBmpBkg)
			{
				Global::pConfig->bBmpBkg=FALSE;
				hbmMachineBkg = (HBITMAP)LoadImage(NULL, Global::pConfig->szBmpBkgFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
				if (hbmMachineBkg)
				{
					if (machinebkg.Attach(hbmMachineBkg))
					{	
						BITMAP bm;
						GetObject(hbmMachineBkg,sizeof(BITMAP),&bm);

						bkgx=bm.bmWidth;
						bkgy=bm.bmHeight;

						if ((bkgx > 0) && (bkgy > 0))
						{
							Global::pConfig->bBmpBkg=TRUE;
						}
					}
				}
			}
		}

		void MachineView::InitSkin()
		{
			LoadMachineBackground();
			bool has_skin = false;
			std::string szOld;
			CNativeGui::uiSetting().LoadMachineDial();
			if (!Global::pConfig->machine_skin.empty())
			{
				szOld = Global::pConfig->machine_skin;
				if (szOld != PSYCLE__PATH__DEFAULT_MACHINE_SKIN)
				{
					BOOL result = FALSE;
					FindMachineSkin(Global::pConfig->GetSkinDir().c_str(),Global::pConfig->machine_skin.c_str(), &result);
					if(result)
					{						
						has_skin = true;
					}
				}
				if ( !has_skin) {
					// load defaults
					szOld = PSYCLE__PATH__DEFAULT_MACHINE_SKIN;
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
					MachineCoords.bHasTransparency = false;
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
					MachineCoords.bHasTransparency = false;
#endif
					machineskin.DeleteObject();
					DeleteObject(hbmMachineSkin);
					machineskinmask.DeleteObject();
					machineskin.LoadBitmap(IDB_MACHINE_SKIN);
				} 
				set_bg_color(Global::pConfig->mv_colour);
				if (Global::pConfig->bBmpBkg)
				{
					set_bg_image(&machinebkg, bkgx, bkgy);
				}
			}
		}

		void MachineView::FindMachineSkin(CString findDir, CString findName, BOOL *result)
		{
			CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*"); // check for subfolders.
			while (loop) 
			{								
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					FindMachineSkin(finder.GetFilePath(),findName,result);
					if ( *result == TRUE) return;
				}
			}
			finder.Close();
			loop = finder.FindFile(findDir + "\\" + findName + ".psm"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psm, does it have a valid matching .bmp?
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					char szOpenName[MAX_PATH];
					sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);

					machineskin.DeleteObject();
					if( hbmMachineSkin) DeleteObject(hbmMachineSkin);
					machineskinmask.DeleteObject();
					hbmMachineSkin = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hbmMachineSkin)
					{
						if (machineskin.Attach(hbmMachineSkin))
						{	
							memset(&MachineCoords,0,sizeof(MachineCoords));
							// load settings
							FILE* hfile;
							sprintf(szOpenName,"%s\\%s.psm",findDir,sName);
							if(!(hfile=fopen(szOpenName,"rb")))
							{
								parent()->MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
								return;
							}
							char buf[512];
							while (fgets(buf, 512, hfile))
							{
								if (strstr(buf,"\"master_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sMaster.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sMaster.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sMaster.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sMaster.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGenerator.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGenerator.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGenerator.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGenerator.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorVu0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVu0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorVu0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorVu0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu_peak_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorVuPeak.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVuPeak.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorVuPeak.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorVuPeak.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_pan_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_mute_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorMute.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorMute.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorMute.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_solo_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorSolo.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorSolo.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorSolo.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorSolo.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffect.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffect.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffect.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffect.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_vu0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectVu0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVu0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectVu0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectVu0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_vu_peak_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectVuPeak.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVuPeak.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectVuPeak.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectVuPeak.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_pan_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_mute_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectMute.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectMute.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectMute.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_bypass_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectBypass.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectBypass.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectBypass.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectBypass.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorVu.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorVu.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dGeneratorVu.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dGeneratorVu.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_pan_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dGeneratorPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dGeneratorPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_mute_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorMute.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"generator_solo_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorSolo.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorSolo.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"generator_name_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorName.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorName.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_vu_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectVu.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectVu.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dEffectVu.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dEffectVu.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_pan_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dEffectPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dEffectPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_mute_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectMute.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_bypass_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectBypass.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectBypass.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_name_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectName.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectName.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"transparency\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										helpers::hexstring_to_integer(q+1, MachineCoords.cTransparency);
										MachineCoords.bHasTransparency = true;
									}
								}
							}
							if (MachineCoords.bHasTransparency)
							{
								PrepareMask(&machineskin,&machineskinmask,MachineCoords.cTransparency);
							}
							fclose(hfile);
							*result = TRUE;
							break;
						}
					}
				}
			}
			finder.Close();
		}


		void MachineView::CenterMaster()
		{
			song()->machine(MASTER_INDEX)->SetPosX((child_view()->CW - MachineCoords.sMaster.width) / 2);			
			song()->machine(MASTER_INDEX)->SetPosY((child_view()->CH - MachineCoords.sMaster.width) / 2);
			UpdatePosition(song()->machine(MASTER_INDEX));				
		}

		bool MachineView::CheckUnsavedSong()
		{
			bool checked = true;
			if (child_view()->UndoMacSaved != child_view()->UndoMacCounter)
			{
				checked = false;
			}
			return checked;
		}
		
	}  // namespace host
}  // namespace psycle
