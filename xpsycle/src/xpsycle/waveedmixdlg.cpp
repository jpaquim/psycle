/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/
#include "waveedmixdlg.h"
#include <ngrs/nslider.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nedit.h>
#include <ngrs/nlabel.h>
#include <ngrs/nbutton.h>
#include <ngrs/ngroupbox.h>
#include <iomanip>
#include <cmath>

namespace psycle
{
	namespace host
	{
		WaveEdMixDialog::WaveEdMixDialog()
			: NDialog()
		{
			setTitle("Mix");
			setPosition(0,0,306,158);
			setPositionToScreenCenter();
						
			m_srcVol = new NSlider();
			m_destVol = new NSlider();
			m_bFadeIn = new NCheckBox("Fade In to Mix");
			m_bFadeOut = new NCheckBox("Fade Out of Mix");
			m_fadeInTime = new NEdit();
			m_fadeOutTime = new NEdit();
			m_destVolText = new NLabel();
			m_srcVolText = new NLabel();
			m_srcVol->setPosition(33,41,18,68);
			m_destVol->setPosition(108,41,18,68);
			m_bFadeIn->setPosition(159,68,140,14);
			m_bFadeOut->setPosition(159,101,140,14);
			m_fadeInTime->setPosition(179,84,50,12);
			m_fadeOutTime->setPosition(179,117,50,12);
			m_destVolText->setPosition(100,109,50,50);
			m_srcVolText->setPosition(25,109,50,50);
			pane()->add(m_srcVol);
			pane()->add(m_destVol);
			pane()->add(m_bFadeIn);
			pane()->add(m_bFadeOut);
			pane()->add(m_fadeInTime);
			pane()->add(m_fadeOutTime);
			pane()->add(m_destVolText);
			pane()->add(m_srcVolText);

			{
				NButton * btn = new NButton("Ok", false);
				pane()->add(btn);
				btn->setPosition(242,7,55,16);
				btn->clicked.connect(this, &WaveEdMixDialog::onOkClicked);
			}
			{
				NButton * btn = new NButton("Cancel", false);
				pane()->add(btn);
				btn->setPosition(242,28,55,16);
				btn->clicked.connect(this, &WaveEdMixDialog::onCancelClicked);
			}
			//these don't appear to do anything yet, but should be uncommented once NGroupBox is working..
/*			{
				NGroupBox * grp = new NGroupBox();
				grp->setHeaderText("Wave");
				pane()->add(grp);
				grp->setPosition(91,24,60,127);
			}
			{
				NGroupBox * grp = new NGroupBox();
				grp->setHeaderText("Clipboard");
				pane()->add(grp);
				grp->setPosition(16,24,60,127);
			}
*/			
			m_srcVol->change.connect(this, &WaveEdMixDialog::onSrcSliderMoved);
			m_srcVol->setRange(0, 2000);		// 0.0% to 200.0%
			m_srcVol->setPos(1000);
			m_destVol->change.connect(this, &WaveEdMixDialog::onDestSliderMoved);
			m_destVol->setRange(0, 2000);
			m_destVol->setPos(1000);

			m_bFadeIn->setCheck(false);
			m_bFadeOut->setCheck(false);
			bFadeIn=false;
			bFadeOut=false;
			std::ostringstream temp;
			temp<<"100%";
			m_srcVolText->setText(temp.str());
			m_destVolText->setText(temp.str());
			temp.str("");
			temp<<"0.000";
			m_fadeInTime->setText(temp.str());
			m_fadeOutTime->setText(temp.str());
			srcVol=destVol=fadeInTime=fadeOutTime=0;
		}
		
		void WaveEdMixDialog::onOkClicked(NButtonEvent* ev) 
		{
			std::string temp;
			srcVol	= (2000-m_srcVol->pos())/1000.0f;
			destVol	= (2000-m_destVol->pos())/1000.0f;
			if(m_bFadeIn->checked())
			{
				temp=m_fadeInTime->text();
				bFadeIn = (fadeInTime=atof(temp.c_str()));
			}
			if(m_bFadeOut->checked())
			{
				temp = m_fadeOutTime->text();
				bFadeOut = (fadeOutTime=atof(temp.c_str()));
			}
			doClose(true);
		}

		void WaveEdMixDialog::onCancelClicked(NButtonEvent* ev) 
		{
			doClose(false);
		}


		void WaveEdMixDialog::onDestSliderMoved( NSlider *slider )
		{     
			std::ostringstream temp;
			temp.setf(std::ios::fixed);
			float vol = (2000 - slider->pos() )/10.0f;
			if(vol==0)
				temp<<"0.0%"<<std::endl<<"(-inf. dB)";
			else
			{
				float db = 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<vol<<"%"<<std::endl<<"("<<db<<"dB)";
			}
			m_destVolText->setText(temp.str());
			m_destVolText->repaint();
		}

		void WaveEdMixDialog::onSrcSliderMoved( NSlider *slider )
		{
			std::ostringstream temp;
			temp.setf(std::ios::fixed);
			float vol = (2000- slider->pos() )/10.0f;
			if(vol==0)
				temp<<"0.0%"<<std::endl<<"(-inf. dB)";
			else
			{
				float db= 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<vol<<"%"<<std::endl<<"("<<db<<"dB)";
			}
			m_srcVolText->setText(temp.str());
			m_srcVolText->repaint();
		}

		void WaveEdMixDialog::OnBnClickedFadeoutcheck( NButtonEvent* ev )
		{
			m_fadeOutTime->setEnable(m_bFadeOut->checked());
		}

		void WaveEdMixDialog::OnBnClickedFadeincheck(NButtonEvent* ev)
		{
			m_fadeInTime->setEnable(m_bFadeIn->checked());
		}

}}
