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
#ifndef INSTRUMENTEDITOR_H
#define INSTRUMENTEDITOR_H

#include <ngrs/window.h>

/**
	@author  Stefan Nattkemper
*/

namespace ngrs {
	class Label;
	class Button;
	class Edit;
	class ComboBox;
	class Slider;
	class NCheckBox;
	class ItemEvent;
}

namespace psycle {
	namespace host{

		class EnvDialog;
		class Song;
		
		class InstrumentEditor : public ngrs::Window
		{
		public:
			InstrumentEditor( Song * song );

			~InstrumentEditor();

			void setInstrument( int index );
			int instrumentIndex();

			virtual int onClose();

		private:

			Song* _pSong;

			ngrs::Label* instNumberLbl;
			ngrs::Button* incInstBtn;
			ngrs::Button* decInstBtn;
			ngrs::Edit* instNameEd;
			ngrs::Button* killBtn;
			ngrs::ComboBox* newNoteActionCb;
			ngrs::Slider* panningSlider;
			ngrs::Label* panningLbl;
			ngrs::NCheckBox* rndPanningCbx;
			ngrs::NCheckBox* rndVCFCutCbx;
			ngrs::NCheckBox* rndVCFResoCbx;
			ngrs::NCheckBox* playSampleFitCbx;
			ngrs::Edit* patRowEdt;
			ngrs::Slider* volumeSlider;
			ngrs::Label* volumeLbl;
			ngrs::Slider* fineTuneSlider;
			ngrs::Label* fineTuneLbl;
			ngrs::Button* octDecBtn;
			ngrs::Button* noteDecBtn;
			ngrs::Button* noteIncBtn;
			ngrs::Button* octIncBtn;
			ngrs::Label* octLbl;
			ngrs::Label* loopLbl;
			ngrs::Label* loopAtFromLbl;
			ngrs::Label* loopAtToLbl;
			ngrs::Label* lenLbl;

			EnvDialog* envelopeEditor;

			void init();
			std::string noteToString( int value );
			void onBtnPress( ngrs::ButtonEvent* ev );
			void onComboSelected( ngrs::ItemEvent* ev );
			void onSliderMove( ngrs::Slider* sender );
			void onShowEnvelopeEditor( ngrs::ButtonEvent* ev );
			void onPatRowEdit( const ngrs::KeyEvent& event );
		};

	}
}

#endif
