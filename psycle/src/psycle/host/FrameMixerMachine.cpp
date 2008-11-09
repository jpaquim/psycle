#include <psycle/project.private.hpp>
#include "FrameMixerMachine.hpp"
#include "Psycle.hpp"
#include "NativeGui.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "internal_machines.hpp"
///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
#include "ChildView.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)


	IMPLEMENT_DYNCREATE(CFrameMixerMachine, CFrameMachine)

	int CFrameMixerMachine::Knob::height(28);
	int CFrameMixerMachine::Knob::width(28);
	int CFrameMixerMachine::Knob::numFrames(64);
	CDC CFrameMixerMachine::Knob::knobDC;

	int CFrameMixerMachine::InfoLabel::xoffset(3);
	int CFrameMixerMachine::InfoLabel::width(32);
	int CFrameMixerMachine::InfoLabel::height(28);

	int CFrameMixerMachine::GraphSlider::height(182);
	int CFrameMixerMachine::GraphSlider::width(28);
	int CFrameMixerMachine::GraphSlider::knobheight(21);
	int CFrameMixerMachine::GraphSlider::knobwidth(16);
	int CFrameMixerMachine::GraphSlider::xoffset(6);
	CDC CFrameMixerMachine::GraphSlider::backDC;
	CDC CFrameMixerMachine::GraphSlider::knobDC;

	int CFrameMixerMachine::SwitchButton::height(28);
	int CFrameMixerMachine::SwitchButton::width(28);
	CDC CFrameMixerMachine::SwitchButton::imgOff;
	CDC CFrameMixerMachine::SwitchButton::imgOn;

	int CFrameMixerMachine::CheckedButton::height(16);
	int CFrameMixerMachine::CheckedButton::width(16);

	int CFrameMixerMachine::VuMeter::height(97);
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
		if ( x< width && y < height) return true;
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
	void CFrameMixerMachine::InfoLabel::DrawHLightB(CDC* dc, CFont* b_font_bold,int x, int y,const char *parName,const char *parValue)
	{
		const int half = height/2;
		const int mywidth = width + Knob::width;
		//		dc->FillSolidRect(x, y, mywidth, half,Global::configuration().machineGUITitleColor);
		//		dc->FillSolidRect(x, y+half, mywidth, half,Global::configuration().machineGUIBottomColor);

		dc->SetBkColor(Global::configuration().machineGUIHTopColor);
		dc->SetTextColor(Global::configuration().machineGUIHFontBottomColor);
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
		int ypos(0);
		if ( value < 0.375 ) ypos = (height-knobheight);
		else if ( value < 0.999) ypos = (((value-0.375f)*1.6f)-1.0f)*-1.0f*(height-knobheight);
		dc->BitBlt(x+xoffset,y+ypos,knobwidth,knobheight,&knobDC,0,0,SRCCOPY);
	}
	bool  CFrameMixerMachine::GraphSlider::LButtonDown(UINT nFlags,int x,int y)
	{
		if ( x< width && y < height) return true;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// SwitchButton class
	void CFrameMixerMachine::SwitchButton::Draw(CDC *dc,int x, int y, bool checked)
	{
		if (checked){ dc->BitBlt(x,y,width,height,&imgOn,0,0,SRCCOPY); }
		else { dc->BitBlt(x,y,width,height,&imgOff,0,0,SRCCOPY); }
		dc->Draw3dRect(x-1,y-1,width+1,height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
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
		dc->BitBlt(x+7,y+35,width,ypos,&VuOff,0,0,SRCCOPY);
		dc->BitBlt(x+7,y+35+ypos,width,height,&VuOn,0,ypos,SRCCOPY);
	}

	//////////////////////////////////////////////////////////////////////////
	// CFrameMixerMachine class
	BEGIN_MESSAGE_MAP(CFrameMixerMachine, CFrameWnd)
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
		,numSends(0)
		,numChans(0)
		,updateBuffer(false)
		,_swapstart(-1)
		,_swapend(-1)
		,isslider(false)
		,refreshheaders(false)
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
			m_switchon.LoadBitmap(IDB_SWITCHON);
			m_switchoff.LoadBitmap(IDB_SWITCHOFF);
		}
		return retVal;
	}

	void CFrameMixerMachine::OnDestroy() 
	{
		CFrameMachine::OnDestroy();
	}

	bool CFrameMixerMachine::UpdateSendsandChans()
	{
		//int sends(0),cols(0);
		for (int i=0; i<_pMixer->numreturns(); i++)
		{
			if (_pMixer->Return(i).IsValid()) {
				sendNames[i]=Global::song()._pMachine[_pMixer->Return(i).Wire().machine_]->GetEditName();
				//sends++;
			}
			else sendNames[i]="";
		}
		/*for (int i=0; i<_pMixer->numinputs(); i++)
		{
			if (_pMixer->ChannelValid(i)) cols++;
		}*/

		if ( numSends != _pMixer->numreturns()/*sends*/ || numChans != _pMixer->numinputs()/*cols*/)
		{
			//numSends= sends; numChans = cols;
			numSends = _pMixer->numreturns(); numChans = _pMixer->numinputs();
			return true;
		}
		return false;
	}
	void CFrameMixerMachine::SelectMachine(Machine* pMachine)
	{
		_pMixer=(Mixer*)(_pMachine = pMachine);
		numParameters = _pMixer->GetNumParams();
		UpdateSendsandChans();

		int winh = InfoLabel::height + ((_pMixer->numsends()+3) * Knob::height) + GraphSlider::height; // + 3 -> pan, gain, mix
		int winw = (_pMixer->numinputs()+_pMixer->numreturns()+2) * (Knob::width+InfoLabel::width); // +2 -> labels column, plus master column.
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
		for (int i=0; i <_pMixer->numsends();i++)
		{
			yoffset+=InfoLabel::height;
			std::string sendtxt = "Send ";
			if ( i < 9 )sendtxt += ('1'+i);
			else { sendtxt += '1'; sendtxt += ('0'+i-9); }
			if ( _pMixer->SendValid(i))
			{
				InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
			}
			else InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),"");
		}
		yoffset+=InfoLabel::height;
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,"Mix","");
		yoffset+=InfoLabel::height;
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,"Gain","");
		yoffset+=InfoLabel::height;
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,"Pan","");

		yoffset+=GraphSlider::height;
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,"Ch. Input","");

		// Column 1 master Volume.
		xoffset+=InfoLabel::width+Knob::width;
		yoffset=0;
		std::string mastertxt = "Master Out";
		InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,mastertxt.c_str(),"");

		yoffset+=(_pMixer->numsends()+1)*InfoLabel::height;
		InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"D/W","");
		yoffset+=InfoLabel::height;
		InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Gain","");
		yoffset+=InfoLabel::height;
		InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Pan","");

		InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
		VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,0);
		yoffset+=InfoLabel::height;
		GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
		bufferDC.Draw3dRect(xoffset-1,0-1,Knob::width+InfoLabel::width+1,yoffset+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);


		// Columns 2 onwards, controls
		xoffset+=InfoLabel::width+Knob::width;
		for (int i=0; i<_pMixer->numinputs(); i++)
		{
			yoffset=0;
			std::string chantxt = _pMixer->GetAudioInputName(int(i+Mixer::chan1));
			if (_pMixer->ChannelValid(i))
			{
				InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),Global::song()._pMachine[_pMixer->_inputMachines[i]]->_editName);
			}
			else InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),"");
			yoffset+=InfoLabel::height;
			for (int j=0; j<_pMixer->numsends(); j++)
			{
				InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Send","");
				yoffset+=Knob::height;
			}
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Mix","");
			yoffset+=InfoLabel::height;
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Gain","");
			yoffset+=InfoLabel::height;
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Pan","");

			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
			VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,0);
			yoffset+=Knob::width;
			GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
			xoffset+=Knob::width+InfoLabel::width;
		}
		for (int i=0; i<_pMixer->numreturns(); i++)
		{
			yoffset=0;
			std::string sendtxt = _pMixer->GetAudioInputName(int(i+Mixer::return1));
			if (_pMixer->ReturnValid(i))
			{
				InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
			}
			else InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),"");
			yoffset+=(2+i)*InfoLabel::height;
			for (int j=i+1; j<_pMixer->numsends(); j++)
			{
				InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Route","");
				yoffset+=Knob::height;
			}
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Master","");

			yoffset=(_pMixer->numsends()+3)*InfoLabel::height;
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Pan","");
			InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"Level","");
			VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,0);
			yoffset+=InfoLabel::height;
			GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
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

				int winh = InfoLabel::height + ((_pMixer->numsends()+3) * Knob::height) + GraphSlider::height; // + 3 -> pan, gain, mix
				int winw = (_pMixer->numinputs()+_pMixer->numreturns()+2) * (Knob::width+InfoLabel::width); // +2 -> labels column, plus master column.

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
		SwitchButton::imgOff.CreateCompatibleDC(&bufferDC);
		CBitmap *switchonbmp=SwitchButton::imgOff.SelectObject(&m_switchoff);
		SwitchButton::imgOn.CreateCompatibleDC(&bufferDC);
		CBitmap *switchoffbmp=SwitchButton::imgOn.SelectObject(&m_switchon);
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

		yoffset=(_pMixer->numsends()+1)*InfoLabel::height;
		_pMixer->GetParamValue(13,value);
		Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(13)/256.0f);
		InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
		bufferDC.Draw3dRect(xoffset-1,yoffset-1,Knob::width+InfoLabel::width+1,Knob::height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);
		yoffset+=InfoLabel::height;
		_pMixer->GetParamValue(14,value);
		Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(14)/1024.0f);
		InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
		yoffset+=InfoLabel::height;
		_pMixer->GetParamValue(15,value);
		Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(15)/256.0f);
		InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);

		_pMixer->GetParamValue(0,value);
		InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,value);
		VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,_pMixer->_volumeDisplay/97.0f);
		yoffset+=InfoLabel::height;
		GraphSlider::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(0)/4096.0f);

		// Columns 2 onwards, controls
		xoffset+=Knob::width+InfoLabel::width;
		for (int i(0); i<_pMixer->numinputs(); i++)
		{
			yoffset=0;
			if ( _swapend != -1 || refreshheaders)
			{
				std::string chantxt = _pMixer->GetAudioInputName(int(i+Mixer::chan1));
				if (_pMixer->ChannelValid(i))
				{
					if ( _swapend == i+chan1)
						InfoLabel::DrawHLightB(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),Global::song()._pMachine[_pMixer->_inputMachines[i]]->_editName);
					else 
						InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),Global::song()._pMachine[_pMixer->_inputMachines[i]]->_editName);
				}
				else
				{
					if ( _swapend == i+chan1)
						InfoLabel::DrawHLightB(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),"");
					else
						InfoLabel::DrawHLightB(&bufferDC,&b_font_bold,xoffset,yoffset,chantxt.c_str(),"");
				}
			}
			if (_pMixer->ChannelValid(i))
			{
				yoffset=InfoLabel::height;
				for (int j=0; j<_pMixer->numsends(); j++)
				{
					int param =(i+1)*0x10+(j+1);
					Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/256.0f);
					_pMixer->GetParamValue(param,value);
					InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
					yoffset+=Knob::height;
				}
				int param =(i+1)*0x10;
				Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/256.0f);
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
				yoffset+=InfoLabel::height;
				param+=14;
				Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/1024.0f);
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
				yoffset+=InfoLabel::height;
				param++;
				Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/256.0f);
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
				param= i+1;
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,value);
				VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,_pMixer->VuChan(i));
				yoffset+=Knob::width;
				GraphSlider::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/4096.0f);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"S",_pMixer->GetSoloState(i));
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"M",_pMixer->Channel(i).Mute());
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset+Knob::width,"D",_pMixer->Channel(i).DryOnly());
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset+Knob::width,"W",_pMixer->Channel(i).WetOnly());
				xoffset+=Knob::width+InfoLabel::width;
			}
			else
			{
				yoffset=InfoLabel::height;
				for (int j=0; j<_pMixer->numsends(); j++)
				{
					InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,"");
					yoffset+=Knob::height;
				}
				Knob::Draw(&bufferDC,xoffset,yoffset,0);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,"");
				yoffset+=InfoLabel::height;
				Knob::Draw(&bufferDC,xoffset,yoffset,0);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,"");
				yoffset+=InfoLabel::height;
				Knob::Draw(&bufferDC,xoffset,yoffset,0);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,"");
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"");
				VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,0);
				yoffset+=Knob::width;
				GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"S",false);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"M",false);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset+Knob::width,"D",false);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset+Knob::width,"W",false);
				xoffset+=Knob::width+InfoLabel::width;
			}
		}
		for (int i(0); i<_pMixer->numreturns(); i++)
		{
			yoffset=0;
			if ( _swapend != -1  || refreshheaders)
			{
				std::string sendtxt = _pMixer->GetAudioInputName(int(i+Mixer::return1));
				if (_pMixer->ReturnValid(i))
				{
					if ( _swapend == i+return1)
						InfoLabel::DrawHLightB(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
					else 
						InfoLabel::DrawHLight(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),sendNames[i].c_str());
				}
				else
				{
					if ( _swapend == i+return1)
						InfoLabel::DrawHLightB(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),"");
					else
						InfoLabel::DrawHLightB(&bufferDC,&b_font_bold,xoffset,yoffset,sendtxt.c_str(),"");
				}
			}

			if (_pMixer->ReturnValid(i))
			{
				yoffset=(2+i)*InfoLabel::height;
				for (int j=i+1; j<_pMixer->numsends(); j++)
				{
					SwitchButton::Draw(&bufferDC,xoffset,yoffset,GetRouteState(i,j));
					yoffset+=Knob::height;
				}
				SwitchButton::Draw(&bufferDC,xoffset,yoffset,GetRouteState(i,13));

				int param =0xF1+i;
				yoffset=(_pMixer->numsends()+3)*InfoLabel::height;
				_pMixer->GetParamValue(param,value);
				Knob::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/256.0f);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,value);
				bufferDC.Draw3dRect(xoffset-1,yoffset-1,Knob::width+InfoLabel::width+1,Knob::height+1,Global::configuration().machineGUITitleColor,Global::configuration().machineGUITitleColor);

				param = 0xE1+i;
				_pMixer->GetParamValue(param,value);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,value);
				VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,_pMixer->VuSend(i));
				yoffset+=InfoLabel::height;
				GraphSlider::Draw(&bufferDC,xoffset,yoffset,_pMixer->GetParamValue(param)/4096.0f);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"S",_pMixer->GetSoloState(i+Mixer::return1));
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"M",_pMixer->Return(i).Mute());
				xoffset+=Knob::width+InfoLabel::width;
			}
			else
			{
				yoffset+=(1+i)*InfoLabel::height;
				for (int j=i+1; j<_pMixer->numsends(); j++)
				{
					InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Route","");
					yoffset+=Knob::height;
				}
				InfoLabel::Draw(&bufferDC,xoffset+Knob::width,yoffset,"Master","");

				yoffset=(_pMixer->numsends()+3)*InfoLabel::height;
				Knob::Draw(&bufferDC,xoffset,yoffset,0);
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset,"");
				InfoLabel::DrawValue(&bufferDC,xoffset+Knob::width,yoffset+GraphSlider::height,"");
				VuMeter::Draw(&bufferDC,xoffset+GraphSlider::width,yoffset+GraphSlider::height-InfoLabel::height-VuMeter::height,0);
				yoffset+=InfoLabel::height;
				GraphSlider::Draw(&bufferDC,xoffset,yoffset,0);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width,yoffset,"S",false);
				CheckedButton::Draw(&bufferDC,&b_font_bold,xoffset+GraphSlider::width+CheckedButton::width,yoffset,"M",false);
				xoffset+=Knob::width+InfoLabel::width;
			}
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
		SwitchButton::imgOff.SelectObject(switchoffbmp);
		SwitchButton::imgOff.DeleteDC();
		SwitchButton::imgOn.SelectObject(switchonbmp);
		SwitchButton::imgOn.DeleteDC();
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
		if ( col < chan1+_pMixer->numinputs()) return col;
		else
		{
			col-=chan1+_pMixer->numinputs();
			return return1+col;
		}
	}
	int CFrameMixerMachine::GetRow(int x,int y,int &yoffset)
	{
		int row = y/InfoLabel::height;
		yoffset=y%InfoLabel::height;
		if (row < send1+_pMixer->numsends()) return row;
		else
		{
			row-=send1+_pMixer->numsends();
			if (row == 0 ) return mix;
			else if (row == 1) return gain;
			else if (row == 2) return pan;
			else if (x < GraphSlider::width)
			{
				yoffset = y - (InfoLabel::height*(_pMixer->numsends()+4 ));
				return slider;
			}
			else if ( row == 3 )
			{
				if ( x < GraphSlider::width+CheckedButton::width)
				{
					return solo;
				}
				else return mute;
			}
			else if (row == 4)
			{
				if ( x < GraphSlider::width+CheckedButton::width)
				{
					return dryonly;
				}
				else return wetonly;
			}
		}
		return -1;
	}
	int CFrameMixerMachine::GetParamFromPos(int col,int row)
	{
		if ( col == collabels || row == rowlabels) return -1;
		else if ( col == colmaster)
		{
			if ( row == slider) return 0;
			else if (row == mix) return 13;
			else if (row == gain) return 14;
			else if (row == pan) return 15;
			return -1;
		}
		else if ( col < chanmax)
		{
			int chan = col - chan1;
			if (row < sendmax) return (chan+1)*0x10+(row-send1+1);
			else if ( row==mix) return (chan+1)*0x10;
			else if ( row==gain) return (chan+1)*0x10 +14;
			else if ( row==pan) return (chan+1)*0x10 +15;
			else if ( row==slider) return (chan+1);
			else if ( row==solo) return 13*0x10;
			else if ( row==mute) return (chan+1)*0x10 +13;
			else if ( row==dryonly) return (chan+1)*0x10 +13;
			else if ( row==wetonly) return (chan+1)*0x10 +13;
		}
		else 
		{
			int chan = col - return1;
			if (row < sendmax) return 13*0x10 +(chan+1);
			else if ( row==mix) return 13*0x10 +(chan+1); // mix is route to master.
			else if ( row==slider) return 14*0x10 +(chan+1);
			else if ( row==pan) return 15*0x10 +(chan+1);
			else if ( row==solo) return 13*0x10;
			else if ( row==mute) return 13*0x10 +(chan+1);
		}
		return -1;
	}
	int CFrameMixerMachine::ConvertXYtoParam(int x, int y)
	{
		int xoffset(0),yoffset(0);
		return GetParamFromPos(GetColumn(x,xoffset),GetRow(x%(Knob::width+InfoLabel::width),y,yoffset));
	}
	bool CFrameMixerMachine::GetRouteState(int ret,int send)
	{
		if (send < sendmax)
			return _pMixer->Return(ret).Send(send);
		else if ( send == 13)
			return _pMixer->Return(ret).MasterSend();
		return false;
	}
	void CFrameMixerMachine::OnLButtonDown(UINT nFlags, CPoint point) 
	{
//		CFrameMachine::OnLButtonDown(nFlags,point);

		int xoffset(0),yoffset(0);
		const int col=GetColumn(point.x,xoffset);
		const int row=GetRow(xoffset,point.y,yoffset);
		
		istweak = false;
		isslider = false;

		if (col == collabels || (row == rowlabels && col == colmaster)) return;
		if (row == rowlabels)
		{
			// move/swap channels.
			_swapstart = _swapend = col;
		}
		else if (col == colmaster)
		{
			if ( row == mix)		{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == gain)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == pan)		{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == slider) { istweak=GraphSlider::LButtonDown(nFlags,xoffset,yoffset);isslider=istweak; }
		}
		else if ( col < chanmax )
		{
			int chan = col - chan1;
			if ( row < sendmax)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == mix)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == gain)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == pan)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == slider) { istweak=GraphSlider::LButtonDown(nFlags,xoffset,yoffset); isslider=istweak;}
			else if ( row == solo)
			{
				chan++;
				tweakpar=GetParamFromPos(col,row);
				int solo = _pMachine->GetParamValue(tweakpar);
				_pMachine->SetParameter(tweakpar,(solo==chan)?0:chan);
			}
			else if ( row == mute)
			{
				tweakpar=GetParamFromPos(col,row);
				int statebits = _pMachine->GetParamValue(tweakpar);
				_pMachine->SetParameter(tweakpar,statebits==3?0:3);
			}
			else if ( row == dryonly)
			{
				tweakpar=GetParamFromPos(col,row);
				int statebits = _pMachine->GetParamValue(tweakpar);
				_pMachine->SetParameter(tweakpar,statebits==1?0:1);
			}
			else if ( row == wetonly)
			{
				tweakpar=GetParamFromPos(col,row);
				int statebits = _pMachine->GetParamValue(tweakpar);
				_pMachine->SetParameter(tweakpar,statebits==2?0:2);
			}
		}
		else
		{
			int ret = col - return1;
			if ( row < sendmax)
			{
				tweakpar=GetParamFromPos(col,row);
				int statebits = _pMachine->GetParamValue(tweakpar);
				// XOR to the "row+1"th bit.
				_pMachine->SetParameter(tweakpar,(statebits & ~(1<<row)) | ((statebits&(1<<row)) ^ (1<<row)));
			}
			else if ( row == mix )
			{
				tweakpar=GetParamFromPos(col,row);
				int statebits = _pMachine->GetParamValue(tweakpar);
				// XOR to the 14th bit.
				_pMachine->SetParameter(tweakpar,(statebits & ~(1<<13)) | ((statebits&(1<<13)) ^ (1<<13)));
			}

			else if ( row == pan)	{ istweak=Knob::LButtonDown(nFlags,xoffset,yoffset); }
			else if ( row == slider) { istweak=GraphSlider::LButtonDown(nFlags,xoffset,yoffset); isslider=istweak; }
			else if ( row == solo)
			{
				ret+=MAX_CONNECTIONS+1;
				tweakpar=GetParamFromPos(col,row);
				int solo = _pMachine->GetParamValue(tweakpar);
				_pMachine->SetParameter(tweakpar,(solo==ret)?0:ret);
			}
			else if ( row == mute)
			{
				tweakpar=GetParamFromPos(col,row);
				int statebits = _pMachine->GetParamValue(tweakpar);
				// XOR to the 1st bit.
				_pMachine->SetParameter(tweakpar,(statebits & ~0x1) | ((statebits&0x1) ^0x1));
			}
		}

		if (istweak)
		{
			SetCapture();
			sourcepoint=point.y;
			tweakpar=GetParamFromPos(col,row); 
			_pMachine->GetParamRange(tweakpar,minval,maxval);
			tweakbase = _pMachine->GetParamValue(tweakpar);
			if ( row == slider)
			{
				float foffset = yoffset/ float(GraphSlider::height-GraphSlider::knobheight);
				float fbase = tweakbase/ float(maxval-minval);
				float knobheight = (GraphSlider::knobheight/float(GraphSlider::height-GraphSlider::knobheight));
				float ypos(0);
				if ( fbase < 0.375) ypos = 1.0f;
				else if ( fbase < 0.999) ypos = (((tweakbase/float(maxval-minval)-0.375f)*1.6f)-1.0f)*-1.0f;

				if ( foffset <= ypos || foffset > ypos+knobheight) // if mouse not over the knob, move the knob first
				{
					foffset = foffset - (knobheight/2.0);
					double freak = (maxval-minval)*0.625; // *0.625 adjust the full range to the visual range
					double nv = (1.0f-foffset)*freak + 0.375*(maxval-minval); // 0.375 to compensate for the visual range.

					tweakbase = nv+0.5f;
					_pMachine->SetParameter(tweakpar,tweakbase);
					///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
					//				wndView->AddMacViewUndo();
					if(Global::configuration()._RecordTweaks)
					{
						if(Global::configuration()._RecordMouseTweaksSmooth)
							wndView->MousePatternTweakSlide(_pMachine->_macIndex, tweakpar, tweakbase);
						else
							wndView->MousePatternTweak(_pMachine->_macIndex, tweakpar, tweakbase);
					}
				}
			}
			prevval = tweakbase;
			wndView->AddMacViewUndo();
		}
		CFrameWnd::OnLButtonDown(nFlags, point);
	}
	void CFrameMixerMachine::OnMouseMove(UINT nFlags, CPoint point) 
	{
		if (isslider)
		{
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

			double freak = (maxval-minval)*0.625/(GraphSlider::height-GraphSlider::knobheight); // *0.625 adjust the full range to the visual range
			if ( ultrafinetweak ) freak /= 10;
			if (finetweak) freak/=4;

			double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase; // +0.375 to compensate for the visual range.

			if (nv < minval) nv = minval;
			if (nv > maxval) nv = maxval;
			_pMachine->SetParameter(tweakpar,(int) (nv+0.5f)); // +0.5f to round correctly, not like "floor".
			prevval=(int)(nv+0.5f);
			///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
			//				wndView->AddMacViewUndo();
			if(Global::configuration()._RecordTweaks)
			{
				if(Global::configuration()._RecordMouseTweaksSmooth)
					wndView->MousePatternTweakSlide(_pMachine->_macIndex, tweakpar, prevval);
				else
					wndView->MousePatternTweak(_pMachine->_macIndex, tweakpar, prevval);
			}


			Invalidate(false);
			CFrameWnd::OnMouseMove(nFlags,point);
		}
		else if (_swapstart > -1)
		{
			int xoffset(0);
			int col = GetColumn(point.x,xoffset);
			if ( _swapstart < chanmax && col >= return1) _swapend = -1;
			else if ( _swapstart >= return1 && col < chanmax) _swapend = -1;
			else _swapend = col;
		}
		else CFrameMachine::OnMouseMove(nFlags,point);
	}
	void CFrameMixerMachine::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		if ( _swapstart >= chan1 && _swapend >= chan1 && _swapstart != _swapend)
		{
			if ( _swapstart < chanmax)
			{
				_swapstart -=chan1; _swapend -= chan1; 
				_pMixer->ExchangeChans(_swapstart,_swapend);
			}
			else 
			{
				_swapstart-=return1; _swapend -= return1;
				_pMixer->ExchangeReturns(_swapstart,_swapend);
			}
		}
		refreshheaders=true;
		Invalidate();
		_swapstart = -1;
		_swapend = -1;
		isslider = false;
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
		//beware, if overloaded, needs to change the index.
		CFrameMachine::OnSetFocus(pOldWnd);
	}

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
