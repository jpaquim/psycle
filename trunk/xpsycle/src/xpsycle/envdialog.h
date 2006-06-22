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
#ifndef ENVDIALOG_H
#define ENVDIALOG_H

#include <ngrs/nwindow.h>
#include <ngrs/npanel.h>

/**
@author Stefan Nattkemper
*/

class NLabel;
class NSlider;
class NComboBox;
class NItemEvent;
class NGraphics;

namespace psycle { namespace host {

class Song;

class EnvDialog : public NWindow
{
	class EnvPanel : public NPanel
	{
	public:
		EnvPanel() {};
		~EnvPanel() {};
		virtual void paint(NGraphics* g);
		void setEnvelope(int attack, int decay, int sustain, int release) {
			m_attack=attack;
			m_decay=decay;
			m_sustain=sustain;
			m_release=release;}
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

	NComboBox	* m_filtercombo;

	NSlider		* m_envelope_slider;
	NSlider		* m_q_slider;
	NSlider		* m_cutoff_slider;
	NSlider		* m_a_attack_slider;
	NSlider		* m_a_decay_slider;
	NSlider		* m_a_sustain_slider;
	NSlider		* m_a_release_slider;
	NSlider		* m_f_attack_slider;
	NSlider		* m_f_decay_slider;
	NSlider		* m_f_sustain_slider;
	NSlider		* m_f_release_slider;

	NLabel		* m_a_a_label;
	NLabel		* m_a_d_label;
	NLabel		* m_a_s_label;
	NLabel		* m_a_r_label;
	NLabel		* m_f_a_label;
	NLabel		* m_f_d_label;
	NLabel		* m_f_s_label;
	NLabel		* m_f_r_label;
	NLabel		* m_envelope_label;
	NLabel		* m_q_label;
	NLabel		* m_cutoff_label;

	EnvPanel	* m_ampframe;
	EnvPanel	* m_filframe;

private:
	void init();

	void onEnvSliderMoved(NSlider *slider, double pos);
	void onSustainSliderMoved(NSlider *slider, double pos);

	void onCutoffSliderMoved(NSlider *slider, double pos);
	void onQSliderMoved(NSlider *slider, double pos);
	void onFilterModeSelected(NItemEvent *ev);
	void onEnvAmtSliderMoved(NSlider *slider, double pos);
};

}}

#endif
