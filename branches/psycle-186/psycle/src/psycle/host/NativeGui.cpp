///\file
///\brief implementation file for psycle::host::CNativeGui.

#include "NativeGui.hpp"

#include "Machine.hpp"
#include "Plugin.hpp" // For default parameter value.

#include "ChildView.hpp"
#include "NewVal.hpp"

#define DEFAULT_ROWWIDTH  150


namespace psycle { namespace host {

	extern CPsycleApp theApp;

	PsycleConfig::MachineParam* CNativeGui::uiSetting = NULL;

	int CNativeGui::InfoLabel::xoffset(3);
	int CNativeGui::GraphSlider::xoffset(6);

	//////////////////////////////////////////////////////////////////////////
	// Knob class
	void CNativeGui::Knob::Draw(CDC& dc, CDC& knobDC, int x_knob,int y_knob,float value)
	{
		const int numFrames = CNativeGui::uiSetting->dialframes;
		const int width = CNativeGui::uiSetting->dialwidth;
		const int height = CNativeGui::uiSetting->dialheight;
		const COLORREF titleColor = CNativeGui::uiSetting->titleColor;

		int pixel = numFrames*value;
		if (pixel >= numFrames) pixel=numFrames-1;
		pixel*=width;
		dc.BitBlt(x_knob, y_knob, width, height, &knobDC, pixel, 0, SRCCOPY);
		dc.Draw3dRect(x_knob, y_knob+height-1, width, 1, titleColor, titleColor);
	}
	bool  CNativeGui::Knob::LButtonDown( UINT nFlags, int x,int y)
	{
		if ( x< CNativeGui::uiSetting->dialwidth
			&& y < CNativeGui::uiSetting->dialheight) return true;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// InfoLabel class
	void CNativeGui::InfoLabel::Draw(CDC& dc, int x, int y, int width, const char *parName,const char *parValue)
	{
		const int height = CNativeGui::uiSetting->dialheight;
		const COLORREF titleColor = CNativeGui::uiSetting->titleColor;

		const int half = height/2;
		dc.Draw3dRect(x,y-1,width,height+1,titleColor,titleColor);
//		dc.FillSolidRect(x, y, width-1, half,topColor);
//		dc.FillSolidRect(x, y+half, width-1, half-1,bottomColor);

		dc.SetBkColor(CNativeGui::uiSetting->topColor);
		dc.SetTextColor(CNativeGui::uiSetting->fontTopColor);
		dc.ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width-1, y+half), CString(parName), 0);
	
		DrawValue(dc,x,y,width,parValue);
	}
	void CNativeGui::InfoLabel::DrawValue(CDC& dc, int x, int y, int width, const char *parValue)
	{
		const int height = CNativeGui::uiSetting->dialheight;
		const int half = height/2;
		dc.SetBkColor(CNativeGui::uiSetting->bottomColor);
		dc.SetTextColor(CNativeGui::uiSetting->fontBottomColor);
		dc.ExtTextOut(x+xoffset, y+half, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y+half, x+width-1, y+height-1), CString(parValue), 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// HLightInfoLabel class
	void CNativeGui::InfoLabel::DrawHLight(CDC& dc, int x, int y, int width, const char *parName,const char *parValue)
	{
		const int height = CNativeGui::uiSetting->dialheight;

		const int half = height/2;
		const int mywidth = width + CNativeGui::uiSetting->dialwidth;
//		dc.FillSolidRect(x, y, mywidth, half,titleColor);
//		dc.FillSolidRect(x, y+half, mywidth, half,bottomColor);

		dc.SetBkColor(CNativeGui::uiSetting->titleColor);
		dc.SetTextColor(CNativeGui::uiSetting->fonttitleColor);
		CFont *oldfont =dc.SelectObject(&CNativeGui::uiSetting->font_bold);
		dc.ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+mywidth-1, y+half), CString(parName), 0);
		dc.SelectObject(oldfont);

		DrawHLightValue(dc,x,y,width,parValue);
		dc.Draw3dRect(x-1,y-1,width+ CNativeGui::uiSetting->dialwidth+1,height+1,CNativeGui::uiSetting->titleColor,CNativeGui::uiSetting->titleColor);
	}
	void CNativeGui::InfoLabel::DrawHLightB(CDC& dc,int x, int y,int width,const char *parName,const char *parValue)
	{
		const int height = CNativeGui::uiSetting->dialheight;

		const int half = height/2;
		const int mywidth = width + CNativeGui::uiSetting->dialwidth;
		//		dc.FillSolidRect(x, y, mywidth, half,titleColor);
		//		dc.FillSolidRect(x, y+half, mywidth, half,bottomColor);

		dc.SetBkColor(CNativeGui::uiSetting->hTopColor);
		dc.SetTextColor(CNativeGui::uiSetting->fonthBottomColor);
		CFont *oldfont =dc.SelectObject(&CNativeGui::uiSetting->font_bold);
		dc.ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+mywidth-1, y+half), CString(parName), 0);
		dc.SelectObject(oldfont);

		DrawHLightValue(dc,x,y,width,parValue);
		dc.Draw3dRect(x-1,y-1,width+ CNativeGui::uiSetting->dialwidth+1,height+1,CNativeGui::uiSetting->titleColor,CNativeGui::uiSetting->titleColor);
	}
	void CNativeGui::InfoLabel::DrawHLightValue(CDC& dc, int x, int y, int width,const char *parValue)
	{
		const int height = CNativeGui::uiSetting->dialheight;
		const int half = height/2;
		const int mywidth = width + CNativeGui::uiSetting->dialwidth;
		dc.SetBkColor(CNativeGui::uiSetting->bottomColor);
		dc.SetTextColor(CNativeGui::uiSetting->fontBottomColor);
		dc.ExtTextOut(x+xoffset, y+half,ETO_OPAQUE | ETO_CLIPPED, CRect(x+1, y+half, x+mywidth-1, y+height), CString(parValue), 0);
	}
	void CNativeGui::InfoLabel::DrawHeader(CDC& dc, int x, int y, int width,const char *parName)
	{
		const int height = CNativeGui::uiSetting->dialheight;
		const int half = height/2;
		const int quarter = height/4;
		const int mywidth = width + CNativeGui::uiSetting->dialwidth;

		dc.FillSolidRect(x, y, mywidth, half,CNativeGui::uiSetting->topColor);
		dc.FillSolidRect(x, y+half, mywidth, half,CNativeGui::uiSetting->bottomColor);

		dc.SetBkColor(CNativeGui::uiSetting->titleColor);
		dc.SetTextColor(CNativeGui::uiSetting->fonttitleColor);

		CFont *oldfont = dc.SelectObject(&CNativeGui::uiSetting->font_bold);
		dc.ExtTextOut(x + xoffset, y + quarter, ETO_OPAQUE | ETO_CLIPPED, CRect(x+1, y + quarter, x+mywidth-1, y+half+quarter), CString(parName), 0);
		dc.SelectObject(&CNativeGui::uiSetting->font);
		dc.Draw3dRect(x-1,y-1,width+CNativeGui::uiSetting->dialwidth+1,height+1,CNativeGui::uiSetting->titleColor,CNativeGui::uiSetting->titleColor);
		dc.SelectObject(oldfont);
	}

	//////////////////////////////////////////////////////////////////////////
	// GraphSlider class
	void CNativeGui::GraphSlider::Draw(CDC& dc, CDC& backDC, CDC& knobDC, int x, int y,float value)
	{
		int width = CNativeGui::uiSetting->sliderwidth;
		int height = CNativeGui::uiSetting->sliderheight;
		dc.BitBlt(x,y,width,height,&backDC,0,0,SRCCOPY);
//		dc.FillSolidRect(x+width, y, InfoLabel::width, height-InfoLabel::height,topColor);
		dc.Draw3dRect(x-1,y-1,width+1,height+1,CNativeGui::uiSetting->titleColor,CNativeGui::uiSetting->titleColor);
		DrawKnob(dc,knobDC,x,y,value);
	}
	void CNativeGui::GraphSlider::DrawKnob(CDC& dc, CDC& knobDC, int x, int y, float value)
	{
		int height = CNativeGui::uiSetting->sliderheight;
		int knobheight = CNativeGui::uiSetting->sliderknobheight;
		int knobwidth = CNativeGui::uiSetting->sliderknobwidth;
		int ypos(0);
		if ( value < 0.375 ) ypos = (height-knobheight);
		else if ( value < 0.999) ypos = (((value-0.375f)*1.6f)-1.0f)*-1.0f*(height-knobheight);
		dc.BitBlt(x+xoffset,y+ypos,knobwidth,knobheight,&knobDC,0,0,SRCCOPY);
	}
	bool  CNativeGui::GraphSlider::LButtonDown( UINT nFlags,int x,int y)
	{
		if ( x< CNativeGui::uiSetting->sliderwidth
			&& y < CNativeGui::uiSetting->sliderheight) return true;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// SwitchButton class
	void CNativeGui::SwitchButton::Draw(CDC& dc, CDC& switchDC, int x, int y)
	{
		int height = CNativeGui::uiSetting->switchheight;
		int width = CNativeGui::uiSetting->switchwidth;
		dc.BitBlt(x,y,width,height,&switchDC,0,0,SRCCOPY);
		dc.Draw3dRect(x-1,y-1,width+1,height+1,CNativeGui::uiSetting->titleColor,CNativeGui::uiSetting->titleColor);
	}

	void CNativeGui::CheckedButton::Draw(CDC& dc, int x, int y,const char* text,bool checked)
	{
		int height = CNativeGui::uiSetting->switchheight;
		int width = CNativeGui::uiSetting->switchwidth;
		if ( checked )
		{
			dc.SetBkColor(CNativeGui::uiSetting->hBottomColor);
			dc.SetTextColor(CNativeGui::uiSetting->fonthBottomColor);
		}
		else{
			dc.SetBkColor(CNativeGui::uiSetting->topColor);
			dc.SetTextColor(CNativeGui::uiSetting->fontTopColor);
		}
		CFont *oldfont =dc.SelectObject(&CNativeGui::uiSetting->font_bold);
		dc.ExtTextOut(x+4, y+1, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width-1, y+height-1), CString(text), 0);
		dc.SelectObject(oldfont);
		dc.Draw3dRect(x-1,y-1,width+1,height+1,CNativeGui::uiSetting->titleColor,CNativeGui::uiSetting->titleColor);
	}


	//////////////////////////////////////////////////////////////////////////
	// Vumeter class
	void CNativeGui::VuMeter::Draw(CDC& dc, CDC& vuOn,CDC& vuOff, int x, int y, float value)
	{
		int height = CNativeGui::uiSetting->switchheight;
		int width = CNativeGui::uiSetting->switchwidth;
		int ypos = (1-value)*height;
		dc.BitBlt(x+7,y+35,width,ypos,&vuOff,0,0,SRCCOPY);
		dc.BitBlt(x+7,y+35+ypos,width,height,&vuOn,0,ypos,SRCCOPY);
	}

	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
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


		CNativeGui::CNativeGui(Machine* effect, CChildView* view)
		:ncol(0)
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
			int const K_XSIZE2=uiSetting->dialwidth+8;

			CRect rect;
			GetClientRect(&rect);

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

				if(type == 2) // STATUS
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
						InfoLabel::DrawHLight(dc,K_XSIZE2+x_knob,y_knob,colwidth,parName,buffer);
					}
					else
					{
						InfoLabel::Draw(dc,K_XSIZE2+x_knob,y_knob,colwidth,parName,buffer);
					}
				}
				else if(type == 1) // HEADER
				{
					InfoLabel::DrawHeader(dc,K_XSIZE2+x_knob,y_knob,colwidth,parName);
				}
				else
				{
					InfoLabel::Draw(dc,K_XSIZE2+x_knob,y_knob,colwidth,"","");
				}
				y_knob += uiSetting->dialheight;

				++knob_c;

				if (knob_c >= parspercol)
				{
					knob_c = 0;
					x_knob += colwidth;
					y_knob = 0;
				}
			}

			int exess= parspercol*ncol;
			if ( exess > numParameters )
			{
				for (int c=numParameters; c<exess; c++)
				{
					InfoLabel::Draw(dc,K_XSIZE2+x_knob,y_knob,colwidth,"","");
					y_knob += uiSetting->dialheight;
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
			if ((y/uiSetting->dialheight) >= parspercol ) return -1; //this if for VST's that use the native gui.
			return (y/uiSetting->dialheight) + ((x/colwidth)*colwidth);
		}

		bool CNativeGui::GetViewSize(CRect& rect)
		{
			rect.left= rect.top = 0;
			rect.right = ncol * colwidth;
			rect.bottom = parspercol * uiSetting->dialheight;
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
