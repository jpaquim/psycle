/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper  *
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
#ifndef FRAMEMACHINE_H
#define FRAMEMACHINE_H

#include "preset.h"
#include <ngrs/window.h>
#include <ngrs/label.h>
#include <ngrs/frameborder.h>
#include <ngrs/dialog.h>


/**
	@author  Stefan Nattkemper
*/

namespace ngrs {
	class TogglePanel;
	class Button;
	class Slider;
}

namespace psycle {
	namespace host {

		class Machine;

		const int K_XSIZE = 28;
		const int K_YSIZE = 28;
		const int K_NUMFRAMES = 63;
		const int W_ROWWIDTH = 150;


		class NewNameDlg : public ngrs::Dialog {
		public:
			NewNameDlg();
			~NewNameDlg();
		};

		class Cell : public ngrs::Panel {
		public:
			Cell();

			~Cell();

			virtual void paint( ngrs::Graphics& g );

			virtual int preferredWidth() const;
			virtual int preferredHeight() const;

		};

		class Knob: public Cell {
		public:

			Knob(int param);

			sigslot::signal3<Knob*,int,int> valueChanged;

			void setValue(int value);
			void setValueAsText(const std::string & text);

			void setRange(int min, int max);
			void setText( const std::string& text );

			virtual void onMousePress( int x, int y, int button );
			virtual void onMouseOver( int x, int y);
			virtual void onMousePressed( int x, int y, int button );

			virtual void paint( ngrs::Graphics& g );
			virtual void resize();

			virtual int preferredWidth() const;
			virtual int preferredHeight() const;


		private:

			int param_;
			int max_range;
			int min_range;
			int value_;
			std::string tvalue;

			bool istweak;
			bool finetweak;
			bool ultrafinetweak;

			//  y position of the mouse when starting a mousedrag (knob tweak)
			int sourcepoint;
			//  value of the knob in that position
			int tweakbase;

			ngrs::Label* label;
			ngrs::Label* vLabel;

		};


		class Header: public Cell {
		public:
			Header();

			void setText( const std::string& text );

			virtual void resize();

		private:
			ngrs::Label* label;
		};


		class FrameMachine : public ngrs::Window {
		public:
			FrameMachine( Machine* pMachine );

			~FrameMachine();

			virtual int onClose();
			virtual void setVisible( bool on );

			Machine* pMac();
			void updateValues();

			sigslot::signal3<int,int,int> patternTweakSlide;

		private:

			std::map< ngrs::Button*, Preset > presetMap;
			std::map<int, Knob*> knobMap;
			std::map<int, Header*> headerMap;
			Machine* pMachine_;

			ngrs::Panel* knobPanel;
			ngrs::TogglePanel* prsPanel;
			ngrs::Button* defaultPrsBtn;
			ngrs::Slider* prsBtnSlider;
			
			void init();
			void initParameterGUI();
			
			void onKnobValueChange( Knob* sender, int value, int param );

			bool loadPresets();
			Preset knobsPreset();

			void onPrsClick( ngrs::ButtonEvent* ev );
			void onLeftBtn( ngrs::ButtonEvent* ev );
			void onRightBtn( ngrs::ButtonEvent* ev );      
			void onAddPrs( ngrs::ButtonEvent* ev );
			void onLoadPrs( ngrs::ButtonEvent* ev );

		};

	}
}

#endif
