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

		#define K_XSIZE				28
		#define K_YSIZE				28
		#define K_NUMFRAMES			63

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
			finetweak=false;
			ultrafinetweak=false;
			
			b_knob.LoadBitmap(IDB_KNOB);
			b_font.CreatePointFont(80,"Tahoma");

			UpdateWindow();
		}

		void CFrameMachine::SelectMachine(Machine* pMachine)
		{
			_pMachine = pMachine;
			me = true;
			int const cxsize=150;

			// Get NumParameters
			int ncol=1;
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				numParameters = ((Plugin*)_pMachine)->GetInfo()->numParameters;
				ncol = ((Plugin*)_pMachine)->GetInfo()->numCols;
				GetMenu()->GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, ((Plugin*)_pMachine)->GetInfo()->Command);
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				try
				{
					numParameters = ((vst::plugin*)_pMachine)->proxy().numParams();
				}
				catch(const std::exception &)
				{
					numParameters = 0;
				}
				while ( (numParameters/ncol)*K_YSIZE > ncol*cxsize ) ncol++;
			}
			parspercol = numParameters/ncol;
			if (parspercol>24)	
			{
				parspercol=24;
				ncol=numParameters/24;
				if (ncol*24 != numParameters)
				{
					ncol++;
				}
			}
			if ( parspercol*ncol < numParameters) parspercol++;
			
			int const winh = parspercol*K_YSIZE;

			CWnd *dsk = GetDesktopWindow();
			CRect rClient;
			dsk->GetClientRect(&rClient);

			if(true)
			{
				// <bohan>
				// Dilvie reported it doesn't work with non default size fonts.
				// Especially, the menu bar can be spanned on several lines,
				// hence, GetSystemMetrics(SM_CYMENUSIZE) is wrong.
				MoveWindow
					(
						rClient.Width() / 2 - cxsize * ncol / 2,
						rClient.Height() / 2 - (48 + winh) / 2,
						cxsize * ncol,
						9 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE) + winh,
						true
					);
			}
			else
			{
				CRect rect
					(
						CPoint
						(
							rClient.Width() / 2 - cxsize * ncol / 2,
							rClient.Height() / 2 - 48 + winh / 2
						),
						CSize
						(
							cxsize * ncol,
							9 + winh
						)
					);

				CalcWindowRect(&rect, adjustOutside);
				MoveWindow(&rect, true);
			}
			
			ShowWindow(SW_SHOWNORMAL);
			//SetActiveWindow();
			//UpdateWindow();
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
			int const cxsize=150;
			int const K_XSIZE2=K_XSIZE+8;
			int const K_YSIZE2=K_YSIZE/2;
		//	int hsp=0;

			CDC memDC;
			CBitmap* oldbmp;

			memDC.CreateCompatibleDC(&dc);
			oldbmp=memDC.SelectObject(&b_knob);

			int y_knob = 0;
			int x_knob = 0;
			int knob_c = 0;
			char parName[64];
			memset(parName,0,64);

			for (int c=0; c<numParameters; c++)
			{
				char buffer[128];

				BOOL bDrawKnob = TRUE;
				int min_v=1;
				int max_v=1;
				int val_v=1;
				if ( _pMachine->_type == MACH_PLUGIN )
				{
					if (((Plugin*)_pMachine)->GetInfo()->Parameters[c]->Flags & MPF_STATE)
					{
						min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MinValue;
						max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MaxValue;
						try
						{
							val_v = ((Plugin*)_pMachine)->proxy().Vals()[c];
						}
						catch(const std::exception &)
						{
							val_v = 0; // hmm
						}
						try
						{
							if(!((Plugin*)_pMachine)->proxy().DescribeValue(buffer, c, val_v))
								std::sprintf(buffer,"%d",val_v);
						}
						catch(const std::exception &)
						{
							std::strcpy(buffer,"fucked up");
						}
					}
					else
					{
						bDrawKnob = FALSE;
					}
					std::strcpy(parName, ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->Name);
				}
				else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
				{
					min_v = 0;
					max_v = vst::quantization;
					try
					{
						val_v = f2i(((vst::plugin*)_pMachine)->proxy().getParameter(c) * vst::quantization);
					}
					catch(const std::exception &)
					{
						val_v = 0; // hmm
					}
					std::memset(buffer,0,sizeof(buffer));
					if(!((vst::plugin*)_pMachine)->DescribeValue(c, buffer))
					{
						std::sprintf(buffer,"%d",val_v);
					}
					try
					{
						((vst::plugin*)_pMachine)->proxy().dispatcher(effGetParamName, c, 0, parName);
					}
					catch(const std::exception &)
					{
						std::strcpy(buffer,"fucked up");
					}
				}
				if(bDrawKnob)
				{
					int const amp_v = max_v - min_v;
					int const rel_v = val_v - min_v;

					int const frame = (K_NUMFRAMES*rel_v)/amp_v;
					int const xn = frame*K_XSIZE;



					dc.BitBlt(x_knob,y_knob,K_XSIZE,K_YSIZE,&memDC,xn,0,SRCCOPY);
				
					int nc;
					
					if ((tweakpar == c) && (istweak))
					{
						nc = 0x00221100;
					}
					else
					{
						nc = 0;
					}
					
					dc.SetBkColor(0x00788D93 + nc*2);
					dc.SetTextColor(0x00CCDDEE + nc);
					dc.ExtTextOut(K_XSIZE2+x_knob, y_knob, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), CString(parName), 0);
					
					dc.SetBkColor(0x00687D83 + nc*2);
					dc.SetTextColor(0x0044EEFF + nc);
					dc.ExtTextOut(K_XSIZE2 + x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), CString(buffer), 0);
				
				}
				else
				{
					if(!std::strlen(parName) /* <bohan> don't know what pooplog's plugins use for separators... */ || std::strlen(parName) == 1)
					{
						dc.SetBkColor(0x00788D93);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), "", 0);

						dc.SetBkColor(0x00687D83);
						dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), "", 0);
					}
					else
					{
						dc.SetBkColor(0x00788D93);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize + x_knob, y_knob + K_YSIZE / 4), "", 0);

						dc.SetBkColor(0x0088a8b4);
						dc.SetTextColor(0x00FFFFFF);
						dc.ExtTextOut(x_knob + 8, y_knob + K_YSIZE / 4, ETO_OPAQUE, CRect(x_knob, y_knob + K_YSIZE / 4, cxsize + x_knob, y_knob + K_YSIZE * 3 / 4), CString(parName), 0);

						dc.SetBkColor(0x00687D83);
						dc.ExtTextOut(x_knob, y_knob + K_YSIZE * 3 / 4, ETO_OPAQUE, CRect(x_knob, y_knob + K_YSIZE * 3 / 4, cxsize + x_knob, y_knob + K_YSIZE), "", 0);
					}
				}
				y_knob += K_YSIZE;

				++knob_c;

				if (knob_c >= parspercol)
				{
					knob_c = 0;
					x_knob += cxsize;
					y_knob = 0;
				}
			}

			int exess=0;
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				exess = parspercol*((Plugin*)_pMachine)->GetInfo()->numCols;
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				int ncol = 1;
				while ( (numParameters/ncol)*K_YSIZE > ncol*cxsize ) ncol++;

				parspercol = numParameters/ncol;
				if (parspercol>24)	
				{
					parspercol=24;
					ncol=numParameters/24;
					if (ncol*24 != numParameters)
					{
						ncol++;
					}
				}

				exess = parspercol*ncol;
			}

			if ( exess > numParameters )
			{
				for (int c=numParameters; c<exess; c++)
				{
					dc.SetBkColor(0x00788D93);
					dc.SetTextColor(0x00CCDDEE);
					dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), "", 0);

					dc.SetBkColor(0x00687D83);
					dc.SetTextColor(0x0044EEFF);
					dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), "", 0);

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

				if ( _pMachine->_type == MACH_PLUGIN )
				{
					try
					{
						tweakbase = ((Plugin*)_pMachine)->proxy().Vals()[tweakpar];
					}
					catch(const std::exception &)
					{
						tweakbase = 0;
					}
				}
				else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
				{
					try
					{
						tweakbase = int(((vst::plugin*)_pMachine)->proxy().getParameter(tweakpar) * vst::quantization);
					}
					catch(const std::exception &)
					{
						tweakbase = 0;
					}
				}
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
				int min_v=1;
				int max_v=1;
				if ( _pMachine->_type == MACH_PLUGIN )
				{
					min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue;
					max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MaxValue;
				}
				else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
				{
					min_v = 0;
					max_v = vst::quantization;
				}

				if (( ultrafinetweak && !(nFlags & MK_SHIFT )) || //shift-key has been left.
					( !ultrafinetweak && (nFlags & MK_SHIFT))) //shift-key has just been pressed
				{
					if ( _pMachine->_type == MACH_PLUGIN )
					{
						try
						{
							tweakbase = ((Plugin*)_pMachine)->proxy().Vals()[tweakpar];
						}
						catch(const std::exception &)
						{
							tweakbase = 0;
						}
					}
					else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
					{
						try
						{
							tweakbase=f2i(((vst::plugin*)_pMachine)->proxy().getParameter(tweakpar) * vst::quantization);
						}
						catch(const std::exception &)
						{
							tweakbase = 0;
						}
					}
					sourcepoint=point.y;
					ultrafinetweak=!ultrafinetweak;
				}
				else if (( finetweak && !(nFlags & MK_CONTROL )) || //control-key has been left.
					( !finetweak && (nFlags & MK_CONTROL))) //control-key has just been pressed
				{
					if ( _pMachine->_type == MACH_PLUGIN )
					{
						try
						{
							tweakbase = ((Plugin*)_pMachine)->proxy().Vals()[tweakpar];
						}
						catch(const std::exception &)
						{
							tweakbase = 0;
						}
					}
					else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
					{
						try
						{
							tweakbase=f2i(((vst::plugin*)_pMachine)->proxy().getParameter(tweakpar) * vst::quantization);
						}
						catch(const std::exception &)
						{
							tweakbase = 0;
						}
					}
					sourcepoint=point.y;
					finetweak=!finetweak;
				}

				double freak;
				int screenh = wndView->CH;
				if ( ultrafinetweak ) freak = 0.5f;
				else if (max_v-min_v < screenh/4) freak = (max_v-min_v)/float(screenh/4);
				else if (max_v-min_v < screenh*2/3) freak = (max_v-min_v)/float(screenh/3);
				else freak = (max_v-min_v)/float(screenh*3/5);
				if (finetweak) freak/=5;

				double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase;

				if (nv < min_v)
				{
					nv = min_v;
				}
				if (nv > max_v)
				{
					nv=max_v;
				}

				wndView->AddMacViewUndo();
				if ( _pMachine->_type == MACH_PLUGIN )
				{
					try
					{
						((Plugin*)_pMachine)->proxy().ParameterTweak(tweakpar, (int) nv);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					// well, this isn't so hard... just put the twk record here
					if (Global::pConfig->_RecordTweaks)
					{
						if (Global::pConfig->_RecordMouseTweaksSmooth)
						{
							wndView->MousePatternTweakSlide(MachineIndex, tweakpar, ((int)nv)-min_v);
						}
						else
						{
							wndView->MousePatternTweak(MachineIndex, tweakpar, ((int)nv)-min_v);
						}
					}
				}
				else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
				{
					try
					{
						((vst::plugin*)_pMachine)->proxy().setParameter(tweakpar,(float)(nv/(float)vst::quantization));
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					// well, this isn't so hard... just put the twk record here
					if (Global::pConfig->_RecordTweaks)
					{
						if (Global::pConfig->_RecordMouseTweaksSmooth)
						{
							wndView->MousePatternTweakSlide(MachineIndex, tweakpar, (int)nv);
						}
						else
						{
							wndView->MousePatternTweak(MachineIndex, tweakpar, (int)nv);
						}
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

					if ( _pMachine->_type == MACH_PLUGIN )
					{
						min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->MinValue;
						max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->MaxValue;
						strcpy(name ,((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->Name);
						try
						{
							dlg.m_Value = ((Plugin*)_pMachine)->proxy().Vals()[thispar];
						}
						catch(const std::exception &)
						{
							dlg.m_Value = 0;
						}
					}
					else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
					{
						min_v = 0;
						max_v = vst::quantization;
						try
						{
							((vst::plugin*)_pMachine)->proxy().dispatcher(effGetParamName, thispar, 0, name);
						}
						catch(const std::exception &)
						{
							std::strcpy(name, "fucked up");
						}
						try
						{
							dlg.m_Value = f2i(((vst::plugin*)_pMachine)->proxy().getParameter(thispar) * vst::quantization);
						}
						catch(const std::exception &)
						{
							dlg.m_Value = 0;
						}
					}
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
					if ( _pMachine->_type == MACH_PLUGIN )
					{
						try
						{
							((Plugin*)_pMachine)->proxy().ParameterTweak(thispar, nv);
						}
						catch(const std::exception &)
						{
							// o_O`
						}
					}
					else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
					{
						try
						{
							((vst::plugin*)_pMachine)->proxy().setParameter(thispar,(float)(nv/(float)vst::quantization));
						}
						catch(const std::exception &)
						{
							// o_O`
						}
						SetFocus();
					}
					Invalidate(false);
				}
			}
			CFrameWnd::OnRButtonUp(nFlags, point);
		}

		void CFrameMachine::OnParametersRandomparameters() 
		{
			// Randomize controls
			for (int c=0; c<((Plugin*)_pMachine)->GetInfo()->numParameters; c++)
			{
				int minran = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MinValue;
				int maxran = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MaxValue;

				int dif = (maxran-minran)+1;

				float randsem = (float)rand()*0.000030517578125f;

				float roffset = randsem*(float)dif;

				wndView->AddMacViewUndo();
				try
				{
					((Plugin*)_pMachine)->proxy().ParameterTweak(c, minran+int(roffset));
				}
				catch(const std::exception &)
				{
					// o_O`
				}
			}

			Invalidate(false);
		}

		void CFrameMachine::OnParametersResetparameters() 
		{
			for (int c=0; c<((Plugin*)_pMachine)->GetInfo()->numParameters; c++)
			{
				int dv = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->DefValue;
				wndView->AddMacViewUndo();
				try
				{
					((Plugin*)_pMachine)->proxy().ParameterTweak(c,dv);
				}
				catch(const std::exception &)
				{
					// o_O`
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

		void CFrameMachine::OnMachineAboutthismachine() 
		{
			if (istweak)
			{
				istweak = false;
			}
			MessageBox
				(
					"Authors: " + CString(((Plugin*)_pMachine)->GetInfo()->Author),
					"About " + CString(((Plugin*)_pMachine)->GetInfo()->Name)
				);
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
