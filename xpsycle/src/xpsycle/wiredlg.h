/***************************************************************************
*   Copyright (C) 2006 by Stefan   *
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
#include "machine.h"
#include "wiregui.h"
#include <ngrs/nwindow.h>
#include <ngrs/nbutton.h>
#include <ngrs/nslider.h>
#include <ngrs/nline.h>


/**
@author Stefan
*/

namespace psycle { 
  namespace host {


    class WireDlg : public ngrs::NWindow
    {
    public:
      WireDlg();

      ~WireDlg();

      void setMachines(Machine* pSrcMachine, Machine* pDstMachine);

      sigslot::signal1<WireDlg*> deleteMe;

      virtual int onClose();

      Machine* pSrcMachine();
      Machine* pDstMachine();

      void setLine(WireGUI* line);
      WireGUI* line();

      virtual void setVisible( bool on);

    private:

      Analyzer* analyzer;

      ngrs::NSlider* volSlider;
      ngrs::NSlider* slider;
      ngrs::NSlider* slider2;

      ngrs::NButton* delBtn;
      ngrs::NButton* modeBtn;
      ngrs::NButton* holdBtn;

      Machine* _pSrcMachine;
      Machine* _pDstMachine;

      WireGUI* line_;

      void onVolPosChanged( ngrs::NSlider* slider );

      void onDeleteBtn( ngrs::NButtonEvent* ev);

      int wireIdx();
    };


  }
}
#endif
