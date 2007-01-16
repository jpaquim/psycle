/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "envdialog.h"
#include "song.h"
#include <iomanip>
#include <ngrs/item.h>
#include <ngrs/combobox.h>
#include <ngrs/slider.h>
#include <ngrs/label.h>
#include <ngrs/itemevent.h>
#include <ngrs/alignlayout.h>
#include <ngrs/tablelayout.h>
#include <ngrs/alignconstraint.h>
#include <ngrs/bevelborder.h>


namespace psycle { 
  namespace host {


    EnvDialog::EnvDialog(Song* pSong_)
      : ngrs::Window()
    {
      setPosition(10,10,500,600);
      setPositionToScreenCenter();
      setTitle("Envelope Editor");
      pane()->setLayout( ngrs::AlignLayout( 5, 5) );

      pSong = pSong_;

      m_filtercombo = new ngrs::ComboBox;

      m_envelope_slider	= new ngrs::Slider;
      m_q_slider		= new ngrs::Slider;
      m_cutoff_slider		= new ngrs::Slider;
      m_a_attack_slider	= new ngrs::Slider;
      m_a_decay_slider	= new ngrs::Slider;
      m_a_sustain_slider	= new ngrs::Slider;
      m_a_release_slider	= new ngrs::Slider;
      m_f_attack_slider	= new ngrs::Slider;
      m_f_decay_slider	= new ngrs::Slider;
      m_f_sustain_slider	= new ngrs::Slider;
      m_f_release_slider	= new ngrs::Slider;

      m_f_a_label	= new ngrs::Label;
      m_f_d_label	= new ngrs::Label;
      m_f_s_label	= new ngrs::Label;
      m_f_r_label	= new ngrs::Label;
      m_a_a_label	= new ngrs::Label;
      m_a_d_label	= new ngrs::Label;
      m_a_s_label	= new ngrs::Label;
      m_a_r_label	= new ngrs::Label;
      m_envelope_label= new ngrs::Label;
      m_q_label 	= new ngrs::Label;
      m_cutoff_label 	= new ngrs::Label;

      m_filframe = new EnvPanel;
      m_ampframe = new EnvPanel;

      m_filtercombo->setPreferredSize(75,15);

      m_envelope_slider->setPreferredSize(200,15);
      m_q_slider->setPreferredSize(200,15);
      m_cutoff_slider->setPreferredSize(300,15);
      m_a_attack_slider->setPreferredSize(300,15);
      m_a_decay_slider->setPreferredSize(300,15);
      m_a_sustain_slider->setPreferredSize(300,15);
      m_a_release_slider->setPreferredSize(300,15);
      m_f_attack_slider->setPreferredSize(300,15);
      m_f_decay_slider->setPreferredSize(300,15);
      m_f_sustain_slider->setPreferredSize(300,15);
      m_f_release_slider->setPreferredSize(300,15);

      pane()->add(new ngrs::Label("Amplitude"), ngrs::nAlTop);
      pane()->add(m_ampframe, ngrs::nAlTop);
      {
        ngrs::Panel* ampEnvPanel = new ngrs::Panel;
        ampEnvPanel->setBorder( ngrs::BevelBorder( ngrs::nNone, ngrs::nLowered ));
        ngrs::TableLayout tableLayout(3, 4);
        tableLayout.setVGap(5);
        tableLayout.setHGap(5);
        ampEnvPanel->setLayout(tableLayout);
        ampEnvPanel->add( new ngrs::Label("Attack"), ngrs::AlignConstraint( ngrs::nAlLeft, 0, 0), true);
        ampEnvPanel->add( m_a_attack_slider, ngrs::AlignConstraint( ngrs::nAlLeft, 1, 0), true);
        ampEnvPanel->add( m_a_a_label, ngrs::AlignConstraint( ngrs::nAlLeft, 2, 0), true);
        ampEnvPanel->add( new ngrs::Label("Decay"), ngrs::AlignConstraint( ngrs::nAlLeft, 0, 1), true);
        ampEnvPanel->add( m_a_decay_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 1), true);
        ampEnvPanel->add( m_a_d_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 1), true);
        ampEnvPanel->add( new ngrs::Label("Sustain"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 2), true);
        ampEnvPanel->add( m_a_sustain_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 2), true);
        ampEnvPanel->add( m_a_s_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 2), true);
        ampEnvPanel->add( new ngrs::Label("Release"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 3), true);
        ampEnvPanel->add( m_a_release_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 3), true);
        ampEnvPanel->add( m_a_r_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 3), true);
        pane()->add(ampEnvPanel, ngrs::nAlTop);
      }
      pane()->add(new ngrs::Label("Filter"), ngrs::nAlTop);
      pane()->add(m_filframe, ngrs::nAlTop);
      {
        ngrs::Panel* filtEnvPanel = new ngrs::Panel();
        filtEnvPanel->setBorder( ngrs::BevelBorder( ngrs::nNone, ngrs::nLowered) );
        ngrs::TableLayout tableLayout( 3, 4 );
        tableLayout.setVGap( 5 );
        tableLayout.setHGap( 5 );
        filtEnvPanel->setLayout( tableLayout );
        filtEnvPanel->add( new ngrs::Label("Attack"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 0), true);
        filtEnvPanel->add( m_f_attack_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 0), true);
        filtEnvPanel->add( m_f_a_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 0), true);
        filtEnvPanel->add( new ngrs::Label("Decay"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 1), true);
        filtEnvPanel->add( m_f_decay_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 1), true);
        filtEnvPanel->add( m_f_d_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 1), true);
        filtEnvPanel->add( new ngrs::Label("Sustain"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 2), true);
        filtEnvPanel->add( m_f_sustain_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 2), true);
        filtEnvPanel->add( m_f_s_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 2), true);
        filtEnvPanel->add( new ngrs::Label("Release"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 3), true);
        filtEnvPanel->add( m_f_release_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 3), true);
        filtEnvPanel->add( m_f_r_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 3), true);
        pane()->add( filtEnvPanel, ngrs::nAlTop );
      }
      pane()->add(new ngrs::Label("Filter Parameters"), ngrs::nAlTop);
      {
        ngrs::Panel* filtParamsPanel = new ngrs::Panel;
        filtParamsPanel->setBorder( ngrs::BevelBorder( ngrs::nNone, ngrs::nLowered ));
        ngrs::TableLayout tableLayout(4, 2);
        tableLayout.setVGap(5);
        tableLayout.setHGap(5);
        filtParamsPanel->setLayout( tableLayout );
        filtParamsPanel->add(new ngrs::Label("Cutoff"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 0), true);
        filtParamsPanel->add(m_cutoff_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 0), true);
        filtParamsPanel->add(m_cutoff_label, ngrs::AlignConstraint(ngrs::nAlRight, 2, 0), true);
        filtParamsPanel->add(new ngrs::Label("Q"), ngrs::AlignConstraint(ngrs::nAlLeft, 0, 1), true);
        filtParamsPanel->add(m_q_slider, ngrs::AlignConstraint(ngrs::nAlLeft, 1, 1), true);
        filtParamsPanel->add(m_q_label, ngrs::AlignConstraint(ngrs::nAlLeft, 2, 1), true);
        filtParamsPanel->add(m_filtercombo, ngrs::AlignConstraint(ngrs::nAlRight,3,0), true);
        pane()->add(filtParamsPanel, ngrs::nAlTop);
      }

      m_ampframe->setPreferredSize(480,100);
      m_filframe->setPreferredSize(480,100);

      // Set slider ranges
      m_a_attack_slider->setRange(1,65536);
      m_a_decay_slider->setRange(1,65536);
      m_a_sustain_slider->setRange(0,100);
      m_a_release_slider->setRange(16,65536);
      m_a_attack_slider->setOrientation(ngrs::nHorizontal);
      m_a_decay_slider->setOrientation(ngrs::nHorizontal);
      m_a_sustain_slider->setOrientation(ngrs::nHorizontal);
      m_a_release_slider->setOrientation(ngrs::nHorizontal);
      m_a_attack_slider->change.connect(this, &EnvDialog::onEnvSliderMoved);
      m_a_decay_slider->change.connect(this, &EnvDialog::onEnvSliderMoved);
      m_a_sustain_slider->change.connect(this, &EnvDialog::onSustainSliderMoved);
      m_a_release_slider->change.connect(this, &EnvDialog::onEnvSliderMoved);

      m_f_attack_slider->setRange(1,65536);
      m_f_decay_slider->setRange(1,65536);
      m_f_sustain_slider->setRange(0,128);
      m_f_release_slider->setRange(16,65536);
      m_f_attack_slider->setOrientation(ngrs::nHorizontal);
      m_f_decay_slider->setOrientation(ngrs::nHorizontal);
      m_f_sustain_slider->setOrientation(ngrs::nHorizontal);
      m_f_release_slider->setOrientation(ngrs::nHorizontal);
      m_f_attack_slider->change.connect(this, &EnvDialog::onEnvSliderMoved);
      m_f_decay_slider->change.connect(this, &EnvDialog::onEnvSliderMoved);
      m_f_sustain_slider->change.connect(this, &EnvDialog::onSustainSliderMoved);
      m_f_release_slider->change.connect(this, &EnvDialog::onEnvSliderMoved);

      m_cutoff_slider->setRange(0,127);
      m_q_slider->setRange(0,127);
      m_cutoff_slider->setOrientation(ngrs::nHorizontal);
      m_q_slider->setOrientation(ngrs::nHorizontal);
      m_cutoff_slider->change.connect(this, &EnvDialog::onCutoffSliderMoved);
      m_q_slider->change.connect(this, &EnvDialog::onQSliderMoved);

      m_envelope_slider->setRange(0,256); // Don't use (-,+) range. It fucks up with the "0"
      m_envelope_slider->setOrientation(ngrs::nHorizontal);
      m_envelope_slider->change.connect(this, &EnvDialog::onEnvAmtSliderMoved);

      m_filtercombo->add(new ngrs::Item("LowPass"));
      m_filtercombo->add(new ngrs::Item("HiPass"));
      m_filtercombo->add(new ngrs::Item("BandPass"));
      m_filtercombo->add(new ngrs::Item("NotchBand"));
      m_filtercombo->add(new ngrs::Item("None"));

      m_filtercombo->itemSelected.connect(this, &EnvDialog::onFilterModeSelected);

      init();
    }


    EnvDialog::~EnvDialog()
    {
    }

    void EnvDialog::init()
    {
      int si = pSong->instSelected;

      m_filtercombo->setIndex(pSong->_pInstrument[si]->ENV_F_TP);

      // Update sliders
      m_a_attack_slider->setPos(pSong->_pInstrument[si]->ENV_AT);
      m_a_decay_slider->setPos(pSong->_pInstrument[si]->ENV_DT);
      m_a_sustain_slider->setPos(pSong->_pInstrument[si]->ENV_SL);
      m_a_release_slider->setPos(pSong->_pInstrument[si]->ENV_RT);

      m_f_attack_slider->setPos(pSong->_pInstrument[si]->ENV_F_AT);
      m_f_decay_slider->setPos(pSong->_pInstrument[si]->ENV_F_DT);
      m_f_sustain_slider->setPos(pSong->_pInstrument[si]->ENV_F_SL);
      m_f_release_slider->setPos(pSong->_pInstrument[si]->ENV_F_RT);

      m_cutoff_slider->setPos(pSong->_pInstrument[si]->ENV_F_CO);
      m_q_slider->setPos(pSong->_pInstrument[si]->ENV_F_RQ);
      m_envelope_slider->setPos(pSong->_pInstrument[si]->ENV_F_EA+128);

      m_filtercombo->repaint();

      m_a_attack_slider->repaint();
      m_a_decay_slider->repaint();
      m_a_sustain_slider->repaint();
      m_a_release_slider->repaint();
      m_f_attack_slider->repaint();
      m_f_decay_slider->repaint();
      m_f_sustain_slider->repaint();
      m_f_release_slider->repaint();
      m_cutoff_slider->repaint();
      m_q_slider->repaint();
      m_envelope_slider->repaint();

      //force initial drawing
      onEnvSliderMoved( m_a_attack_slider );
      onEnvSliderMoved( m_a_decay_slider );
      onSustainSliderMoved(m_a_sustain_slider );
      onEnvSliderMoved( m_a_release_slider );

      onEnvSliderMoved(m_f_attack_slider );
      onEnvSliderMoved(m_f_decay_slider );
      onSustainSliderMoved(m_f_sustain_slider );
      onEnvSliderMoved(m_f_release_slider );
      onCutoffSliderMoved(m_cutoff_slider );

      onQSliderMoved( m_q_slider );
      onEnvAmtSliderMoved( m_envelope_slider );
    }
    void EnvDialog::InstChanged()
    {
      init();
    }

    int EnvDialog::onClose( )
    {
      setVisible(false);
      return ngrs::nHideWindow;
    }

    void EnvDialog::onEnvSliderMoved( ngrs::Slider *slider )
    {
      double pos = slider->pos();
      int si = pSong->instSelected;

      std::ostringstream buffer;
      buffer << std::setprecision(2);
      buffer.setf(std::ios::fixed | std::ios::showpoint);
      buffer << pos*0.0226757f << " ms.";

      if(slider==m_a_attack_slider || slider==m_a_decay_slider || slider==m_a_release_slider)
      {
        if(slider==m_a_attack_slider)	//amp attack
        {
          pSong->_pInstrument[si]->ENV_AT = static_cast<int>( pos );
          m_a_a_label->setText(buffer.str());
          m_a_a_label->repaint();
        }
        else if(slider==m_a_decay_slider)
        {
          pSong->_pInstrument[si]->ENV_DT = static_cast<int>( pos );
          m_a_d_label->setText( buffer.str() );
          m_a_d_label->repaint();
        }
        else if(slider==m_a_release_slider)
        {
          pSong->_pInstrument[si]->ENV_RT = static_cast<int>( pos );
          m_a_r_label->setText(buffer.str());
          m_a_r_label->repaint();
        }
        // Update ADSR
        m_ampframe->setEnvelope(
          pSong->_pInstrument[si]->ENV_AT,
          pSong->_pInstrument[si]->ENV_DT,
          pSong->_pInstrument[si]->ENV_SL,
          pSong->_pInstrument[si]->ENV_RT);
        m_ampframe->repaint();
      }
      else if(slider==m_f_attack_slider || slider==m_f_decay_slider || slider==m_f_release_slider)
      {
        if(slider==m_f_attack_slider)	//amp attack
        {
          pSong->_pInstrument[si]->ENV_F_AT = static_cast<int>( pos );
          m_f_a_label->setText(buffer.str());
          m_f_a_label->repaint();
        }
        else if(slider==m_f_decay_slider)
        {
          pSong->_pInstrument[si]->ENV_F_DT = static_cast<int>( pos );
          m_f_d_label->setText( buffer.str() );
          m_f_d_label->repaint();
        }
        else if(slider==m_f_release_slider)
        {
          pSong->_pInstrument[si]->ENV_F_RT = static_cast<int>( pos );
          m_f_r_label->setText(buffer.str());
          m_f_r_label->repaint();
        }
        // Update filter ADSR
        m_filframe->setEnvelope(
          pSong->_pInstrument[si]->ENV_F_AT,
          pSong->_pInstrument[si]->ENV_F_DT,
          pSong->_pInstrument[si]->ENV_F_SL*100/128,
          pSong->_pInstrument[si]->ENV_F_RT);
        m_filframe->repaint();
      }
      pane()->resize();
    }

    void EnvDialog::onSustainSliderMoved( ngrs::Slider *slider )
    {
      double pos = slider->pos();
      int si=pSong->instSelected;

      std::ostringstream buffer;

      if(slider==m_a_sustain_slider)
      {
        buffer << (int)(pos) << "%";
        pSong->_pInstrument[si]->ENV_SL = static_cast<int>( pos );
        m_a_s_label->setText(buffer.str());
        m_a_s_label->repaint();
        // Update ADSR
        m_ampframe->setEnvelope(
          pSong->_pInstrument[si]->ENV_AT,
          pSong->_pInstrument[si]->ENV_DT,
          pSong->_pInstrument[si]->ENV_SL,
          pSong->_pInstrument[si]->ENV_RT);
        m_ampframe->repaint();
      }
      else	//filter sustain
      {
        buffer << (int)(pos*0.78125) << "%";	//filt sustain ranges from 0 to 128
        pSong->_pInstrument[si]->ENV_F_SL = static_cast<int>( pos );
        m_f_s_label->setText(buffer.str());
        m_f_s_label->repaint();
        // Update filter ADSR
        m_filframe->setEnvelope(
          pSong->_pInstrument[si]->ENV_F_AT,
          pSong->_pInstrument[si]->ENV_F_DT,
          pSong->_pInstrument[si]->ENV_F_SL*100/128,
          pSong->_pInstrument[si]->ENV_F_RT);
        m_filframe->repaint();
      }
      pane()->resize();
    }

    void EnvDialog::onCutoffSliderMoved( ngrs::Slider *slider )
    {
      double pos = slider->pos();
      int si=pSong->instSelected;

      pSong->_pInstrument[si]->ENV_F_CO = (int)pos;
      std::ostringstream buffer;
      buffer<<(int)pos;
      m_cutoff_label->setText(buffer.str());
      m_cutoff_label->repaint();
      pane()->resize();
    }

    void EnvDialog::onQSliderMoved( ngrs::Slider *slider )
    {
      double pos = slider->pos();
      int si=pSong->instSelected;

      pSong->_pInstrument[si]->ENV_F_RQ = static_cast<int>( pos );

      std::ostringstream buffer;
      buffer << (int)(pos*0.78740) << "%";
      m_q_label->setText(buffer.str());
      m_q_label->repaint();
      pane()->resize();
    }
    void EnvDialog::onFilterModeSelected( ngrs::ItemEvent *ev )
    {
      int si=pSong->instSelected;
      pSong->_pInstrument[si]->ENV_F_TP = m_filtercombo->selIndex();
    }
    void EnvDialog::onEnvAmtSliderMoved( ngrs::Slider *slider )
    {
      double pos = slider->pos();
      int si=pSong->instSelected;
      pSong->_pInstrument[si]->ENV_F_EA = static_cast<int>( pos ) - 128;
      std::ostringstream buffer;
      buffer<<(int)(pos*0.78125);
      m_envelope_label->setText(buffer.str());
      m_envelope_label->repaint();
      pane()->resize();
    }

    void EnvDialog::EnvPanel::paint( ngrs::Graphics&g )
    {
      int AX=(int)(m_attack/420.0f);
      int BX=(int)(m_decay/420.0f);
      int CX=m_sustain;
      int DX=(int)(m_release/420.0f);

      int CH=clientHeight();
      //	int CW=512;

      ngrs::NPoint pol[5];

      //dc.FillSolidRect(0,0,494,CH+1,0x4422CC);
      g.setForeground( ngrs::Color( 0, 0, 0) );
      g.fillRect( 0, 0, clientWidth(), CH );
      g.setForeground( ngrs::Color( 50, 50, 50 ) );
      g.drawRect( 0, 0, clientWidth(), CH );

      g.setForeground( ngrs::Color(0x4a, 0x81, 0xbb) );

      pol[0].setX( 0  );
      pol[0].setY( CH );

      pol[1].setX( AX );
      pol[1].setY( 0 );

      pol[2].setX( AX+BX );
      pol[2].setY( CH-CX );

      pol[3].setX( AX+BX );
      pol[3].setY( CH-CX );

      pol[4].setX( AX+BX+DX );
      pol[4].setY( CH );

      //CBrush newBrush(RGB(255, 128, 64));
      //dc.SelectObject(&newBrush);		
      g.fillPolygon( pol, 5 );

      g.setForeground( ngrs::Color( 0, 0, 0 ) );
      g.drawLine( AX+BX, CH-CX, AX+BX, CH );

    }

  }
}
