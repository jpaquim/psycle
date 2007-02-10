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

#include <ngrs/dialog.h>

namespace ngrs{
  class Slider;
  class Label;
  class ButtonEvent;
}

namespace psycle
{
  namespace host
  {
    class WaveEdCrossfadeDialog : public ngrs::Dialog
    {
    public:
      WaveEdCrossfadeDialog();
    public:
      float srcStartVol;
      float srcEndVol;
      float destStartVol;
      float destEndVol;
    protected:
      void onOkClicked( ngrs::ButtonEvent* ev );
      void onCancelClicked( ngrs::ButtonEvent* ev );
      void onSliderMoved( ngrs::Slider* slider );
    private:
      ngrs::Slider		*m_srcStartVol;
      ngrs::Slider		*m_srcEndVol;
      ngrs::Slider		*m_destStartVol;
      ngrs::Slider		*m_destEndVol;
      ngrs::Label		*m_srcStartVolText;
      ngrs::Label		*m_srcEndVolText;
      ngrs::Label		*m_destStartVolText;
      ngrs::Label		*m_destEndVolText;
    };

  }
}
