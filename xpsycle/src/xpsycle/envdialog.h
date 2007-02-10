/***************************************************************************
  *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#ifndef ENVDIALOG_H
#define ENVDIALOG_H

#include <ngrs/window.h>
#include <ngrs/panel.h>

/**
@author  Stefan Nattkemper
*/

namespace ngrs {
  class Label;
  class Slider;
  class ComboBox;
  class ItemEvent;
  class Graphics;
}

namespace psycle { 
  namespace host {

class Song;

class EnvDialog : public ngrs::Window
{
	class EnvPanel : public ngrs::Panel
	{
	public:
		EnvPanel() {};
		~EnvPanel() {};
        virtual void paint( ngrs::Graphics& g );
		void setEnvelope( int attack, int decay, int sustain, int release ) {
		  m_attack=attack;
		  m_decay=decay;
		  m_sustain=sustain;
		  m_release=release;
        }
	private:
		int m_attack;
		int m_decay;
		int m_sustain;
		int m_release;
	};
public:
	EnvDialog(Song *pSong_);
	~EnvDialog();
	void InstChanged();

	virtual int onClose();

	Song* pSong;

    ngrs::ComboBox* m_filtercombo;
	ngrs::Slider* m_envelope_slider;
	ngrs::Slider* m_q_slider;
	ngrs::Slider* m_cutoff_slider;
	ngrs::Slider* m_a_attack_slider;
	ngrs::Slider* m_a_decay_slider;
	ngrs::Slider* m_a_sustain_slider;
	ngrs::Slider* m_a_release_slider;
	ngrs::Slider* m_f_attack_slider;
	ngrs::Slider* m_f_decay_slider;
	ngrs::Slider* m_f_sustain_slider;
	ngrs::Slider* m_f_release_slider;

	ngrs::Label* m_a_a_label;
	ngrs::Label* m_a_d_label;
	ngrs::Label* m_a_s_label;
	ngrs::Label* m_a_r_label;
	ngrs::Label* m_f_a_label;
	ngrs::Label* m_f_d_label;
	ngrs::Label* m_f_s_label;
	ngrs::Label* m_f_r_label;
	ngrs::Label* m_envelope_label;
	ngrs::Label* m_q_label;
	ngrs::Label* m_cutoff_label;

	EnvPanel* m_ampframe;
	EnvPanel* m_filframe;

private:
	void init();

	void onEnvSliderMoved( ngrs::Slider* slider );
	void onSustainSliderMoved( ngrs::Slider* slider );

	void onCutoffSliderMoved( ngrs::Slider* slider );
	void onQSliderMoved( ngrs::Slider *slider );
    void onFilterModeSelected( ngrs::ItemEvent* ev );
	void onEnvAmtSliderMoved( ngrs::Slider* slider );
};

}}

#endif
