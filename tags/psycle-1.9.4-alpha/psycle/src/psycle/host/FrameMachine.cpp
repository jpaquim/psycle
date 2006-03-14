///\file
///\brief implementation file for psycle::host::CFrameMachine.
#include <project.private.hpp>
#include "Psycle.hpp"
#include "FrameMachine.hpp"
#include "Childview.hpp"
#include "NewVal.hpp"
#include "PresetsDlg.hpp"
#include "Plugin.hpp"
#include "VSTHost.hpp"
#include "InputHandler.hpp"
#include "Helpers.hpp"
#include "MainFrm.hpp"
#include "Machine.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		extern CPsycleApp theApp;

		IMPLEMENT_DYNCREATE(CFrameMachine, CFrameWnd)

		CFrameMachine::CFrameMachine()
		{
		}

		CFrameMachine::~CFrameMachine()
		{
		}

		BEGIN_MESSAGE_MAP(CFrameMachine, CFrameWnd)
			//{{AFX_MSG_MAP(CFrameMachine)
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_WM_RBUTTONUP()
			ON_WM_TIMER()
			ON_COMMAND(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
			ON_COMMAND(ID_PARAMETERS_RESETPARAMETERS, OnParametersResetparameters)
			ON_COMMAND(ID_MACHINE_COMMAND, OnParametersCommand)
			ON_COMMAND(ID_MACHINE_ABOUTTHISMACHINE, OnMachineAboutthismachine)
			ON_WM_CREATE()
			ON_WM_DESTROY()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_COMMAND(ID_PARAMETERS_SHOWPRESET, OnParametersShowpreset)
			ON_WM_SETFOCUS()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		void CFrameMachine::Generate()
		{
			istweak=false;
			tweakpar=0;
			tweakbase=0;
			minval=0;
			maxval=0;
			finetweak=false;
			ultrafinetweak=false;
			
			b_font.CreatePointFont(80,"Tahoma");
//			b_font_bold.CreatePointFont(80,"Tahoma Bold");
			CString sFace("Tahoma");
			LOGFONT lf = LOGFONT();
			lf.lfWeight = FW_BOLD;
			lf.lfHeight = 80;
			lf.lfQuality = NONANTIALIASED_QUALITY;
			std::strncpy(lf.lfFaceName,(LPCTSTR)sFace,32);
			if(!b_font_bold.CreatePointFontIndirect(&lf))


			UpdateWindow();
		}

		void CFrameMachine::SelectMachine(Machine* pMachine)
		{
			_pMachine = pMachine;

			// Get NumParameters
			numParameters = _pMachine->GetNumParams();
			ncol = _pMachine->GetNumCols();
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				GetMenu()->GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, ((Plugin*)_pMachine)->GetInfo()->Command);
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				while ( (numParameters/ncol)*K_YSIZE > ncol*W_ROWWIDTH ) ncol++;
			}
			parspercol = numParameters/ncol;
			if (parspercol>24)	// check for "too big" windows
			{
				parspercol=24;
				ncol=numParameters/24;
				if (ncol*24 != numParameters)
				{
					ncol++;
				}
			}
			if ( parspercol*ncol < numParameters) parspercol++; // check if all the parameters are visible.
			
			int const winh = parspercol*K_YSIZE;

			CWnd *dsk = GetDesktopWindow();
			CRect rClient;
			dsk->GetClientRect(&rClient);


/*			\todo: For some reason, the compiler doesn't see PMENUBARINFO nor GetMenuBarInfo(). They are defined in Winuser.h (PlatformSDK).

			PMENUBARINFO pinfo;
			GetMenuBarInfo(OBJID_MENU,0,pinfo);
			CRect rect
				(
					CPoint
					(
					rClient.Width() / 2 - W_ROWWIDTH * ncol / 2,
					rClient.Height() / 2 - (48 + winh) / 2
					),
					CSize
					(
						W_ROWWIDTH * ncol + GetSystemMetrics(SM_CXFRAME),
						winh + GetSystemMetrics(SM_CYCAPTION) +  (pinfo->rcBar.bottom-pinfo->rcBar.top) + GetSystemMetrics(SM_CYEDGE)
					)
				);

			CalcWindowRect(&rect, adjustBorder);
			MoveWindow(&rect, true);
*/
			CRect rect,rect2;
			//Show the window in the usual way, without worrying about the exact sizes.
			MoveWindow
				(
				rClient.Width() / 2 - W_ROWWIDTH * ncol / 2,
				rClient.Height() / 2 - winh / 2,
				W_ROWWIDTH * ncol,
				winh + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYEDGE),
				false
				);
			ShowWindow(SW_SHOW);
			//Get the coordinates (sizes) of the client area, and the frame.
			GetClientRect(&rect);
			GetWindowRect(&rect2);
			//Using the previous values, resize the window to the desired sizes.
			MoveWindow
				(
				0,
				0,
				(rect2.right-rect2.left)+((W_ROWWIDTH*ncol)-rect.right),
				(rect2.bottom-rect2.top)+(winh-rect.bottom),
				true
				);
		}

		void CFrameMachine::OnDestroy() 
		{
			if ( _pActive != NULL ) *_pActive = false;
			KillTimer(2104+MachineIndex);
			CFrameWnd::OnDestroy();
		}

		void CFrameMachine::OnTimer(UINT nIDEvent) 
		{
			if ( nIDEvent == 2104+MachineIndex )
			{
				Invalidate(false);
			}
			CFrameWnd::OnTimer(nIDEvent);
		}


		///////////////////////////////////////////////////////////////////////
		// PAINT GUI HERE
		///////////////////////////////////////////////////////////////////////

		void CFrameMachine::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting

			dc.SelectObject(&b_font);

			CRect rect;
			GetClientRect(&rect);
			int const K_XSIZE2=K_XSIZE+8;
			int const K_YSIZE2=K_YSIZE/2;
		//	int hsp=0;

			CDC memDC;
			CBitmap* oldbmp;

			memDC.CreateCompatibleDC(&dc);
			oldbmp=memDC.SelectObject(&wndView->machinedial);

			int y_knob = 0;
			int x_knob = 0;
			int knob_c = 0;
			char parName[64];
			std::memset(parName,0,64);

			for (int c=0; c<numParameters; c++)
			{
				char buffer[128];
				
				bool bDrawKnob;
				int min_v,max_v,val_v;

				_pMachine->GetParamName(c,parName);
				_pMachine->GetParamRange(c,min_v,max_v);
				val_v = _pMachine->GetParamValue(c);
				_pMachine->GetParamValue(c,buffer);
				bDrawKnob = (min_v==max_v)?false:true;

				if(bDrawKnob)
				{
					int const amp_v = max_v - min_v;
					int const rel_v = val_v - min_v;

					int const frame = (K_NUMFRAMES*rel_v)/amp_v;
					int const xn = frame*K_XSIZE;

					dc.BitBlt(x_knob,y_knob,K_XSIZE,K_YSIZE,&memDC,xn,0,SRCCOPY);
				
					//the old code which did the parameter highlight
					/*int nc;
					
					if ((tweakpar == c) && (istweak))
					{
						nc = 0x00221100;
					}
					else
					{
						nc = 0;
					}*/
					
					//commented out by Alk when enabling custom colours
					//and all throughout this function
					//dc.SetBkColor(0x00788D93 + nc*2);
					//dc.SetTextColor(0x00CCDDEE + nc);
					if ((tweakpar == c) && (istweak))
					{
						dc.SetBkColor(Global::pConfig->machineGUIHTopColor);
						dc.SetTextColor(Global::pConfig->machineGUIHFontTopColor);
					}
					else
					{
						dc.SetBkColor(Global::pConfig->machineGUITopColor);
						dc.SetTextColor(Global::pConfig->machineGUIFontTopColor);
					}
					dc.ExtTextOut(K_XSIZE2+x_knob, y_knob, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), CString(parName), 0);
					
					//dc.SetBkColor(0x00687D83 + nc*2);
					//dc.SetTextColor(0x0044EEFF + nc);
					if ((tweakpar == c) && (istweak))
					{
						dc.SetBkColor(Global::pConfig->machineGUIHBottomColor);
						dc.SetTextColor(Global::pConfig->machineGUIHFontBottomColor);
					}
					else
					{
						dc.SetBkColor(Global::pConfig->machineGUIBottomColor);
						dc.SetTextColor(Global::pConfig->machineGUIFontBottomColor);
					}
					dc.ExtTextOut(K_XSIZE2 + x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+K_YSIZE), CString(buffer), 0);
				
				}
				else
				{
					if(!std::strlen(parName) /* <bohan> don't know what pooplog's plugins use for separators... */ || std::strlen(parName) == 1)
					{
						//dc.SetBkColor(0x00788D93);
						dc.SetBkColor(Global::pConfig->machineGUITopColor);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), "", 0);
						

						//dc.SetBkColor(0x00687D83);
						dc.SetBkColor(Global::pConfig->machineGUIBottomColor);
						dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+K_YSIZE), "", 0);
					}
					else
					{
						//dc.SetBkColor(0x00788D93);
						dc.SetBkColor(Global::pConfig->machineGUITopColor);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH + x_knob, y_knob + K_YSIZE / 4), "", 0);
					
						//dc.SetBkColor(0x0088a8b4);
						//dc.SetTextColor(0x00FFFFFF);
						dc.SetBkColor(Global::pConfig->machineGUITitleColor);
						dc.SetTextColor(Global::pConfig->machineGUITitleFontColor);

						dc.SelectObject(&b_font_bold);
						dc.ExtTextOut(x_knob + 8, y_knob + K_YSIZE / 4, ETO_OPAQUE, CRect(x_knob, y_knob + K_YSIZE / 4, W_ROWWIDTH + x_knob, y_knob + K_YSIZE * 3 / 4), CString(parName), 0);
						dc.SelectObject(&b_font);

						//dc.SetBkColor(0x00687D83);
						dc.SetBkColor(Global::pConfig->machineGUIBottomColor);
						dc.ExtTextOut(x_knob, y_knob + K_YSIZE * 3 / 4, ETO_OPAQUE, CRect(x_knob, y_knob + K_YSIZE * 3 / 4, W_ROWWIDTH + x_knob, y_knob + K_YSIZE), "", 0);
					}
				}
				y_knob += K_YSIZE;

				++knob_c;

				if (knob_c >= parspercol)
				{
					knob_c = 0;
					x_knob += W_ROWWIDTH;
					y_knob = 0;
				}
			}

			int exess= parspercol*ncol;
			if ( exess > numParameters )
			{
				for (int c=numParameters; c<exess; c++)
				{
					//dc.SetBkColor(0x00788D93);
					//dc.SetTextColor(0x00CCDDEE);
					dc.SetBkColor(Global::pConfig->machineGUITopColor);
					dc.SetTextColor(Global::pConfig->machineGUIFontTopColor);
					dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), "", 0);

					//dc.SetBkColor(0x00687D83);
					//dc.SetTextColor(0x0044EEFF);
					dc.SetBkColor(Global::pConfig->machineGUIBottomColor);
					dc.SetTextColor(Global::pConfig->machineGUIFontBottomColor);
					dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+K_YSIZE), "", 0);

					y_knob += K_YSIZE;
				}
			}
			memDC.SelectObject(oldbmp);
			memDC.DeleteDC();
		}

		void CFrameMachine::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			tweakpar = (point.y/K_YSIZE) + ((point.x/150)*parspercol);
			if ((tweakpar > -1) && (tweakpar < numParameters))
			{
				sourcepoint = point.y;
				tweakbase = _pMachine->GetParamValue(tweakpar);
				_pMachine->GetParamRange(tweakpar,minval,maxval);
				istweak = true;
				SetCapture();
			}
			else
			{
				istweak = false;
			}
			CFrameWnd::OnLButtonDown(nFlags, point);
		}

		void CFrameMachine::OnMouseMove(UINT nFlags, CPoint point) 
		{
			if (istweak)
			{
				if (( ultrafinetweak && !(nFlags & MK_SHIFT )) || //shift-key has been left.
					( !ultrafinetweak && (nFlags & MK_SHIFT))) //shift-key has just been pressed
				{
					tweakbase = _pMachine->GetParamValue(tweakpar);
					sourcepoint=point.y;
					ultrafinetweak=!ultrafinetweak;
				}
				else if (( finetweak && !(nFlags & MK_CONTROL )) || //control-key has been left.
					( !finetweak && (nFlags & MK_CONTROL))) //control-key has just been pressed
				{
					tweakbase = _pMachine->GetParamValue(tweakpar);
					sourcepoint=point.y;
					finetweak=!finetweak;
				}

				double freak;
				int screenh = wndView->CH;
				if ( ultrafinetweak ) freak = 0.5f;
				else if (maxval-minval < screenh/4) freak = (maxval-minval)/float(screenh/4);
				else if (maxval-minval < screenh*2/3) freak = (maxval-minval)/float(screenh/3);
				else freak = (maxval-minval)/float(screenh*3/5);
				if (finetweak) freak/=5;

				double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase;

				if (nv < minval) nv = minval;
				if (nv > maxval) nv = maxval;

				wndView->AddMacViewUndo();
				_pMachine->SetParameter(tweakpar,(int) nv);

				if (Global::pConfig->_RecordTweaks)
				{
					if (Global::pConfig->_RecordMouseTweaksSmooth)
					{
						wndView->MousePatternTweakSlide(MachineIndex, tweakpar, ((int)nv)-minval);
					}
					else
					{
						wndView->MousePatternTweak(MachineIndex, tweakpar, ((int)nv)-minval);
					}
				}

				Invalidate(false);
			}
			CFrameWnd::OnMouseMove(nFlags, point);
		}

		void CFrameMachine::OnLButtonUp(UINT nFlags, CPoint point) 
		{
			istweak = false;
			Invalidate(false);	
			ReleaseCapture();
			CFrameWnd::OnLButtonUp(nFlags, point);
		}

		void CFrameMachine::OnRButtonUp(UINT nFlags, CPoint point) 
		{
			tweakpar = (point.y/K_YSIZE) + ((point.x/150)*parspercol);
			if ((tweakpar > -1) && (tweakpar < numParameters))
			{
				if (nFlags & MK_CONTROL)
				{
					Global::_pSong->seqBus = MachineIndex;//Global::_pSong->FindBusFromIndex(MachineIndex);
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(FALSE);
					CComboBox *cb2=(CComboBox *)((CMainFrame *)theApp.m_pMainWnd)->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cb2->SetCurSel(AUX_PARAMS); // PARAMS
					Global::_pSong->auxcolSelected=tweakpar;
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboIns();
				}
				else 
				{		
					int const thispar = (point.y/K_YSIZE) + ((point.x/150)*parspercol);
					int min_v=1;
					int max_v=1;
					char name[64];
					memset(name,0,64);
					CNewVal dlg;

					_pMachine->GetParamName(thispar,name);
					_pMachine->GetParamRange(thispar,min_v,max_v);
					dlg.m_Value = _pMachine->GetParamValue(thispar);

					std::sprintf
						(
							dlg.title, "Param:'%.2x:%s' (Range from %d to %d)\0",
							thispar,
							name,
							min_v,
							max_v
						);
					dlg.min = min_v;
					dlg.max = max_v;
					dlg.macindex = MachineIndex;
					dlg.paramindex = tweakpar;
					dlg.DoModal();
					int nv = dlg.m_Value;
					if (nv < min_v)
					{
						nv = min_v;
					}
					if (nv > max_v)
					{
						nv = max_v;
					}
					wndView->AddMacViewUndo();
					_pMachine->SetParameter(thispar,(int)nv);
					Invalidate(false);
				}
			}
			CFrameWnd::OnRButtonUp(nFlags, point);
		}

		void CFrameMachine::OnParametersRandomparameters() 
		{
			int numpars = _pMachine->GetNumParams();
			for (int c=0; c<numpars; c++)
			{
				int minran,maxran;
				_pMachine->GetParamRange(c,minran,maxran);

				int dif = (maxran-minran);
				float randsem = (float)rand()*0.000030517578125f;
				float roffset = randsem*(float)dif;

				wndView->AddMacViewUndo();
				_pMachine->SetParameter(c,minran+int(roffset));
			}
			Invalidate(false);
		}

		void CFrameMachine::OnParametersResetparameters() 
		{
			if ( _pMachine->_type == MACH_PLUGIN)
			{
				int numpars = _pMachine->GetNumParams();
				for (int c=0; c<numpars; c++)
				{
					int dv = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->DefValue;
					wndView->AddMacViewUndo();
					_pMachine->SetParameter(c,dv);
				}
			}
			if (istweak)
			{
				istweak = false;
			}
			Invalidate(false);
		}

		void CFrameMachine::OnParametersCommand() 
		{
			if ( _pMachine->_type == MACH_PLUGIN)
			{
				((Plugin*)_pMachine)->GetCallback()->hWnd = m_hWnd;
				try
				{
					((Plugin*)_pMachine)->proxy().Command();
				}
				catch(const std::exception &)
				{
					// o_O`
				}
			}
		}

		void CFrameMachine::OnMachineAboutthismachine() 
		{
			if (istweak)
			{
				istweak = false;
			}
			if ( _pMachine->_type == MACH_PLUGIN)
			{
				MessageBox
					(
						"Authors: " + CString(((Plugin*)_pMachine)->GetInfo()->Author),
						"About " + CString(((Plugin*)_pMachine)->GetInfo()->Name)
					);
			}
		}

		int CFrameMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			SetTimer(2104+MachineIndex,100,0);
			return 0;
		}

		void CFrameMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			// ignore repeats: nFlags&0x4000
			const BOOL bRepeat = nFlags&0x4000;
			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			if(!bRepeat && cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					{
						const int outnote = cmd.GetNote();
						if ( _pMachine->_mode == MACHMODE_GENERATOR || Global::pConfig->_notesToEffects)
							Global::pInputHandler->PlayNote(outnote,127,true,_pMachine);
						else
							Global::pInputHandler->PlayNote(outnote,127,true, 0);
					}
					break;

				case CT_Immediate:
					Global::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}

			//wndView->KeyDown(nChar,nRepCnt,nFlags);
			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
		}

		void CFrameMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{

			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			const int outnote = cmd.GetNote();
			if(outnote>=0)
			{
				if ( _pMachine->_mode == MACHMODE_GENERATOR ||Global::pConfig->_notesToEffects)
				{
					Global::pInputHandler->StopNote(outnote,true,_pMachine);
				}
				else Global::pInputHandler->StopNote(outnote,true,NULL);
			}

			//wndView->KeyUp(nChar, nRepCnt, nFlags);
			CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}

		void CFrameMachine::OnParametersShowpreset() 
		{
			CPresetsDlg dlg;
			dlg._pMachine=_pMachine;
			dlg.m_wndFrame=this;
			dlg.DoModal();
		}

		void CFrameMachine::OnSetFocus(CWnd* pOldWnd) 
		{
			CFrameWnd::OnSetFocus(pOldWnd);
			Invalidate(false);
		}
	NAMESPACE__END
NAMESPACE__END
