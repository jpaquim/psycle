/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
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
#ifndef WIREDLG_H
#define WIREDLG_H

#include "analyzer.h"
#include "wiregui.h"
#include <psycore/machine.h>
#include <ngrs/window.h>
#include <ngrs/button.h>
#include <ngrs/slider.h>
#include <ngrs/line.h>

/**
@author  Stefan
*/

namespace psy { 
	namespace host {


		class WireDlg : public ngrs::Window
		{
		public:
			WireDlg();

			~WireDlg();

			void setMachines( psy::core::Machine* pSrcMachine, psy::core::Machine* pDstMachine);

			sigslot::signal1<WireDlg*> deleteMe;

			virtual int onClose();

			psy::core::Machine* pSrcMachine();
			psy::core::Machine* pDstMachine();

			void setLine(WireGUI* line);
			WireGUI* line();

			virtual void setVisible( bool on);

		private:

			Analyzer* analyzer;

			ngrs::Slider* volSlider;
			ngrs::Slider* slider;
			ngrs::Slider* slider2;

			ngrs::Button* delBtn;
			ngrs::Button* modeBtn;
			ngrs::Button* holdBtn;

			psy::core::Machine* _pSrcMachine;
			psy::core::Machine* _pDstMachine;

			WireGUI* line_;

			void onVolPosChanged( ngrs::Slider* slider );

			void onDeleteBtn( ngrs::ButtonEvent* ev);

			int wireIdx();
		};


	}
}
#endif
