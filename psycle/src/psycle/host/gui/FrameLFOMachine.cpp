//implementation of CFrameLFOMachine.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "FrameLFOMachine.hpp"
#include "psycle.hpp"
#include "ChildView.hpp"
#include <psycle/host/configuration.hpp>
#include <psycle/host/engine/song.hpp>
#include <psycle/host/engine/machine.hpp>

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

	CFrameLFOMachine::Knob::Knob(int x, int y, int minVal, int maxVal, char* label)
		: CFrameLFOMachine::LFOControl(x, y, minVal, maxVal)
		{
			d_showLabel=top;
			d_showValue=bottom;
			d_valBkColor = Global::pConfig->machineGUIBottomColor;
			d_lblBkColor = Global::pConfig->machineGUIBottomColor;
			d_valTextColor = Global::pConfig->machineGUIFontBottomColor;
			d_lblTextColor = Global::pConfig->machineGUIFontBottomColor;
			strcpy(d_lblString, label);
		}

	void CFrameLFOMachine::Knob::Paint(CDC* dc,int value, char* valString)
	{
		int range=d_maxValue-d_minValue;

		if(value>d_maxValue) value=d_maxValue;
		else if(value<d_minValue) value=d_minValue;
		dc->BitBlt(	d_x, d_y, 
					s_width, s_height, &s_knobDC, 
					(s_numFrames-1) * (value-d_minValue)/(range==0?1:range) * s_width, 0, 
					SRCCOPY);

		int textX=0, textY=0;
		CSize valTextSize = dc->GetTextExtent(valString);
		CSize lblTextSize = dc->GetTextExtent(d_lblString);

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
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);
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
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), d_lblString, 0);
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
		return x > d_x && x < d_x+s_width	&&
			   y > d_y && y < d_y+s_height;
	}

	/////////////////////////////////////////////////////////////////////////
	// CFrameLFOMachine::ComboBox class
	CFrameLFOMachine::ComboBox::ComboBox(int x, int y, int minVal, int maxVal, int length, char* label)
		:CFrameLFOMachine::LFOControl(x, y, minVal, maxVal)
		,d_length(length)
		{
			d_showLabel=top;
			d_showValue=bottom;
			d_lblBkColor = Global::pConfig->machineGUIBottomColor;
			d_valTextColor = Global::pConfig->machineGUIFontBottomColor;
			d_lblTextColor = Global::pConfig->machineGUIFontBottomColor;
			strcpy(d_lblString, label);
		}

	void CFrameLFOMachine::ComboBox::Paint(CDC* dc,int value, char* valString)
	{
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


		CSize valTextSize = dc->GetTextExtent(valString);
		CSize lblTextSize = dc->GetTextExtent(d_lblString);

		int textX, textY;
		if(d_showValue!=off)
		{
			
			textX=d_x+d_length/2+s_width  - valTextSize.cx/2;
			textY=d_y+s_height/2		  - valTextSize.cy/2;

			dc->SetBkColor(dc->GetPixel(d_x+s_width+1, d_y+s_height/2));
			dc->SetTextColor(d_valTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);

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
			}
			dc->SetBkColor(d_lblBkColor);
			dc->SetTextColor(d_lblTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), d_lblString, 0);
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
		int newval;
		float deltay= d_twkSrc_y-y;
		if(deltay==0.0) return false;

		float pixel = (d_maxValue-d_minValue) / 192.0f;

		if(nFlags&MK_SHIFT) pixel=.5;
		if(nFlags&MK_CONTROL){deltay*=.2f;}

		newval = deltay*pixel + d_tweakBase;

		if(newval>d_maxValue) newval=d_maxValue;
		else if(newval<d_minValue) newval=d_minValue;
		value = newval;
		return true; 
	}

	bool CFrameLFOMachine::ComboBox::PointInParam(int x, int y)
	{
		return x > d_x && x < d_x+s_width*2+d_length	&&
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

		d_pParams[prm_waveform] = new ComboBox(WIN_CX/5-35-ComboBox::s_width,	30, 0, 4, 50, "Waveform");
		d_pParams[prm_waveform]->d_bDblClkReset=false;
		d_pParams[prm_pwidth]   = new Knob(2 * WIN_CX/5-Knob::s_width/2,	20, 0, 200, "Pulse Width");
		d_pParams[prm_pwidth]->d_defValue=100;
		d_pParams[prm_speed]	= new Knob(3 * WIN_CX/5-Knob::s_width/2,	20, 0, LFO::MAX_SPEED, "LFO Speed");
		d_pParams[prm_speed]->d_defValue=LFO::MAX_SPEED/6;
		d_pParams[prm_lfopos]   = new Knob(4 * WIN_CX/5-Knob::s_width/2,	20, 0, LFO::LFO_SIZE, "Position");

		char temp[64];
		for(int i(0);i<4;++i)
		{
			sprintf(temp, "LFO Dest. Machine %i", i);
			d_pParams[prm_macout0+i] = new ComboBox(WIN_CX/2-64-ComboBox::s_width, 80+95*i,  -1, MAX_BUSES*2-1, 128, temp);
			d_pParams[prm_prmout0+i] = new Knob(    WIN_CX/4-Knob::s_width/2+5, 115+95*i, -1, 128, "Param");
			d_pParams[prm_level0+i]  = new Knob(2 * WIN_CX/4-Knob::s_width/2+25, 115+95*i,  0, LFO::MAX_DEPTH*2, "Depth");
			d_pParams[prm_phase0+i]  = new Knob(3 * WIN_CX/4-Knob::s_width/2+25, 115+95*i,  0, LFO::MAX_PHASE, "Phase");
			d_pParams[prm_macout0+i]->d_bDblClkReset=false;
			d_pParams[prm_prmout0+i]->d_defValue=-1;
			d_pParams[prm_level0+i]->d_defValue=LFO::MAX_DEPTH;
			d_pParams[prm_phase0+i]->d_defValue=LFO::MAX_PHASE/2;
		}

		m_combobox.LoadBitmap(IDB_COMBOBOX);
	}


	CFrameLFOMachine::~CFrameLFOMachine()
	{
		if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
		for(int i(0);i<num_params;++i) delete d_pParams[i];
	}

	int CFrameLFOMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
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
		UpdateNames();


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
			UpdateNames();
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

		bufferDC.FillSolidRect(0,0,rect.right,rect.bottom,Global::pConfig->machineGUIBottomColor);
		char temp[128];
		for(int i(0);i<num_params;++i)
		{
			d_pLFO->GetParamValue(i, temp);
			d_pParams[i]->Paint(&bufferDC, d_pLFO->GetParamValue(i), temp);
		}
		dc.BitBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,SRCCOPY);

		Knob::s_knobDC.SelectObject(oldbmp);
		Knob::s_knobDC.DeleteDC();
		ComboBox::s_ComboBoxDC.SelectObject(oldbmp2);
		ComboBox::s_ComboBoxDC.DeleteDC();

		bufferDC.SelectObject(oldfont);
		bufferDC.SelectObject(bufferbmp);
		bufferDC.DeleteDC();

	}

	void CFrameLFOMachine::UpdateNames()
	{
		d_paramNames.clear();
		d_machNames.clear();

		int lastMac(0);
		int numParams(0);

		for(int i(0);i<MAX_MACHINES-1; ++i)
		{
			if(Global::_pSong->_pMachine[i])
			{
				d_machNames.push_back(Global::_pSong->_pMachine[i]->GetEditName());
				std::vector<std::string> tempnames;
				char tempname[128];
				numParams = Global::_pSong->_pMachine[i]->GetNumParams();
				for(int j(0);j<numParams; ++j)
				{
					Global::_pSong->_pMachine[i]->GetParamName(j, tempname);
					tempnames.push_back(tempname);
				}
				d_paramNames[i]=tempnames;
				lastMac=i;
			}
		}

		int minVal, maxVal;
		for(int i(0);i<LFO::NUM_CHANS;++i)
		{
			d_pLFO->GetParamRange(prm_prmout0+i, minVal, maxVal);
			d_pParams[prm_prmout0+i]->d_maxValue=maxVal;
			d_pParams[prm_macout0+i]->d_maxValue=lastMac;
		}
	}

	int CFrameLFOMachine::ConvertXYtoParam(int x, int y)
	{
		int i(0);
		while(i<num_params && !(d_pParams[i]->PointInParam(x, y)))
			++i;
		if(i<num_params)
			return i;
		else
			return -1;
	}

	void CFrameLFOMachine::OnLButtonDown(UINT nFlags, CPoint point) 
	{
		tweakpar = ConvertXYtoParam(point.x, point.y);
		if(tweakpar>-1 && tweakpar<num_params)
		{
			int value = d_pLFO->GetParamValue(tweakpar);
			if(d_pParams[tweakpar]->LButtonDown(nFlags, point.x, point.y, value))
				d_pLFO->SetParameter(tweakpar, value);
			istweak = true;
			SetCapture();
		}
		Invalidate(false);
	
		CFrameWnd::OnLButtonDown(nFlags,point);

	}

	void CFrameLFOMachine::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		tweakpar = ConvertXYtoParam(point.x, point.y);
		if(tweakpar>-1 && tweakpar<num_params)
		{
			if(d_pParams[tweakpar]->d_bDblClkReset)
				d_pLFO->SetParameter(tweakpar, d_pParams[tweakpar]->d_defValue);
			else
			{
				int value = d_pLFO->GetParamValue(tweakpar);
				if(d_pParams[tweakpar]->LButtonDown(nFlags, point.x, point.y, value))
					d_pLFO->SetParameter(tweakpar, value);
				istweak = true;
				SetCapture();
			}
		}
		Invalidate(false);

		CFrameWnd::OnLButtonDblClk(nFlags,point);
	}
	void CFrameLFOMachine::OnMouseMove(UINT nFlags, CPoint point) 
	{
		if(istweak)
		{
			if(tweakpar>-1 && tweakpar<num_params)
			{
				int value=d_pLFO->GetParamValue(tweakpar);
				if(d_pParams[tweakpar]->MouseMove(nFlags, point.x, point.y, value))
					d_pLFO->SetParameter(tweakpar, value);
			}
		}

		Invalidate(false);

		CFrameWnd::OnMouseMove(nFlags,point);
	}
	void CFrameLFOMachine::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		if(tweakpar>-1 && tweakpar<num_params)
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
