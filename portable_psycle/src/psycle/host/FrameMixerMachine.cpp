#include <project.private.hpp>
#include "Psycle.hpp"
#include "Constants.hpp"
#include "FrameMixerMachine.hpp"
#include "Configuration.hpp"
#include "ChildView.hpp"

NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

	int CFrameMixerMachine::Knob::height(28);
	int CFrameMixerMachine::Knob::width(28);
	int CFrameMixerMachine::Knob::numFrames(64);
	CDC* CFrameMixerMachine::Knob::pKnobDC(0);

	int CFrameMixerMachine::InfoLabel::xoffset(8);
	int CFrameMixerMachine::InfoLabel::width(28);
	int CFrameMixerMachine::InfoLabel::height(28);

	int CFrameMixerMachine::GraphSlider::height(128);
	int CFrameMixerMachine::GraphSlider::width(20);
	int CFrameMixerMachine::GraphSlider::knobheight(22);
	int CFrameMixerMachine::GraphSlider::knobwidth(15);
	int CFrameMixerMachine::GraphSlider::xoffset(2);

	//////////////////////////////////////////////////////////////////////////
	// Knob class
	CFrameMixerMachine::Knob::Knob(){};
	CFrameMixerMachine::Knob::~Knob(){};

	void CFrameMixerMachine::Knob::Draw(CDC* dc,int x_knob,int y_knob,float value)
	{
		int pixel = numFrames*width*value;
		dc->BitBlt(x_knob,y_knob,width,height,pKnobDC,pixel,0,SRCCOPY);
	}

	//////////////////////////////////////////////////////////////////////////
	// InfoLabel class
	void CFrameMixerMachine::InfoLabel::Draw(CDC* dc, int x, int y,char *parName, char *parValue)
	{
		const int half = height/2;
		dc->SetBkColor(Global::pConfig->machineGUITopColor);
		dc->SetTextColor(Global::pConfig->machineGUIFontTopColor);
		dc->ExtTextOut(x+xoffset, y, ETO_OPAQUE, CRect(x, y, x+width, y+half), CString(parName), 0);

		dc->SetBkColor(Global::pConfig->machineGUIBottomColor);
		dc->SetTextColor(Global::pConfig->machineGUIFontBottomColor);
		dc->ExtTextOut(x+xoffset, y+half, ETO_OPAQUE, CRect(x, y+half, x+width, y+height), CString(parValue), 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// HLightInfoLabel class
	void CFrameMixerMachine::HLightInfoLabel::Draw(CDC* dc, int x, int y,char *parName, char *parValue)
	{
		const int half = height/2;
		dc->SetBkColor(Global::pConfig->machineGUIHTopColor);
		dc->SetTextColor(Global::pConfig->machineGUIHFontTopColor);
		dc->ExtTextOut(x+xoffset, y, ETO_OPAQUE, CRect(x, y, x+width, y+half), CString(parName), 0);

		dc->SetBkColor(Global::pConfig->machineGUIHBottomColor);
		dc->SetTextColor(Global::pConfig->machineGUIHFontBottomColor);
		dc->ExtTextOut(x+xoffset, y+half, ETO_OPAQUE, CRect(x, y+half, x+width, y+height), CString(parValue), 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// HeaderInfoLabel class
	void CFrameMixerMachine::HeaderInfoLabel::Draw(CDC* dc, int x, int y,char *parName, char *parValue)
	{
		const int half = height/2;
		const int quarter = height/4;
		dc->FillSolidRect(x, y, x+width, y + quarter,Global::pConfig->machineGUITopColor);

		dc->SetBkColor(Global::pConfig->machineGUITitleColor);
		dc->SetTextColor(Global::pConfig->machineGUITitleFontColor);

		dc->SelectObject(&font_bold);
		dc->ExtTextOut(x + xoffset, y + quarter, ETO_OPAQUE, CRect(x, y + quarter, x + width, y + half + quarter), CString(parName), 0);
		dc->SelectObject(&font);

		dc->FillSolidRect(x, y+half+quarter, x+width, y + height,Global::pConfig->machineGUIBottomColor);
	}

	//////////////////////////////////////////////////////////////////////////
	// GraphSlider class
	void CFrameMixerMachine::GraphSlider::Draw(CDC *dc,int x, int y, float value)
	{
		int ypos = (1-value)*(height-knobheight);
		dc->BitBlt(x,y,width,height,pBackDC,0,0,SRCCOPY);
		dc->BitBlt(x+xoffset,y+ypos,knobwidth,knobheight,pKnobDC,0,0,SRCCOPY);
	}

	//////////////////////////////////////////////////////////////////////////
	// CFrameMixerMachine class
	IMPLEMENT_DYNCREATE(CFrameMixerMachine, CFrameWnd)

		BEGIN_MESSAGE_MAP(CFrameMachine, CFrameWnd)
			//{{AFX_MSG_MAP(CFrameMachine)
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_WM_RBUTTONUP()
			ON_WM_TIMER()
			ON_WM_CREATE()
			ON_WM_DESTROY()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_WM_SETFOCUS()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

	CFrameMixerMachine::CFrameMixerMachine() :
		_pMixer(0)
		,numSends(0)
		,numChannels(0)
	{
		m_sliderback.LoadBitmap(IDB_SLIDERBACK);
		m_sliderknob.LoadBitmap(IDB_SLIDERKNOB);
	}

	void CFrameMixerMachine::Generate()
	{
	}
	void CFrameMixerMachine::SelectMachine(Machine* pMachine)
	{
	}

	int CFrameMixerMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
		if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		{
			return -1;
		}
		SetTimer(2104+MachineIndex,100,0);
		return 0;
	}

	void CFrameMixerMachine::OnDestroy() 
	{
		if ( _pActive != NULL ) *_pActive = false;
		KillTimer(2204+MachineIndex);
		CFrameWnd::OnDestroy();
	}

	void CFrameMixerMachine::OnTimer(UINT nIDEvent) 
	{
		if ( nIDEvent == 2204+MachineIndex )
		{
			Invalidate(false);
		}
		CFrameWnd::OnTimer(nIDEvent);
	}
	void CFrameMixerMachine::OnPaint() 
	{
		CPaintDC dc(this); // device context for painting

		CDC memDC;
		CDC memDC2;
		CBitmap* oldbmp,oldbmp2;
		
		memDC.CreateCompatibleDC(&dc);
		oldbmp=memDC.SelectObject(&wndView->machinedial);

		int sends(0),cols(0);
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->_sendValid[i]) { cols++; sends++; }
		}
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->_inputCon[i]) cols++;
		}
		if (cols != numCols || sends != numSends)
		{
			MoveWindow
				(
				0,
				0,
				 * cols,
				winh + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYEDGE),
				false
				);
			numCols =cols;
			numSends=sends;
		}
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->_inputCon[i])
			{
			}
		}

		memDC2.SelectObject(oldbmp2);
		memDC2.DeleteDC();
		memDC.SelectObject(oldbmp);
		memDC.DeleteDC();

	}
	void CFrameMixerMachine::OnLButtonDown(UINT nFlags, CPoint point) 
	{
	}
	void CFrameMixerMachine::OnMouseMove(UINT nFlags, CPoint point) 
	{
	}
	void CFrameMixerMachine::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		istweak = false;
		Invalidate(false);	
		ReleaseCapture();
		CFrameWnd::OnLButtonUp(nFlags, point);
	}

	void CFrameMixerMachine::OnRButtonUp(UINT nFlags, CPoint point) 
	{
	}
	void CFrameMixerMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
    }
	void CFrameMixerMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
	}
	void CFrameMixerMachine::OnSetFocus(CWnd* pOldWnd) 
	{
		CFrameWnd::OnSetFocus(pOldWnd);
		Invalidate(false);
	}

	NAMESPACE__END
NAMESPACE__END
