#include <project.private.hpp>
#include "Psycle.hpp"
#include "Constants.hpp"
#include "ChildView.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "FrameMixerMachine.hpp"

NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

	IMPLEMENT_DYNCREATE(CFrameMixerMachine, CFrameWnd)

	int CFrameMixerMachine::Knob::height(28);
	int CFrameMixerMachine::Knob::width(28);
	int CFrameMixerMachine::Knob::numFrames(64);
	CDC CFrameMixerMachine::Knob::knobDC;

	int CFrameMixerMachine::InfoLabel::xoffset(4);
	int CFrameMixerMachine::InfoLabel::width(28);
	int CFrameMixerMachine::InfoLabel::height(28);
	CFont CFrameMixerMachine::InfoLabel::font;
	CFont CFrameMixerMachine::InfoLabel::font_bold;

	int CFrameMixerMachine::GraphSlider::height(128);
	int CFrameMixerMachine::GraphSlider::width(28);
	int CFrameMixerMachine::GraphSlider::knobheight(22);
	int CFrameMixerMachine::GraphSlider::knobwidth(15);
	int CFrameMixerMachine::GraphSlider::xoffset(6);
	CDC CFrameMixerMachine::GraphSlider::backDC;
	CDC CFrameMixerMachine::GraphSlider::knobDC;

	//////////////////////////////////////////////////////////////////////////
	// Knob class
	void CFrameMixerMachine::Knob::Draw(CDC* dc,int x_knob,int y_knob,float value)
	{
		int pixel = numFrames*width*value;
		dc->BitBlt(x_knob,y_knob,width,height,&knobDC,pixel,0,SRCCOPY);
//		dc->Draw3dRect(x_knob,y_knob,width,1,RGB(20,20,20),RGB(20,20,20));
	}

	//////////////////////////////////////////////////////////////////////////
	// InfoLabel class
	void CFrameMixerMachine::InfoLabel::Draw(CDC* dc, int x, int y,const char *parName, const char *parValue)
	{
		dc->Draw3dRect(x,y-1,width,height+1,RGB(20,20,20),RGB(20,20,20));
		const int half = height/2;
		dc->SetBkColor(Global::pConfig->machineGUITopColor);
		dc->SetTextColor(Global::pConfig->machineGUIFontTopColor);
		dc->ExtTextOut(x+1, y, ETO_OPAQUE, CRect(x, y, x+width-1, y+half), CString(parName), 0);

		dc->SetBkColor(Global::pConfig->machineGUIBottomColor);
		dc->SetTextColor(Global::pConfig->machineGUIFontBottomColor);
		dc->ExtTextOut(x+1, y+half, ETO_OPAQUE, CRect(x, y+half, x+width-1, y+height-1), CString(parValue), 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// HLightInfoLabel class
	void CFrameMixerMachine::InfoLabel::DrawHLight(CDC* dc, int x, int y,const char *parName, const char *parValue)
	{
		const int half = height/2;
		const int mywidth = width + Knob::width;
		dc->FillSolidRect(x, y, mywidth, half,Global::pConfig->machineGUITitleColor);
		dc->FillSolidRect(x, y+half, mywidth, half,Global::pConfig->machineGUIBottomColor);
		dc->SetBkMode(TRANSPARENT);
//		dc->SetBkColor(Global::pConfig->machineGUITitleColor);
		dc->SetTextColor(Global::pConfig->machineGUITitleFontColor);
		dc->SelectObject(&font_bold);
		dc->ExtTextOut(x+xoffset, y, ETO_CLIPPED, CRect(x+1, y, x+mywidth-1, y+half), CString(parName), 0);
		dc->SelectObject(&font);

//		dc->SetBkColor(Global::pConfig->machineGUIBottomColor);
		dc->SetTextColor(Global::pConfig->machineGUIFontBottomColor);
		dc->ExtTextOut(x+xoffset, y+half, ETO_CLIPPED, CRect(x+1, y+half, x+mywidth-1, y+height), CString(parValue), 0);
		dc->Draw3dRect(x-1,y-1,width+ Knob::width+1,height+1,RGB(20,20,20),RGB(20,20,20));
	}

	//////////////////////////////////////////////////////////////////////////
	// HeaderInfoLabel class
	void CFrameMixerMachine::InfoLabel::DrawHeader(CDC* dc, int x, int y,const char *parName, const char *parValue)
	{
		const int half = height/2;
		const int quarter = height/4;
		const int mywidth = width + Knob::width;
		dc->FillSolidRect(x, y, mywidth, half,Global::pConfig->machineGUITopColor);
		dc->FillSolidRect(x, y+half, mywidth, half,Global::pConfig->machineGUIBottomColor);

		dc->SetBkColor(Global::pConfig->machineGUITitleColor);
		dc->SetTextColor(Global::pConfig->machineGUITitleFontColor);

		dc->SelectObject(&font_bold);
		dc->ExtTextOut(x + xoffset, y + quarter, ETO_OPAQUE | ETO_CLIPPED, CRect(x+1, y + quarter, x+mywidth-1, y+half+quarter), CString(parName), 0);
		dc->SelectObject(&font);
		dc->Draw3dRect(x-1,y-1,width+Knob::width+1,height+1,RGB(20,20,20),RGB(20,20,20));

	}

	//////////////////////////////////////////////////////////////////////////
	// GraphSlider class
	void CFrameMixerMachine::GraphSlider::Draw(CDC *dc,int x, int y, float value)
	{
		int ypos = (1-value)*(height-knobheight);
		dc->BitBlt(x,y,width,height,&backDC,0,0,SRCCOPY);
		dc->BitBlt(x+xoffset,y+ypos,knobwidth,knobheight,&knobDC,0,0,SRCCOPY);
//		dc->FillSolidRect(x+width, y, InfoLabel::width, height-InfoLabel::height,Global::pConfig->machineGUITopColor);
		dc->Draw3dRect(x-1,y-1,width+1,height+1,RGB(20,20,20),RGB(20,20,20));
	}

	//////////////////////////////////////////////////////////////////////////
	// CFrameMixerMachine class

	BEGIN_MESSAGE_MAP(CFrameMixerMachine, CFrameWnd)
		//{{AFX_MSG_MAP(CFrameMixerMachine)
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

	CFrameMixerMachine::CFrameMixerMachine()
	{
	}
	CFrameMixerMachine::CFrameMixerMachine(int dum) :
		_pMixer(0)
		,numSends(0)
		,numChannels(0)
	{
		MachineIndex = dum;
		m_sliderback.LoadBitmap(IDB_SLIDERBACK);
		m_sliderknob.LoadBitmap(IDB_SLIDERKNOB);
		InfoLabel::font.CreatePointFont(80,"Tahoma");
		CString sFace("Tahoma");
		LOGFONT lf = LOGFONT();
		lf.lfWeight = FW_BOLD;
		lf.lfHeight = 80;
		lf.lfQuality = NONANTIALIASED_QUALITY;
		std::strncpy(lf.lfFaceName,(LPCTSTR)sFace,32);
		if(!InfoLabel::font_bold.CreatePointFontIndirect(&lf))
		{
			InfoLabel::font_bold.CreatePointFont(80,"Tahoma Bold");
		}
	}
	CFrameMixerMachine::~CFrameMixerMachine()
	{
	}

	void CFrameMixerMachine::Generate()
	{
	}
	void CFrameMixerMachine::SelectMachine(Machine* pMachine)
	{
		_pMixer=(Mixer*)pMachine;

		int sends(0),cols(0);
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->SendValid(i)) {
				sendNames[sends]=Global::_pSong->_pMachine[_pMixer->GetSend(i)]->GetEditName();
				sends++;
			}
		}
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->_inputCon[i]) cols++;
		}
		int winh = InfoLabel::height + ((sends+1) * Knob::height) + GraphSlider::height;
		int winw = (cols+sends+1) * (Knob::width+InfoLabel::width);

		CWnd *dsk = GetDesktopWindow();
		CRect rClient;
		dsk->GetClientRect(&rClient);
		MoveWindow
			(
			0,
			0,
			winw + 2*GetSystemMetrics(SM_CXDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
			winh + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + 2*GetSystemMetrics(SM_CYDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
			false
			);
		numChannels =cols;
		numSends=sends;

		ShowWindow(SW_SHOW);

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
		KillTimer(2104+MachineIndex);
		CFrameWnd::OnDestroy();
	}

	void CFrameMixerMachine::OnTimer(UINT nIDEvent) 
	{
		if ( nIDEvent == 2104+MachineIndex )
		{
			int sends(0),cols(0);
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_pMixer->SendValid(i)) {
					sendNames[sends]=Global::_pSong->_pMachine[_pMixer->GetSend(i)]->GetEditName();
					sends++;
				}
			}
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_pMixer->_inputCon[i]) cols++;
			}
			if (cols != numChannels || sends != numSends)
			{
				int winh = InfoLabel::height + ((sends+1) * Knob::height) + GraphSlider::height;
				int winw = (cols+sends+1) * (Knob::width+InfoLabel::width);

				CRect rect;
				GetWindowRect(&rect);
				MoveWindow
					(
					rect.left,
					rect.top,
					winw + 3*GetSystemMetrics(SM_CXDLGFRAME),
					winh + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + 3*GetSystemMetrics(SM_CYDLGFRAME),
					true
					);
				numChannels =cols;
				numSends=sends;
			}
			else Invalidate(false);
		}
		CFrameWnd::OnTimer(nIDEvent);
	}
	void CFrameMixerMachine::OnPaint() 
	{
		if (!_pMixer) return;
		CPaintDC dc(this); // device context for painting

		CDC memDC;
		CDC memDC2;
		CBitmap* oldbmp,*oldbmp2,*oldbmp3;
		
		int xoffset(0), yoffset(0);
		//dc.FillSolidRect(0,0,1200,1024,RGB(0x90,0x90,0x90));
		dc.FillSolidRect(0,0,1200,1024,Global::pConfig->machineGUIBottomColor);
		// Column 0, Labels.
		dc.SelectObject(&InfoLabel::font);
//		InfoLabel::Draw(&dc,xoffset,yoffset,"","");
		for (int i=0; i <numSends;i++)
		{
			yoffset+=InfoLabel::height;
			std::string sendtxt = "Send ";
			sendtxt += ('0'+i+1);
			InfoLabel::DrawHLight(&dc,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
		}
		yoffset+=InfoLabel::height;
		InfoLabel::DrawHeader(&dc,xoffset,yoffset,"Dry Mix","");

		yoffset+=GraphSlider::height;
		InfoLabel::DrawHeader(&dc,xoffset,yoffset,"Ch. Input","");
//		dc.Draw3dRect(xoffset,0,Knob::width+InfoLabel::width,yoffset+InfoLabel::height,RGB(20,20,20),RGB(20,20,20));
		

		// Colums 1 onwards, controls
		xoffset+=InfoLabel::width+Knob::width;
		Knob::knobDC.CreateCompatibleDC(&dc);
		oldbmp=Knob::knobDC.SelectObject(&wndView->machinedial);
		GraphSlider::knobDC.CreateCompatibleDC(&dc);
		oldbmp2=GraphSlider::knobDC.SelectObject(&m_sliderknob);
		GraphSlider::backDC.CreateCompatibleDC(&dc);
		oldbmp3=GraphSlider::backDC.SelectObject(&m_sliderback);

		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			yoffset=0;
			if (_pMixer->_inputCon[i])
			{
				InfoLabel::DrawHeader(&dc,xoffset,yoffset,Global::_pSong->_pMachine[_pMixer->_inputMachines[i]]->GetEditName(),"");

				yoffset+=InfoLabel::height;
				for (int i=0; i<numSends; i++)
				{
					Knob::Draw(&dc,xoffset,yoffset,0.0f);
					InfoLabel::Draw(&dc,xoffset+Knob::width,yoffset,"Send","");
					yoffset+=Knob::height;
				}
				Knob::Draw(&dc,xoffset,yoffset,0.0f);
				InfoLabel::Draw(&dc,xoffset+Knob::width,yoffset,"Mix","");
				InfoLabel::Draw(&dc,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
				yoffset+=Knob::width;
				GraphSlider::Draw(&dc,xoffset,yoffset,0);
				
				xoffset+=Knob::width+InfoLabel::width;
			}
		}
		for (int i=0; i<numSends; i++)
		{
			yoffset=0;
			InfoLabel::DrawHeader(&dc,xoffset,yoffset,sendNames[i].c_str(),"");
			yoffset+=(numSends+1)*InfoLabel::height;
			InfoLabel::Draw(&dc,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
			yoffset+=InfoLabel::height;
			GraphSlider::Draw(&dc,xoffset,yoffset,0);
			dc.Draw3dRect(xoffset-1,0-1,Knob::width+InfoLabel::width+1,yoffset+1,RGB(20,20,20),RGB(20,20,20));
			xoffset+=Knob::width+InfoLabel::width;
		}

		GraphSlider::backDC.SelectObject(oldbmp3);
		GraphSlider::backDC.DeleteDC();
		GraphSlider::knobDC.SelectObject(oldbmp2);
		GraphSlider::knobDC.DeleteDC();
		Knob::knobDC.SelectObject(oldbmp);
		Knob::knobDC.DeleteDC();

	}
	void CFrameMixerMachine::OnLButtonDown(UINT nFlags, CPoint point) 
	{
		CFrameWnd::OnLButtonDown(nFlags, point);
	}
	void CFrameMixerMachine::OnMouseMove(UINT nFlags, CPoint point) 
	{
		CFrameWnd::OnMouseMove(nFlags, point);
	}
	void CFrameMixerMachine::OnLButtonUp(UINT nFlags, CPoint point) 
	{
/*		istweak = false;
		Invalidate(false);	
		ReleaseCapture();
*/
		CFrameWnd::OnLButtonUp(nFlags, point);
	}

	void CFrameMixerMachine::OnRButtonUp(UINT nFlags, CPoint point) 
	{
		CFrameWnd::OnRButtonUp(nFlags, point);
	}
	void CFrameMixerMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
    }
	void CFrameMixerMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	void CFrameMixerMachine::OnSetFocus(CWnd* pOldWnd) 
	{
		CFrameWnd::OnSetFocus(pOldWnd);
		Invalidate(false);
	}

	NAMESPACE__END
NAMESPACE__END
