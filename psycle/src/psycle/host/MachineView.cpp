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
#include "DummyGui.hpp"
#include "MixerGui.hpp"
#include "VstGenGui.hpp"
#include "VstFxGui.hpp"
#include "RecorderGui.hpp"
#include "SamplerGui.hpp"
#include "XmSamplerGui.hpp"
#include "MainFrm.hpp"
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

		MachineView::MachineView(CChildView* parent, CMainFrame* main, Song* song)
			: PsycleCanvas::Canvas(parent),
			  parent_(parent),
			  main_(main),
			  song_(song),
			  del_line_(0),
			  rewire_line_(0),
			  del_machine_(0),
			  is_locked_(false),
			  del_in_engine_(false)  			
		{
			// set_bg_color(Global::pConfig->mv_colour);
			// InitSkin();
		}

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
			it = gui_map_.find(mac);
			assert(it != gui_map_.end());
			MachineGui* gui = it->second;
			int propMac = mac->_macIndex;
			CMacProp dlg(gui);			
			dlg.pMachine = mac;
			dlg.pSong = song();
			dlg.thisMac = propMac;
			if(dlg.DoModal() == IDOK)
			{
				sprintf(dlg.pMachine->_editName, dlg.txt);
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
					int mac_prop = gui->mac()->_macIndex;
					DeleteMachineGui(gui->mac());
					song()->DestroyMachine(mac_prop);
					main()->UpdateEnvInfo();
					main()->UpdateComboGen();
					if (main()->pGearRackDialog) {
					main()->RedrawGearRackList();
				}								
			}
			} else if (dlg.replaced) {
				int index = mac->_macIndex;
				ShowNewMachineDlg(mac->_x, mac->_y, mac, from_event);
				strcpy(dlg.txt, song()->_pMachine[index]->_editName);
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
				int prop_mac = del_machine_->mac()->_macIndex;
				del_machine_->RemoveWires();
				del_machine_->set_manage(false);
				delete del_machine_;
				del_machine_ = 0;
				if (del_in_engine_) {
					song()->DestroyMachine(prop_mac);
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
			CNewMachine dlg;
			if(mac)
			{
				if (mac->_macIndex < MAX_BUSES)
				{
					dlg.selectedMode = modegen;
				}
				else
				{
					dlg.selectedMode = modefx;
				}
			}
			if ((dlg.DoModal() == IDOK) && (dlg.Outputmachine >= 0)) {
				// AddMacViewUndo();
				int fb,xs,ys;
				if (!mac) {					
					if (dlg.selectedMode == modegen)  {
						fb = song()->GetFreeBus();
						xs = MachineCoords.sGenerator.width;
						ys = MachineCoords.sGenerator.height;
					}	else {
						fb = song()->GetFreeFxBus();
						xs = MachineCoords.sEffect.width;
						ys = MachineCoords.sEffect.height;
					}
				} else {
					if (mac->_macIndex >= MAX_BUSES && dlg.selectedMode != modegen)
					{
//						child_view()->AddMacViewUndo(); maybe todo
						fb = mac->_macIndex;
						xs = MachineCoords.sEffect.width;
						ys = MachineCoords.sEffect.height;
						// delete machine if it already exists
						if (song()->_pMachine[fb])
						{
							x = song()->_pMachine[fb]->_x;
							y = song()->_pMachine[fb]->_y;
							if (!from_event)
								DeleteMachineGui(song()->_pMachine[fb]);
							else 
								SetDeleteMachineGui(song()->_pMachine[fb], false);							
						}
					}
					else if (mac->_macIndex < MAX_BUSES && dlg.selectedMode == modegen)
					{
//						child_view()->AddMacViewUndo(); maybe todo
						fb = mac->_macIndex;
						xs = MachineCoords.sGenerator.width;
						ys = MachineCoords.sGenerator.height;
						// delete machine if it already exists
						if (song()->_pMachine[fb])
						{
							x = song()->_pMachine[fb]->_x;
							y = song()->_pMachine[fb]->_y;
							if (!from_event)
								DeleteMachineGui(song()->_pMachine[fb]);
							else 
								SetDeleteMachineGui(song()->_pMachine[fb], false);
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
							if (Global::_pSong->_pMachine[i])
							{
								if ((abs(song()->_pMachine[i]->_x - x) < 32) &&
									(abs(song()->_pMachine[i]->_y - y) < 32))
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
				if (song()->_pMachine[fb] ) {
					created = song()->ReplaceMachine(song()->_pMachine[fb],(MachineType)dlg.Outputmachine, (int)x, (int)y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
				}
				else  {
					created = song()->CreateMachine((MachineType)dlg.Outputmachine, (int)x, (int)y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
				}
				if (!created) {
					child_view()->MessageBox("Machine Creation Failed","Error!",MB_OK);
				} else {
					CreateMachineGui(song()->_pMachine[fb]);
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
					MachineGui* gui = CreateMachineGui(song_->_pMachine[idx]);
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
			gui->SetXY(mac->_x, mac->_y);
		}

		MachineGui* MachineView::CreateMachineGui(Machine* mac) {
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
					else
						if ( mac->_mode == MACHMODE_FX)
							gui = new DummyEffectGui(this, mac);
					break;
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
			return gui;
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
								wireUi->set_wires(w, pout->FindInputWire(mac->_macIndex));
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
				   if (song_->InsertConnection(tmac, dmac,0,dsttype)== -1) {
				   	  del_line_ = sender;
					  child_view()->MessageBox("Couldn't connect the selected machines!","Error!", MB_ICONERROR);				
				   } else {
					  // a new connection has been inserted
					  sender->set_wires(tmac->FindOutputWire(dmac->_macIndex),
										dmac->FindInputWire(tmac->_macIndex));
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
							sender->set_wires(tmac->FindOutputWire(dmac->_macIndex),
											  dmac->FindInputWire(tmac->_macIndex));
						} else {
							sender->SetGuiConnectors(connect_to_gui, connect_from_gui, 0);
							sender->set_wires(dmac->FindOutputWire(tmac->_macIndex),
											  tmac->FindInputWire(dmac->_macIndex));
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

		bool MachineView::RewireDest(Machine* tmac, Machine* dmac)
		{
			// rewire dest;
			int w(-1);
			///\todo: hardcoded for the Mixer machine. This needs to be extended with multi-io.
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
			song()->_pMachine[MASTER_INDEX]->_x = (child_view()->CW - MachineCoords.sMaster.width) / 2;			
			song()->_pMachine[MASTER_INDEX]->_y = (child_view()->CH - MachineCoords.sMaster.width) / 2;
			UpdatePosition(song()->_pMachine[MASTER_INDEX]);				
		}

		
	}  // namespace host
}  // namespace psycle
