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

#include <ngrs/ndialog.h>

namespace ngrs{
  class NSlider;
  class NLabel;
  class NButtonEvent;
}

namespace psycle
{
  namespace host
  {
    class WaveEdCrossfadeDialog : public ngrs::NDialog
    {
    public:
      WaveEdCrossfadeDialog();
    public:
      float srcStartVol;
      float srcEndVol;
      float destStartVol;
      float destEndVol;
    protected:
      void onOkClicked( ngrs::NButtonEvent* ev );
      void onCancelClicked( ngrs::NButtonEvent* ev );
      void onSliderMoved( ngrs::NSlider* slider );
    private:
      ngrs::NSlider		*m_srcStartVol;
      ngrs::NSlider		*m_srcEndVol;
      ngrs::NSlider		*m_destStartVol;
      ngrs::NSlider		*m_destEndVol;
      ngrs::NLabel		*m_srcStartVolText;
      ngrs::NLabel		*m_srcEndVolText;
      ngrs::NLabel		*m_destStartVolText;
      ngrs::NLabel		*m_destEndVolText;
    };

  }
}
