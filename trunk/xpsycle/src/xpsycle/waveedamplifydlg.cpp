/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "waveedamplifydlg.h"
#include <ngrs/nslider.h>
#include <ngrs/nedit.h>
#include <ngrs/nbutton.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <cstdlib>

namespace psycle
{
	namespace host
	{
		WaveEdAmplifyDialog::WaveEdAmplifyDialog()
			: NDialog()
		{
			setTitle("Amplify Dialog");
			NPanel* sliderPanel = new NPanel();
			sliderPanel->setLayout(NAlignLayout(5,5));
			m_slider = new NSlider;
			m_slider->setPreferredSize(250,15);
			m_dbedit = new NEdit;
			m_dbedit->setPreferredSize(50,20);
			{
				NLabel *lbl = new NLabel("db");
				sliderPanel->add(lbl,nAlRight);
			}
			sliderPanel->add(m_dbedit,nAlRight);
			sliderPanel->add(m_slider,nAlClient);

			NPanel* okPanel = new NPanel();
			okPanel->setLayout(NAlignLayout(5,5));
			{
				NButton *btn = new NButton("OK", false);
				btn->clicked.connect(this, &WaveEdAmplifyDialog::onOkClicked);
				okPanel->add(btn,nAlRight);
			}
			{
				NButton *btn = new NButton("Cancel", false);
				btn->clicked.connect(this, &WaveEdAmplifyDialog::onCancelClicked);
				okPanel->add(btn,nAlRight);
			}
			pane()->add(okPanel,nAlBottom);
			{
				NLabel *lbl = new NLabel("Adjust volume:");
				pane()->add(lbl,nAlTop);
			}
			

			m_slider->setOrientation(nHorizontal);
			pane()->add(sliderPanel,nAlTop);
			m_slider->change.connect(this, &WaveEdAmplifyDialog::onSliderPosChanged);
			m_slider->setRange(0, 14400); 	// Don't use (-,+) range. It fucks up with the "0"
			m_slider->setPos(9600);
			pack();
		}

		void WaveEdAmplifyDialog::onSliderPosChanged(NSlider* slider) 
		{
      double pos = slider->pos();
			float db =  (float) (pos-9600)*0.01f;
			std::ostringstream s;
			s<<db;
			std::string tmp = s.str();
			if (tmp[tmp.size()-1]=='.')
				tmp.resize(tmp.size()-1);
			
			m_dbedit->setText(tmp);
			m_dbedit->repaint();
		}

		void WaveEdAmplifyDialog::onOkClicked(NButtonEvent* ev) 
		{
			
			std::string db_t = m_dbedit->text();
			db_i = (int)(100*atof(db_t.c_str()));
			if(db_i)
				doClose(true);
			else
				doClose(false);
		}

		void WaveEdAmplifyDialog::onCancelClicked(NButtonEvent* ev) 
		{
			db_i = 0;
			doClose(false);
		}

}}
