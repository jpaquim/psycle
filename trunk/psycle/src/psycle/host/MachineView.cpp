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
#include "EffectGui.hpp"
#include "GeneratorGui.hpp"
#include "MixerGui.hpp"
#include "VstFxGui.hpp"
#include "RecorderGui.hpp"
#include "SamplerGui.hpp"
#include "XmSamplerGui.hpp"
#include "MainFrm.hpp"

namespace psycle {
	namespace host {

		MachineView::MachineView(CChildView* parent, CMainFrame* main, Song* song)
			: TestCanvas::Canvas(parent),
			  parent_(parent),
			  main_(main),
			  song_(song),
			  del_line_(0),
			  del_machine_(0),
			  is_locked_(false)
		{
			// set_bg_color(Global::pConfig->mv_colour);
			// InitSkin();
		}

		MachineView::~MachineView()
		{
		}

		CMainFrame* MachineView::main()
		{
			return main_;
		}

		void MachineView::SetSolo(Machine* tmac)
		{
			int smac = tmac->_macIndex;
			if (song()->machineSoloed == smac ){
				song()->machineSoloed = -1;
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if (song()->_pMachine[i]) {
						if (( song()->_pMachine[i]->_mode == MACHMODE_GENERATOR ))
						{
							song()->_pMachine[i]->_mute = false;
						}
					}
				}
			} else {
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if ( song()->_pMachine[i] )
					{
						if (( song()->_pMachine[i]->_mode == MACHMODE_GENERATOR ) && (i != smac))
						{
							song()->_pMachine[i]->_mute = true;
							song()->_pMachine[i]->_volumeCounter=0.0f;
							song()->_pMachine[i]->_volumeDisplay=0;
						}
					}
				}
				tmac->_mute = false;
				song()->machineSoloed = smac;
			}
			UpdateSoloMuteBypass();
			parent_->Invalidate();
		}

		void MachineView::UpdateSoloMuteBypass()
		{
			std::map<Machine*, MachineGui*>::iterator it = gui_map_.begin();
			for ( ; it != gui_map_.end(); ++it ) {
				MachineGui* mac_gui = (*it).second;
				mac_gui->SetMute(mac_gui->mac()->_mute);
				mac_gui->SetSolo(mac_gui->mac()->_macIndex == song()->machineSoloed);
				mac_gui->SetBypass(mac_gui->mac()->Bypass());
			}
		}

		void MachineView::UpdateVUs()
		{
			if (!is_locked_) {
			std::map<Machine*, MachineGui*>::iterator it = gui_map_.begin();
			  for ( ; it != gui_map_.end(); ++it ) {
				(*it).second->UpdateVU();
			  }
			}
		}

		void MachineView::LockVu()
		{
			is_locked_ = true;
		}

		void MachineView::UnlockVu()
		{
			is_locked_ = false;
		}

		void MachineView::OnEvent(TestCanvas::Event* ev)
		{
			TestCanvas::Canvas::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				if ( !root()->intersect(ev->x, ev->y) ) {
					ShowNewMachineDlg();
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
				del_machine_->RemoveWires();
				del_machine_->set_manage(false);
				delete del_machine_;
				del_machine_ = 0;
				parent_->Invalidate();
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
			LockVu();
			root()->Clear();
			gui_map_.clear();
			for ( int idx = 0; idx < MAX_MACHINES; ++idx ) {
				if (song_->_pMachine[idx]) {
					CreateMachineGui(song_->_pMachine[idx]);
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

		void MachineView::CreateMachineGui(Machine* mac) {
			assert(mac);
			MachineGui* gui;
			switch ( mac->_type ) {
				case MACH_MASTER:
					gui = new MasterGui(this, mac);
				break;
				case MACH_PLUGIN:
					if ( mac->_mode == MACHMODE_GENERATOR)
						gui = new GeneratorGui(this, mac);
					else
					if ( mac->_mode == MACHMODE_FX)
						gui = new EffectGui(this, mac);
				break;
				case MACH_DUMMY:
					if ( mac->_mode == MACHMODE_GENERATOR)
						gui = new GeneratorGui(this, mac);
					else
					if ( mac->_mode == MACHMODE_FX)
						gui = new EffectGui(this, mac);
				break;
				case MACH_MIXER:
					gui = new MixerGui(this, mac);
				break;
				case MACH_VST:
					gui = new VstFxGui(this, mac);
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
				default:
					gui = new MachineGui(this, mac);
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
								root()->Insert(root()->begin(), wireUi);
								wireUi->set_manage(true);
								fromIt->second->AttachWire(wireUi,0);
								toIt->second->AttachWire(wireUi,1);
								wireUi->SetGuiConnectors(fromIt->second,
														 toIt->second,
														 0);
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
			root()->Add(line);
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
				int dsttype=0;
				if (song_->InsertConnection(tmac, dmac,0,dsttype)== -1) {
					del_line_ = sender;
					//MessageBox("Couldn't connect the selected machines!","Error!", MB_ICONERROR);				
				} else {
					connect_from_gui->AttachWire(sender,0);
					connect_to_gui->AttachWire(sender,1);
					sender->SetGuiConnectors(connect_from_gui, connect_to_gui, 0);
					sender->set_manage(true);
					sender->UpdatePosition();
				}			
			} else {
				del_line_ = sender; // set wire for deletion
				parent_->Invalidate();
			}
		}

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
										MachineCoords.bHasTransparency = TRUE;
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

	}  // namespace host
}  // namespace psycle
