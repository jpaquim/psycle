#pragma once
#include <psycle/host/resources/resources.hpp>
#include "plugin_interface.hpp"
#include <map>
//#include <afxwin.h>

enum ShowText
{
	off=0,
	top,
	right,
	bottom,
	left
};

class CMachineGuiParameter : public CMachineParameter
{
public:
	CMachineGuiParameter(CMachineParameter cmp, int _x, int _y): CMachineParameter(cmp), x(_x), y(_y) {}
	CMachineGuiParameter(CMachineParameter cmp, int _x, int _y, int _xSize, int _ySize): 
								CMachineParameter(cmp), x(_x), y(_y), xSize(_xSize), ySize(_ySize) {}

	virtual ~CMachineGuiParameter() {}


	virtual int WhatDoITweak(int* paramnums, int* vals,					//this function is called when the mouse is tweaking a bTweakable parameter.  it's 
							 int mousex, int mousey,					//each parameter's responsibility to interpret both axes of mouse movement, and
							 unsigned int flags) { return 0; }			//fill paramnums[] and vals[] with a list of parameters and values to be tweaked.
																		//most controls will only change themselves, and paramnums will be pre-initialized
																		//to reference the parameter being called, so you can usually leave it alone,
																		//and just set vals[0] to the value you want to be tweaked to.  the return value
																		//is the number of parameters needing tweaking-- usually 1 for knobs/sliders, 0
																		//for bitmaps/displays, and only >1 for special stuff.

	virtual void Paint(CDC* devc, char* valString) {}											// self-explanatory

	virtual bool PointInParam(int _x, int _y) const {return (_x>x && _x<x+xSize && _y>y && _y<y+ySize);  }
	virtual CPoint GetExtent() const {	return CPoint(x+xSize, y+ySize);	}	//bottom-right corner of the visible area of the parameter
																				//(used for sizing the window)

	virtual bool LButtonDown(int _x, int _y, int& val) {return false;}	//called for bClickable -or- bTweakable controls
	virtual bool LButtonUp(int _x, int _y, int& val) {return false;}	//if either returns true, the value of the parameter will be
																		//set to whatever you put in val.  for the moment, a click can only change itself,
																		//but it might be preferable for these to work like WhatDoITweak()
																		//(for example, clicking on a one-node xy grid should move the node to that point)

	virtual void ResetTweakSrc(CPoint newSrc) { tweakSrc=newSrc; tweakBase=*value;}


	bool bTweakable, bClickable;	//whether the control can be tweaked (knob,slider) or clicked (button,switch).
									//for the moment, if bTweakable is true, bClickable doesn't matter (it's assumed to be true as well)

	int* value;		//pointer to parameter's value
	int x, y;	//location within the gui window
	CBitmap* bmp;	// bitmap used to draw the control.  if you want to use your own bitmaps, this is what you should set.
	int xSize, ySize;	// size of the bitmap used.  if you set a different bitmap for a parameter, make sure you set
						// these as well

	CPoint tweakSrc;	//starting point for tweaks
	int tweakBase;


	ShowText showValue;	//if/where to display the value of the parameter
	ShowText showLabel; //if/where to display the label for the parameter

	char lblString[128];
	CFont valFont;
	CFont lblFont;
	COLORREF valTextColor, valBkColor;
	COLORREF lblTextColor, lblBkColor;

};

class CMachineKnob : public CMachineGuiParameter
{
public:
	CMachineKnob(CMachineParameter cmp, int _x, int _y, int _size=28) : CMachineGuiParameter(cmp, _x, _y, _size, _size)	
	{
		strcpy(lblString, cmp.Name);
		Init();
	}

	void Init()
	{
		bTweakable = true;
		bClickable = false;
		bReversed = false;
		bmp = new CBitmap;
		bmp->LoadBitmap(IDB_KNOB);
		srcXSize=28;
		srcYSize=28;

		showValue=bottom;
		showLabel=top;
		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
	}

	virtual ~CMachineKnob()	{bmp->DeleteObject(); delete bmp;}

	virtual bool LButtonDown(int _x, int _y, int& val) 
	{
		tweakSrc.x=_x;
		tweakSrc.y=_y;
		tweakBase=*value;
		return false;
	}
	virtual bool LButtonUp(int _x, int _y, int& val)
	{
		return false;
	}

	int WhatDoITweak(int* paramnums, int* vals, int mousex, int mousey, unsigned int flags)
	{
		int newval;
		float deltay= mousey-tweakSrc.y;
//		if(flags&MK_SHIFT) { deltay*=.5; }
//		if(flags&MK_CONTROL){deltay*=.5; }

		float pixel = (MaxValue-MinValue) / 192.0f;

		if(flags&MK_SHIFT) pixel=.5;
		if(flags&MK_CONTROL){deltay*=.2f;}

		if(!bReversed) deltay*=-1;

		newval = deltay*pixel + tweakBase;

		if(newval>MaxValue) newval=MaxValue;
		else if(newval<MinValue) newval=MinValue;
		vals[0]=newval;
		return 1;
	}

	void Paint(CDC* devc, char* valString)	
	{
		if(MaxValue>MinValue)
		{
			CDC memdc;
			CBitmap* oldbmp;
			memdc.CreateCompatibleDC(devc);
			oldbmp=memdc.SelectObject(bmp);
			if(bReversed)
//				devc->BitBlt(x, y, xSize, ySize, &memdc, (63-63*(*value-MinValue)/(MaxValue-MinValue))* xSize, 0, SRCCOPY);
				devc->StretchBlt(	x, y,	xSize, ySize, 
									&memdc, (63-63*(*value-MinValue)/(MaxValue-MinValue))*srcXSize, 0, 
									srcXSize,srcYSize, SRCCOPY);
			else
//				devc->BitBlt(x, y, xSize, ySize, &memdc,     63*(*value-MinValue)/(MaxValue-MinValue) * xSize, 0, SRCCOPY);
				devc->StretchBlt(	x,y,		xSize,ySize, 
									&memdc,     63*(*value-MinValue)/(MaxValue-MinValue)*srcXSize, 0, 
									srcXSize, srcYSize,		SRCCOPY);

			int textX=0, textY=0;
			
			devc->SelectObject(&valFont);
			CSize valTextSize = devc->GetTextExtent(valString);
			devc->SelectObject(&lblFont);
			CSize lblTextSize = devc->GetTextExtent(lblString);

			if(showValue!=off)
			{
				devc->SelectObject(&valFont);
				switch(showValue)
				{
				case top:
					textX = x + xSize/2 - valTextSize.cx/2;
					textY = y - 10 - valTextSize.cy/2;
					break;
				case bottom:
					textX = x + xSize/2 - valTextSize.cx/2;
					textY = y + ySize + 10 - valTextSize.cy/2;
					if(showLabel==bottom) textY+=lblTextSize.cy;
					break;
				case left:
					textX = x - 10 - valTextSize.cx;
					textY = y + ySize/2 - valTextSize.cy/2;
					if(showLabel==left) textY+=lblTextSize.cy/2;
					break;
				case right:
					textX = x + xSize + 10;
					textY = y + ySize/2 - valTextSize.cy/2;
					if(showLabel==right) textY+=lblTextSize.cy/2;
					break;
				}
			
				devc->SetBkColor(valBkColor);
				devc->SetTextColor(valTextColor);
				devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);
			}
			if(showLabel!=off)
			{
				devc->SelectObject(&lblFont);
				switch(showLabel)
				{
				case top:
					textX = x + xSize/2 - lblTextSize.cx/2;
					textY = y - 10 - lblTextSize.cy/2;
					if(showValue==top) textY-=valTextSize.cy;					
					break;
				case bottom:
					textX = x + xSize/2 - lblTextSize.cx/2;
					textY = y + ySize + 10 - lblTextSize.cy/2;
					break;
				case left:
					textX = x - 10 - lblTextSize.cx;
					textY = y + ySize/2 - lblTextSize.cy/2;
					if(showValue==left) textY-=lblTextSize.cy/2;
					break;
				case right:
					textX = x + xSize + 10;
					textY = y + ySize/2 - lblTextSize.cy/2;
					if(showValue==right) textY-=lblTextSize.cy/2;
					break;
				}
				devc->SetBkColor(lblBkColor);
				devc->SetTextColor(lblTextColor);
				devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), lblString, 0);
			}
			memdc.SelectObject(oldbmp);
			memdc.DeleteDC();
		}
	}

	bool bReversed;
	int srcXSize, srcYSize;
};

/*class CMachineBigKnob : public CMachineKnob
{
public:
	CMachineBigKnob(CMachineParameter cmp, int _x, int _y)	: CMachineKnob(cmp, _x, _y)
	{
		strcpy(lblString, cmp.Name);
		xSize=42;
		ySize=42;
	}

	~CMachineBigKnob() {}


};

class CMachineSmallKnob : public CMachineKnob
{
public:
	CMachineSmallKnob(CMachineParameter cmp, int _x, int _y) : CMachineKnob(cmp, _x, _y)
	{
		strcpy(lblString, cmp.Name);
		xSize=21;
		ySize=21;
	}

	~CMachineSmallKnob() {}

};
*/

class CMachineVSlider : public CMachineGuiParameter
{
public:
	CMachineVSlider(CMachineParameter cmp, int _x, int _y, int _length=127) : CMachineGuiParameter(cmp, _x, _y), length(_length)
	{
		strcpy(lblString, cmp.Name);
		Init();
	}

	void Init()
	{
		bTweakable = true;
		bClickable = true;
		bReversed = false;
		bmp= new CBitmap;
		bmp->LoadBitmap(IDB_SLIDERKNOBV);
		xSize=15;
		ySize=22;
		sliderback.LoadBitmap(IDB_SLIDERBACKV);

		showValue=bottom;
		showLabel=top;
		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
	}
		
	~CMachineVSlider()	{bmp->DeleteObject(); delete bmp;}

	int WhatDoITweak(int* paramnums, int* vals, int mousex, int mousey, unsigned int flags)
	{
		int newval;
		float deltay= mousey-tweakSrc.y;
//		if(flags&MK_SHIFT) { deltay*=.5; }
//		if(flags&MK_CONTROL){deltay*=.5; }

		float pixel = (MaxValue-MinValue) / (float)(length==0?1:length);

		if(flags&MK_SHIFT) pixel=.5;
		if(flags&MK_CONTROL){deltay*=.2f;}

		if(!bReversed)
			deltay*=-1;

//		if(bReversed)
//			newval = pixel*(newpos) + MinValue;
//		else
//			newval = pixel*(length-1-newpos) + MinValue;

		newval = deltay*pixel + tweakBase;

		if(newval>MaxValue) newval=MaxValue;
		else if(newval<MinValue) newval=MinValue;
		vals[0]=newval;
		return 1;
	}

	bool PointInParam(int _x, int _y) const
	{
		return (_x>x && _x<x+xSize && _y>y && _y<y+ySize+length);  
	}
	virtual CPoint GetExtent() const
	{
		return CPoint(x+xSize, y+ySize+length);
	}


	virtual bool LButtonDown(int _x, int _y, int& val) 
	{
		if(length==0) return false;

		float valtemp;
		if(bReversed)
			valtemp = (_y-y-(ySize/2)) / (float)length;
		else
			valtemp= (length-(_y-y-(ySize/2))) / (float)length;

		if (valtemp<0) valtemp=0;
		else if (valtemp>1) valtemp=1;

		valtemp *= MaxValue-MinValue;
		valtemp += MinValue;

		val = int(valtemp);
		tweakSrc.x=_x;
		tweakSrc.y=_y;
		tweakBase=val;

		return true;
	}	

	virtual bool LButtonUp(int _x, int _y, int& val)	{return false;}	  //since the sliders are tweakable as well as clickable, anything we'd
																		  //set here would already have been set by WhatDoITweak().


	void Paint(CDC* devc, char* valString)
	{
		if(MaxValue>MinValue)
		{
			CDC memdc;
			CBitmap* oldbmp;
			memdc.CreateCompatibleDC(devc);

			oldbmp=memdc.SelectObject(&sliderback);
			devc->StretchBlt(x-3, y, 21, length+ySize, &memdc, 0, 0, 20, 128, SRCCOPY);
			
			memdc.SelectObject(bmp);
			if(bReversed)
				devc->BitBlt(x, y+length*(*value-MinValue)/(MaxValue-MinValue), xSize, ySize, &memdc, 0, 0, SRCCOPY);
			else
				devc->BitBlt(x, y+length-(length*(*value-MinValue)/(MaxValue-MinValue)), xSize, ySize, &memdc, 0, 0, SRCCOPY);
			int textX=0, textY=0;
			devc->SelectObject(&valFont);
			CSize valTextSize = devc->GetTextExtent(valString);
			devc->SelectObject(&lblFont);
			CSize lblTextSize = devc->GetTextExtent(lblString);

			if(showValue!=off)
			{
				devc->SelectObject(&valFont);
				switch(showValue)
				{
				case top:
					textX = x + xSize/2 - valTextSize.cx/2;
					textY = y - 10 - valTextSize.cy/2;
					break;
				case bottom:
					textX = x + xSize/2 - valTextSize.cx/2;
					textY = y + ySize+length + 10 - valTextSize.cy/2;
					if(showLabel==bottom) textY+=lblTextSize.cy;
					break;
				case left:
					textX = x - 10 - valTextSize.cx;
					textY = y + (ySize+length)/2 - valTextSize.cy/2;
					if(showLabel==left) textY+=lblTextSize.cy/2;
					break;
				case right:
					textX = x + xSize + 10;
					textY = y + (ySize+length)/2 - valTextSize.cy/2;
					if(showLabel==right) textY+=lblTextSize.cy/2;
					break;
				}
			
				devc->SetBkColor(valBkColor);
				devc->SetTextColor(valTextColor);
				devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);

			}
			if(showLabel!=off)
			{
				devc->SelectObject(&lblFont);
				switch(showLabel)
				{
				case top:
					textX = x + xSize/2 - lblTextSize.cx/2;
					textY = y - 10 - lblTextSize.cy/2;
					if(showValue==top) textY-=valTextSize.cy;					
					break;
				case bottom:
					textX = x + xSize/2 - lblTextSize.cx/2;
					textY = y + ySize+length + 10 - lblTextSize.cy/2;
					break;
				case left:
					textX = x - 10 - lblTextSize.cx;
					textY = y + (ySize+length)/2 - lblTextSize.cy/2;
					if(showValue==left) textY-=lblTextSize.cy/2;
					break;
				case right:
					textX = x + xSize + 10;
					textY = y + (ySize+length)/2 - lblTextSize.cy/2;
					if(showValue==right) textY-=lblTextSize.cy/2;
					break;
				}
				devc->SetBkColor(lblBkColor);
				devc->SetTextColor(lblTextColor);
				devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), lblString, 0);
			}


			memdc.SelectObject(oldbmp);
			memdc.DeleteDC();

		}
	}							

	CBitmap sliderback;
	bool bReversed;
	int length;
};

class CMachineHSlider : public CMachineGuiParameter
{
public:
	CMachineHSlider(CMachineParameter cmp, int _x, int _y, int _length=127) : CMachineGuiParameter(cmp, _x, _y), length(_length)
	{
		strcpy(lblString, cmp.Name);
		Init();
	}

	void Init()
	{
		bTweakable = true;
		bClickable = true;
		bReversed = false;
		bmp = new CBitmap;
		bmp->LoadBitmap(IDB_SLIDERKNOBH);
		xSize=22;
		ySize=15;
		sliderback.LoadBitmap(IDB_SLIDERBACKH);

		showValue=bottom;
		showLabel=top;
		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
	}

	~CMachineHSlider()	{bmp->DeleteObject(); delete bmp;}

	int WhatDoITweak(int* paramnums, int* vals, int mousex, int mousey, unsigned int flags)
	{
		int newval;
		float deltax= mousex-tweakSrc.x;
//		if(flags&MK_SHIFT) { deltax*=.5; }
//		if(flags&MK_CONTROL){deltax*=.5; }

		float pixel = (MaxValue-MinValue) / (float)(length==0?1:length);

		if(flags&MK_SHIFT) pixel=.5;
		if(flags&MK_CONTROL){deltax*=.2f;}

		if(bReversed)
			deltax*=-1;

		newval = deltax*pixel + tweakBase;

		if(newval>MaxValue) newval=MaxValue;
		else if(newval<MinValue) newval=MinValue;
		vals[0]=newval;
		return 1;
	}

	bool PointInParam(int _x, int _y) const
	{
		return (_x>x && _x<x+xSize+length && _y>y && _y<y+ySize);  
	}
	virtual CPoint GetExtent() const
	{
		return CPoint(x+xSize+length, y+ySize);
	}

	virtual bool LButtonDown(int _x, int _y, int& val) 
	{
		if(length==0) return false;

		float valtemp;
		if(bReversed)
			valtemp = (length-(_x-x-(xSize/2))) / (float)length;
		else
			valtemp = (_x-x-(xSize/2)) / (float)length;

		if (valtemp<0) valtemp=0;
		else if (valtemp>1) valtemp=1;

		valtemp *= MaxValue-MinValue;
		valtemp += MinValue;

		val = int(valtemp);

		tweakSrc.x=_x;
		tweakSrc.y=_y;
		tweakBase=val;

		return true;
	}	

	virtual bool LButtonUp(int _x, int _y, int& val)	{return false;}


	void Paint(CDC* devc, char* valString)
	{
		if(MaxValue>MinValue)
		{
			CDC memdc;
			CBitmap* oldbmp;
			memdc.CreateCompatibleDC(devc);
			oldbmp=memdc.SelectObject(&sliderback);
			devc->StretchBlt(x, y-3, length+xSize, 21, &memdc, 0, 0, 128, 20, SRCCOPY);

			memdc.SelectObject(bmp);
			if(bReversed)
				devc->BitBlt(x+length-(length*(*value-MinValue)/(MaxValue-MinValue)), y, xSize, ySize, &memdc, 0, 0, SRCCOPY);
			else
				devc->BitBlt(x+(length*(*value-MinValue)/(MaxValue-MinValue)), y, xSize, ySize, &memdc, 0, 0, SRCCOPY);

			int textX=0, textY=0;
			devc->SelectObject(&valFont);
			CSize valTextSize = devc->GetTextExtent(valString);
			devc->SelectObject(&lblFont);
			CSize lblTextSize = devc->GetTextExtent(lblString);

			if(showValue!=off)
			{
				devc->SelectObject(&valFont);
				switch(showValue)
				{
				case top:
					textX = x + (xSize+length)/2 - valTextSize.cx/2;
					textY = y - 10 - valTextSize.cy/2;
					break;
				case bottom:
					textX = x + (xSize+length)/2 - valTextSize.cx/2;
					textY = y + ySize + 10 - valTextSize.cy/2;
					if(showLabel==bottom) textY+=valTextSize.cy;
					break;
				case left:
					textX = x - 10 - valTextSize.cx;
					textY = y + ySize/2 - valTextSize.cy/2;
					if(showLabel==left) textY+=lblTextSize.cy;
					break;
				case right:
					textX = x + xSize+length + 10;
					textY = y + ySize/2 - valTextSize.cy/2;
					if(showLabel==right) textY+=lblTextSize.cy;
					break;
				}
			
				devc->SetBkColor(valBkColor);
				devc->SetTextColor(valTextColor);
				devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);

			}
			if(showLabel!=off)
			{
				devc->SelectObject(&lblFont);
				switch(showLabel)
				{
				case top:
					textX = x + (xSize+length)/2 - lblTextSize.cx/2;
					textY = y - 10 - lblTextSize.cy/2;
					if(showValue==top) textY-=valTextSize.cy;					
					break;
				case bottom:
					textX = x + (xSize+length)/2 - lblTextSize.cx/2;
					textY = y + 10 - lblTextSize.cy/2;
					break;
				case left:
					textX = x - 10 - lblTextSize.cx;
					textY = y + ySize/2 - lblTextSize.cy/2;
					if(showValue==left) textY-=lblTextSize.cy/2;
					break;
				case right:
					textX = x + xSize+length + 10;
					textY = y + ySize/2 - lblTextSize.cy/2;
					if(showValue==right) textY-=lblTextSize.cy/2;
					break;
				}
				devc->SetBkColor(lblBkColor);
				devc->SetTextColor(lblTextColor);
				devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), lblString, 0);
			}

			memdc.SelectObject(oldbmp);
			memdc.DeleteDC();

		}
	}

	int length;
	CBitmap sliderback;
	bool bReversed;
};

class CMachineButton : public CMachineGuiParameter
{
public:
	CMachineButton(CMachineParameter cmp, int _x, int _y) : CMachineGuiParameter(cmp, _x, _y)
	{
		strcpy(lblString, cmp.Name);
		Init();
	}

	void Init()
	{
		bClickable = true;
		bTweakable = false;
		bReversed = false;
		bmp = new CBitmap;
		bmp->LoadBitmap(IDB_BUTTON);
		xSize = 39;
		ySize = 19;
		bButtonDown=false;

		showValue=bottom;
		showLabel=top;
		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
	}

	~CMachineButton()
	{
		bmp->DeleteObject(); delete bmp;
	}


	bool LButtonDown(int _x, int _y, int& val)
	{
		bButtonDown=true;
		return 0;
	}
	bool LButtonUp(int _x, int _y, int& val)
	{
		bButtonDown=false;
		if( this->PointInParam(_x, _y) )		// in case the lbutton is lifted outside of the control
		{
			if(*value==MinValue)
				val = MaxValue;
			else
				val = MinValue;
			return 1;
		}
		return 0;
	}

	void Paint(CDC *devc, char* valString)
	{
		CDC memdc;
		CBitmap* oldbmp;
		memdc.CreateCompatibleDC(devc);
		oldbmp=memdc.SelectObject(bmp);
		if(bReversed)
			devc->BitBlt(x, y, xSize, ySize, &memdc, (*value==MinValue? xSize : 0)+(bButtonDown? xSize*2 : 0), 0, SRCCOPY);
		else
			devc->BitBlt(x, y, xSize, ySize, &memdc, (*value==MinValue? 0 : xSize)+(bButtonDown? xSize*2 : 0), 0, SRCCOPY);
		//there should be four buttons in the image: on and up, off and up, on and down, off and down.

		int textX=0, textY=0;
		devc->SelectObject(&valFont);
		CSize valTextSize = devc->GetTextExtent(valString);
		devc->SelectObject(&lblFont);
		CSize lblTextSize = devc->GetTextExtent(lblString);

		if(showValue!=off)
		{
			devc->SelectObject(&valFont);
			switch(showValue)
			{
			case top:
				textX = x + xSize/2 - valTextSize.cx/2;
				textY = y - 10 - valTextSize.cy/2;
				break;
			case bottom:
				textX = x + xSize/2 - valTextSize.cx/2;
				textY = y + ySize + 10 - valTextSize.cy/2;
				if(showLabel==bottom) textY+=lblTextSize.cy;
				break;
			case left:
				textX = x - 10 - valTextSize.cx;
				textY = y + ySize/2 - valTextSize.cy/2;
				if(showLabel==left) textY+=lblTextSize.cy/2;
				break;
			case right:
				textX = x + xSize + 10;
				textY = y + ySize/2 - valTextSize.cy/2;
				if(showLabel==right) textY+=lblTextSize.cy/2;
				break;
			}
		
			devc->SetBkColor(valBkColor);
			devc->SetTextColor(valTextColor);
			devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);
		}
		if(showLabel!=off)
		{
			devc->SelectObject(&lblFont);
			switch(showLabel)
			{
			case top:
				textX = x + xSize/2 - lblTextSize.cx/2;
				textY = y - 10 - lblTextSize.cy/2;
				if(showValue==top) textY-=valTextSize.cy;					
				break;
			case bottom:
				textX = x + xSize/2 - lblTextSize.cx/2;
				textY = y + ySize + 10 - lblTextSize.cy/2;
				break;
			case left:
				textX = x - 10 - lblTextSize.cx;
				textY = y + ySize/2 - lblTextSize.cy/2;
				if(showValue==left) textY-=lblTextSize.cy/2;
				break;
			case right:
				textX = x + xSize + 10;
				textY = y + ySize/2 - lblTextSize.cy/2;
				if(showValue==right) textY-=lblTextSize.cy/2;
				break;
			}
			devc->SetBkColor(lblBkColor);
			devc->SetTextColor(lblTextColor);
			devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), lblString, 0);
		}

		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();
	}

	bool bReversed;
	bool bButtonDown;

};

class CMachineVSwitchBank : public CMachineGuiParameter
{
public:
	CMachineVSwitchBank(CMachineParameter cmp, int _x, int _y, int _num) : CMachineGuiParameter(cmp, _x, _y), num(_num)
	{
		strcpy(lblString, cmp.Name);
		Init();
	}

	void Init()
	{
		bTweakable=false;
		bClickable=true;
		bReversed = false;
		bmp=new CBitmap;
		bmp->LoadBitmap(IDB_BUTTON);
		xSize=39;
		ySize=19;
		ySpace=4;
		buttonDown=MinValue-1;

		showValue=bottom;
		showLabel=top;
		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
	}

	~CMachineVSwitchBank()
	{
		bmp->DeleteObject(); delete bmp;
	}

	bool PointInParam(int _x, int _y) const
	{
		return	  (_x>x		&& _x < x+xSize 
				&& _y>y		&& _y < y + (ySize*num) + (ySpace * (num-1)) );  
	}
	virtual CPoint GetExtent() const
	{
		return CPoint(x+xSize, y+(ySize*num) + (ySpace * (num-1)));
	}


	bool LButtonDown(int _x, int _y, int& val) 
	{
		int hit = (_y-y)%(ySize+ySpace);
		int which = (_y-y)/(ySize+ySpace);
		if(hit<=ySize)	//if we didn't click on a crack..
			buttonDown = MinValue + which;
		return false;
	}

	bool LButtonUp(int _x, int _y, int& val) 
	{
		int hit = (_y-y)%(ySize+ySpace);
		int which = (_y-y)/(ySize+ySpace);
		if(hit<=ySize && MinValue+which==buttonDown)
		{
			val=buttonDown;
			buttonDown=MinValue-1;
			return true;
		}

		buttonDown=MinValue-1;
		return false;
	}


	void Paint(CDC* devc, char* valString)
	{
		CDC memdc;
		memdc.CreateCompatibleDC(devc);
		CBitmap* oldbmp;
		oldbmp=memdc.SelectObject(bmp);

		for(int i(MinValue);i<MinValue+num;++i)
		{
			if(i<=MaxValue)
			{
				if(*value==i)
					devc->BitBlt(x, y + i*(ySpace+ySize), xSize, ySize, &memdc, (buttonDown==i ? 3*xSize : xSize), 0, SRCCOPY);
				else
					devc->BitBlt(x, y + i*(ySpace+ySize), xSize, ySize, &memdc, (buttonDown==i ? 2*xSize : 0), 0, SRCCOPY);
			}
		}

		int textX=0, textY=0;
		devc->SelectObject(&valFont);
		CSize valTextSize = devc->GetTextExtent(valString);
		devc->SelectObject(&lblFont);
		CSize lblTextSize = devc->GetTextExtent(lblString);

		if(showValue!=off)
		{
			devc->SelectObject(&valFont);
			switch(showValue)
			{
			case top:
				textX = x + xSize/2 - valTextSize.cx/2;
				textY = y - 10 - valTextSize.cy/2;
				break;
			case bottom:
				textX = x + xSize/2 - valTextSize.cx/2;
				textY = y + ySize*num + ySpace*(num-1) + 10 - valTextSize.cy/2;
				if(showLabel==bottom) textY+=lblTextSize.cy;
				break;
			case left:
				textX = x - 10 - valTextSize.cx;
				textY = y + (ySize*num+ySpace*(num-1))/2 - valTextSize.cy/2;
				if(showLabel==left) textY+=lblTextSize.cy/2;
				break;
			case right:
				textX = x + xSize + 10;
				textY = y + (ySize*num+ySpace*(num-1))/2 - valTextSize.cy/2;
				if(showLabel==right) textY+=lblTextSize.cy/2;
				break;
			}
		
			devc->SetBkColor(valBkColor);
			devc->SetTextColor(valTextColor);
			devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);

		}
		if(showLabel!=off)
		{
			devc->SelectObject(&lblFont);
			switch(showLabel)
			{
			case top:
				textX = x + xSize/2 - lblTextSize.cx/2;
				textY = y - 10 - lblTextSize.cy/2;
				if(showValue==top) textY-=valTextSize.cy;					
				break;
			case bottom:
				textX = x + xSize/2 - lblTextSize.cx/2;
				textY = y + ySize*num + ySpace*(num-1) + 10 - lblTextSize.cy/2;
				break;
			case left:
				textX = x - 10 - lblTextSize.cx;
				textY = y + (ySize*num+ySpace*(num-1))/2 - lblTextSize.cy/2;
				if(showValue==left) textY-=lblTextSize.cy/2;
				break;
			case right:
				textX = x + xSize + 10;
				textY = y + (ySize*num+ySpace*(num-1))/2 - lblTextSize.cy/2;
				if(showValue==right) textY-=lblTextSize.cy/2;
				break;
			}
			devc->SetBkColor(lblBkColor);
			devc->SetTextColor(lblTextColor);
			devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), lblString, 0);
		}

		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();

	}//end Paint()

	int num;
	int ySpace;
	int buttonDown;	
	bool bReversed;		//does nothing here atm!
};

class CMachineBitmap : public CMachineGuiParameter
{
public:
	CMachineBitmap(CMachineParameter cmp, int _x, int _y, CBitmap* _bmp, int _xSize, int _ySize) : CMachineGuiParameter(cmp, _x, _y, _xSize, _ySize)
	{
		bTweakable=false;
		bClickable=false;
		bmp=_bmp;
	}
	virtual bool PointInParam(int _x, int _y) const {return false;}

	virtual int WhatDoITweak(int* paramnums, int* vals, int mousex, int mousey, unsigned int flags)		{return 0;}

	virtual void Paint(CDC* devc, char* valString)
	{
		CDC memdc;
		CBitmap* oldbmp;
		memdc.CreateCompatibleDC(devc);
		oldbmp = memdc.SelectObject(bmp);
		devc->BitBlt(x, y, xSize, ySize, &memdc, 0, 0, SRCCOPY);
		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();

	}
};

class CMachineBackground : public CMachineBitmap
{
public:
	CMachineBackground(CMachineParameter cmp, CBitmap* _bmp, int _xSize, int _ySize) : CMachineBitmap(cmp, 0, 0, _bmp, _xSize, _ySize)
	{
		bTweakable=false;
		bClickable=false;
	}

	void Paint(CDC* devc, char* valString)
	{
		CDC memdc;
		memdc.CreateCompatibleDC(devc);

		CBitmap* oldbmp;

		oldbmp = memdc.SelectObject(bmp);
		devc->BitBlt(0, 0, xSize, ySize, &memdc, 0, 0, SRCCOPY);


		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();

	}
};
	
class CMachineDisplay : public CMachineGuiParameter
{
public:
	CMachineDisplay(CMachineParameter cmp, int _x, int _y, int _xSize, int _ySize) : CMachineGuiParameter(cmp, _x, _y, _xSize, _ySize)
	{
		bTweakable = false;
		bClickable = false;
		displaydata.resize((_xSize+1)*(_ySize+1));
	}
	virtual bool PointInParam(int _x, int _y) const {return false;  }


	void Paint(CDC* devc, char* valString)
	{
		CBitmap* bmp = new CBitmap;
		CBitmap* oldbmp;
		bmp->CreateCompatibleBitmap(devc, xSize, ySize);
		CDC memdc;
		memdc.CreateCompatibleDC(devc);
		oldbmp = memdc.SelectObject(bmp);

		for(int i(0);i<xSize;++i)
		{
			for(int j(0);j<ySize;++j)
			{
				memdc.SetPixel(CPoint(i, j), displaydata[j*xSize+i]);
			}
		}

		devc->BitBlt(x, y, xSize, ySize, &memdc, 0, 0, SRCCOPY);

		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();
		bmp->DeleteObject(); delete bmp;
	}

	void ClearData(COLORREF color=0x00000000)
	{
		for(int i(0);i<xSize;++i)
			for(int j(0);j<ySize;++j)
				displaydata[j*xSize+i]=color;
	}

	void PutPixel(CPoint putat, COLORREF color=0x00EEEEEE)
	{
		if(putat.x>=0 && putat.x<xSize && putat.y>=0 && putat.y<ySize)
			displaydata[putat.y*xSize+putat.x] = color;
	}

	void Line(CPoint start, CPoint end, COLORREF color, int width)
	{
		if(width>1)
		{
			start = start - CPoint(width/2, width/2) + CPoint(1,1);
			end   = end   - CPoint(width/2, width/2) + CPoint(1,1);
		}
		
		for(int x(0);x<width;++x)
			for(int y(0);y<width;++y)
				Line(start+CPoint(x,y), end+CPoint(x,y), color);
	}

	void Line(CPoint start, CPoint end, COLORREF color=0x00EEEEEE)
	{
		if(end.x<start.x)
		{
			CPoint temp;
			temp=end;
			end=start;
			start=temp;
		}
		if(end.y==start.y)
		{
			for(int i=start.x;i<end.x;++i)
				PutPixel(CPoint(i,start.y), color);
		}
		if(end.x==start.x)
		{
			if(end.y<start.y)
			{
				CPoint temp;
				temp=end;
				end=start;
				start=temp;
			}
			for(int i=start.y;i<end.y;++i)
				PutPixel(CPoint(start.x, i), color);
		}
		int deltaX = end.x-start.x;
		int deltaY = end.y-start.y;
		float slope = deltaY/(float)(deltaX==0?0.01:deltaX);
		if(slope<1&& slope>-1)
		{
			int y;
			for(int i(start.x);i<end.x;++i)
			{
				y=start.y+((i-start.x)*slope);
				PutPixel(CPoint(i, y), color);
			}
		}
		else
		{
			if(end.y<start.y)
			{
				CPoint temp;
				temp=end;
				end=start;
				start=temp;
			}
			slope = 1/slope;
			int x;
			for(int i(start.y);i<end.y;++i)
			{
				x=start.x+((i-start.y)*slope);
				PutPixel(CPoint(x, i), color);
			}
		}
	}

protected:
	std::vector<COLORREF> displaydata;
};

class CMachineInvisible : public CMachineGuiParameter
{
public:
	CMachineInvisible(CMachineParameter cmp)	:	CMachineGuiParameter(cmp, 0, 0, 0, 0)
	{
		bClickable = false;
		bTweakable = false;
	}

	bool PointInParam(int _x, int _y)		{return false;}

	void Paint(CDC *devc, char *valString)	{return;}
};


class CMachineComboBox : public CMachineGuiParameter
{
public:
	CMachineComboBox(CMachineParameter cmp, int _x, int _y, int _length) : CMachineGuiParameter(cmp, _x, _y), length(_length)
	{
		bClickable=true;
		bTweakable=false;
		bReversed=false;
		bmp=new CBitmap;
		bmp->LoadBitmap(IDB_COMBOBOX);
		xSize=18;
		ySize=18;

		strcpy(lblString, cmp.Name);

		showValue=top;		//as long as it's not 'off', this value doesn't matter
		showLabel=top;
		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=0x00000000;
		lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
	}
	~CMachineComboBox()
	{
		bmp->DeleteObject();
		delete bmp;
	}

	bool PointInParam(int _x, int _y) const
	{
		return (_x>x && _x<x+xSize*2+length && _y>y && _y<y+ySize);  
	}
	virtual CPoint GetExtent() const
	{
		return CPoint(x+xSize*2+length, y+ySize);
	}


	bool LButtonDown(int _x, int _y, int &val)
	{
		if(_x>x && _x<x+xSize)
			val=*value-1;
		else if(_x>x+length+xSize && _x<x+length+xSize*2)
			val=*value+1;
		else
			return false;

		if(val>MaxValue)	val=MaxValue;
		else if(val<MinValue)val=MinValue;

		return true;
	}

	bool LButtonUp(int _x, int _y, int &val)
	{
		return false;
	}

	void Paint(CDC *devc, char *valString)
	{
		CDC memdc;
		memdc.CreateCompatibleDC(devc);
		CBitmap* oldbmp;
		oldbmp=memdc.SelectObject(bmp);
	
		devc->BitBlt(x, y, xSize, ySize, &memdc, 0+(*value==MinValue ? xSize*2 : 0), 0, SRCCOPY);
		devc->BitBlt(x+length+xSize, y, xSize, ySize, &memdc, xSize+(*value==MaxValue ? xSize*2 : 0), 0, SRCCOPY);

		devc->BitBlt(x+xSize, y, 1, ySize, &memdc, xSize*4, 0, SRCCOPY);
		devc->BitBlt(x+xSize+length-1, y, 1, ySize, &memdc, xSize*4+2, 0, SRCCOPY);
		for(int i(xSize+1);i<length+xSize-1;++i)
			devc->BitBlt(x+i, y, 1, ySize, &memdc, xSize*4+1, 0, SRCCOPY);

		int textX=0, textY=0;
		devc->SelectObject(&valFont);
		CSize valTextSize = devc->GetTextExtent(valString);

		if(showValue!=off)
		{
			
			textX=x+length/2+xSize  - valTextSize.cx/2;
			textY=y+ySize/2		  - valTextSize.cy/2;

			devc->SetBkColor(devc->GetPixel(x+xSize+1, y+ySize/2));
			devc->SetTextColor(valTextColor);
			devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), valString, 0);

		}
		if(showLabel!=off)
		{
			devc->SelectObject(&lblFont);
			CSize lblTextSize = devc->GetTextExtent(lblString);

			switch(showLabel)
			{
			case top:
				textX = x + length/2+xSize - lblTextSize.cx/2;
				textY = y - 10 - lblTextSize.cy/2;
				break;
			case bottom:
				textX = x + length/2+xSize - lblTextSize.cx/2;
				textY = y + ySize + 10 - lblTextSize.cy/2;
				break;
			case left:
				textX = x - 10 - lblTextSize.cx;
				textY = y + ySize/2 - lblTextSize.cy/2;
				break;
			case right:
				textX = x + xSize + 10;
				textY = y + ySize/2 - lblTextSize.cy/2;
				break;
			}
			devc->SetBkColor(lblBkColor);
			devc->SetTextColor(lblTextColor);
			devc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), lblString, 0);
		}

		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();

	}

	bool bReversed;
	int length;
};



//////////////////////////////////////////////////////////////////////////////////////////////////////
/////						class GridNode
/////


class GridNode
{
public:
	GridNode() 
	{
		x=0; y=0;
		xSize=9; ySize=9;
		data.resize((xSize+1) * (ySize+1));
		bActive = true;
	}

	GridNode(int _xSize, int _ySize) : xSize(_xSize), ySize(_ySize)
	{
		x=y=0;
		data.resize((xSize+1)*(ySize+1));
		bActive = true;
	}

	bool PointInNode (int xPoint, int yPoint, int maxDistance=1)
	{
		return (	xPoint>x-xSize/2-maxDistance && xPoint<x+xSize/2+maxDistance
				 && yPoint>y-ySize/2-maxDistance && yPoint<y+ySize/2+maxDistance);
	}

	void SetMax(CPoint maxPoint)	{xMax=maxPoint.x;		yMax=maxPoint.y;}
	void SetMaxX(int _xMax)			{xMax=_xMax;}
	void SetMaxY(int _yMax)			{yMax=_yMax;}
	void SetMin(CPoint minPoint)	{xMin=minPoint.x;		yMin=minPoint.y;}
	void SetMinX(int _xMin)			{xMin=_xMin;}
	void SetMinY(int _yMin)			{yMin=_yMin;}

	int xSize, ySize;
	int x, y;
	bool bActive;
	int xMin, xMax;
	int yMin, yMax;
	std::vector<COLORREF>data;
};

class CMachineXYGrid : public CMachineDisplay
{
public:
	CMachineXYGrid(	CMachineParameter cmp,
					CMachineGuiParameter* _xParam, 
					CMachineGuiParameter* _yParam,
					int _x, int _y, int _xSize, int _ySize) 
						: CMachineDisplay(cmp, _x, _y, _xSize, _ySize),
						  xParam(_xParam), yParam(_yParam)
	{
		bClickable = true;
		bTweakable = true;

		showValue=off;
		showLabel=off;
		strcpy(lblString, "");

		valFont.CreatePointFont(80,"Tahoma");
		lblFont.CreatePointFont(80,"Tahoma");
		valTextColor=lblTextColor=0x00EEEEEE;
		valBkColor=lblBkColor=0x00000000;
		for (int i(0);i<node.xSize;++i)
			for(int j(0);j<node.ySize;++j)
				node.data[j*node.xSize+i]=0x00EEEEEE;
	}

	virtual bool PointInParam(int _x, int _y) const {return (_x>x && _x<x+xSize && _y>y && _y<y+ySize);  }

	virtual int WhatDoITweak(int* paramnums, int* vals,	int mousex, int mousey, unsigned int flags)
	{
		float deltax= mousex-tweakSrc.x;
		float deltay= mousey-tweakSrc.y;
//		if(flags&MK_SHIFT)	 {deltax*=.5; deltay*=.5;}
//		if(flags&MK_CONTROL) {deltax*=.5; deltay*=.5;}

		int newvalx, newvaly;
		float pixelx = (xParam->MaxValue-xParam->MinValue) / (float)(xSize==0?1:xSize);
		float pixely = (yParam->MaxValue-yParam->MinValue) / (float)(ySize==0?1:ySize);

		if(flags&MK_SHIFT) {pixelx=.5; pixely=.5;}
		if(flags&MK_CONTROL){deltay*=.2f; deltax*=.2f;}

		newvalx=(deltax) *  pixelx + tweakBase_x;
		newvaly=(deltay) * -pixely + tweakBase_y;		//y direction should increase as it gets higher, not lower

		if(newvalx>xParam->MaxValue) newvalx=xParam->MaxValue;
		else if(newvalx<xParam->MinValue) newvalx=xParam->MinValue;
		vals[0]=newvalx;
		paramnums[0]=xParamNum;

		if(newvaly>yParam->MaxValue) newvaly=yParam->MaxValue;
		else if(newvaly<yParam->MinValue) newvaly=yParam->MinValue;
		vals[1]=newvaly;
		paramnums[1]=yParamNum;
	
		return 2;
	}

	void ResetTweakSrc(CPoint newSrc)
	{
		tweakSrc=newSrc;
		tweakBase_x=*(xParam->value);
		tweakBase_y=*(yParam->value);
	}

	bool LButtonDown(int _x, int _y, int& val) 
	{
		tweakSrc.x=_x;
		tweakSrc.y=_y;
		tweakBase_x=*(xParam->value);
		tweakBase_y=*(yParam->value);
		return false;
	}
	bool LButtonUp(int _x, int _y, int& val) 
	{
		return false;
	}

	void Paint(CDC *devc, char *valString)
	{
		CBitmap* bmp = new CBitmap;
		CBitmap* oldbmp;
		bmp->CreateCompatibleBitmap(devc, xSize, ySize);
		CDC memdc;
		memdc.CreateCompatibleDC(devc);
		oldbmp = memdc.SelectObject(bmp);

		for(int i(0);i<xSize;++i)
			for(int j(0);j<ySize;++j)
				memdc.SetPixel(CPoint(i, j), displaydata[j*xSize+i]);

		node.x=   (*(xParam->value)-xParam->MinValue) / float(xParam->MaxValue-xParam->MinValue)  * xSize;
		node.y=(1-(*(yParam->value)-yParam->MinValue) / float(yParam->MaxValue-yParam->MinValue)) * ySize;
		int xp, yp;
		int xbase = node.x-node.xSize/2;
		int ybase = node.y-node.ySize/2;
		for(int i(0);i<node.xSize;++i)
			for(int j(0);j<node.ySize;++j)
			{
				xp=xbase+i;
				yp=ybase+j;
				if(xp>=0 && xp<xSize && yp>=0 && yp<ySize)
					memdc.SetPixel(CPoint(xp, yp), node.data[j*node.xSize+i]);
			}

		devc->BitBlt(x, y, xSize, ySize, &memdc, 0, 0, SRCCOPY);

		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();
		bmp->DeleteObject(); delete bmp;
	}

	GridNode node;
	int xParamNum;
	int yParamNum;
	int tweakBase_x, tweakBase_y;

	CMachineGuiParameter *xParam, *yParam;
};

class CMachineXYMultiNode : public CMachineDisplay
{
public:
	CMachineXYMultiNode(	CMachineParameter cmp, 
							int _x,	int _y,
							int _xSize, int _ySize,
							CMachineGuiParameter** _xParams, CMachineGuiParameter** _yParams,
							int _numNodes)
						:	CMachineDisplay(cmp, _x, _y, _xSize, _ySize),
							numNodes(_numNodes), 
							xParams(_xParams), yParams(_yParams)
	{
		Init();
	}

	CMachineXYMultiNode(	CMachineParameter cmp, 
							int _x,	int _y,
							int _xSize, int _ySize,
							CMachineGuiParameter** _xParams, CMachineGuiParameter** _yParams,
							int *_xParamNums,	int *_yParamNums,
							int _numNodes)
						:	CMachineDisplay(cmp, _x, _y, _xSize, _ySize),
							numNodes(_numNodes), 
							xParams(_xParams), yParams(_yParams)
	{
		xParamNums.resize(numNodes);
		yParamNums.resize(numNodes);
		for(int i(0);i<numNodes;++i)
		{
			xParamNums[i]=_xParamNums[i];
			yParamNums[i]=_yParamNums[i];
		}
		Init();
	}

	void Init()
	{
		bTweakable = true;
		bClickable = true;
		numActiveNodes = numNodes;
		nodes.resize(numNodes);
		for(int i(0);i<numNodes;++i)
		{
			nodes[i].xMin=0; 
			nodes[i].yMin=0;
			nodes[i].xMax=xSize;
			nodes[i].yMax=ySize;
		}

		for(int c(0);c<numNodes;++c)
			for (int i(0);i<nodes[c].xSize;++i)
				for(int j(0);j<nodes[c].ySize;++j)
					nodes[c].data[j*nodes[c].xSize+i]=0x00EEEEEE;
	}

	bool LButtonDown(int _x, int _y, int& val) 
	{
		curNode=0;
		while(curNode<numNodes && !nodes[curNode].PointInNode(_x-x, _y-y, 2))
		{	++curNode;	}

		if(curNode>=numActiveNodes)
			return false;

		tweakSrc.x=_x;
		tweakSrc.y=_y;
		tweakBase_x=*(xParams[curNode]->value);
		tweakBase_y=*(yParams[curNode]->value);
		return false;
	}
	bool LButtonUp(int _x, int _y, int& val) 
	{
		return false;
	}

	void ResetTweakSrc(CPoint newSrc)
	{
		tweakSrc=newSrc;
		tweakBase_x=*(xParams[curNode]->value);
		tweakBase_y=*(yParams[curNode]->value);
	}


	virtual int WhatDoITweak(int* paramnums, int* vals,	int mousex, int mousey, unsigned int flags)
	{
		if(curNode>=numActiveNodes) return 0;

		float deltax= mousex-tweakSrc.x;
		float deltay= mousey-tweakSrc.y;
//		if(flags&MK_SHIFT)	 {deltax*=.5; deltay*=.5;}
//		if(flags&MK_CONTROL) {deltax*=.5; deltay*=.5;}


		//this check isn't working right for some reason..  i've implemented it in a much less efficient way below.
		//this should really be looked at at some point

/*		if		(tweakSrc.y+deltay > nodes[curNode].yMax)
			deltay = nodes[curNode].yMax-tweakSrc.y;
		else if (tweakSrc.y+deltay < nodes[curNode].yMin)
			deltay = nodes[curNode].yMin-tweakSrc.y;
		
		if		(tweakSrc.x+deltax > nodes[curNode].xMax)
			deltax = nodes[curNode].xMax-tweakSrc.x;
		else if (tweakSrc.x+deltax < nodes[curNode].xMin)
			deltax = nodes[curNode].xMin-tweakSrc.x;
*/
		int newvalx, newvaly;
		float pixelx = (xParams[curNode]->MaxValue-xParams[curNode]->MinValue) / (float)(xSize==0?1:xSize);
		float pixely = (yParams[curNode]->MaxValue-yParams[curNode]->MinValue) / (float)(ySize==0?1:ySize);

		if(flags&MK_SHIFT) {pixely=.5; pixelx=.5;}
		if(flags&MK_CONTROL){deltay*=.2f; deltax*=.2f;}

		newvalx=(deltax) *  pixelx + tweakBase_x;
		newvaly=(deltay) * -pixely + tweakBase_y;


		//it's ridiculous that i'd have to check x/yMax/Min this way, but the way i tried above didn't work properly.
		//(it either stops the node a pixel or two early, or a pixel or two late, seemingly chosen at random)

		if(( newvalx-xParams[curNode]->MinValue)/(float)(xParams[curNode]->MaxValue-xParams[curNode]->MinValue)*xSize > nodes[curNode].xMax)
			newvalx=nodes[curNode].xMax * pixelx + xParams[curNode]->MinValue;
		if(( newvalx-xParams[curNode]->MinValue)/(float)(xParams[curNode]->MaxValue-xParams[curNode]->MinValue)*xSize < nodes[curNode].xMin)
			newvalx=nodes[curNode].xMin * pixelx + xParams[curNode]->MinValue;
		if(( newvaly-yParams[curNode]->MinValue)/(float)(yParams[curNode]->MaxValue-yParams[curNode]->MinValue)*ySize > nodes[curNode].yMax)
			newvaly=nodes[curNode].yMax * pixely + xParams[curNode]->MinValue;
		if(( newvaly-yParams[curNode]->MinValue)/(float)(yParams[curNode]->MaxValue-yParams[curNode]->MinValue)*ySize < nodes[curNode].yMin)
			newvaly=nodes[curNode].yMin * pixely + yParams[curNode]->MinValue;

		if		(newvalx>xParams[curNode]->MaxValue) newvalx=xParams[curNode]->MaxValue;
		else if	(newvalx<xParams[curNode]->MinValue) newvalx=xParams[curNode]->MinValue;
		vals[0]=newvalx;
		paramnums[0]=xParamNums[curNode];

		if		(newvaly>yParams[curNode]->MaxValue) newvaly=yParams[curNode]->MaxValue;
		else if	(newvaly<yParams[curNode]->MinValue) newvaly=yParams[curNode]->MinValue;
		vals[1]=newvaly;
		paramnums[1]=yParamNums[curNode];

		nodes[curNode].x=   (newvalx - xParams[curNode]->MinValue) / float(xParams[curNode]->MaxValue - xParams[curNode]->MinValue)  * xSize;
		nodes[curNode].y=(1-(newvaly - yParams[curNode]->MinValue) / float(yParams[curNode]->MaxValue - yParams[curNode]->MinValue)) * ySize;

	
		return 2;
	}

	virtual bool PointInParam(int _x, int _y) const {return (_x>x && _x<x+xSize && _y>y && _y<y+ySize);  }

	CPoint GetNodeLocation(int nodeNum)
	{
		CPoint location(nodes[nodeNum].x, nodes[nodeNum].y);
		return location;
	}


	void Paint(CDC *devc, char *valString)
	{
		CBitmap* bmp = new CBitmap;
		CBitmap* oldbmp;
		bmp->CreateCompatibleBitmap(devc, xSize, ySize);
		CDC memdc;
		memdc.CreateCompatibleDC(devc);
		oldbmp = memdc.SelectObject(bmp);

		for(int i(0);i<xSize;++i)
			for(int j(0);j<ySize;++j)
				memdc.SetPixel(CPoint(i, j), displaydata[j*xSize+i]);

		for(int c(0);c<numNodes;++c)
		{
			nodes[c].x=   (*(xParams[c]->value)-xParams[c]->MinValue) / float(xParams[c]->MaxValue-xParams[c]->MinValue)  * xSize;
			nodes[c].y=(1-(*(yParams[c]->value)-yParams[c]->MinValue) / float(yParams[c]->MaxValue-yParams[c]->MinValue)) * ySize;
			int xp, yp;
			int xbase = nodes[c].x-nodes[c].xSize/2;
			int ybase = nodes[c].y-nodes[c].ySize/2;
			for(int i(0);i<nodes[c].xSize;++i)
				for(int j(0);j<nodes[c].ySize;++j)
				{
					xp=xbase+i;
					yp=ybase+j;
					if(xp>=0 && xp<xSize && yp>=0 && yp<ySize)
						memdc.SetPixel(CPoint(xp, yp), nodes[c].data[j*nodes[c].xSize+i]);
				}
		}
		devc->BitBlt(x, y, xSize, ySize, &memdc, 0, 0, SRCCOPY);

		memdc.SelectObject(oldbmp);
		memdc.DeleteDC();
		bmp->DeleteObject(); delete bmp;
	}

	std::vector<GridNode> nodes;
	int numNodes;
	int numActiveNodes;
	int curNode;
	CMachineGuiParameter **xParams, **yParams;
	std::vector<int>xParamNums;
	std::vector<int>yParamNums;
	int tweakBase_y, tweakBase_x;
};


//InstMap is a std::map<int, CMachineGuiParameter**> -- the key is the machine index, and the pointers are the parameters for that
//index.  the old-style parameters had no instance-specific variables, so they could be shared, but since guiparameters contain
//instance-specific pointers to the value, among other things, the parameters have to be created and destroyed in the machineinterface's
//constructor/destructor.  InstMap is used to keep track of each instance's parameter pointers.
#define PSYCLE__GUIPLUGIN__INSTANCIATOR(typename, info, InstMap) \
	extern "C" \
	{ \
		PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT ::CMachineInfo const * const PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION GetInfo() { return &info; } \
		PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT ::CMachineInterface *        PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION CreateMachine(int index) { return new typename(index); } \
		PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT void                         PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION DeleteMachine(::CMachineInterface & plugin) { delete &plugin; } \
		PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT ::CMachineGuiParameter **    PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION GetParams(int index)	{ return InstMap[index];} \
	}
