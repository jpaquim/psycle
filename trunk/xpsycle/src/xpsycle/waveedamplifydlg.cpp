///\file
///\brief implementation file for psycle::host::CWaveEdAmplifyDialog.
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
			 
			setPosition(300, 200, 264, 77);
			m_slider = new NSlider;
			m_dbedit = new NEdit;
			pane()->add(m_slider);
			pane()->add(m_dbedit);
			m_slider->setPosition(5, 30, 200, 15);
			m_dbedit->setPosition(210, 30, 25, 15);
			
			{
				NButton *btn = new NButton("OK", false);
				btn->clicked.connect(this, &WaveEdAmplifyDialog::onOkClicked);
				pane()->add(btn);
				btn->setPosition(155, 60, 50, 14);
			}
			{
				NButton *btn = new NButton("Cancel", false);
				btn->clicked.connect(this, &WaveEdAmplifyDialog::onCancelClicked);
				pane()->add(btn);
				btn->setPosition(210, 60, 50, 14);
			}
			{
				NLabel *lbl = new NLabel("Adjust volume:");
				pane()->add(lbl);
				lbl->setPosition(3, 5, 256, 15);
			}
			{
				NLabel *lbl = new NLabel("db");
				pane()->add(lbl);
				lbl->setPosition(242,30,15,15);
			}

			m_slider->setOrientation(nHorizontal);
			m_slider->posChanged.connect(this, &WaveEdAmplifyDialog::onSliderPosChanged);
			m_slider->setRange(0, 14400); 	// Don't use (-,+) range. It fucks up with the "0"
			m_slider->setPos(9600);
			m_slider->repaint();
			onSliderPosChanged(m_slider, 9600);	//needs a push
			
		}

		void WaveEdAmplifyDialog::onSliderPosChanged(NSlider*, double pos) 
		{
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
			onClose();
		}

		void WaveEdAmplifyDialog::onCancelClicked(NButtonEvent* ev) 
		{
			db_i = 0;
			onClose();
		}

}}
