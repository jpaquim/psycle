///\file
///\brief implementation file for psycle::host::CFrameMachine.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "FrameMachine.hpp"
#include "psycle.hpp"
#include "Childview.hpp"
#include "NewVal.hpp"
#include "PresetsDlg.hpp"
//#include "InputHandler.hpp"
#include "MainFrm.hpp"
#include <psycle/engine/machine.hpp>
#include <psycle/engine/plugin.hpp>
#include <psycle/engine/VSTHost.hpp>
#include <psycle/helpers/helpers.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		extern CPsycleApp theApp;

		IMPLEMENT_DYNCREATE(CFrameMachine, CFrameWnd)

		BEGIN_MESSAGE_MAP(CFrameMachine, CFrameWnd)
			//{{AFX_MSG_MAP(CFrameMachine)
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()
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

		CFrameMachine::CFrameMachine()
		{
			//do not use! Use OnCreate Instead.
		}

		CFrameMachine::~CFrameMachine()
		{
			//do not use! Use OnDestroy Instead.
		}

		int CFrameMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			istweak=false;
			tweakpar=0;
			tweakbase=0;
			minval=0;
			maxval=0;
			prevval=0;
			finetweak=false;
			ultrafinetweak=false;
			numParameters=0;
			ncol=0;
			parspercol=0;

			font.CreatePointFont(80,"Tahoma");
			//			font_bold.CreatePointFont(80,"Tahoma Bold");
			CString sFace("Tahoma");
			LOGFONT lf = LOGFONT();
			lf.lfWeight = FW_BOLD;
			lf.lfHeight = 80;
			lf.lfQuality = NONANTIALIASED_QUALITY;
			std::strncpy(lf.lfFaceName,(LPCTSTR)sFace,32);
			if(!font_bold.CreatePointFontIndirect(&lf))
			{
				font_bold.CreatePointFont(80,"Tahoma Bold");
			}

			SetTimer(2104+MachineIndex,100,0);
			return 0;
		}

		void CFrameMachine::OnDestroy() 
			{
			if ( _pActive != NULL ) *_pActive = false;
			font.DeleteObject();
			font_bold.DeleteObject();
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

		void CFrameMachine::OnSetFocus(CWnd* pOldWnd) 
		{
			CFrameWnd::OnSetFocus(pOldWnd);
			Invalidate(false);
		}
		void CFrameMachine::Generate()
		{
			UpdateWindow();
		}

		void CFrameMachine::SelectMachine(Machine* pMachine)
		{
			_pMachine = pMachine;

			// Get NumParameters
			numParameters = _pMachine->GetNumParams();
			ncol = _pMachine->GetNumCols();
			if ( _pMachine->subclass() == MACH_PLUGIN )
			{
				GetMenu()->GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, ((Plugin*)_pMachine)->GetInfo()->Command);
				if( ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI)
				{
					//all this is to set the window size for the plugin in terms of oldstyle parameters
					int maxX=0, maxY=0;
					int curX=0, curY=0;

					for(int c(0); c<numParameters; ++c)
					{
						curX = ((Plugin*)_pMachine)->GetParam(c)->GetExtent().x;
						curY = ((Plugin*)_pMachine)->GetParam(c)->GetExtent().y;

						if(curX>maxX) maxX=curX;
						if(curY>maxY) maxY=curY;
					}

					while(ncol*W_ROWWIDTH < maxX) ncol++;
					parspercol=1;
					while(parspercol*K_YSIZE < maxY) parspercol++;
				}
			}
			else if ( _pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX )
			{
				while ( (numParameters/ncol)*K_YSIZE > ncol*W_ROWWIDTH ) ncol++;
			}

			if( !(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI))
			{
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
			}

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


		///////////////////////////////////////////////////////////////////////
		// PAINT GUI HERE
		///////////////////////////////////////////////////////////////////////

		void CFrameMachine::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting

			CFont* oldfont=dc.SelectObject(&font);

			CRect rect;
			GetClientRect(&rect);
			int const K_XSIZE2=K_XSIZE+8;
			int const K_YSIZE2=K_YSIZE/2;
		//	int hsp=0;

			if(!(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI))
			{
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
							dc.SetBkColor(UIGlobal::configuration().machineGUIHTopColor);
							dc.SetTextColor(UIGlobal::configuration().machineGUIHFontTopColor);
						}
						else
						{
							dc.SetBkColor(UIGlobal::configuration().machineGUITopColor);
							dc.SetTextColor(UIGlobal::configuration().machineGUIFontTopColor);
						}
						dc.ExtTextOut(K_XSIZE2+x_knob, y_knob, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), CString(parName), 0);
						
						//dc.SetBkColor(0x00687D83 + nc*2);
						//dc.SetTextColor(0x0044EEFF + nc);
						if ((tweakpar == c) && (istweak))
						{
							dc.SetBkColor(UIGlobal::configuration().machineGUIHBottomColor);
							dc.SetTextColor(UIGlobal::configuration().machineGUIHFontBottomColor);
						}
						else
						{
							dc.SetBkColor(UIGlobal::configuration().machineGUIBottomColor);
							dc.SetTextColor(UIGlobal::configuration().machineGUIFontBottomColor);
						}
						dc.ExtTextOut(K_XSIZE2 + x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+K_YSIZE), CString(buffer), 0);
					}
					else
					{
						if(!std::strlen(parName) /* <bohan> don't know what pooplog's plugins use for separators... */ || std::strlen(parName) == 1)
						{
							//dc.SetBkColor(0x00788D93);
							dc.SetBkColor(UIGlobal::configuration().machineGUITopColor);
							dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), "", 0);
							
							//dc.SetBkColor(0x00687D83);
							dc.SetBkColor(UIGlobal::configuration().machineGUIBottomColor);
							dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+K_YSIZE), "", 0);
						}
						else
						{
							//dc.SetBkColor(0x00788D93);
							dc.SetBkColor(UIGlobal::configuration().machineGUITopColor);
							dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH + x_knob, y_knob + K_YSIZE / 4), "", 0);
						
							//dc.SetBkColor(0x0088a8b4);
							//dc.SetTextColor(0x00FFFFFF);
							dc.SetBkColor(UIGlobal::configuration().machineGUITitleColor);
							dc.SetTextColor(UIGlobal::configuration().machineGUITitleFontColor);

							dc.SelectObject(&font_bold);
							dc.ExtTextOut(x_knob + 8, y_knob + K_YSIZE / 4, ETO_OPAQUE, CRect(x_knob, y_knob + K_YSIZE / 4, W_ROWWIDTH + x_knob, y_knob + K_YSIZE * 3 / 4), CString(parName), 0);
							dc.SelectObject(&font);

							//dc.SetBkColor(0x00687D83);
							dc.SetBkColor(UIGlobal::configuration().machineGUIBottomColor);
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
						dc.SetBkColor(UIGlobal::configuration().machineGUITopColor);
						dc.SetTextColor(UIGlobal::configuration().machineGUIFontTopColor);
						dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, W_ROWWIDTH+x_knob, y_knob+K_YSIZE2), "", 0);

						//dc.SetBkColor(0x00687D83);
						//dc.SetTextColor(0x0044EEFF);
						dc.SetBkColor(UIGlobal::configuration().machineGUIBottomColor);
						dc.SetTextColor(UIGlobal::configuration().machineGUIFontBottomColor);
						dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, W_ROWWIDTH+x_knob, y_knob+K_YSIZE), "", 0);

						y_knob += K_YSIZE;
					}
				}
				memDC.SelectObject(oldbmp);
				memDC.DeleteDC();
				dc.SelectObject(oldfont);
			}
			else
			{
				char paramValue[128];
				CRect clientRect;
				GetClientRect(&clientRect);
				CBitmap* bmpBuffer = new CBitmap;
				CBitmap* oldbmp;
				bmpBuffer->CreateCompatibleBitmap(&dc, clientRect.right-clientRect.left, clientRect.bottom-clientRect.top);

				CDC memdc;
				memdc.CreateCompatibleDC(&dc);
				oldbmp = memdc.SelectObject(bmpBuffer);

				for(int c=numParameters-1;c>=0;c--)
				{
					_pMachine->GetParamValue(c,paramValue);
					((Plugin*)_pMachine)->GetParam(c)->Paint(&memdc,paramValue);
				}
				dc.BitBlt(0, 0, clientRect.right-clientRect.left, clientRect.bottom-clientRect.top, &memdc, 0, 0, SRCCOPY);

				memdc.SelectObject(oldbmp);
				memdc.DeleteDC();
				bmpBuffer->DeleteObject();
			}
		}

		int CFrameMachine::ConvertXYtoParam(int x, int y)
		{
			if ((y/K_YSIZE) >= parspercol ) return -1; //this if for VST's that use the native gui.
			return (y/K_YSIZE) + ((x/150)*parspercol);
		}
		void CFrameMachine::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			if(!(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI))
			{
				tweakpar = ConvertXYtoParam(point.x,point.y);
				if ((tweakpar > -1) && (tweakpar < numParameters))
				{
					sourcepoint = point.y;
					tweakbase = _pMachine->GetParamValue(tweakpar);
					prevval = tweakbase;
					_pMachine->GetParamRange(tweakpar,minval,maxval);
					istweak = true;
					wndView->AddMacViewUndo();
					SetCapture();
				}
				else
				{
					istweak = false;
				}
			}
			else
			{
				int c=0, newval;
				while(c<numParameters && !((Plugin*)_pMachine)->GetParam(c)->PointInParam(point.x,point.y))
				{
					++c;
				}
				tweakpar=c;
				if(tweakpar<numParameters && tweakpar>=0)
				{
					if(((Plugin*)_pMachine)->GetParam(tweakpar)->bClickable || ((Plugin*)_pMachine)->GetParam(tweakpar)->bTweakable)
					{
						if(((Plugin*)_pMachine)->GetParam(tweakpar)->LButtonDown(point.x, point.y, newval))
						{
							wndView->AddMacViewUndo();
							_pMachine->SetParameter(tweakpar,newval);
							if (UIGlobal::configuration()._RecordTweaks)
							{
								if (UIGlobal::configuration()._RecordMouseTweaksSmooth)
								{
									wndView->MousePatternTweakSlide(MachineIndex, tweakpar, newval-((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue);
								}
								else
								{
									wndView->MousePatternTweak(MachineIndex, tweakpar, newval-((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue);
								}
							}
 						}
					}
					if(((Plugin*)_pMachine)->GetParam(tweakpar)->bTweakable)
					{
						istweak = true;
						SetCapture();
					}
				}
				else
					istweak = false;	//not sure why this is necessary on mousedown, but everybody else is doing it, so whatever

			}
			CFrameWnd::OnLButtonDown(nFlags, point);
		}


	void CFrameMachine::OnLButtonDblClk(UINT nFlags, CPoint point)
		{
			if( _pMachine->subclass() == MACH_PLUGIN)
			{
				if( !(((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI) )
				{
					int par = ConvertXYtoParam(point.x,point.y);
					if(par>=0 && par <= ((Plugin*)_pMachine)->GetNumParams() )
					{
						wndView->AddMacViewUndo();
						_pMachine->SetParameter(par,  ((Plugin*)_pMachine)->GetInfo()->Parameters[par]->DefValue);
					}
				}
				else
				{
					int newval;
					int c=0;
					while(	c<numParameters && !((Plugin*)_pMachine)->GetParam(c)->PointInParam(point.x,point.y))
					{
						++c;
					}
					tweakpar=c;
					if(tweakpar<numParameters && tweakpar>=0)
					{
						if(((Plugin*)_pMachine)->GetParam(tweakpar)->bClickable || ((Plugin*)_pMachine)->GetParam(tweakpar)->bTweakable)

						{
							if(((Plugin*)_pMachine)->GetParam(tweakpar)->LButtonDown(point.x, point.y, newval))
							{
								wndView->AddMacViewUndo();
								_pMachine->SetParameter(tweakpar,newval);
								if (UIGlobal::configuration()._RecordTweaks)
								{
									if (UIGlobal::configuration()._RecordMouseTweaksSmooth)
									{
										wndView->MousePatternTweakSlide(MachineIndex, tweakpar, newval-((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue);
									}
									else
									{
										wndView->MousePatternTweak(MachineIndex, tweakpar, newval-((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue);
									}
								}
							}
						}
						if(((Plugin*)_pMachine)->GetParam(tweakpar)->bTweakable)

						{
							istweak = true;
							SetCapture();
						}
					}
					else
						istweak = false;	//not sure why this is necessary on mousedown, but everybody else is doing it, so whatever
				}
			}
			Invalidate(false);

			CFrameWnd::OnLButtonDblClk(nFlags, point);
		}


		void CFrameMachine::OnMouseMove(UINT nFlags, CPoint point) 
		{
			if (istweak)
			{
				int curval = _pMachine->GetParamValue(tweakpar);
				tweakbase -= prevval-curval;					//adjust base for tweaks from somewhere else
				if(tweakbase<minval) tweakbase=minval;
				if(tweakbase>maxval) tweakbase=maxval;

				if (( ultrafinetweak && !(nFlags & MK_SHIFT )) || //shift-key has been left.
					( !ultrafinetweak && (nFlags & MK_SHIFT))) //shift-key has just been pressed
				{
					tweakbase = _pMachine->GetParamValue(tweakpar);
					sourcepoint=point.y;
					ultrafinetweak=!ultrafinetweak;
					if(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI)
					{
						((Plugin*)_pMachine)->GetParam(tweakpar)->ResetTweakSrc(point);
					}
				}
				else if (( finetweak && !(nFlags & MK_CONTROL )) || //control-key has been left.
					( !finetweak && (nFlags & MK_CONTROL))) //control-key has just been pressed
				{
					tweakbase = _pMachine->GetParamValue(tweakpar);
					sourcepoint=point.y;
					finetweak=!finetweak;
					if(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI)
					{
						((Plugin*)_pMachine)->GetParam(tweakpar)->ResetTweakSrc(point);
					}
				}

				double freak;
				int screenh = wndView->CH;
				if ( ultrafinetweak ) freak = 0.5f;
				else if (maxval-minval < screenh/4) freak = (maxval-minval)/float(screenh/4);
				else if (maxval-minval < screenh*2/3) freak = (maxval-minval)/float(screenh/3);
				else freak = (maxval-minval)/float(screenh*3/5);
				if (finetweak) freak/=5;

				if(!(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI))
				{
					double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase;

					if (nv < minval) nv = minval;
					if (nv > maxval) nv = maxval;

					_pMachine->SetParameter(tweakpar,(int) nv);
					prevval=(int)nv;

					if (UIGlobal::configuration()._RecordTweaks)
					{
						if (UIGlobal::configuration()._RecordMouseTweaksSmooth)
						{
							wndView->MousePatternTweakSlide(MachineIndex, tweakpar, ((int)nv)-minval);
						}
						else
						{
							wndView->MousePatternTweak(MachineIndex, tweakpar, ((int)nv)-minval);
						}
					}
				}
				else
				{
					int setvals[5];			//i can't imagine needing 5, but who am i to judge?
					int setparams[5];
					setparams[0]=tweakpar;
					int setcount = ((Plugin*)_pMachine)->GetParam(tweakpar)->WhatDoITweak(setparams, setvals, point.x, point.y, nFlags);
					for(int i(0);i<setcount;++i)
					{
						if(	   setparams[i] < numParameters		&&		setparams[i] >= 0 
							&& setvals[i] >= ((Plugin*)_pMachine)->GetInfo()->Parameters[setparams[i]]->MinValue
							&& setvals[i] <= ((Plugin*)_pMachine)->GetInfo()->Parameters[setparams[i]]->MaxValue)
						{
							_pMachine->SetParameter(setparams[i],setvals[i]);
							if (UIGlobal::configuration()._RecordTweaks)	//todo: while i haven't tested it, i can't imagine recording tweaks on something like an
							{									//xymod grid coming out too well.. to do it properly, we'd need to implement a means
																//of splitting the recording into separate tracks for each parameter.
								if (UIGlobal::configuration()._RecordMouseTweaksSmooth)
								{
									wndView->MousePatternTweakSlide(MachineIndex, setparams[i], setvals[i]-((Plugin*)_pMachine)->GetInfo()->Parameters[setparams[i]]->MinValue);
								}
								else
								{
									wndView->MousePatternTweak(MachineIndex, setparams[i], setvals[i]-((Plugin*)_pMachine)->GetInfo()->Parameters[setparams[i]]->MinValue);
								}
							}//end if
						}//end if
					}//end for	
				}//end else

				

				Invalidate(false);
			}
			CFrameWnd::OnMouseMove(nFlags, point);
		}

		void CFrameMachine::OnLButtonUp(UINT nFlags, CPoint point) 
		{
			if(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI)
			{
				if(tweakpar>=0 && tweakpar<numParameters)
				{
					if(((Plugin*)_pMachine)->GetParam(tweakpar)->bClickable || ((Plugin*)_pMachine)->GetParam(tweakpar)->bTweakable)
					{
						int newval;
						if( ((Plugin*)_pMachine)->GetParam(tweakpar)->LButtonUp(point.x, point.y, newval))
						{
							_pMachine->SetParameter(tweakpar, newval);
							if (UIGlobal::configuration()._RecordTweaks)
							{
								if (UIGlobal::configuration()._RecordMouseTweaksSmooth)
								{
									wndView->MousePatternTweakSlide(MachineIndex, tweakpar, newval-((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue);
								}
								else
								{
									wndView->MousePatternTweak(MachineIndex, tweakpar, newval-((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue);
								}
							}
						}
					}//endif
				}
			}//endif

			istweak = false;
			Invalidate(false);	
			ReleaseCapture();
			CFrameWnd::OnLButtonUp(nFlags, point);
		}

		void CFrameMachine::OnRButtonUp(UINT nFlags, CPoint point) 
		{
			if(!(_pMachine->subclass() == MACH_PLUGIN && ((Plugin*)_pMachine)->GetInfo()->Flags & plugin_interface::CUSTOM_GUI))
				tweakpar = ConvertXYtoParam(point.x,point.y);
			else
			{
				int c=0;
				while(c<numParameters && !((Plugin*)_pMachine)->GetParam(c)->PointInParam(point.x, point.y))
				{
					++c;
				}
				tweakpar=c;
			}

			if ((tweakpar > -1) && (tweakpar < numParameters))
			{
				if (nFlags & MK_CONTROL)
				{
					Global::song().seqBus = MachineIndex;//Global::song().FindBusFromIndex(MachineIndex);
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(false);
					CComboBox *cb2=(CComboBox *)((CMainFrame *)theApp.m_pMainWnd)->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cb2->SetCurSel(AUX_PARAMS); // PARAMS
					Global::song().auxcolSelected=tweakpar;
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboIns();
				}
				else 
				{	
					int min_v=1;
					int max_v=1;
					char name[64];
					std::memset(name,0,64);
					CNewVal dlg;

					_pMachine->GetParamName(tweakpar,name);
					_pMachine->GetParamRange(tweakpar,min_v,max_v);
					dlg.m_Value = _pMachine->GetParamValue(tweakpar);

					std::sprintf
						(
							dlg.title, "Param:'%.2x:%s' (Range from %d to %d)\0",
							tweakpar,
							name,
							min_v,
							max_v
						);
					dlg.min = min_v;
					dlg.max = max_v;
					dlg.macindex = MachineIndex;
					dlg.paramindex = tweakpar;
					if ( dlg.DoModal() == IDOK)
					{
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
						_pMachine->SetParameter(tweakpar,(int)nv);
					}
					Invalidate(false);
				}
			}
			CFrameWnd::OnRButtonUp(nFlags, point);
		}

		void CFrameMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			// ignore repeats: nFlags&0x4000
			const BOOL bRepeat = nFlags&0x4000;
			CmdDef cmd(UIGlobal::pInputHandler->KeyToCmd(nChar,nFlags));
			if(!bRepeat && cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					{
						const int outnote = cmd.GetNote();
						if ( _pMachine->_mode == MACHMODE_GENERATOR || UIGlobal::configuration()._notesToEffects)
							UIGlobal::pInputHandler->PlayNote(outnote,127,true,_pMachine);
						else
							UIGlobal::pInputHandler->PlayNote(outnote,127,true, 0);
					}
					break;

				case CT_Immediate:
					UIGlobal::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}

			//wndView->KeyDown(nChar,nRepCnt,nFlags);
			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
		}

		void CFrameMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{

			CmdDef cmd(UIGlobal::pInputHandler->KeyToCmd(nChar,nFlags));
			const int outnote = cmd.GetNote();
			if(outnote>=0)
			{
				if ( _pMachine->_mode == MACHMODE_GENERATOR ||UIGlobal::configuration()._notesToEffects)
				{
					UIGlobal::pInputHandler->StopNote(outnote,true,_pMachine);
				}
				else UIGlobal::pInputHandler->StopNote(outnote,true,NULL);
			}

			//wndView->KeyUp(nChar, nRepCnt, nFlags);
			CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
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
			if ( _pMachine->subclass() == MACH_PLUGIN)
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
			if ( _pMachine->subclass() == MACH_PLUGIN)
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
			if ( _pMachine->subclass() == MACH_PLUGIN)
			{
				MessageBox
					(
						"Authors: " + CString(((Plugin*)_pMachine)->GetInfo()->Author),
						"About " + CString(((Plugin*)_pMachine)->GetInfo()->Name)
					);
			}
		}


		void CFrameMachine::OnParametersShowpreset() 
		{
			CPresetsDlg dlg;
			dlg._pMachine=_pMachine;
			dlg.m_wndFrame=this;
			dlg.DoModal();
		}


	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

