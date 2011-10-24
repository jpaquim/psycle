///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "PsycleConfig.hpp"
namespace psycle {
namespace host {

	class Knob
	{
	public:
		inline static void Draw(CDC& dc,CDC& knobDC, int x_knob,int y_knob,float value);
		inline static bool LButtonDown(UINT nFlags, int x, int y);
/*		inline static void MouseMove(UINT nFlags, int x, int y);
		inline static bool LButtonUp(UINT nFlags,int x, int y);
*/
		static PsycleConfig::MachineParam* uiSetting;
	};

	class InfoLabel
	{
	public:
		inline static void Draw(CDC& dc, int x, int y, int width, const char *parName,const char *parValue);
		inline static void DrawValue(CDC& dc, int x, int y, int width, const char *parValue);
		inline static void DrawHLight(CDC& dc, int x, int y, int width, const char *parName,const char *parValue);
		inline static void DrawHLightValue(CDC& dc, int x, int y, int width, const char *parValue);
		inline static void DrawHeader(CDC& dc, int x, int y,int width, const char *parName);
		inline static void DrawHeader2(CDC& dc, int x, int y, int width, const char *parName,const char *parValue);

		static int xoffset;
		static PsycleConfig::MachineParam* uiSetting;
	};

	class GraphSlider
	{
	public:
		inline static void Draw(CDC& dc,CDC& backDC, int x, int y);
		inline static void DrawKnob(CDC& dc,CDC& knobDC, int x, int y, float value);
		inline static bool LButtonDown(UINT nFlags, int x, int y);
/*		inline static void MouseMove(UINT nFlags, int x, int y);
		inline static bool LButtonUp(UINT nFlags,int x, int y);
*/
		static int xoffset;
		static PsycleConfig::MachineParam* uiSetting;
	};

	class SwitchButton
	{
	public:
		inline static void Draw(CDC& dc,CDC& SwitchDC, int x, int y);
		static PsycleConfig::MachineParam* uiSetting;
	};

	class CheckedButton
	{
	public:
		inline static void Draw(CDC& dc,CDC& switchDC, int x, int y,const char* text);
		static PsycleConfig::MachineParam* uiSetting;
	};

	class VuMeter
	{
	public:
		inline static void DrawBack(CDC& dc, CDC& VuOff, int x, int y);
		inline static void Draw(CDC& dc, CDC& VuOn, int x, int y, float value);
		static PsycleConfig::MachineParam* uiSetting;
	};

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Knob class
	void Knob::Draw(CDC& dc, CDC& knobDC, int x_knob,int y_knob,float value)
	{
		const int numFrames = uiSetting->dialframes;
		const int width = uiSetting->dialwidth;
		const int height = uiSetting->dialheight;

		int pixel = numFrames*value;
		if (pixel >= numFrames) pixel=numFrames-1;
		pixel*=width;
		dc.BitBlt(x_knob, y_knob, width, height, &knobDC, pixel, 0, SRCCOPY);
	}
	bool  Knob::LButtonDown( UINT nFlags, int x,int y)
	{
		if ( x< uiSetting->dialwidth
			&& y < uiSetting->dialheight) return true;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// InfoLabel class
	void InfoLabel::Draw(CDC& dc, int x, int y, int width, const char *parName,const char *parValue)
	{
		const int height = uiSetting->dialheight;
		const int half = height/2;
		dc.SetBkColor(uiSetting->topColor);
		dc.SetTextColor(uiSetting->fontTopColor);
		dc.ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width, y+half), CString(parName), 0);
	
		DrawValue(dc,x,y,width,parValue);
	}
	void InfoLabel::DrawValue(CDC& dc, int x, int y, int width, const char *parValue)
	{
		const int height = uiSetting->dialheight;
		const int half = height/2;
		dc.SetBkColor(uiSetting->bottomColor);
		dc.SetTextColor(uiSetting->fontBottomColor);
		dc.ExtTextOut(x+xoffset, y+half, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y+half, x+width, y+height), CString(parValue), 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// HLightInfoLabel class
	void InfoLabel::DrawHLight(CDC& dc,int x, int y,int width,const char *parName,const char *parValue)
	{
		const int height = uiSetting->dialheight;
		const int half = height/2;
		dc.SetBkColor(uiSetting->hTopColor);
		dc.SetTextColor(uiSetting->fonthTopColor);
		CFont *oldfont =dc.SelectObject(&uiSetting->font_bold);
		dc.ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width, y+half), CString(parName), 0);
		dc.SelectObject(oldfont);

		DrawHLightValue(dc,x,y,width,parValue);
	}
	void InfoLabel::DrawHLightValue(CDC& dc, int x, int y, int width,const char *parValue)
	{
		const int height = uiSetting->dialheight;
		const int half = height/2;
		dc.SetBkColor(uiSetting->hBottomColor);
		dc.SetTextColor(uiSetting->fonthBottomColor);
		dc.ExtTextOut(x+xoffset, y+half,ETO_OPAQUE | ETO_CLIPPED, CRect(x, y+half, x+width, y+height), CString(parValue), 0);
	}
	void InfoLabel::DrawHeader(CDC& dc, int x, int y, int width,const char *parName)
	{
		const int height = uiSetting->dialheight;
		const int half = height/2;
		const int quarter = height/4;

		dc.FillSolidRect(x, y, width, quarter,uiSetting->topColor);
		dc.FillSolidRect(x, y+half+quarter, width, quarter,uiSetting->bottomColor);

		CFont *oldfont = dc.SelectObject(&uiSetting->font_bold);
		dc.SetBkColor(uiSetting->titleColor);
		dc.SetTextColor(uiSetting->fonttitleColor);
		dc.ExtTextOut(x + xoffset, y + quarter, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y + quarter, x+width, y+half+quarter), CString(parName), 0);
		dc.SelectObject(oldfont);
	}
	void InfoLabel::DrawHeader2(CDC& dc, int x, int y, int width, const char *parName,const char *parValue)
	{
		const int height = uiSetting->dialheight;
		const int half = height/2;

		dc.SetBkColor(uiSetting->titleColor);
		dc.SetTextColor(uiSetting->fonttitleColor);
		CFont *oldfont =dc.SelectObject(&uiSetting->font_bold);
		dc.ExtTextOut(x+xoffset, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y, x+width, y+half), CString(parName), 0);
		dc.SelectObject(oldfont);

		DrawValue(dc,x,y,width,parValue);
	}
	//////////////////////////////////////////////////////////////////////////
	// GraphSlider class
	void GraphSlider::Draw(CDC& dc, CDC& backDC, int x, int y)
	{
		const int width = uiSetting->sliderwidth;
		const int height = uiSetting->sliderheight;
		dc.BitBlt(x,y,width,height,&backDC,0,0,SRCCOPY);
	}
	void GraphSlider::DrawKnob(CDC& dc, CDC& knobDC, int x, int y, float value)
	{
		const int height = uiSetting->sliderheight;
		const int knobheight = uiSetting->sliderknobheight;
		const int knobwidth = uiSetting->sliderknobwidth;
		int ypos(0);
		if ( value < 0.375 ) ypos = (height-knobheight);
		else if ( value < 0.999) ypos = (((value-0.375f)*1.6f)-1.0f)*-1.0f*(height-knobheight);
		dc.BitBlt(x+xoffset,y+ypos,knobwidth,knobheight,&knobDC,0,0,SRCCOPY);
	}
	bool GraphSlider::LButtonDown( UINT nFlags,int x,int y)
	{
		if ( x< uiSetting->sliderwidth
			&& y < uiSetting->sliderheight) return true;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// SwitchButton class
	void SwitchButton::Draw(CDC& dc, CDC& switchDC, int x, int y)
	{
		int height = uiSetting->switchheight;
		int width = uiSetting->switchwidth;
		dc.BitBlt(x,y,width,height,&switchDC,0,0,SRCCOPY);
	}

	void CheckedButton::Draw(CDC& dc, CDC& switchDC, int x, int y,const char* text)
	{
		const int height = uiSetting->checkedheight;
		const int width = uiSetting->checkedwidth;

		dc.BitBlt(x,y+1,width,height,&switchDC,0,0,SRCCOPY);
		CFont *oldfont =dc.SelectObject(&uiSetting->font_bold);
		dc.ExtTextOut(x+width+1, y, ETO_OPAQUE | ETO_CLIPPED, CRect(x+width, y, x+width+width-1, y+height), CString(text), 0);
		dc.SelectObject(oldfont);
	}


	//////////////////////////////////////////////////////////////////////////
	// Vumeter class
	void VuMeter::DrawBack(CDC& dc, CDC& vuOff, int x, int y)
	{
		int height = uiSetting->vuheight;
		int width = uiSetting->vuwidth;
		dc.BitBlt(x,y,width,height,&vuOff,0,0,SRCCOPY);
	}
	void VuMeter::Draw(CDC& dc, CDC& vuOn, int x, int y, float value)
	{
		int height = uiSetting->vuheight;
		int width = uiSetting->vuwidth;
		int ypos = (1.f-value)*height;
		dc.BitBlt(x,y+ypos,width,height,&vuOn,0,ypos,SRCCOPY);
	}

	}   // namespace host
}   // namespace psycle
