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
#include "waveedxfadedlg.h"
#include <ngrs/slider.h>
#include <ngrs/button.h>
#include <ngrs/label.h>
#include <ngrs/groupbox.h>
#include <cmath>
#include <iomanip>

namespace psycle
{
	namespace host
	{
		
		WaveEdCrossfadeDialog::WaveEdCrossfadeDialog()
			: Dialog()
		{
			setTitle("Crossfade");
			setPosition(0,0,357,161);
			setPositionToScreenCenter();
						
			m_srcStartVol = new ngrs::Slider();
			m_srcEndVol = new ngrs::Slider();
			m_destStartVol = new ngrs::Slider();
			m_destEndVol = new ngrs::Slider();
			m_srcStartVolText = new ngrs::Label();
			m_srcEndVolText = new ngrs::Label();
			m_destStartVolText = new ngrs::Label();
			m_destEndVolText = new ngrs::Label();

			m_srcStartVol->setPosition(33, 47, 19, 69);
			m_srcEndVol->setPosition(90,47,19,69);
			m_destStartVol->setPosition(177,47,19,69);
			m_destEndVol->setPosition(236,47,19,69);
			m_srcStartVolText->setPosition(13,37,58,12);
			m_srcEndVolText->setPosition(74,37,54,12);
			m_destStartVolText->setPosition(157,37,58,12);
			m_destEndVolText->setPosition(218,37,54,12);
			
			pane()->add(m_srcStartVol);
			pane()->add(m_destStartVol);
			pane()->add(m_srcEndVol);
			pane()->add(m_destEndVol);
			pane()->add(m_destStartVolText);
			pane()->add(m_srcStartVolText);
			pane()->add(m_destEndVolText);
			pane()->add(m_srcEndVolText);

			{
				ngrs::Button * btn = new ngrs::Button("Ok", false);
				pane()->add(btn);
				btn->setPosition(288,7,60,16);
				btn->clicked.connect(this, &WaveEdCrossfadeDialog::onOkClicked);
			}
			{
				ngrs::Button * btn = new ngrs::Button("Cancel", false);
				pane()->add(btn);
				btn->setPosition(288,23,60,16);
				btn->clicked.connect(this, &WaveEdCrossfadeDialog::onCancelClicked);
			}
			//these don't appear to do anything yet, but should be uncommented once ngrs::GroupBox is working..
/*			{
				GroupBox * grp = new ngrs::GroupBox();
				grp->setHeaderText("Wave");
				pane()->add(grp);
				grp->setPosition(160,17,110,125);
			}
			{
				GroupBox * grp = new ngrs::GroupBox();
				grp->setHeaderText("Clipboard");
				pane()->add(grp);
				grp->setPosition(16,17,110,125);
			}
*/			
			
			m_srcStartVol->setRange(0, 2000);		// 0.0% to 200.0%
			m_destStartVol->setRange(0, 2000);
			m_srcEndVol->setRange(0, 2000);
			m_destEndVol->setRange(0, 2000);
			m_srcStartVol->setPos(2000);
			m_destStartVol->setPos(1000);
			m_srcEndVol->setPos(1000);
			m_destEndVol->setPos(2000);
			m_srcStartVol->change.connect(this, &WaveEdCrossfadeDialog::onSliderMoved);
			m_destStartVol->change.connect(this, &WaveEdCrossfadeDialog::onSliderMoved);
			m_srcEndVol->change.connect(this, &WaveEdCrossfadeDialog::onSliderMoved);
			m_destEndVol->change.connect(this, &WaveEdCrossfadeDialog::onSliderMoved);
			

		}


		void WaveEdCrossfadeDialog::onOkClicked( ngrs::ButtonEvent* ev ) 
		{
			srcStartVol		= (2000-m_srcStartVol->pos())	/ 1000.0f;
			srcEndVol		= (2000-m_srcEndVol->pos())	/ 1000.0f;
			destStartVol	= (2000-m_destStartVol->pos())/ 1000.0f;
			destEndVol		= (2000-m_destEndVol->pos())	/ 1000.0f;
			doClose(true);
		}

		void WaveEdCrossfadeDialog::onCancelClicked( ngrs::ButtonEvent* ev )
		{
			doClose(false);
		}


		void WaveEdCrossfadeDialog::onSliderMoved( ngrs::Slider* slider )
		{
			float vol;
			std::ostringstream temp;
			temp.setf(std::ios::fixed);
			
			vol = (2000-m_srcStartVol->pos())/10.0f;
			if(vol==0)
				temp<<"0.0%"<<std::endl<<"(-inf. dB)";
			else
			{
				float db = 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<vol<<"%"<<std::endl<<"("<<db<<"dB)";
			}
			m_srcStartVolText->setText(temp.str());
			m_srcStartVolText->repaint();

			temp.str("");

			vol = (2000-m_srcEndVol->pos())/10.0f;
			if(vol==0)
				temp<<"0.0%"<<std::endl<<"(-inf. dB)";
			else
			{
				float db= 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<vol<<"%"<<std::endl<<"("<<db<<"dB)";
			}
			m_srcEndVolText->setText(temp.str());
			m_srcEndVolText->repaint();

			temp.str("");

			vol = (2000-m_destStartVol->pos())/10.0f;
			if(vol==0)
				temp<<"0.0%"<<std::endl<<"(-inf. dB)";
			else
			{
				float db = 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<vol<<"%"<<std::endl<<"("<<db<<"dB)";
			}
			m_destStartVolText->setText(temp.str());
			m_destStartVolText->repaint();

			temp.str("");

			vol = (2000-m_destEndVol->pos())/10.0f;
			if(vol==0)
				temp<<"0.0%"<<std::endl<<"(-inf. dB)";
			else
			{
				float db= 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<vol<<"%"<<std::endl<<"("<<db<<"dB)";
			}
			m_destEndVolText->setText(temp.str());
			m_destEndVolText->repaint();

		}

}}
