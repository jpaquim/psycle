//implementation of CFrameAutomator.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "FrameAutomator.hpp"
#include "psycle.hpp"
#include "ChildView.hpp"
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/song.hpp>
#include <psycle/engine/machine.hpp>

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

	IMPLEMENT_DYNCREATE(CFrameAutomator, CFrameWnd)

	int CFrameAutomator::Knob::s_height(28);
	int CFrameAutomator::Knob::s_width(28);
	int CFrameAutomator::Knob::s_numFrames(64);
	CDC CFrameAutomator::Knob::s_knobDC;

	int CFrameAutomator::ComboBox::s_height(18);
	int CFrameAutomator::ComboBox::s_width(18);
	CDC CFrameAutomator::ComboBox::s_ComboBoxDC;

	int CFrameAutomator::Switch::s_height(10);
	int CFrameAutomator::Switch::s_width(10);
	int CFrameAutomator::Switch::s_spacer(4);
	CDC CFrameAutomator::Switch::s_SwitchDC;



	//////////////////////////////////////////////////////////////////////////
	// Knob class

	CFrameAutomator::Knob::Knob(int x, int y, int minVal, int maxVal, const std::string& label)
		: CFrameAutomator::AutomatorCtrl(x, y, minVal, maxVal)
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

		void CFrameAutomator::Knob::Paint(CDC* dc,int value, const std::string& valString)
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
	bool CFrameAutomator::Knob::LButtonDown(UINT nFlags,int x,int y, int &value)
	{
		d_twkSrc_x = x;
		d_twkSrc_y = y;
		d_tweakBase=value;

		return false;
	}
	bool CFrameAutomator::Knob::LButtonUp(UINT nFlags, int x, int y, int &value)
	{
		return false;
	}

	bool CFrameAutomator::Knob::MouseMove(UINT nFlags, int x, int y, int &value)
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

	bool CFrameAutomator::Knob::PointInParam(int x, int y)
	{
		return	d_bVisible	&&
				x > d_x && x < d_x+s_width	&&
				y > d_y && y < d_y+s_height;
	}

	/////////////////////////////////////////////////////////////////////////
	// CFrameAutomator::ComboBox class
	CFrameAutomator::ComboBox::ComboBox(int x, int y, int minVal, int maxVal, int length, const std::string& label)
		:CFrameAutomator::AutomatorCtrl(x, y, minVal, maxVal)
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

	void CFrameAutomator::ComboBox::Paint(CDC* dc,int value, const std::string& valString)
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
	bool CFrameAutomator::ComboBox::LButtonDown(UINT nFlags,int x,int y, int &value)
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
	bool CFrameAutomator::ComboBox::LButtonUp(UINT nFlags, int x, int y, int &value)
	{
		return false;
	}

	bool CFrameAutomator::ComboBox::MouseMove(UINT nFlags, int x, int y, int &value)
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

	bool CFrameAutomator::ComboBox::PointInParam(int x, int y)
	{
		return	d_bVisible	&&
				x > d_x && x < d_x+s_width*2+d_length	&&
				y > d_y && y < d_y+s_height;
	}


	/////////////////////////////////////////////////////////////////////////
	// CFrameAutomator::Switch class
	CFrameAutomator::Switch::Switch(int x, int y, int minVal, int maxVal, const std::string &label, const std::vector<std::string>& valStrings)
		: CFrameAutomator::AutomatorCtrl(x, y, minVal, maxVal)
	{
			d_showLabel=top;
			d_showValue=bottom;
			d_lblBkColor = UIGlobal::configuration().machineGUIBottomColor;
			d_valBkColor = UIGlobal::configuration().machineGUIBottomColor;
			d_valTextColor = UIGlobal::configuration().machineGUIFontBottomColor;
			d_lblTextColor = UIGlobal::configuration().machineGUIFontBottomColor;
			d_lblString = label;
			d_bTweakable = false;
			d_count=(maxVal-minVal+1);

			assert(valStrings.size()==d_count);
			d_valStrings=valStrings;

	}
	void CFrameAutomator::Switch::Paint(CDC* dc,int value, const std::string& valString)
	{
		if(!d_bVisible) return;
		for(int i(d_minValue); i<=d_maxValue; ++i)
		{
			dc->BitBlt(	d_x, d_y+(s_height+s_spacer)*(i-d_minValue),
						s_width, s_height, &s_SwitchDC,
						(value==i? s_width: 0), 0, SRCCOPY);

			if(d_showValue!=off)
			{
				CSize valTextSize = dc->GetTextExtent(d_valStrings[i-d_minValue].c_str());
				
				int textX = d_x + s_width + 10;
				int textY = d_y + s_height/2 + (i-d_minValue)*(s_height+s_spacer)  - valTextSize.cy/2;

				dc->SetBkColor(d_valBkColor);
				dc->SetTextColor(d_valTextColor);
				dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+valTextSize.cx, textY+valTextSize.cy), d_valStrings[i-d_minValue].c_str(), 0);

			}
		}


		if(d_showLabel!=off)
		{
			int textX, textY;
			CSize lblTextSize = dc->GetTextExtent(d_lblString.c_str());
			switch(d_showLabel)
			{
			case right:
			case top:
				textX = d_x;
				textY = d_y - 10 - lblTextSize.cy/2;
				break;
			case bottom:
				textX = d_x;
				textY = d_y + s_height + 10 - lblTextSize.cy/2;
				break;
			case left:
				textX = d_x - 10 - lblTextSize.cx;
				textY = d_y + (s_height+s_spacer)*(d_count)/2 - lblTextSize.cy/2;
				break;
			default:
				throw;
			}
			dc->SetBkColor(d_lblBkColor);
			dc->SetTextColor(d_lblTextColor);
			dc->ExtTextOut(textX, textY, ETO_OPAQUE, CRect(textX, textY, textX+lblTextSize.cx, textY+lblTextSize.cy), d_lblString.c_str(), 0);
		}
	}
	bool CFrameAutomator::Switch::LButtonDown(UINT nFlags,int x,int y, int &value)
	{
		int hit = (d_y-y)%(s_height+s_spacer);
		int which = (d_y-y)/(s_height+s_spacer);
		if(hit<=s_height)
		{
			value=which+d_minValue;
			return true;
		}
		return false;
	}

	bool CFrameAutomator::Switch::LButtonUp(UINT nFlags, int x, int y, int &value)
	{
		return false;
	}

	bool CFrameAutomator::Switch::MouseMove(UINT nFlags, int x, int y, int &value)
	{
		return false;
	}

	bool CFrameAutomator::Switch::PointInParam(int x, int y)
	{
		return	d_bVisible	&&
				x > d_x && x < d_x+s_width	&&
				y > d_y && y < d_y+(d_count)*(s_height+s_spacer);
	}

	void EnvelopeWindow::Paint(CDC* dc)
	{
		dc->FillSolidRect(d_x, d_y, d_width, d_height, RGB(0, 0, 0) );

		if(d_pAutomator->IsDiscrete())
		{
			CBrush brush(RGB(0, 90, 142));
			const float width = d_width/(float)d_pAutomator->dLength;
			if(d_pAutomator->IsRelative())
			{
				int top, bottom;
				//todo: it would probably be quicker to iterate through pixels, instead of the envelope array.. at least for long envelopes.
				for(int i(0);i<d_pAutomator->dLength;++i)
				{
					//this color fading stuff is completely useless, completely superfluous, and a 
					//complete waste of resources, but it's just so pretty!
					brush.DeleteObject();
					brush.CreateSolidBrush(RGB(0, 70+(int)(70 * i/d_pAutomator->dLength), 80+(int)(90 * i/d_pAutomator->dLength)));
					top = bottom = d_y+d_height/2;
					float value = d_pAutomator->dTable[i];
					(value>0.5f? top: bottom) = d_y + d_height - value*d_height;
					CRect rect(d_x+(int)(i*width), top, d_x+(int)((i+1)*width), bottom);
					//dc->FillSolidRect(&rect, RGB(0, 90, 142));
					dc->FillRect(&rect, &brush);
				}
			}
			else	//absolute
			{
				for(int i(0);i<d_pAutomator->dLength;++i)
				{
					brush.DeleteObject();
					brush.CreateSolidBrush(RGB(0, 70+(int)(70 * i/d_pAutomator->dLength), 80+(int)(90 * i/d_pAutomator->dLength)));
					int height = d_pAutomator->dTable[i] * d_height;
					CRect rect(d_x+(int)(i*width), d_y+d_height-height, d_x+(int)((i+1)*width), d_y+d_height);
					//dc->FillSolidRect(&rect, RGB(0, 90, 142));
					dc->FillRect(&rect, &brush);
				}
			}

		}
		else	//continuous
		{
			CPen bluePen(0, 3, RGB(0, 90, 142));
			CPen grayPen(0, 1, RGB(24, 24, 24));
			CPen* oldPen;
			oldPen = dc->SelectObject(&grayPen);
			if(d_pAutomator->IsRelative())
			{
				dc->MoveTo(d_x, d_y+d_height/2);
				dc->LineTo(d_x+d_width, d_y+d_height/2);
				dc->MoveTo(d_x, d_y+d_height/2+1);
				dc->LineTo(d_x+d_width, d_y+d_height/2+1);
			}
			dc->SelectObject(&bluePen);

			std::vector<Automator::Node>::iterator iter = d_pAutomator->cTable.begin();

			dc->MoveTo(d_x + (iter->time * d_width/d_pAutomator->cLength), d_y + d_height - (iter->value*d_height));
			for(++iter; iter!=d_pAutomator->cTable.end(); ++iter)
				dc->LineTo(d_x + iter->time*d_width/d_pAutomator->cLength,  d_y + d_height - (iter->value*d_height) );
			
			dc->SelectObject(oldPen);
		}
	}
	bool EnvelopeWindow::LButtonDown(unsigned int nflags, int x, int y)
	{
		if(d_pAutomator->IsDiscrete())
		{
			x-=d_x;
			int index = x * d_pAutomator->dLength / d_width;
			if(index>d_pAutomator->dLength-1) index = d_pAutomator->dLength-1;
			if(index<0) index = 0;
			if(y<d_y) y=d_y;
			else if(y>d_y+d_height) y=d_y+d_height;
			d_pAutomator->dTable[index]=1.0f - (y-d_y)/(float)d_height;
			return true;
		}
		else	//continuous
		{
			int xInMs = (x-d_x)*d_pAutomator->cLength/d_width;
			int xleeway = 10*d_pAutomator->cLength/d_width;
			float yAdapted = 1 - (y-d_y)/(float)d_height;
			float yleeway = 10/(float)d_height;

			for(std::vector<Automator::Node>::iterator iter = d_pAutomator->cTable.begin(); iter!= d_pAutomator->cTable.end(); ++iter)
			{
				if( abs( iter->time-xInMs ) <= xleeway && abs( iter->value-yAdapted ) <= yleeway )
				{
					d_curNode = iter;
					return true;
				}
			}
			return false;
		}
	}
	bool EnvelopeWindow::LButtonDblClk(unsigned int nflags, int x, int y)
	{
		if(d_pAutomator->IsDiscrete())
			return this->LButtonDown(nflags, x, y);

		//continuous:
		int xAdapted = (x-d_x)*d_pAutomator->cLength/d_width;
		int xleeway = 10*d_pAutomator->cLength/d_width;
		float yAdapted = 1 - (y-d_y)/(float)d_height;
		float yleeway = 10/(float)d_height;

		for(std::vector<Automator::Node>::iterator iter = d_pAutomator->cTable.begin(); iter!= d_pAutomator->cTable.end(); ++iter)
		{
			if( abs( iter->time-xAdapted ) <= xleeway && abs( iter->value-yAdapted ) <= yleeway )
			{
				//found one, delete it!
				d_pAutomator->cTable.erase(iter);
				std::sort(d_pAutomator->cTable.begin(), d_pAutomator->cTable.end());
				return false;
			}
		}
		//no nearby nodes, so we insert one:

		Automator::Node newnode(xAdapted, yAdapted);
		d_pAutomator->cTable.push_back(newnode);
		std::sort(d_pAutomator->cTable.begin(), d_pAutomator->cTable.end());
		//we don't know where the new node will be after sorting, so we have to seek it out manually..
		for(std::vector<Automator::Node>::iterator iter = d_pAutomator->cTable.begin(); iter!= d_pAutomator->cTable.end(); ++iter)
		{
			if( iter->time==xAdapted && iter->value==yAdapted )
			{
				d_curNode = iter;
				return true;
			}
		}
		return false;	//if we make it here, something's gone awry..


	}
	void EnvelopeWindow::MouseMove(unsigned int nflags, int x, int y)
	{
		if(d_pAutomator->IsDiscrete())
		{
			x-=d_x;
			int index = x * d_pAutomator->dLength / d_width;
			if(index>d_pAutomator->dLength-1) index = d_pAutomator->dLength-1;
			if(index<0) index = 0;
			if(y<d_y) y=d_y;
			else if(y>d_y+d_height) y=d_y+d_height;
			d_pAutomator->dTable[index]=1.0f - (y-d_y)/(float)d_height;
		}
		else	//continuous
		{
			if(x<d_x)				x = d_x;
			else if(x>=d_x+d_width)	x = d_x + d_width-1;
			if(y<d_y)				y = d_y;
			else if(y>=d_y+d_height)y = d_y + d_height-1;
			int newIdx = (int)((x-d_x)*d_pAutomator->cLength/d_width);
			float newVal = 1 - (y-d_y)/(float)d_height;

			d_curNode->value = newVal;
			if(d_curNode!=d_pAutomator->cTable.begin() && d_curNode!=d_pAutomator->cTable.end()-1)
				d_curNode->time=newIdx; 

			std::sort(d_pAutomator->cTable.begin(), d_pAutomator->cTable.end());
			for(std::vector<Automator::Node>::iterator iter = d_pAutomator->cTable.begin(); iter!= d_pAutomator->cTable.end(); ++iter)
				if( iter->time==newIdx && iter->value==newVal )
					d_curNode = iter;
			
		
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// CFrameAutomator class

	BEGIN_MESSAGE_MAP(CFrameAutomator, CFrameWnd)
		//{{AFX_MSG_MAP(CFrameAutomator)
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
		ON_WM_SIZE()
		ON_WM_HSCROLL()

		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	CFrameAutomator::CFrameAutomator()
	{
	}
	CFrameAutomator::CFrameAutomator(int dum) :
		d_pAutomator(0)
		,bmpDC(0)
	{
		MachineIndex = dum;

		d_pParams.resize(Automator::prms::num_params);
		std::vector<std::string> names;
		names.push_back("Continuous"); names.push_back("Discrete");
		d_pParams[Automator::prms::discrete]=	new Switch(10, 20, 0, 1, "Sequence Type", names);
		d_pParams[Automator::prms::discrete]->d_defValue=1;

		names.clear();
		names.push_back("Absolute"); names.push_back("Relative");
		d_pParams[Automator::prms::relative]=	new Switch(110, 20, 0, 1, "Sequence Mode", names);
		d_pParams[Automator::prms::relative]->d_defValue=0;
		d_pParams[Automator::prms::clength]=	new Knob(200, 20, 0, Automator::CLENGTH_MAX, "Length");
		d_pParams[Automator::prms::clength]->d_defValue=10000;
		d_pParams[Automator::prms::dlength]=	new Knob(200, 20, 0, Automator::DLENGTH_MAX, "Length");
		d_pParams[Automator::prms::dlength]->d_defValue=64;
		d_pParams[Automator::prms::dstep]=		new Knob(260, 20, 0, Automator::DSTEP_MAX, "Step Size");
		d_pParams[Automator::prms::dstep]->d_defValue=1*Automator::DSTEP_SCALER;

		envWind = new EnvelopeWindow(10, 70, 565, 165);

		m_combobox.LoadBitmap(IDB_COMBOBOX);
		m_switch.LoadBitmap(IDB_LEDS);

	}


	CFrameAutomator::~CFrameAutomator()
	{
		if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
		for(int i(0);i<Automator::prms::num_params;++i) delete d_pParams[i];
		delete envWind;
	}

	int CFrameAutomator::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
//		zoombar.Create(this, IDD_AUTOMATOR_ZOOMBAR, CBRS_BOTTOM | WS_CHILD, AFX_IDW_DIALOGBAR);
		return CFrameMachine::OnCreate(lpCreateStruct);
	}

	void CFrameAutomator::OnDestroy() 
	{
		CFrameMachine::OnDestroy();
	}

	void CFrameAutomator::OnSize(UINT nType, int cx, int cy)
	{
		CFrameWnd::OnSize(nType, cx, cy);
		envWind->SetWidth(  cx - 2*envWind->GetXPos());
		envWind->SetHeight( cy -   envWind->GetYPos() - 15);
/*
		int cyZoombar = GetSystemMetrics(SM_CYHSCROLL);

		int clientcy = cy - cyZoombar;


		zoombar.MoveWindow(0, clientcy, cx, cyZoombar);

		CButton* cb;
		cb=(CButton *)zoombar.GetDlgItem(IDC_ZOOMIN);
		cb->SetWindowPos(NULL, cx-20, 0, 15, cyZoombar, SWP_NOZORDER);
		cb=(CButton *)zoombar.GetDlgItem(IDC_ZOOMOUT);
		cb->SetWindowPos(NULL, cx-35, 0, 15, cyZoombar, SWP_NOZORDER);

		CScrollBar *csb;
		csb = (CScrollBar*)zoombar.GetDlgItem(IDC_HSCROLL);
		
		csb->SetWindowPos(NULL, 0, 0, cx-35, cyZoombar, SWP_NOZORDER);
*/		
	}

	void CFrameAutomator::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
	{

	}


	void CFrameAutomator::SelectMachine(Machine* pMachine)
	{
		d_pAutomator=(Automator*)(_pMachine = pMachine);
		envWind->SetAutomator(d_pAutomator);

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

	void CFrameAutomator::OnTimer(UINT nIDEvent) 
	{
		if ( nIDEvent == 2104+MachineIndex )
		{
			Invalidate(false);
		}
		CFrameWnd::OnTimer(nIDEvent);

	}

	void CFrameAutomator::OnPaint() 
	{
		if (!d_pAutomator) return;
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
		Switch::s_SwitchDC.CreateCompatibleDC(&bufferDC);
		CBitmap *oldbmp3=Switch::s_SwitchDC.SelectObject(&m_switch);

		//decide what's visible
		bool bDiscrete = d_pAutomator->IsDiscrete();
		d_pParams[Automator::prms::clength]->d_bVisible=!bDiscrete;
		d_pParams[Automator::prms::dlength]->d_bVisible=bDiscrete;
		d_pParams[Automator::prms::dstep]->d_bVisible=bDiscrete;
		//draw bg color
		bufferDC.FillSolidRect(0,0,rect.right,rect.bottom,UIGlobal::configuration().machineGUIBottomColor);
		//draw params
		char temp[128];
		for(int i(0);i<Automator::prms::num_params;++i)
		{
			d_pAutomator->GetParamValue(i, temp);
			d_pParams[i]->Paint(&bufferDC, d_pAutomator->GetParamValue(i), temp);
		}

		envWind->Paint(&bufferDC);
		//blt buffer to screen
		dc.BitBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,SRCCOPY);

		//cleanup
		Knob::s_knobDC.SelectObject(oldbmp);
		Knob::s_knobDC.DeleteDC();
		ComboBox::s_ComboBoxDC.SelectObject(oldbmp2);
		ComboBox::s_ComboBoxDC.DeleteDC();
		Switch::s_SwitchDC.SelectObject(oldbmp3);
		Switch::s_SwitchDC.DeleteDC();

		bufferDC.SelectObject(oldfont);
		bufferDC.SelectObject(bufferbmp);
		bufferDC.DeleteDC();

	}

	int CFrameAutomator::ConvertXYtoParam(int x, int y)
	{
		int i(0);
		while(i<Automator::prms::num_params && !(d_pParams[i]->PointInParam(x, y)))
			++i;
		if(i<Automator::prms::num_params)
			return i;
		else if(envWind->PointInParam(x, y))
			return Automator::prms::num_params;
        return -1;
	}

	void CFrameAutomator::OnLButtonDown(UINT nFlags, CPoint point) 
	{
		tweakpar = ConvertXYtoParam(point.x, point.y);
		if(tweakpar>-1 && tweakpar<Automator::prms::num_params)
		{
			int value = d_pAutomator->GetParamValue(tweakpar);
			if(d_pParams[tweakpar]->LButtonDown(nFlags, point.x, point.y, value))
				d_pAutomator->SetParameter(tweakpar, value);
			if(d_pParams[tweakpar]->d_bTweakable)
			{
				istweak = true;
				SetCapture();
				d_shiftAndCtrlState = (nFlags & (MK_SHIFT | MK_CONTROL));
			}
		}
		else if(tweakpar==Automator::prms::num_params)
		{
			if(envWind->LButtonDown(nFlags, point.x, point.y))
			{
				SetCapture();
				istweak = true;
			}
		}

		
		Invalidate(false);
		CFrameWnd::OnLButtonDown(nFlags,point);
	}

	void CFrameAutomator::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		tweakpar = ConvertXYtoParam(point.x, point.y);
		if(tweakpar>-1 && tweakpar<Automator::prms::num_params)
		{
			if(d_pParams[tweakpar]->d_bDblClkReset)
				d_pAutomator->SetParameter(tweakpar, d_pParams[tweakpar]->d_defValue);
			else
			{
				int value = d_pAutomator->GetParamValue(tweakpar);
				if(d_pParams[tweakpar]->LButtonDown(nFlags, point.x, point.y, value))
					d_pAutomator->SetParameter(tweakpar, value);
				if(d_pParams[tweakpar]->d_bTweakable)
				{
					istweak = true;
					SetCapture();
					d_shiftAndCtrlState = (nFlags & (MK_SHIFT | MK_CONTROL));
				}
			}
		}
		else if(tweakpar==Automator::prms::num_params)	//envelope window
		{
			if(envWind->LButtonDblClk(nFlags, point.x, point.y))
			{
				SetCapture();
				istweak = true;
			}
		}

		Invalidate(false);
		CFrameWnd::OnLButtonDblClk(nFlags,point);
	}
	void CFrameAutomator::OnMouseMove(UINT nFlags, CPoint point) 
	{
		if(istweak)
		{
			if(tweakpar>-1 && tweakpar<Automator::prms::num_params)
			{
				int value=d_pAutomator->GetParamValue(tweakpar);

				if( (nFlags & (MK_CONTROL | MK_SHIFT)) != d_shiftAndCtrlState )	//state of either shift or control has changed
				{
					d_pParams[tweakpar]->ResetTweakSrc(point.x, point.y, value);
					d_shiftAndCtrlState = (nFlags & (MK_CONTROL | MK_SHIFT));
				}

				if(d_pParams[tweakpar]->MouseMove(nFlags, point.x, point.y, value))
					d_pAutomator->SetParameter(tweakpar, value);
			}
			else if(tweakpar==Automator::prms::num_params)
				envWind->MouseMove(nFlags, point.x, point.y);

		}

		Invalidate(false);

		CFrameWnd::OnMouseMove(nFlags,point);
	}
	void CFrameAutomator::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		if(tweakpar>-1 && tweakpar<Automator::prms::num_params)
		{
			int value=d_pAutomator->GetParamValue(tweakpar);
			if(d_pParams[tweakpar]->LButtonUp(nFlags, point.x, point.y, value))
				d_pAutomator->SetParameter(tweakpar, value);
		}
		istweak = false;
		Invalidate(false);	
		ReleaseCapture();
		CFrameWnd::OnLButtonUp(nFlags, point);
	}

	void CFrameAutomator::OnRButtonUp(UINT nFlags, CPoint point) 
	{
		CFrameMachine::OnRButtonUp(nFlags, point);
	}
	void CFrameAutomator::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameMachine::OnKeyDown(nChar, nRepCnt, nFlags);
    }
	void CFrameAutomator::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		CFrameMachine::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	void CFrameAutomator::OnSetFocus(CWnd* pOldWnd) 
	{
		CFrameMachine::OnSetFocus(pOldWnd);
	}

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
