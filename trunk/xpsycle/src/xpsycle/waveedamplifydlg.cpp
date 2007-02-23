/***************************************************************************
	*   Copyright (C) 2005 by  Stefan   *
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
#include <ngrs/slider.h>
#include <ngrs/edit.h>
#include <ngrs/button.h>
#include <ngrs/alignlayout.h>
#include <ngrs/label.h>
#include <cstdlib>

namespace psycle
{
	namespace host
	{
		WaveEdAmplifyDialog::WaveEdAmplifyDialog()
			: ngrs::Dialog()
		{
			setTitle("Amplify Dialog");
			ngrs::Panel* sliderPanel = new ngrs::Panel();
			sliderPanel->setLayout( ngrs::AlignLayout(5,5) );
			m_slider = new ngrs::Slider;
			m_slider->setPreferredSize(250,15);
			m_dbedit = new ngrs::Edit;
			m_dbedit->setPreferredSize(50,20);
			{
				ngrs::Label *lbl = new ngrs::Label("db");
				sliderPanel->add(lbl,ngrs::nAlRight);
			}
			sliderPanel->add(m_dbedit,ngrs::nAlRight);
			sliderPanel->add(m_slider,ngrs::nAlClient);

			ngrs::Panel* okPanel = new ngrs::Panel();
			okPanel->setLayout( ngrs::AlignLayout( 5, 5) );
			{
				ngrs::Button *btn = new ngrs::Button("OK", false);
				btn->clicked.connect(this, &WaveEdAmplifyDialog::onOkClicked);
				okPanel->add(btn,ngrs::nAlRight);
			}
			{
				ngrs::Button *btn = new ngrs::Button("Cancel", false);
				btn->clicked.connect(this, &WaveEdAmplifyDialog::onCancelClicked);
				okPanel->add(btn,ngrs::nAlRight);
			}
			pane()->add( okPanel, ngrs::nAlBottom );
			{
				ngrs::Label *lbl = new ngrs::Label("Adjust volume:");
				pane()->add( lbl, ngrs::nAlTop );
			}
			

			m_slider->setOrientation( ngrs::nHorizontal );
			pane()->add( sliderPanel, ngrs::nAlTop );
			m_slider->change.connect( this, &WaveEdAmplifyDialog::onSliderPosChanged );
			m_slider->setRange( 0, 14400 ); 	// Don't use (-,+) range. It fucks up with the "0"
			m_slider->setPos( 9600 );
			pack();
		}

		void WaveEdAmplifyDialog::onSliderPosChanged( ngrs::Slider* slider )
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

		void WaveEdAmplifyDialog::onOkClicked( ngrs::ButtonEvent* ev )
		{
			std::string db_t = m_dbedit->text();
			db_i = (int)(100*atof(db_t.c_str()));
			if(db_i)
				doClose(true);
			else
				doClose(false);
		}

		void WaveEdAmplifyDialog::onCancelClicked( ngrs::ButtonEvent* ev ) 
		{
			db_i = 0;
			doClose(false);
		}

}
}
