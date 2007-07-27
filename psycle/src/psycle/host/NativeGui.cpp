///\file
///\brief implementation file for psycle::host::CNativeGui.
#include <psycle/project.private.hpp>
#include "NativeGui.hpp"
#include "psycle.hpp"
#include "Machine.hpp"
#include "Configuration.hpp"
#include "InputHandler.hpp"
#include "NewVal.hpp"
///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
#include "MainFrm.hpp"
#include "ChildView.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		extern CPsycleApp theApp;

		BEGIN_MESSAGE_MAP(CNativeGui, CWnd)
			ON_WM_CREATE()
			ON_WM_DESTROY()
			ON_WM_SETFOCUS()
			ON_WM_TIMER()
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_WM_RBUTTONUP()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
		END_MESSAGE_MAP()


		NativeGUISettings::NativeGUISettings()
		{
			topColor = Global::pConfig->machineGUITopColor;
			bottomColor = Global::pConfig->machineGUIBottomColor;
			hTopColor = Global::pConfig->machineGUIHTopColor;
			hBottomColor = Global::pConfig->machineGUIHBottomColor;
			titleColor = Global::pConfig->machineGUITitleColor;
			fontTopColor = Global::pConfig->machineGUIFontTopColor;
			fontBottomColor = Global::pConfig->machineGUIFontBottomColor;
			fonthTopColor = Global::pConfig->machineGUIHFontTopColor;
			fonthBottomColor = Global::pConfig->machineGUIHFontBottomColor;
			fonttitleColor = Global::pConfig->machineGUITitleFontColor;

			b_font.CreatePointFont(80,"Tahoma");
			CString sFace("Tahoma");
			LOGFONT lf = LOGFONT();
			lf.lfWeight = FW_BOLD;
			lf.lfHeight = 80;
			lf.lfQuality = ANTIALIASED_QUALITY;
			std::strncpy(lf.lfFaceName,(LPCTSTR)sFace,32);
			if(!b_font_bold.CreatePointFontIndirect(&lf))
			{
				b_font_bold.CreatePointFont(80,"Tahoma Bold");
			}
			LoadMachineDial();

			///\todo: Get this information from the dial bitmap instead of having it hardcoded.
			dialwidth = 28;
			dialheight = 28;
			dialframes = 63;
		}
		NativeGUISettings::~NativeGUISettings()
		{
			b_font.DeleteObject();
			b_font_bold.DeleteObject();
		}

		void NativeGUISettings::LoadMachineDial()
		{
			dial.DeleteObject();
			if ( hbmMachineDial) DeleteObject(hbmMachineDial);
			if (Global::pConfig->bBmpDial)
			{
				Global::pConfig->bBmpDial=FALSE;
				hbmMachineDial = (HBITMAP)LoadImage(NULL, Global::pConfig->szBmpDialFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
				if (hbmMachineDial)
				{
					if (dial.Attach(hbmMachineDial))
					{	
						BITMAP bm;
						GetObject(hbmMachineDial,sizeof(BITMAP),&bm);

						if ((bm.bmWidth == 1792) && (bm.bmHeight == 28))
						{
							Global::pConfig->bBmpDial=TRUE;
						}
						else
							dial.LoadBitmap(IDB_KNOB);
					}
				}
			}
			else 
				dial.LoadBitmap(IDB_KNOB);
		}


		CNativeGui::CNativeGui(Machine* effect)
		:ncol(0)
		,numParameters(0)
		,parspercol(0)
		,istweak(false)
		,finetweak(false)
		,ultrafinetweak(false)
		,tweakpar(0)
		,tweakbase(0)
		,minval(0)
		,maxval(0)
		,sourcepoint(0)
		,prevval(0)
		{
			SelectMachine(effect);
		}

		bool CNativeGui::GetViewSize(CRect& rect)
		{
			rect.left= rect.top = 0;
			rect.right = ncol * W_ROWWIDTH;
			rect.bottom = parspercol * uiSetting().dialheight;
			return true;
		}

		void CNativeGui::SelectMachine(Machine* pMachine)
		{
			_pMachine = pMachine;
			numParameters = _pMachine->GetNumParams();
			ncol = _pMachine->GetNumCols();
			if ( ncol == 0 )
			{
				ncol = 1;
				while ( (numParameters/ncol)*uiSetting().dialheight > ncol*W_ROWWIDTH ) ncol++;
			}
			parspercol = numParameters/ncol;
			if ( parspercol*ncol < numParameters) parspercol++; // check if all the parameters are visible.
		}

		int CNativeGui::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			GetDesktopWindow()->GetWindowRect(&uiSetting().deskrect);
			return 0;
		}

		void CNativeGui::OnDestroy() 
		{
			CWnd::OnDestroy();
		}

		void CNativeGui::OnTimer(UINT nIDEvent) 
		{
			if ( nIDEvent == 2104+machine()._macIndex )
			{
				Invalidate(false);
			}
			CWnd::OnTimer(nIDEvent);
		}

		void CNativeGui::OnSetFocus(CWnd* pOldWnd) 
		{
//			GetParent()->OnSetFocus(pOldWnd);
			WindowIdle();
		}


		///////////////////////////////////////////////////////////////////////
		// GUI PAINTING HERE
		///////////////////////////////////////////////////////////////////////

		void CNativeGui::OnPaint() 
		{
			int const K_XSIZE2=uiSetting().dialwidth+8;
			int const K_YSIZE2=uiSetting().dialheight/2;

			CRect rect;
			GetClientRect(&rect);

			CPaintDC dc(this);
			CFont* oldfont=dc.SelectObject(&uiSetting().b_font);

			CDC memDC;
			CBitmap* oldbmp;
			memDC.CreateCompatibleDC(&dc);
			oldbmp=memDC.SelectObject(&uiSetting().dial);

			int y_knob, x_knob,knob_c;
			y_knob = x_knob = knob_c = 0;
			char parName[64];
			std::memset(parName,0,64);

			for (int c=0; c<numParameters; c++)
			{
				char buffer[128];

				BOOL bDrawKnob = TRUE;
				int min_v, max_v, val_v;
				min_v = max_v = val_v = 1;

				_pMachine->GetParamName(c,parName);
				_pMachine->GetParamRange(c,min_v,max_v);
				val_v = _pMachine->GetParamValue(c);
				_pMachine->GetParamValue(c,buffer);
				bDrawKnob = (min_v==max_v)?false:true;

				if(bDrawKnob && (max_v - min_v)>0)
				{
					int const amp_v = max_v - min_v;
					int const rel_v = val_v - min_v;

					int const frame = (uiSetting().dialframes*rel_v)/amp_v;
					int const xn = frame*uiSetting().dialwidth;

					dc.BitBlt(x_knob,y_knob,uiSetting().dialwidth,uiSetting().dialheight,&memDC,xn,0,SRCCOPY);
				
					if ((tweakpar == c) && (istweak))
					{
						dc.SetBkColor(uiSetting().hTopColor);
						dc.SetTextColor(uiSetting().fonthTopColor);
					}
					else
					{
						dc.SetBkColor(uiSetting().topColor);
						dc.SetTextColor(uiSetting().fontTopColor);
					}
					dc.ExtTextOut(K_XSIZE2+x_knob, y_knob, ETO_OPAQUE, CRect(uiSetting().dialwidth+x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), CString(parName), 0);
					
					if ((tweakpar == c) && (istweak))
					{
						dc.SetBkColor(uiSetting().hBottomColor);
						dc.SetTextColor(uiSetting().fonthBottomColor);
					}
					else
					{
						dc.SetBkColor(uiSetting().bottomColor);
						dc.SetTextColor(uiSetting().fontBottomColor);
					}
					dc.ExtTextOut(K_XSIZE2 + x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(uiSetting().dialwidth+x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+uiSetting().dialheight), CString(buffer), 0);
				}
				else
				{
					if(!std::strlen(parName) /* <bohan> don't know what pooplog's plugins use for separators... */ || std::strlen(parName) == 1)
					{
						dc.SetBkColor(uiSetting().topColor);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), "", 0);
						
						dc.SetBkColor(uiSetting().bottomColor);
						dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+uiSetting().dialheight), "", 0);
					}
					else
					{
						dc.SetBkColor(uiSetting().topColor);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH + x_knob, y_knob + uiSetting().dialheight / 4), "", 0);
					
						dc.SetBkColor(uiSetting().titleColor);
						dc.SetTextColor(uiSetting().fonttitleColor);

						dc.SelectObject(&uiSetting().b_font_bold);
						dc.ExtTextOut(x_knob + 8, y_knob + uiSetting().dialheight / 4, ETO_OPAQUE, CRect(x_knob, y_knob + uiSetting().dialheight / 4, W_ROWWIDTH + x_knob, y_knob + uiSetting().dialheight * 3 / 4), CString(parName), 0);
						dc.SelectObject(&uiSetting().b_font);

						dc.SetBkColor(uiSetting().bottomColor);
						dc.ExtTextOut(x_knob, y_knob + uiSetting().dialheight * 3 / 4, ETO_OPAQUE, CRect(x_knob, y_knob + uiSetting().dialheight * 3 / 4, W_ROWWIDTH + x_knob, y_knob + uiSetting().dialheight), "", 0);
					}
				}
				y_knob += uiSetting().dialheight;

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
					dc.SetBkColor(uiSetting().topColor);
					dc.SetTextColor(uiSetting().fontTopColor);
					dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), "", 0);

					dc.SetBkColor(uiSetting().bottomColor);
					dc.SetTextColor(uiSetting().fontBottomColor);
					dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+uiSetting().dialheight), "", 0);

					y_knob += uiSetting().dialheight;
				}
			}
			memDC.SelectObject(oldbmp);
			memDC.DeleteDC();
			dc.SelectObject(oldfont);
		}

		int CNativeGui::ConvertXYtoParam(int x, int y)
		{
			if ((y/uiSetting().dialheight) >= parspercol ) return -1; //this if for VST's that use the native gui.
			return (y/uiSetting().dialheight) + ((x/W_ROWWIDTH)*parspercol);
		}

		void CNativeGui::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			tweakpar = ConvertXYtoParam(point.x,point.y);
			if ((tweakpar > -1) && (tweakpar < numParameters))
			{
				sourcepoint = point.y;
				tweakbase = _pMachine->GetParamValue(tweakpar);
				prevval = tweakbase;
				_pMachine->GetParamRange(tweakpar,minval,maxval);
				istweak = true;
//				wndView->AddMacViewUndo();
				SetCapture();
			}
			else
			{
				istweak = false;
			}
			CWnd::OnLButtonDown(nFlags, point);
		}

		void CNativeGui::OnLButtonDblClk(UINT nFlags, CPoint point)
		{
/*
			if( _pMachine->_type == MACH_PLUGIN)
			{
				int par = ConvertXYtoParam(point.x,point.y);
				if(par>=0 && par <= ((Plugin*)_pMachine)->GetNumParams() )
				{
					wndView->AddMacViewUndo();
					_pMachine->SetParameter(par,  ((Plugin*)_pMachine)->GetInfo()->Parameters[par]->DefValue);
				}
			}
			Invalidate(false);
*/
			CWnd::OnLButtonDblClk(nFlags, point);
		}

		void CNativeGui::OnMouseMove(UINT nFlags, CPoint point) 
		{
			if (istweak)
			{
				///\todo: This code fools some VST's that have quantized parameters (i.e. tweaking to 0x3579 rounding to 0x3000)
				///       It should be interesting to know what is "somewhere else".
/*				int curval = _pMachine->GetParamValue(tweakpar);
				tweakbase -= prevval-curval;					//adjust base for tweaks from somewhere else
				if(tweakbase<minval) tweakbase=minval;
				if(tweakbase>maxval) tweakbase=maxval;
*/
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
				int screenh = uiSetting().deskrect.bottom;
				if ( ultrafinetweak ) freak = 0.5f;
				else if (maxval-minval < screenh/4) freak = (maxval-minval)/float(screenh/4);
				else if (maxval-minval < screenh*2/3) freak = (maxval-minval)/float(screenh/3);
				else freak = (maxval-minval)/float(screenh*3/5);
				if (finetweak) freak/=5;

				double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase;

				if (nv < minval) nv = minval;
				if (nv > maxval) nv = maxval;

				_pMachine->SetParameter(tweakpar,(int) (nv+0.5f));  // +0.5f to round correctly, not like "floor".
				prevval=(int)(nv+0.5f);
				///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
//				wndView->AddMacViewUndo();
				if(Global::configuration()._RecordTweaks)
				{
					if(Global::configuration()._RecordMouseTweaksSmooth)
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(machine()._macIndex, tweakpar, prevval);
					else
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(machine()._macIndex, tweakpar, prevval);
				}
//				if(pParamGui)
//					pParamGui->UpdateNew(index, value);


				Invalidate(false);
			}
			CWnd::OnMouseMove(nFlags, point);
		}

		void CNativeGui::OnLButtonUp(UINT nFlags, CPoint point) 
		{
			istweak = false;
			Invalidate(false);	
			ReleaseCapture();
			CWnd::OnLButtonUp(nFlags, point);
		}

		void CNativeGui::OnRButtonUp(UINT nFlags, CPoint point) 
		{
			tweakpar = ConvertXYtoParam(point.x,point.y);

			if ((tweakpar > -1) && (tweakpar < numParameters))
			{
				if (nFlags & MK_CONTROL)
				{
/*					Global::_pSong->seqBus = MachineIndex;//Global::_pSong->FindBusFromIndex(MachineIndex);
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(FALSE);
					CComboBox *cb2=(CComboBox *)((CMainFrame *)theApp.m_pMainWnd)->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cb2->SetCurSel(AUX_PARAMS); // PARAMS
					Global::_pSong->auxcolSelected=tweakpar;
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboIns();
*/
				}
				else 
				{		
					int min_v=1;
					int max_v=1;
					char name[64],title[128];
					memset(name,0,64);

					_pMachine->GetParamRange(tweakpar,min_v,max_v);
					_pMachine->GetParamName(tweakpar,name);
					std::sprintf
						(
						title, "Param:'%.2x:%s' (Range from %d to %d)\0",
						tweakpar,
						name,
						min_v,
						max_v
						);

					CNewVal dlg(machine()._macIndex,tweakpar,_pMachine->GetParamValue(tweakpar),min_v,max_v,title);
					if ( dlg.DoModal() == IDOK)
					{
//						wndView->AddMacViewUndo();
						_pMachine->SetParameter(tweakpar,(int)dlg.m_Value);
					}
					Invalidate(false);
				}
			}
			CWnd::OnRButtonUp(nFlags, point);
		}

		void CNativeGui::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			// ignore repeats: nFlags&0x4000
			const BOOL bRepeat = nFlags&0x4000;
			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			if(cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					if (!bRepeat){
						const int outnote = cmd.GetNote();
						if ( _pMachine->_mode == MACHMODE_GENERATOR || Global::pConfig->_notesToEffects)
							Global::pInputHandler->PlayNote(outnote,127,true,_pMachine);
						else
							Global::pInputHandler->PlayNote(outnote,127,true, 0);
					}
					break;

				case CT_Immediate:
				case CT_Editor:
					Global::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}

			this->SetFocus();

			//wndView->KeyDown(nChar,nRepCnt,nFlags);
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
		}

		void CNativeGui::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
			CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
