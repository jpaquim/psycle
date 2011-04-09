///\file
///\brief implementation file for psycle::host::MixerFrameView.

#include "MixerFrameView.hpp"

#include "internal_machines.hpp"

#include "ChildView.hpp"

namespace psycle { namespace host {


	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
		BEGIN_MESSAGE_MAP(MixerFrameView, CWnd)
			ON_WM_CREATE()
			ON_WM_SETFOCUS()
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_WM_RBUTTONUP()
		END_MESSAGE_MAP()


		MixerFrameView::MixerFrameView(Machine* effect, CChildView* view)
		:CNativeGui(effect, view)
		,bmpDC(0)
		,numSends(0)
		,numChans(0)
		,updateBuffer(false)
		,_swapstart(-1)
		,_swapend(-1)
		,isslider(false)
		,refreshheaders(false)
		{
			colwidth=uiSetting->dialwidth+32;
			SelectMachine(effect);
		}
		MixerFrameView::~MixerFrameView()
		{
			if ( bmpDC ) { bmpDC->DeleteObject(); delete bmpDC; }
		}

		void MixerFrameView::OnPaint() 
		{
			if (!_pMachine) return;
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
			CFont *oldfont=bufferDC.SelectObject(&uiSetting->font);

			CDC knobDC;
			CDC SliderKnobDC;
			CDC SliderbackDC;
			CDC SwitchOffDC;
			CDC SwitchOnDC;
			CDC VuOnDC;
			CDC VuOffDC;
			knobDC.CreateCompatibleDC(&bufferDC);
			SliderKnobDC.CreateCompatibleDC(&bufferDC);
			SliderbackDC.CreateCompatibleDC(&bufferDC);
			SwitchOffDC.CreateCompatibleDC(&bufferDC);
			SwitchOnDC.CreateCompatibleDC(&bufferDC);
			VuOnDC.CreateCompatibleDC(&bufferDC);
			VuOffDC.CreateCompatibleDC(&bufferDC);
			CBitmap *oldbmp=knobDC.SelectObject(&CNativeGui::uiSetting->dial);
			CBitmap *oldbmp2=SliderKnobDC.SelectObject(&CNativeGui::uiSetting->sliderKnob);
			CBitmap *sliderbmp=SliderbackDC.SelectObject(&CNativeGui::uiSetting->sliderBack);
			CBitmap *switchonbmp=SwitchOffDC.SelectObject(&CNativeGui::uiSetting->switchOff);
			CBitmap *switchoffbmp=SwitchOnDC.SelectObject(&CNativeGui::uiSetting->switchOn);
			CBitmap *oldbmp3=VuOnDC.SelectObject(&CNativeGui::uiSetting->vuOn);
			CBitmap *vubmp=VuOffDC.SelectObject(&CNativeGui::uiSetting->vuOff);

			if (updateBuffer) 
			{
				bufferDC.FillSolidRect(0,0,rect.right,rect.bottom,uiSetting->bottomColor);
				GenerateBackground(bufferDC, SliderbackDC, SliderKnobDC, VuOnDC, VuOffDC);
				updateBuffer=false;
			}
			// Column 1 Master volume
			int xoffset(colwidth), yoffset(0);
			int infowidth(colwidth-uiSetting->dialwidth);
			int checkedwidth(16);
			char value[48];

			yoffset=(mixer().numsends()+1)*uiSetting->dialheight;
			mixer().GetParamValue(13,value);
			Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(13)/256.0f);
			InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
			bufferDC.Draw3dRect(xoffset-1,yoffset-1,colwidth+1,uiSetting->dialheight+1,uiSetting->titleColor,uiSetting->titleColor);
			yoffset+=uiSetting->dialheight;
			mixer().GetParamValue(14,value);
			Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(14)/1024.0f);
			InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
			yoffset+=uiSetting->dialheight;
			mixer().GetParamValue(15,value);
			Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(15)/256.0f);
			InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);

			mixer().GetParamValue(0,value);
			InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,value);
			VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,mixer()._volumeDisplay/97.0f);
			yoffset+=uiSetting->dialheight;
			GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,mixer().GetParamValue(0)/4096.0f);

			// Columns 2 onwards, controls
			xoffset+=colwidth;
			Machine** machines = Global::song()._pMachine;
			for (int i(0); i<mixer().numinputs(); i++)
			{
				yoffset=0;
				if ( _swapend != -1 || refreshheaders)
				{
					std::string chantxt = mixer().GetAudioInputName(int(i+Mixer::chan1));
					if (mixer().ChannelValid(i))
					{
						if ( _swapend == i+chan1)
							InfoLabel::DrawHLightB(bufferDC,xoffset,yoffset,infowidth,chantxt.c_str(),machines[mixer()._inputMachines[i]]->_editName);
						else 
							InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,chantxt.c_str(),machines[mixer()._inputMachines[i]]->_editName);
					}
					else
					{
						if ( _swapend == i+chan1)
							InfoLabel::DrawHLightB(bufferDC,xoffset,infowidth,yoffset,chantxt.c_str(),"");
						else
							InfoLabel::DrawHLightB(bufferDC,xoffset,infowidth,yoffset,chantxt.c_str(),"");
					}
				}
				if (mixer().ChannelValid(i))
				{
					yoffset=uiSetting->dialheight;
					for (int j=0; j<mixer().numsends(); j++)
					{
						int param =(i+1)*0x10+(j+1);
						Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(param)/256.0f);
						mixer().GetParamValue(param,value);
						InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
						yoffset+=uiSetting->dialheight;
					}
					int param =(i+1)*0x10;
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(param)/256.0f);
					mixer().GetParamValue(param,value);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
					yoffset+=uiSetting->dialheight;
					param+=14;
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(param)/1024.0f);
					mixer().GetParamValue(param,value);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
					yoffset+=uiSetting->dialheight;
					param++;
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(param)/256.0f);
					mixer().GetParamValue(param,value);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
					param= i+1;
					mixer().GetParamValue(param,value);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,value);
					VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,mixer().VuChan(i));
					yoffset+=uiSetting->dialwidth;
					GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,mixer().GetParamValue(param)/4096.0f);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth,yoffset,"S",mixer().GetSoloState(i));
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth+checkedwidth,yoffset,"M",mixer().Channel(i).Mute());
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->dialwidth,"D",mixer().Channel(i).DryOnly());
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth+checkedwidth,yoffset+uiSetting->dialwidth,"W",mixer().Channel(i).WetOnly());
					xoffset+=colwidth;
				}
				else
				{
					yoffset=uiSetting->dialheight;
					for (int j=0; j<mixer().numsends(); j++)
					{
						InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"");
						yoffset+=uiSetting->dialheight;
					}
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,0);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"");
					yoffset+=uiSetting->dialheight;
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,0);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"");
					yoffset+=uiSetting->dialheight;
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,0);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"");
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,"");
					VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,0);
					yoffset+=uiSetting->dialwidth;
					GraphSlider::Draw(bufferDC,SliderbackDC, SliderKnobDC,xoffset,yoffset,0);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth,yoffset,"S",false);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth+checkedwidth,yoffset,"M",false);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->dialwidth,"D",false);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth+checkedwidth,yoffset+uiSetting->dialwidth,"W",false);
					xoffset+=colwidth;
				}
			}
			for (int i(0); i<mixer().numreturns(); i++)
			{
				yoffset=0;
				if ( _swapend != -1  || refreshheaders)
				{
					std::string sendtxt = mixer().GetAudioInputName(int(i+Mixer::return1));
					if (mixer().ReturnValid(i))
					{
						if ( _swapend == i+return1)
							InfoLabel::DrawHLightB(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),sendNames[i].c_str());
						else 
							InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),sendNames[i].c_str());
					}
					else
					{
						if ( _swapend == i+return1)
							InfoLabel::DrawHLightB(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),"");
						else
							InfoLabel::DrawHLightB(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),"");
					}
				}

				if (mixer().ReturnValid(i))
				{
					yoffset=(2+i)*uiSetting->dialheight;
					for (int j=i+1; j<mixer().numsends(); j++)
					{
						if(GetRouteState(i,j)) {
							SwitchButton::Draw(bufferDC,SwitchOnDC,xoffset,yoffset);
						}
						else {
							SwitchButton::Draw(bufferDC,SwitchOffDC,xoffset,yoffset);
						}
						yoffset+=uiSetting->dialheight;
					}
					if(GetRouteState(i,13)) {
						SwitchButton::Draw(bufferDC,SwitchOnDC,xoffset,yoffset);
					}
					else {
						SwitchButton::Draw(bufferDC,SwitchOffDC,xoffset,yoffset);
					}

					int param =0xF1+i;
					yoffset=(mixer().numsends()+3)*uiSetting->dialheight;
					mixer().GetParamValue(param,value);
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,mixer().GetParamValue(param)/256.0f);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,value);
					bufferDC.Draw3dRect(xoffset-1,yoffset-1,colwidth+1,uiSetting->dialheight+1,uiSetting->titleColor,uiSetting->titleColor);

					param = 0xE1+i;
					mixer().GetParamValue(param,value);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,value);
					VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,mixer().VuSend(i));
					yoffset+=uiSetting->dialheight;
					GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,mixer().GetParamValue(param)/4096.0f);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth,yoffset,"S",mixer().GetSoloState(i+Mixer::return1));
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth+checkedwidth,yoffset,"M",mixer().Return(i).Mute());
					xoffset+=colwidth;
				}
				else
				{
					yoffset+=(1+i)*uiSetting->dialheight;
					for (int j=i+1; j<mixer().numsends(); j++)
					{
						InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Route","");
						yoffset+=uiSetting->dialheight;
					}
					InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Master","");

					yoffset=(mixer().numsends()+3)*uiSetting->dialheight;
					Knob::Draw(bufferDC,knobDC,xoffset,yoffset,0);
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"");
					InfoLabel::DrawValue(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,"");
					VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,0);
					yoffset+=uiSetting->dialheight;
					GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,0);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth,yoffset,"S",false);
					CheckedButton::Draw(bufferDC,xoffset+uiSetting->sliderwidth+checkedwidth,yoffset,"M",false);
					xoffset+=colwidth;
				}
			}

			dc.BitBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,SRCCOPY);

			VuOnDC.SelectObject(oldbmp3);
			VuOnDC.DeleteDC();
			SliderKnobDC.SelectObject(oldbmp2);
			SliderKnobDC.DeleteDC();
			knobDC.SelectObject(oldbmp);
			knobDC.DeleteDC();
			VuOffDC.SelectObject(vubmp);
			VuOffDC.DeleteDC();
			SwitchOffDC.SelectObject(switchoffbmp);
			SwitchOffDC.DeleteDC();
			SwitchOnDC.SelectObject(switchonbmp);
			SwitchOnDC.DeleteDC();
			SliderbackDC.SelectObject(sliderbmp);
			SliderbackDC.DeleteDC();

			bufferDC.SelectObject(oldfont);
			bufferDC.SelectObject(bufferbmp);
			bufferDC.DeleteDC();
		}

		void MixerFrameView::OnLButtonDown(UINT nFlags, CPoint point) 
		{

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
					float foffset = yoffset/ float(uiSetting->sliderheight-uiSetting->sliderknobheight);
					float fbase = tweakbase/ float(maxval-minval);
					float knobheight = (uiSetting->sliderknobheight/float(uiSetting->sliderheight-uiSetting->sliderknobheight));
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
						if(Global::psycleconf().inputHandler()._RecordTweaks)
						{
							if(Global::psycleconf().inputHandler()._RecordMouseTweaksSmooth)
								mainView->MousePatternTweakSlide(_pMachine->_macIndex, tweakpar, tweakbase);
							else
								mainView->MousePatternTweak(_pMachine->_macIndex, tweakpar, tweakbase);
						}
					}
				}
				prevval = tweakbase;
				mainView->AddMacViewUndo();
			}
			CWnd::OnLButtonDown(nFlags, point);
		}

		void MixerFrameView::OnMouseMove(UINT nFlags, CPoint point) 
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

				double freak = (maxval-minval)*0.625/(uiSetting->sliderheight-uiSetting->sliderknobheight); // *0.625 adjust the full range to the visual range
				if ( ultrafinetweak ) freak /= 10;
				if (finetweak) freak/=4;

				double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase; // +0.375 to compensate for the visual range.

				if (nv < minval) nv = minval;
				if (nv > maxval) nv = maxval;
				_pMachine->SetParameter(tweakpar,(int) (nv+0.5f)); // +0.5f to round correctly, not like "floor".
				prevval=(int)(nv+0.5f);
				///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
				//				wndView->AddMacViewUndo();
				if(Global::psycleconf().inputHandler()._RecordTweaks)
				{
					if(Global::psycleconf().inputHandler()._RecordMouseTweaksSmooth)
						mainView->MousePatternTweakSlide(_pMachine->_macIndex, tweakpar, prevval);
					else
						mainView->MousePatternTweak(_pMachine->_macIndex, tweakpar, prevval);
				}


				Invalidate(false);
				CWnd::OnMouseMove(nFlags,point);
			}
			else if (_swapstart > -1)
			{
				int xoffset(0);
				int col = GetColumn(point.x,xoffset);
				if ( _swapstart < chanmax && col >= return1) _swapend = -1;
				else if ( _swapstart >= return1 && col < chanmax) _swapend = -1;
				else _swapend = col;
				CWnd::OnMouseMove(nFlags,point);
			}
			else CNativeGui::OnMouseMove(nFlags,point);
		}

		void MixerFrameView::OnLButtonUp(UINT nFlags, CPoint point) 
		{
			if ( _swapstart >= chan1 && _swapend >= chan1 && _swapstart != _swapend)
			{
				if ( _swapstart < chanmax)
				{
					_swapstart -=chan1; _swapend -= chan1; 
					mixer().ExchangeChans(_swapstart,_swapend);
				}
				else 
				{
					_swapstart-=return1; _swapend -= return1;
					mixer().ExchangeReturns(_swapstart,_swapend);
				}
			}
			refreshheaders=true;
			_swapstart = -1;
			_swapend = -1;
			isslider = false;
			Invalidate();
			ReleaseCapture();
			CWnd::OnLButtonUp(nFlags, point);
		}

		int MixerFrameView::ConvertXYtoParam(int x, int y)
		{
			int xoffset(0),yoffset(0);
			return GetParamFromPos(GetColumn(x,xoffset),GetRow(x%(colwidth),y,yoffset));
		}

		bool MixerFrameView::GetViewSize(CRect& rect)
		{
			rect.left= rect.top = 0;
			rect.right = ncol * colwidth;
			rect.bottom = parspercol * uiSetting->dialheight;
			return true;
		}

		void MixerFrameView::SelectMachine(Machine* pMachine)
		{
			_pMachine = pMachine;
			UpdateSendsandChans();

			// +2 -> labels column, plus master column.
			ncol = mixer().numinputs()+mixer().numreturns()+2;
			 // + 5 -> labels row, pan, gain, mix and slider
			parspercol = mixer().numsends()+5;
			updateBuffer=true;
		}

			
		bool MixerFrameView::UpdateSendsandChans()
		{
			//int sends(0),cols(0);
			Machine** machines=Global::song()._pMachine;
			for (int i=0; i<mixer().numreturns(); i++)
			{
				if (mixer().Return(i).IsValid()) {
					sendNames[i]=machines[mixer().Return(i).Wire().machine_]->GetEditName();
					//sends++;
				}
				else sendNames[i]="";
			}
			/*for (int i=0; i<mixer().numinputs(); i++)
			{
				if (mixer().ChannelValid(i)) cols++;
			}*/

			if ( numSends != mixer().numreturns()/*sends*/ || numChans != mixer().numinputs()/*cols*/)
			{
				//numSends= sends; numChans = cols;
				numSends = mixer().numreturns(); numChans = mixer().numinputs();
				return true;
			}
			return false;
		}

		void MixerFrameView::GenerateBackground(CDC &bufferDC, CDC &SliderbackDC, CDC& SliderKnobDC, CDC &VuOnDC, CDC& VuOffDC)
		{
			// Draw to buffer.
			// Column 0, Labels.
			int xoffset(0), yoffset(0);
			int infowidth(colwidth-uiSetting->dialwidth);
			for (int i=0; i <mixer().numsends();i++)
			{
				yoffset+=uiSetting->dialheight;
				std::string sendtxt = "Send ";
				if ( i < 9 )sendtxt += ('1'+i);
				else { sendtxt += '1'; sendtxt += ('0'+i-9); }
				if ( mixer().SendValid(i))
				{
					InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),sendNames[i].c_str());
				}
				else InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),"");
			}
			yoffset+=uiSetting->dialheight;
			InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,"Mix","");
			yoffset+=uiSetting->dialheight;
			InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,"Gain","");
			yoffset+=uiSetting->dialheight;
			InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,"Pan","");

			yoffset+=uiSetting->sliderheight;
			InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,"Ch. Input","");

			// Column 1 master Volume.
			xoffset+=colwidth;
			yoffset=0;
			std::string mastertxt = "Master Out";
			InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,mastertxt.c_str(),"");

			yoffset+=(mixer().numsends()+1)*uiSetting->dialheight;
			InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"D/W","");
			yoffset+=uiSetting->dialheight;
			InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Gain","");
			yoffset+=uiSetting->dialheight;
			InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Pan","");

			InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,"Level","");
			VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,0);
			yoffset+=uiSetting->dialheight;
			GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,0);
			bufferDC.Draw3dRect(xoffset-1,0-1,colwidth+1,yoffset+1,uiSetting->titleColor,uiSetting->titleColor);


			// Columns 2 onwards, controls
			xoffset+=colwidth;
			for (int i=0; i<mixer().numinputs(); i++)
			{
				yoffset=0;
				std::string chantxt = mixer().GetAudioInputName(int(i+Mixer::chan1));
				if (mixer().ChannelValid(i))
				{
					InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,chantxt.c_str(),Global::song()._pMachine[mixer()._inputMachines[i]]->_editName);
				}
				else InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,chantxt.c_str(),"");
				yoffset+=uiSetting->dialheight;
				for (int j=0; j<mixer().numsends(); j++)
				{
					InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Send","");
					yoffset+=uiSetting->dialheight;
				}
				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Mix","");
				yoffset+=uiSetting->dialheight;
				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Gain","");
				yoffset+=uiSetting->dialheight;
				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Pan","");

				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,"Level","");
				VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,0);
				yoffset+=uiSetting->dialwidth;
				GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,0);
				xoffset+=colwidth;
			}
			for (int i=0; i<mixer().numreturns(); i++)
			{
				yoffset=0;
				std::string sendtxt = mixer().GetAudioInputName(int(i+Mixer::return1));
				if (mixer().ReturnValid(i))
				{
					InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),sendNames[i].c_str());
				}
				else InfoLabel::DrawHLight(bufferDC,xoffset,yoffset,infowidth,sendtxt.c_str(),"");
				yoffset+=(2+i)*uiSetting->dialheight;
				for (int j=i+1; j<mixer().numsends(); j++)
				{
					InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Route","");
					yoffset+=uiSetting->dialheight;
				}
				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Master","");

				yoffset=(mixer().numsends()+3)*uiSetting->dialheight;
				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset,infowidth,"Pan","");
				InfoLabel::Draw(bufferDC,xoffset+uiSetting->dialwidth,yoffset+uiSetting->sliderheight,infowidth,"Level","");
				VuMeter::Draw(bufferDC,VuOnDC,VuOffDC,xoffset+uiSetting->sliderwidth,yoffset+uiSetting->sliderheight-uiSetting->dialheight-uiSetting->vuheight,0);
				yoffset+=uiSetting->dialheight;
				GraphSlider::Draw(bufferDC,SliderbackDC,SliderKnobDC,xoffset,yoffset,0);
				bufferDC.Draw3dRect(xoffset-1,0-1,colwidth+1,yoffset+1,uiSetting->titleColor,uiSetting->titleColor);
				xoffset+=colwidth;
			}
		}
		int MixerFrameView::GetColumn(int x, int &xoffset)
		{
			int col=x/(colwidth);
			xoffset=x%(colwidth);
			if ( col < chan1+mixer().numinputs()) return col;
			else
			{
				col-=chan1+mixer().numinputs();
				return return1+col;
			}
		}
		int MixerFrameView::GetRow(int x,int y,int &yoffset)
		{
			int checkedwidth(16);
			int row = y/uiSetting->dialheight;
			yoffset=y%uiSetting->dialheight;
			if (row < send1+mixer().numsends()) return row;
			else
			{
				row-=send1+mixer().numsends();
				if (row == 0 ) return mix;
				else if (row == 1) return gain;
				else if (row == 2) return pan;
				else if (x < uiSetting->sliderwidth)
				{
					yoffset = y - (uiSetting->dialheight*(mixer().numsends()+4 ));
					return slider;
				}
				else if ( row == 3 )
				{
					if ( x < uiSetting->sliderwidth+checkedwidth)
					{
						return solo;
					}
					else return mute;
				}
				else if (row == 4)
				{
					if ( x < uiSetting->sliderwidth+checkedwidth)
					{
						return dryonly;
					}
					else return wetonly;
				}
			}
			return -1;
		}
		int MixerFrameView::GetParamFromPos(int col,int row)
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

		bool MixerFrameView::GetRouteState(int ret,int send)
		{
			if (send < sendmax)
				return mixer().Return(ret).Send(send);
			else if ( send == 13)
				return mixer().Return(ret).MasterSend();
			return false;
		}
	}   // namespace
}   // namespace
