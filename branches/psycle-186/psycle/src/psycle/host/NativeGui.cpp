///\file
///\brief implementation file for psycle::host::CNativeGui.

#include "NativeGui.hpp"
#include "NativeGraphics.hpp"
#include "Machine.hpp"
#include "Plugin.hpp" // For default parameter value.

#include "ChildView.hpp"
#include "NewVal.hpp"

#define DEFAULT_ROWWIDTH  150


namespace psycle { namespace host {

	extern CPsycleApp theApp;
		
	PsycleConfig::MachineParam* CNativeGui::uiSetting;

		BEGIN_MESSAGE_MAP(CNativeGui, CWnd)
			ON_WM_CREATE()
			ON_WM_SETFOCUS()
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_WM_RBUTTONUP()
		END_MESSAGE_MAP()


		CNativeGui::CNativeGui(CFrameMachine* frame,Machine* effect, CChildView* view)
		:CBaseParamView(frame)
		,ncol(0)
		,numParameters(0)
		,parspercol(0)
		,colwidth(DEFAULT_ROWWIDTH)
		,istweak(false)
		,finetweak(false)
		,ultrafinetweak(false)
		,tweakpar(0)
		,tweakbase(0)
		,visualtweakvalue(0.0f)
		,minval(0)
		,maxval(0)
		,sourcepoint(0)
		,prevval(0)
		,mainView(view)
		{
			SelectMachine(effect);
		}

		int CNativeGui::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			return 0;
		}

		BOOL CNativeGui::PreCreateWindow(CREATESTRUCT& cs)
		{
			if (!CWnd::PreCreateWindow(cs))
				return FALSE;
			
			cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
			cs.style &= ~WS_BORDER;
			cs.lpszClass = AfxRegisterWndClass(0);
			return TRUE;
		}

		void CNativeGui::OnSetFocus(CWnd* pOldWnd) 
		{
			CWnd::OnSetFocus(pOldWnd);
			GetParent()->SetFocus();
		}


		void CNativeGui::OnPaint() 
		{
			int realheight = uiSetting->dialheight+1;
			int realwidth = colwidth+1;
			int const K_XSIZE2=uiSetting->dialwidth;
			int const infowidth=colwidth-uiSetting->dialwidth;
			const COLORREF titleColor = uiSetting->titleColor;

			CPaintDC dc(this);
			CFont* oldfont=dc.SelectObject(&uiSetting->font);

			CDC knobDC;
			CBitmap* oldKnobbmp;
			knobDC.CreateCompatibleDC(&dc);
			oldKnobbmp=knobDC.SelectObject(&uiSetting->dial);

			int y_knob, x_knob,knob_c;
			char parName[64];
			y_knob = x_knob = knob_c = 0;
			std::memset(parName,0,64);

			for (int c=0; c<numParameters; c++)
			{
				_pMachine->GetParamName(c,parName);
				int type = _pMachine->GetParamType(c);
				if(type == 2) // STATE
				{
					char buffer[128];
					int min_v, max_v;
					min_v = max_v = 0;

					_pMachine->GetParamRange(c,min_v,max_v);
					_pMachine->GetParamValue(c,buffer);

					int const amp_v = max_v - min_v;
					float rel_v;
					if ( istweak && c == tweakpar)
					{
						rel_v = visualtweakvalue - min_v;
					} else {
						rel_v = _pMachine->GetParamValue(c) - min_v;
					}
					Knob::Draw(dc,knobDC,x_knob,y_knob,rel_v/amp_v);
					if ( istweak && c == tweakpar)
					{
						InfoLabel::DrawHLight(dc,K_XSIZE2+x_knob,y_knob,infowidth,parName,buffer);
					}
					else
					{
						InfoLabel::Draw(dc,K_XSIZE2+x_knob,y_knob,infowidth,parName,buffer);
					}
				}
				else if(type == 1) // HEADER
				{
					InfoLabel::DrawHeader(dc,x_knob,y_knob,colwidth,parName);
				}
				else
				{
					InfoLabel::Draw(dc,x_knob,y_knob,colwidth,"","");
				}
				dc.Draw3dRect(x_knob-1,y_knob-1,colwidth+2,uiSetting->dialheight+2,titleColor,titleColor);
				y_knob += realheight;

				++knob_c;

				if (knob_c >= parspercol)
				{
					knob_c = 0;
					x_knob += realwidth;
					y_knob = 0;
				}
			}

			int exess= parspercol*ncol;
			if ( exess > numParameters )
			{
				for (int c=numParameters; c<exess; c++)
				{
					InfoLabel::Draw(dc,x_knob,y_knob,colwidth,"","");
					dc.Draw3dRect(x_knob-1,y_knob-1,colwidth+2,uiSetting->dialheight+2,titleColor,titleColor);
					y_knob += realheight;
				}
			}
			knobDC.SelectObject(oldKnobbmp);
			knobDC.DeleteDC();
			dc.SelectObject(oldfont);
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
				visualtweakvalue= tweakbase;
				mainView->AddMacViewUndo();
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
			if( _pMachine->_type == MACH_PLUGIN)
			{
				int par = ConvertXYtoParam(point.x,point.y);
				if(par>=0 && par <= _pMachine->GetNumParams() )
				{
					mainView->AddMacViewUndo();
					_pMachine->SetParameter(par,  ((Plugin*)_pMachine)->GetInfo()->Parameters[par]->DefValue);
				}
			}
			Invalidate(false);
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
				int screenh = uiSetting->deskrect.bottom;
				if ( ultrafinetweak ) freak = 0.5f;
				else if (maxval-minval < screenh/4) freak = (maxval-minval)/float(screenh/4);
				else if (maxval-minval < screenh*2/3) freak = (maxval-minval)/float(screenh/3);
				else freak = (maxval-minval)/float(screenh*3/5);
				if (finetweak) freak/=5;

				double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase;

				if (nv < minval) nv = minval;
				if (nv > maxval) nv = maxval;
				visualtweakvalue = nv;
				_pMachine->SetParameter(tweakpar,(int) (nv+0.5f));  // +0.5f to round correctly, not like "floor".
				prevval=(int)(nv+0.5f);
				///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
				mainView->AddMacViewUndo();
				if(Global::psycleconf().inputHandler()._RecordTweaks)
				{
					if(Global::psycleconf().inputHandler()._RecordMouseTweaksSmooth)
					{
						mainView->MousePatternTweakSlide(machine()._macIndex, tweakpar, prevval);
					}
					else
					{
						mainView->MousePatternTweak(machine()._macIndex, tweakpar, prevval);
					}
				}
/*
				if(pParamGui)
					pParamGui->UpdateNew(index, value);
*/

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
					
/*					Global::_pSong->seqBus = machine()._macIndex;
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(FALSE);
					CComboBox *cb2=(CComboBox *)((CMainFrame *)theApp.m_pMainWnd)->m_machineBar.GetDlgItem(IDC_AUXSELECT);
					cb2->SetCurSel(AUX_PARAMS);
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
						mainView->AddMacViewUndo();
						_pMachine->SetParameter(tweakpar,(int)dlg.m_Value);
					}
					Invalidate(false);
				}
			}
			CWnd::OnRButtonUp(nFlags, point);
		}

		int CNativeGui::ConvertXYtoParam(int x, int y)
		{
			int realheight = uiSetting->dialheight+1;
			int realwidth = colwidth+1;
			if (y/realheight >= parspercol ) return -1; //this if for VST's that use the native gui.
			return (y/realheight) + ((x/realwidth)*parspercol);
		}

		bool CNativeGui::GetViewSize(CRect& rect)
		{
			int realheight = uiSetting->dialheight+1;
			int realwidth = colwidth+1;
			rect.left= rect.top = 0;
			rect.right = ncol * realwidth;
			rect.bottom = parspercol * realheight;
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
				while ( (numParameters/ncol)*uiSetting->dialheight > ncol*colwidth ) ncol++;
			}
			parspercol = numParameters/ncol;
			if ( parspercol*ncol < numParameters) parspercol++; // check if all the parameters are visible.
		}

	}   // namespace
}   // namespace
