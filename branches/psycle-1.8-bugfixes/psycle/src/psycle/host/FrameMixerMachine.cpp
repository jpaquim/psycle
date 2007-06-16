#include <project.private.hpp>
#include "Psycle.hpp"
#include "FrameMixerMachine.hpp"
#include "NativeGui.hpp"
#include "ChildView.hpp"
#include "configuration.hpp"
#include "song.hpp"
#include "internal_machines.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

	IMPLEMENT_DYNCREATE(CFrameMixerMachine, CFrameWnd)

	int CFrameMixerMachine::Knob::height(28);
	int CFrameMixerMachine::Knob::width(28);
	int CFrameMixerMachine::Knob::numFrames(64);
	CDC CFrameMixerMachine::Knob::knobDC;

	int CFrameMixerMachine::InfoLabel::xoffset(3);
	int CFrameMixerMachine::InfoLabel::width(32);
	int CFrameMixerMachine::InfoLabel::height(28);

	int CFrameMixerMachine::GraphSlider::height(140);
	int CFrameMixerMachine::GraphSlider::width(28);
	int CFrameMixerMachine::GraphSlider::knobheight(21);
	int CFrameMixerMachine::GraphSlider::knobwidth(16);
	int CFrameMixerMachine::GraphSlider::xoffset(6);
	CDC CFrameMixerMachine::GraphSlider::backDC;
	CDC CFrameMixerMachine::GraphSlider::knobDC;

	int CFrameMixerMachine::CheckedButton::height(16);
	int CFrameMixerMachine::CheckedButton::width(16);

	int CFrameMixerMachine::VuMeter::height(90);
	int CFrameMixerMachine::VuMeter::width(16);
	CDC CFrameMixerMachine::VuMeter::VuOff;
	CDC CFrameMixerMachine::VuMeter::VuOn;

	//////////////////////////////////////////////////////////////////////////
	// Knob class
	void CFrameMixerMachine::Knob::Draw(CDC* dc,int x_knob,int y_knob,float value)
	{
		int pixel = numFrames*value;
		if (pixel >= 64) pixel=63;
		pixel*=width;
		dc->BitBlt(x_knob,y_knob,width,height,&knobDC,pixel,0,SRCCOPY);
		dc->Draw3dRect(x_knob,y_knob+height-1,width,1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
	}
	bool  CFrameMixerMachine::Knob::LButtonDown(UINT nFlags,int x,int y)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// InfoLabel class
	void CFrameMixerMachine::InfoLabel::Draw(CDC* dc, int x, int y,const char *parName,const char *parValue)
	{
		const int half = height/2;
		dc->Draw3dRect(x,y-1,width,height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
//		dc->FillSolidRect(x, y, width-1, half,Global::configuration().machineGUITopColor);
//		dc->FillSolidRect(x, y+half, width-1, half-1,Global::configuration().machineGUIBottomColor);

		dc->SetBkColor(Global::configuration().machineGUITopColor);
		dc->SetTextColor(Global::configuration().machineGUIFontTopColor);
		dc->ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width-1, y+half), CString(parName), 0);
	
		DrawValue(dc,x,y,parValue);
	}
	void CFrameMixerMachine::InfoLabel::DrawValue(CDC* dc, int x, int y,const char *parValue)
	{
		const int half = height/2;
		dc->SetBkColor(Global::configuration().machineGUIBottomColor);
		dc->SetTextColor(Global::configuration().machineGUIFontBottomColor);
		dc->ExtTextOut(x+xoffset, y+half, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y+half, x+width-1, y+height-1), CString(parValue), 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// HLightInfoLabel class
	void CFrameMixerMachine::InfoLabel::DrawHLight(CDC* dc, CFont* b_font_bold,int x, int y,const char *parName,const char *parValue)
	{
		const int half = height/2;
		const int mywidth = width + Knob::width;
//		dc->FillSolidRect(x, y, mywidth, half,Global::configuration().machineGUITitleColor);
//		dc->FillSolidRect(x, y+half, mywidth, half,Global::configuration().machineGUIBottomColor);

		dc->SetBkColor(Global::configuration().machineGUITitleColor);
		dc->SetTextColor(Global::configuration().machineGUITitleFontColor);
		CFont *oldfont =dc->SelectObject(b_font_bold);
		dc->ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+mywidth-1, y+half), CString(parName), 0);
		dc->SelectObject(oldfont);

		DrawHLightValue(dc,x,y,parValue);
		dc->Draw3dRect(x-1,y-1,width+ Knob::width+1,height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
	}
	void CFrameMixerMachine::InfoLabel::DrawHLightValue(CDC* dc, int x, int y,const char *parValue)
	{
		const int half = height/2;
		const int mywidth = width + Knob::width;
		dc->SetBkColor(Global::configuration().machineGUIBottomColor);
		dc->SetTextColor(Global::configuration().machineGUIFontBottomColor);
		dc->ExtTextOut(x+xoffset, y+half,ETO_OPAQUE | ETO_CLIPPED, CRect(x+1, y+half, x+mywidth-1, y+height), CString(parValue), 0);
	}
	
	//////////////////////////////////////////////////////////////////////////
	// HeaderInfoLabel class
/*	void CFrameMixerMachine::InfoLabel::DrawHeader(CDC* dc, int x, int y,const char *parName, const char *parValue, bool checked)
	{
		const int half = height/2;
		const int quarter = height/4;
		const int mywidth = width + Knob::width;

		dc->FillSolidRect(x, y, mywidth, half,Global::configuration().machineGUITopColor);
		dc->FillSolidRect(x, y+half, mywidth, half,Global::configuration().machineGUIBottomColor);

		dc->SetBkColor(Global::configuration().machineGUITitleColor);
		dc->SetTextColor(Global::configuration().machineGUITitleFontColor);

		dc->SelectObject(&b_font_bold);
		dc->ExtTextOut(x + xoffset, y + quarter, ETO_OPAQUE | ETO_CLIPPED, CRect(x+1, y + quarter, x+mywidth-1, y+half+quarter), CString(parName), 0);
		dc->SelectObject(&b_font);
		dc->Draw3dRect(x-1,y-1,width+Knob::width+1,height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);

	}
*/
	//////////////////////////////////////////////////////////////////////////
	// GraphSlider class
	void CFrameMixerMachine::GraphSlider::Draw(CDC *dc,int x, int y,float value)
	{
		dc->BitBlt(x,y,width,height,&backDC,0,0,SRCCOPY);
//		dc->FillSolidRect(x+width, y, InfoLabel::width, height-InfoLabel::height,Global::configuration().machineGUITopColor);
		dc->Draw3dRect(x-1,y-1,width+1,height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
		DrawKnob(dc,x,y,value);
	}
	void CFrameMixerMachine::GraphSlider::DrawKnob(CDC *dc,int x, int y, float value)
	{
		int ypos = (1.0-value)*(height-knobheight);
		dc->BitBlt(x+xoffset,y+ypos,knobwidth,knobheight,&knobDC,0,0,SRCCOPY);
	}
	bool  CFrameMixerMachine::GraphSlider::LButtonDown(UINT nFlags,int x,int y)
	{
		return false;
	}

	void CFrameMixerMachine::CheckedButton::Draw(CDC* dc, CFont* b_font_bold,int x, int y,const char* text,bool checked)
	{
		if ( checked )
		{
			dc->SetBkColor(Global::configuration().machineGUIHBottomColor);
			dc->SetTextColor(Global::configuration().machineGUIHFontBottomColor);
		}
		else{
			dc->SetBkColor(Global::configuration().machineGUITopColor);
			dc->SetTextColor(Global::configuration().machineGUIFontTopColor);
		}
		CFont *oldfont =dc->SelectObject(b_font_bold);
		dc->ExtTextOut(x+4, y+1, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width-1, y+height-1), CString(text), 0);
		dc->SelectObject(oldfont);
		dc->Draw3dRect(x-1,y-1,width+1,height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
	}


	//////////////////////////////////////////////////////////////////////////
	// Vumeter class
	void CFrameMixerMachine::VuMeter::Draw(CDC *dc,int x, int y, float value)
	{
		int ypos = (1-value)*height;
		dc->BitBlt(x+7,y+19,width,ypos,&VuOff,0,0,SRCCOPY);
		dc->BitBlt(x+7,y+19+ypos,width,height,&VuOn,0,ypos,SRCCOPY);
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
		ON_WM_SETFOCUS()
		ON_WM_KEYDOWN()
		ON_WM_KEYUP()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	CFrameMixerMachine::CFrameMixerMachine()
	{
		//do not use. use CFrameMixerMachine(int dum)
	}
	CFrameMixerMachine::CFrameMixerMachine(int dum)
		///\todo: this line doesn't work. why?
//		:CFrameMachine::CFrameMachine(dum)
		:_pMixer(0)
		,bmpDC(0)
		,numChans(0)
		,numSends(0)
		,updateBuffer(false)
	{
		MachineIndex = dum;
	}
	CFrameMixerMachine::~CFrameMixerMachine()
	{
		if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
	}

	int CFrameMixerMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
		int retVal = CFrameMachine::OnCreate(lpCreateStruct);
		if ( retVal >= 0 )
		{
			m_sliderback.LoadBitmap(IDB_SLIDERBACKV);
			m_vumeteroff.LoadBitmap(IDB_VUMETEROFF);
			m_sliderknob.LoadBitmap(IDB_SLIDERKNOBV);
			m_vumeteron.LoadBitmap(IDB_VUMETERON);
		}
		return retVal;
	}

	void CFrameMixerMachine::OnDestroy() 
	{
		CFrameMachine::OnDestroy();
	}

	bool CFrameMixerMachine::UpdateSendsandChans()
	{
		int sends(0),cols(0);
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->SendValid(i)) {
				sendNames[sends]=Global::song()._pMachine[_pMixer->GetSend(i)]->GetEditName();
				sends++;
			}
		}
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->_inputCon[i]) cols++;
		}
		if ( numSends!=sends || numChans !=cols)
		{
			numSends=sends; numChans=cols;
			return true;
		}
		return false;
	}
	void CFrameMixerMachine::SelectMachine(Machine* pMachine)
	{
		_pMixer=(Mixer*)(_pMachine = pMachine);
		numParameters = 255;
		UpdateSendsandChans();

		int winh = InfoLabel::height + ((numSends+1) * Knob::height) + GraphSlider::height;
		int winw = (numChans+numSends+1+1) * (Knob::width+InfoLabel::width); // 1+1 -> labels column, plus master volume.
		CRect rect;
		GetWindowRect(&rect);
		MoveWindow
			(
			rect.left,
			rect.top,
			winw + 2*GetSystemMetrics(SM_CXDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
			winh + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + 2*GetSystemMetrics(SM_CYDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
			false
			);
		updateBuffer=true;
		ShowWindow(SW_SHOW);
	}

	void CFrameMixerMachine::Generate(CDC &bufferDC)
	{
		// Draw to buffer.
		// Column 0, Labels.
		int xoffset(0), yoffset(0);
		for (int i=0; i <numSends;i++)
		{
			yoffset+=InfoLabel::height;
			std::string sendtxt = "Send ";
			sendtxt += ('0'+i+1);
			InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
		}
		yoffset+=InfoLabel::height;
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,"Dry Mix","");

		yoffset+=GraphSlider::height;
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,"Ch. Input","");

		// Column 1 master Volume.
		xoffset+=InfoLabel::width+Knob::width;
		yoffset=0;
		std::string mastertxt = "Master Out";
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,mastertxt.c_str(),"");
		yoffset+=(numSends+1)*InfoLabel::height;
		InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
		yoffset+=InfoLabel::height;
		GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
		VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset,0);
		bufferDC.Draw3dRect(xoffset-1,0-1,Knob::width+InfoLabel::width+1,yoffset+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);


		// Columns 2 onwards, controls
		xoffset+=InfoLabel::width+Knob::width;
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			yoffset=0;
			if (_pMixer->_inputCon[i])
			{
				std::string chantxt = _pMixer->GetAudioInputName(int(i+Mixer::chan1));
				InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),Global::song()._pMachine[_pMixer->_inputMachines[i]]->_editName);

				yoffset+=InfoLabel::height;
				for (int j=0; j<numSends; j++)
				{
					InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Send","");
					yoffset+=Knob::height;
				}
				InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Mix","");
				InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
				yoffset+=Knob::width;
				GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
				VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset,0);
				xoffset+=Knob::width+InfoLabel::width;
			}
		}
		for (int i=0; i<numSends; i++)
		{
			yoffset=0;
			std::string sendtxt = _pMixer->GetAudioInputName(int(i+Mixer::return1));
			InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
			yoffset+=(numSends+1)*InfoLabel::height;
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
			yoffset+=InfoLabel::height;
			GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
			VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset,0);
			bufferDC.Draw3dRect(xoffset-1,0-1,Knob::width+InfoLabel::width+1,yoffset+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
			xoffset+=Knob::width+InfoLabel::width;
		}
	}

	void CFrameMixerMachine::OnTimer(UINT nIDEvent) 
	{
		if ( nIDEvent == 2104+MachineIndex )
		{
			if (UpdateSendsandChans())
			{
				updateBuffer=true;

				int winh = InfoLabel::height + ((numSends+1) * Knob::height) + GraphSlider::height;
				int winw = (numChans+numSends+1+1) * (Knob::width+InfoLabel::width); // +1+1 -> labels column + master out

				CRect rect;
				GetWindowRect(&rect);
				MoveWindow
					(
					rect.left,
					rect.top,
					winw + 2*GetSystemMetrics(SM_CXDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
					winh + GetSystemMetrics(SM_CYCAPTION) +  GetSystemMetrics(SM_CYMENU) + 2*GetSystemMetrics(SM_CYDLGFRAME)+2*GetSystemMetrics(SM_CXEDGE),
					true
					);
			}
			else Invalidate(false);
		}
		CFrameWnd::OnTimer(nIDEvent);
	}
	void CFrameMixerMachine::OnPaint() 
	{
		if (!_pMixer) return;
		CPaintDC dc(this); // device context for painting

		CRect rect;
		GetClientRect(&rect);
		if (updateBuffer) 
		{
			if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
			bmpDC = new CBitmap;
			bmpDC->CreateCompatibleBitmap(&dc,rect.right-rect.left,rect.bottom-rect.top);
		}

		CDC bufferDC;
		bufferDC.CreateCompatibleDC(&dc);
		CBitmap *bufferbmp = bufferDC.SelectObject(bmpDC);
		CFont *oldfont=bufferDC.SelectObject(&b_font);

		Knob::knobDC.CreateCompatibleDC(&bufferDC);
		CBitmap *oldbmp=Knob::knobDC.SelectObject(&CNativeGui::uiSetting().dial);
		GraphSlider::knobDC.CreateCompatibleDC(&bufferDC);
		CBitmap *oldbmp2=GraphSlider::knobDC.SelectObject(&m_sliderknob);
		VuMeter::VuOn.CreateCompatibleDC(&bufferDC);
		CBitmap *oldbmp3=VuMeter::VuOn.SelectObject(&m_vumeteron);
		GraphSlider::backDC.CreateCompatibleDC(&bufferDC);
		CBitmap *sliderbmp=GraphSlider::backDC.SelectObject(&m_sliderback);
		VuMeter::VuOff.CreateCompatibleDC(&bufferDC);
		CBitmap *vubmp=VuMeter::VuOff.SelectObject(&m_vumeteroff);

		if (updateBuffer) 
		{
			bufferDC.FillSolidRect(0,0,rect.right,rect.bottom,Global::configuration().machineGUIBottomColor);
			Generate(bufferDC);
			updateBuffer=false;
		}
		// Column 1 Master volume
		int xoffset(InfoLabel::width+Knob::width), yoffset(0);
		char value[48];

		yoffset=(numSends+1)*InfoLabel::height;
		_pMixer->GetParamValue(0,value);
		InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,value);
		yoffset+=InfoLabel::height;
		GraphSlider::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(0)/256.0f);
		CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"D",_pMixer->Bypass());
		CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"W",_pMixer->GetMuteState(0));
		VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset,_pMixer->_volumeDisplay/97.0f);

		// Columns 2 onwards, controls
		xoffset+=Knob::width+InfoLabel::width;
		for (int i(0); i<MAX_CONNECTIONS; i++)
		{
			if (_pMixer->_inputCon[i])
			{
				yoffset=InfoLabel::height;
				for (int j=0; j<numSends; j++)
				{
					int param =(i+1)*0x10+(j+1);
					Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/100.0f);
					_pMixer->GetParamValue(param,value);
					InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
					yoffset+=Knob::height;
				}
				int param =(i+1)*0x10;
				Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/100.0f);
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
				param= 0xE0+i+1;
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,value);
				yoffset+=Knob::width;
				GraphSlider::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/100.0f);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"S",_pMixer->GetSoloState(i));
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"M",_pMixer->GetMuteState(i));
				VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset,_pMixer->VuChan(i));
				xoffset+=Knob::width+InfoLabel::width;
			}
		}
		for (int i(0); i<numSends; i++)
		{
			int param =0xF0+i+1;
			yoffset=(numSends+1)*InfoLabel::height;
			_pMixer->GetParamValue(param,value);
			InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,value);
			yoffset+=InfoLabel::height;
			GraphSlider::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/100.0f);
			CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"S",_pMixer->GetSoloState(i+MAX_CONNECTIONS));
			CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"M",_pMixer->GetMuteState(i+MAX_CONNECTIONS));
			VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset,_pMixer->VuSend(i));
			xoffset+=Knob::width+InfoLabel::width;
		}

		dc.BitBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,SRCCOPY);

		VuMeter::VuOn.SelectObject(oldbmp3);
		VuMeter::VuOn.DeleteDC();
		GraphSlider::knobDC.SelectObject(oldbmp2);
		GraphSlider::knobDC.DeleteDC();
		Knob::knobDC.SelectObject(oldbmp);
		Knob::knobDC.DeleteDC();
		VuMeter::VuOff.SelectObject(vubmp);
		VuMeter::VuOff.DeleteDC();
		GraphSlider::backDC.SelectObject(sliderbmp);
		GraphSlider::backDC.DeleteDC();

		bufferDC.SelectObject(oldfont);
		bufferDC.SelectObject(bufferbmp);
		bufferDC.DeleteDC();

	}
	int CFrameMixerMachine::GetColumn(int x, int &xoffset)
	{
		int col=x/(Knob::width+InfoLabel::width);
		xoffset=x%(Knob::width+InfoLabel::width);
		if ( col == 0) return Mixer::collabels;
		else if ( col == 1) return Mixer::colmastervol;
		else if ( col-Mixer::chan1 < numChans)
		{
			col-=Mixer::chan1;
			return Mixer::chan1+col;
		}
		else
		{
			col-=numChans+Mixer::chan1;
			return Mixer::return1+col;
		}
	}
	int CFrameMixerMachine::GetRow(int y,int &yoffset)
	{
		int row = y/InfoLabel::height;
		yoffset=y%InfoLabel::height;
		if ( row == 0) return rowlabels;
		else if ( row <= numSends)
		{
			row-=send1;
			return send1+row;
		}
		else
		{
			row-=numSends+send1;
			if (row == 0 ) return dry;
			else if (y < GraphSlider::width)
			{
				return slider;
			}
			else if ( y < GraphSlider::width+CheckedButton::width)
			{
				return Mixer::solo;
			}
			else return Mixer::mute;
		}
	}
	int CFrameMixerMachine::GetParamFromPos(int col,int row)
	{
		if ( col == Mixer::colmastervol) return 0;
		if ( col < Mixer::chan12)
		{
			if (row < dry) return (col-Mixer::chan1+1)*0x10+(row-send1+1);
			else if ( row==dry) return (col-Mixer::chan1+1)*0x10;
			else return 0xE0+(col-Mixer::chan1+1);
		}
		else 
		{
			if (row > dry) return 0xF0+(col-Mixer::return1+1);
		}
		return -1;
	}
	int CFrameMixerMachine::ConvertXYtoParam(int x, int y)
	{
		int xoffset(0),yoffset(0);
		return GetParamFromPos(GetColumn(x,xoffset),GetRow(y,yoffset));
	}
	void CFrameMixerMachine::OnLButtonDown(UINT nFlags, CPoint point) 
	{
		CFrameMachine::OnLButtonDown(nFlags,point);

/*		int xoffset(0),yoffset(0);
		const int col=GetColumn(point.x,xoffset);
		const int row=GetRow(point.y,yoffset);

		if (row == rowlabels)  return;
		else if ( col == collabels)
	{
			if ( row >=slider) {
				//mute all
				//unmute all
				//mute returns
				//mute dry
			}
		}
		else if ( col <= chan12 )
		{
			if ( row <= dry)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else { istweak=GraphSlider::LButtonDown(nFlags,xoffset,yoffset); }
		}
		else
		{
			if ( row > dry)	{ istweak=GraphSlider::LButtonDown(nFlags,xoffset,yoffset); }
		}

		if (istweak)
		{
			SetCapture();
			sourcepoint=point.y;
			tweakpar=GetParamFromPos(col,row); 
			tweakbase = _pMachine->GetParamValue(tweakpar);
			_pMachine->GetParamRange(tweakpar,minval,maxval);
		}
		CFrameWnd::OnLButtonDown(nFlags, point);
*/
	}
	void CFrameMixerMachine::OnMouseMove(UINT nFlags, CPoint point) 
	{
		CFrameMachine::OnMouseMove(nFlags,point);
	}
	void CFrameMixerMachine::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		CFrameMachine::OnLButtonUp(nFlags,point);
/*		istweak = false;
		Invalidate(false);
		ReleaseCapture();
		CFrameWnd::OnLButtonUp(nFlags, point);
*/
	}

	void CFrameMixerMachine::OnRButtonUp(UINT nFlags, CPoint point) 
	{
		CFrameMachine::OnRButtonUp(nFlags, point);
	}

	void CFrameMixerMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameMachine::OnKeyDown(nChar, nRepCnt, nFlags);
    }
	void CFrameMixerMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameMachine::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	void CFrameMixerMachine::OnSetFocus(CWnd* pOldWnd) 
	{
		CFrameMachine::OnSetFocus(pOldWnd);
	}

NAMESPACE__END
NAMESPACE__END
