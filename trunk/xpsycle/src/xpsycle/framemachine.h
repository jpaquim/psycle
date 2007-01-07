/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper  *
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

#include "presetsdlg.h"
#include <ngrs/nwindow.h>
#include <ngrs/nlabel.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ndialog.h>


class NTogglePanel;
class NButton;
class NSlider;

namespace psycle {
	namespace host {

		class Machine;

		const int K_XSIZE = 28;
		const int K_YSIZE = 28;
		const int K_NUMFRAMES = 63;
		const int W_ROWWIDTH = 150;

		/**
		@author Stefan Nattkemper
		*/

		class NewNameDlg : public NDialog {
		public:
			NewNameDlg();
			~NewNameDlg();
		};

		class Cell : public NPanel {
		public:
			Cell();

			~Cell();

			virtual void paint(NGraphics* g);

			virtual int preferredWidth() const;
			virtual int preferredHeight() const;

		};

		class Knob: public Cell {
		public:

			Knob(int param);

			signal3<Knob*,int,int> valueChanged;

			void setValue(int value);
			void setValueAsText(const std::string & text);

			void setRange(int min, int max);
			void setText(const std::string & text);

			virtual void onMousePress(int x, int y, int button);
			virtual void onMouseOver(int x, int y);
			virtual void onMousePressed(int x, int y, int button);

			virtual void paint(NGraphics* g);
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

			NLabel* label;
			NLabel* vLabel;

		};


		class Header: public Cell {
		public:
			Header();

			void setText(const std::string & text);

			virtual void resize();

		private:
			NLabel* label;
		};


		class FrameMachine : public NWindow
		{
		public:
			FrameMachine(Machine* pMachine);

			~FrameMachine();

			virtual int onClose();
			virtual void setVisible( bool on );

			Machine* pMac();
			void updateValues();

			signal3<int,int,int> patternTweakSlide;

		private:

			std::map<NButton*,Preset> presetMap;
			std::map<int, Knob*>   knobMap;
			std::map<int, Header*> headerMap;

			Machine* pMachine_;
			NPanel* knobPanel;
			NTogglePanel* prsPanel;
			NButton* defaultPrsBtn;
			NSlider* prsBtnSlider;

			Preset knobsPreset();

			void onItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
			void init();
			void initParameterGUI();

			void onPrsClick(NButtonEvent* ev);

			void onKnobValueChange(Knob* sender, int value, int param);

			void onLeftBtn(NButtonEvent* ev);
			void onRightBtn(NButtonEvent* ev);

			void loadPresets();
			void onAddPrs(NButtonEvent* ev);
			void onLoadPrs(NButtonEvent* ev);


		};

	}
}

#endif
