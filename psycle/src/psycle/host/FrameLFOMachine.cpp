//implementation of CFrameLFOMachine.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "FrameLFOMachine.hpp"
#include "psycle.hpp"
#include "ChildView.hpp"
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/song.hpp>
#include <psycle/engine/machine.hpp>
#include <psycle/engine/internal_machines.hpp>

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

	IMPLEMENT_DYNCREATE(CFrameLFOMachine, CFrameWnd)

	int CFrameLFOMachine::Knob::s_height(28);
	int CFrameLFOMachine::Knob::s_width(28);
	int CFrameLFOMachine::Knob::s_numFrames(64);
	CDC CFrameLFOMachine::Knob::s_knobDC;

	int CFrameLFOMachine::ComboBox::s_height(18);
	int CFrameLFOMachine::ComboBox::s_width(18);
	CDC CFrameLFOMachine::ComboBox::s_ComboBoxDC;



	//////////////////////////////////////////////////////////////////////////
	// Knob class

	CFrameLFOMachine::Knob::Knob(int x, int y, int minVal, int maxVal, const std::string& label)
		: CFrameLFOMachine::LFOControl(x, y, minVal, maxVal)
		{
			d_showLabel=top;
			d_showValue=bottom;
			d_valBkColor = UIGlobal::configuration().machineGUIBottomColor;
			d_lblBkColor = UIGlobal::configuration().machineGUIBottomColor;
			d_valTextColor = UIGlobal::configuration().machineGUIFontBottomColor;
			d_lblTextColor = UIGlobal::configuration().machineGUIFontBottomColor;
			d_lblString=label;
			d_bTweakable = true;
		}

		void CFrameLFOMachine::Knob::Paint(CDC* dc,int value, const std::string& valString)
	{
		if(!d_bVisible) return;
		int range=d_maxValue-d_minValue;

		if(value>d_maxValue) value=d_maxValue;
		else if(value<d_minValue) value=d_minValue;
		dc->BitBlt(	d_x, d_y, 
					s_width, s_height, &s_knobDC, 
					(s_numFrames-1) * (value-d_minValue)/(range==0?1:range) * s_width, 0, 
					SRCCOPY);

		int textX=0, textY=0;
		CSize valTextSize = dc->GetTextExtent(valString.c_str());
		CSize lblTextSize = dc->GetTextExtent(d_lblString.c_str());

		if(d_showValue!=off)
		{
			switch(d_showValue)
			{
			case top:
				textX = d_x + s_width/2 - valTextSize.cx/2;
				textY = d_y - 10 - valTextSize.cy/2;
				break;
			case bottom:
				textX = d_x + s_width/2 - valTextSize.cx/2;
				textY = d_y + s_height + 10 - valTextSize.cy/2;
				if(d_showLabel==bottom) textY+=lblTextSize.cy;
				break;
			case left:
				textX = d_x - 10 - valTextSize.cx;
				textY = d_y + s_height/2 - valTextSize.cy/2;
				if(d_showLabel==left) textY+=lblTextSize.cy/2;
				break;
			case right:
				textX = d_x + s_width + 10;
				textY = d_y + s_height/2 - valTextSize.cy/2;
				if(d_showLabel==right) textY+=lblTextSize.cy/2;
				break;
			}
		
			dc->SetBkColor(d_valBkColor);
			dc->SetTextColor(d_valTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString.c_str(), 0);
		}
		if(d_showLabel!=off)
		{
			switch(d_showLabel)
			{
			case top:
				textX = d_x + s_width/2 - lblTextSize.cx/2;
				textY = d_y - 10 - lblTextSize.cy/2;
				if(d_showValue==top) textY-=valTextSize.cy;					
				break;
			case bottom:
				textX = d_x + s_width/2 - lblTextSize.cx/2;
				textY = d_y + s_height + 10 - lblTextSize.cy/2;
				break;
			case left:
				textX = d_x - 10 - lblTextSize.cx;
				textY = d_y + s_height/2 - lblTextSize.cy/2;
				if(d_showValue==left) textY-=lblTextSize.cy/2;
				break;
			case right:
				textX = d_x + s_width + 10;
				textY = d_y + s_height/2 - lblTextSize.cy/2;
				if(d_showValue==right) textY-=lblTextSize.cy/2;
				break;
			}
			dc->SetBkColor(d_lblBkColor);
			dc->SetTextColor(d_lblTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), d_lblString.c_str(), 0);
		}
	}
	bool CFrameLFOMachine::Knob::LButtonDown(UINT nFlags,int x,int y, int &value)
	{
		d_twkSrc_x = x;
		d_twkSrc_y = y;
		d_tweakBase=value;

		return false;
	}
	bool CFrameLFOMachine::Knob::LButtonUp(UINT nFlags, int x, int y, int &value)
	{
		return false;
	}

	bool CFrameLFOMachine::Knob::MouseMove(UINT nFlags, int x, int y, int &value)
	{
		if(!d_bTweakable) return false;

		int newval;
		float deltay= d_twkSrc_y-y;
		if (deltay==0.0) return false;

		float pixel = (d_maxValue-d_minValue) / 192.0f;

		if(nFlags&MK_SHIFT) pixel=.5;
		if(nFlags&MK_CONTROL){deltay*=.2f;}

		newval = deltay*pixel + d_tweakBase;

		if(newval>d_maxValue) newval=d_maxValue;
		else if(newval<d_minValue) newval=d_minValue;
		value= newval;
		return true;
	}

	bool CFrameLFOMachine::Knob::PointInParam(int x, int y)
	{
		return	d_bVisible	&&
				x > d_x && x < d_x+s_width	&&
				y > d_y && y < d_y+s_height;
	}

	/////////////////////////////////////////////////////////////////////////
	// CFrameLFOMachine::ComboBox class
	CFrameLFOMachine::ComboBox::ComboBox(int x, int y, int minVal, int maxVal, int length, const std::string& label)
		:CFrameLFOMachine::LFOControl(x, y, minVal, maxVal)
		,d_length(length)
		{
			d_showLabel=top;
			d_showValue=bottom;
			d_lblBkColor = UIGlobal::configuration().machineGUIBottomColor;
			d_valTextColor = UIGlobal::configuration().machineGUIFontBottomColor;
			d_lblTextColor = UIGlobal::configuration().machineGUIFontBottomColor;
			d_lblString = label;
			d_bTweakable = true;
		}

		void CFrameLFOMachine::ComboBox::Paint(CDC* dc,int value, const std::string& valString)
	{
		if(!d_bVisible) return;
		dc->BitBlt(	d_x,	d_y, 
					s_width, s_height, &s_ComboBoxDC,    
					(value==d_minValue ? s_width*2 : 0), 0, SRCCOPY);
		dc->BitBlt(	d_x+d_length+s_width, d_y, 
					s_width, s_height, &s_ComboBoxDC, 
					s_width+(value==d_maxValue ? s_width*2 : 0), 0, SRCCOPY);

		dc->BitBlt(d_x+s_width, d_y,			1, s_height, &s_ComboBoxDC, s_width*4,   0, SRCCOPY);
		dc->BitBlt(d_x+s_width+d_length-1, d_y, 1, s_height, &s_ComboBoxDC, s_width*4+2, 0, SRCCOPY);

		for(int i(s_width+1);i<d_length+s_width-1;++i)
			dc->BitBlt(d_x+i, d_y, 1, s_height, &s_ComboBoxDC, s_width*4+1, 0, SRCCOPY);


		CSize valTextSize = dc->GetTextExtent(valString.c_str());
		CSize lblTextSize = dc->GetTextExtent(d_lblString.c_str());

		int textX, textY;
		if(d_showValue!=off)
		{
			
			textX=d_x+d_length/2+s_width  - valTextSize.cx/2;
			textY=d_y+s_height/2		  - valTextSize.cy/2;

			dc->SetBkColor(dc->GetPixel(d_x+s_width+1, d_y+s_height/2));
			dc->SetTextColor(d_valTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString.c_str(), 0);

		}

		if(d_showLabel!=off)
		{
			switch(d_showLabel)
			{
			case top:
				textX = d_x + s_width + d_length/2 - lblTextSize.cx/2;
				textY = d_y - 10 - lblTextSize.cy/2;
				break;
			case bottom:
				textX = d_x + s_width + d_length/2 - lblTextSize.cx/2;
				textY = d_y + s_height + 10 - lblTextSize.cy/2;
				break;
			case left:
				textX = d_x - 10 - lblTextSize.cx;
				textY = d_y + s_height/2 - lblTextSize.cy/2;
				break;
			case right:
				textX = d_x + s_width*2 + d_length + 10;
				textY = d_y + s_height/2 - lblTextSize.cy/2;
				break;
			default:
				throw;
			}
			dc->SetBkColor(d_lblBkColor);
			dc->SetTextColor(d_lblTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), d_lblString.c_str(), 0);
		}
	}
	bool CFrameLFOMachine::ComboBox::LButtonDown(UINT nFlags,int x,int y, int &value)
	{
		d_twkSrc_x = x;
		d_twkSrc_y = y;

		int newval;
		if(x>d_x && x<d_x+s_width)
			newval=value-1;
		else if(x>d_x+d_length+s_width && x<d_x+d_length+s_width*2)
			newval=value+1;
		else
		{
			d_tweakBase=value;
			return false;
		}
		if(newval<d_minValue)
			newval=d_minValue;
		else if(newval>d_maxValue)
			newval=d_maxValue;
		value=newval;
		d_tweakBase=value;
		return true;


	}
	bool CFrameLFOMachine::ComboBox::LButtonUp(UINT nFlags, int x, int y, int &value)
	{
		return false;
	}

	bool CFrameLFOMachine::ComboBox::MouseMove(UINT nFlags, int x, int y, int &value)
	{
		if(!d_bTweakable) return false;
		int newval;
		float deltax= x-d_twkSrc_x;
		if(deltax==0.0) return false;

		float pixel = (d_maxValue-d_minValue) / 192.0f;

		if(nFlags&MK_SHIFT) pixel=.5;
		if(nFlags&MK_CONTROL){deltax*=.2f;}

		newval = deltax*pixel + d_tweakBase;

		if(newval>d_maxValue) newval=d_maxValue;
		else if(newval<d_minValue) newval=d_minValue;
		value = newval;
		return true; 
	}

	bool CFrameLFOMachine::ComboBox::PointInParam(int x, int y)
	{
		return	d_bVisible	&&
				x > d_x && x < d_x+s_width*2+d_length	&&
				y > d_y && y < d_y+s_height;
	}


	//////////////////////////////////////////////////////////////////////////
	// CFrameLFOMachine class

	BEGIN_MESSAGE_MAP(CFrameLFOMachine, CFrameWnd)
		//{{AFX_MSG_MAP(CFrameLFOMachine)
		ON_WM_PAINT()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONUP()
		ON_WM_RBUTTONUP()
		ON_WM_TIMER()
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_SETFOCUS()
		ON_WM_KEYDOWN()
		ON_WM_KEYUP()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	CFrameLFOMachine::CFrameLFOMachine()
	{
	}
	CFrameLFOMachine::CFrameLFOMachine(int dum) :
		d_pLFO(0)
		,bmpDC(0)
	{
		MachineIndex = dum;

		d_pParams.resize(LFO::prms::num_params);

		d_pParams[LFO::prms::wave] = new ComboBox	(65-25-ComboBox::s_width,	40, 0, LFO::lfo_types::num_lfos-1, 50, "Waveform");
		d_pParams[LFO::prms::wave]->d_bDblClkReset=false;
		d_pParams[LFO::prms::speed]	= new Knob		(65-   Knob::s_width/2,		105, 0, LFO::MAX_SPEED, "LFO Speed");
		d_pParams[LFO::prms::speed]->d_defValue=LFO::MAX_SPEED/10;

		for( int i(0); i<LFO::NUM_CHANS; ++i )
		{
			std::ostringstream temp;
			temp<<"LFO Dest. Machine "<<i;
			d_pParams[LFO::prms::mac0+i] = new ComboBox(2 * WIN_CX/3-64-ComboBox::s_width+10,	27+115*(i%2), -1, MAX_BUSES*2-1, 128, temp.str());
			d_pParams[LFO::prms::prm0+i] = new Knob(	2 * WIN_CX/5-Knob::s_width/2+15,	70+115*(i%2), -1, 128, "Param");
			d_pParams[LFO::prms::level0+i]  = new Knob(	3 * WIN_CX/5-Knob::s_width/2+35,	70+115*(i%2),  0, LFO::MAX_DEPTH*2, "Depth");
			d_pParams[LFO::prms::phase0+i]  = new Knob(	4 * WIN_CX/5-Knob::s_width/2+35,	70+115*(i%2),  0, LFO::MAX_PHASE, "Phase");
			d_pParams[LFO::prms::mac0+i]->d_bDblClkReset=false;
			d_pParams[LFO::prms::mac0+i]->d_bTweakable = false;
			d_pParams[LFO::prms::prm0+i]->d_defValue=-1;
			d_pParams[LFO::prms::level0+i]->d_defValue=LFO::MAX_DEPTH;
			d_pParams[LFO::prms::phase0+i]->d_defValue=LFO::MAX_PHASE/2;
		}

		d_pView = new ComboBox(65-25-ComboBox::s_width,		185, 0, LFO::NUM_CHANS/2-1, 50, "Switch View");
		d_pView->d_bDblClkReset=false;

		m_combobox.LoadBitmap(IDB_COMBOBOX);
	}


	CFrameLFOMachine::~CFrameLFOMachine()
	{
		if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
		for(int i(0);i<LFO::prms::num_params;++i) delete d_pParams[i];
		delete d_pView;
	}

	int CFrameLFOMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
		d_view = 0;
		SwitchView();
		return CFrameMachine::OnCreate(lpCreateStruct);
	}

	void CFrameLFOMachine::OnDestroy() 
	{
		CFrameMachine::OnDestroy();
	}

	void CFrameLFOMachine::SelectMachine(Machine* pMachine)
	{
		d_pLFO=(LFO*)(_pMachine = pMachine);
		numParameters = d_pLFO->GetNumParams();
		UpdateParamRanges();

		CRect rect;
		GetWindowRect(&rect);
		MoveWindow
			(
			rect.left,
			rect.top,
			WIN_CX + 2*GetSystemMetrics(SM_CXDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
			WIN_CY + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + 2*GetSystemMetrics(SM_CYDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
			false
			);
		ShowWindow(SW_SHOW);
	}

	void CFrameLFOMachine::OnTimer(UINT nIDEvent) 
	{
		if ( nIDEvent == 2104+MachineIndex )
		{
			UpdateParamRanges();
			Invalidate(false);
		}
		CFrameWnd::OnTimer(nIDEvent);

	}

	void CFrameLFOMachine::OnPaint() 
	{
		if (!d_pLFO) return;
		CPaintDC dc(this); // device context for painting

		CRect rect;
		GetClientRect(&rect);

		if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
		bmpDC = new CBitmap;
		bmpDC->CreateCompatibleBitmap(&dc,rect.right-rect.left,rect.bottom-rect.top);

		CDC bufferDC;
		bufferDC.CreateCompatibleDC(&dc);
		CBitmap *bufferbmp = bufferDC.SelectObject(bmpDC);
		CFont *oldfont=bufferDC.SelectObject(&font);

		Knob::s_knobDC.CreateCompatibleDC(&bufferDC);
		CBitmap *oldbmp=Knob::s_knobDC.SelectObject(&wndView->machinedial);
		ComboBox::s_ComboBoxDC.CreateCompatibleDC(&bufferDC);
		CBitmap *oldbmp2=ComboBox::s_ComboBoxDC.SelectObject(&m_combobox);

		//draw bg color
		bufferDC.FillSolidRect(0,0,rect.right,rect.bottom,UIGlobal::configuration().machineGUIBottomColor);
		//draw lfo params
		char temp[128];
		for(int i(0);i<LFO::prms::num_params;++i)
		{
			d_pLFO->GetParamValue(i, temp);
			d_pParams[i]->Paint(&bufferDC, d_pLFO->GetParamValue(i), temp);
		}
		//draw view switch control
		sprintf(temp, "%i and %i", d_view*2, d_view*2+1);
		d_pView->Paint(&bufferDC, d_view, temp);
		//blt buffer to screen
		dc.BitBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,SRCCOPY);

		//cleanup
		Knob::s_knobDC.SelectObject(oldbmp);
		Knob::s_knobDC.DeleteDC();
		ComboBox::s_ComboBoxDC.SelectObject(oldbmp2);
		ComboBox::s_ComboBoxDC.DeleteDC();

		bufferDC.SelectObject(oldfont);
		bufferDC.SelectObject(bufferbmp);
		bufferDC.DeleteDC();

	}

	void CFrameLFOMachine::UpdateParamRanges()
	{
		int lastMac;
		int minVal, maxVal;

		for(lastMac=MAX_MACHINES-2; lastMac>-1 && !Global::song()._pMachine[lastMac]; --lastMac)
			;

		for(int i(0);i<LFO::NUM_CHANS;++i)
		{
			d_pLFO->GetParamRange(LFO::prms::prm0+i, minVal, maxVal);
			d_pParams[LFO::prms::prm0+i]->d_maxValue=maxVal;
			d_pParams[LFO::prms::mac0+i]->d_maxValue=lastMac;
		}
	}

	int CFrameLFOMachine::ConvertXYtoParam(int x, int y)
	{
		int i(0);
		while(i<LFO::prms::num_params && !(d_pParams[i]->PointInParam(x, y)))
			++i;
		if(i<LFO::prms::num_params)
			return i;
		else
			return -1;
	}

	void CFrameLFOMachine::OnLButtonDown(UINT nFlags, CPoint point) 
	{
		tweakpar = ConvertXYtoParam(point.x, point.y);
		if(tweakpar>-1 && tweakpar<LFO::prms::num_params)
		{
			int value = d_pLFO->GetParamValue(tweakpar);
			if(d_pParams[tweakpar]->LButtonDown(nFlags, point.x, point.y, value))
				d_pLFO->SetParameter(tweakpar, value);
			if(d_pParams[tweakpar]->d_bTweakable)
			{
				istweak = true;
				SetCapture();
				d_shiftAndCtrlState = (nFlags & (MK_SHIFT | MK_CONTROL));
			}
		}
		else if(d_pView->PointInParam(point.x, point.y))
		{
			if(d_pView->LButtonDown(nFlags, point.x, point.y, d_view))
				SwitchView();
		}

		
		Invalidate(false);
	
		CFrameWnd::OnLButtonDown(nFlags,point);

	}

	void CFrameLFOMachine::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		tweakpar = ConvertXYtoParam(point.x, point.y);
		if(tweakpar>-1 && tweakpar<LFO::prms::num_params)
		{
			if(d_pParams[tweakpar]->d_bDblClkReset)
				d_pLFO->SetParameter(tweakpar, d_pParams[tweakpar]->d_defValue);
			else
			{
				int value = d_pLFO->GetParamValue(tweakpar);
				if(d_pParams[tweakpar]->LButtonDown(nFlags, point.x, point.y, value))
					d_pLFO->SetParameter(tweakpar, value);
				if(d_pParams[tweakpar]->d_bTweakable)
				{
					istweak = true;
					SetCapture();
					d_shiftAndCtrlState = (nFlags & (MK_SHIFT | MK_CONTROL));
				}
			}
		}
		else if(d_pView->PointInParam(point.x, point.y))
		{
			if(d_pView->LButtonDown(nFlags, point.x, point.y, d_view))
				SwitchView();
		}
		Invalidate(false);

		CFrameWnd::OnLButtonDblClk(nFlags,point);
	}
	void CFrameLFOMachine::OnMouseMove(UINT nFlags, CPoint point) 
	{
		if(istweak)
		{
			if(tweakpar>-1 && tweakpar<LFO::prms::num_params)
			{
				int value=d_pLFO->GetParamValue(tweakpar);

				if( (nFlags & (MK_CONTROL | MK_SHIFT)) != d_shiftAndCtrlState )	//state of either shift or control has changed
				{
					d_pParams[tweakpar]->ResetTweakSrc(point.x, point.y, value);
					d_shiftAndCtrlState = (nFlags & (MK_CONTROL | MK_SHIFT));
				}

				if(d_pParams[tweakpar]->MouseMove(nFlags, point.x, point.y, value))
					d_pLFO->SetParameter(tweakpar, value);
			}
		}

		Invalidate(false);

		CFrameWnd::OnMouseMove(nFlags,point);
	}
	void CFrameLFOMachine::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		if(tweakpar>-1 && tweakpar<LFO::prms::num_params)
		{
			int value=d_pLFO->GetParamValue(tweakpar);
			if(d_pParams[tweakpar]->LButtonUp(nFlags, point.x, point.y, value))
				d_pLFO->SetParameter(tweakpar, value);
		}
		istweak = false;
		Invalidate(false);	
		ReleaseCapture();
		CFrameWnd::OnLButtonUp(nFlags, point);
	}

	void CFrameLFOMachine::OnRButtonUp(UINT nFlags, CPoint point) 
	{
		CFrameMachine::OnRButtonUp(nFlags, point);
	}

	void CFrameLFOMachine::SwitchView()
	{
		for(int i(0);i<LFO::NUM_CHANS;i++)
		{
			bool isVis = (i == d_view*2 || i == d_view*2+1);
			d_pParams[LFO::prms::mac0+i]->Visible(isVis);
			d_pParams[LFO::prms::prm0+i]->Visible(isVis);
			d_pParams[LFO::prms::phase0+i]->Visible(isVis);
			d_pParams[LFO::prms::level0+i]->Visible(isVis);
		}
		Invalidate(false);
	}

	void CFrameLFOMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameMachine::OnKeyDown(nChar, nRepCnt, nFlags);
    }
	void CFrameLFOMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameMachine::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	void CFrameLFOMachine::OnSetFocus(CWnd* pOldWnd) 
	{
		CFrameMachine::OnSetFocus(pOldWnd);
	}

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
