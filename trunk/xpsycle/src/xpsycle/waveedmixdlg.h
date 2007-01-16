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
#include <ngrs/dialog.h>

namespace ngrs {
  class Slider;
  class NCheckBox;
  class Edit;
  class Label;
  class ButtonEvent;
}
namespace psycle
{
  namespace host
  {
    class WaveEdMixDialog : public ngrs::Dialog
    {
    public:
      WaveEdMixDialog();
      ngrs::Slider		*m_srcVol;
      ngrs::Slider		*m_destVol;
      ngrs::NCheckBox	*m_bFadeIn;
      ngrs::NCheckBox	*m_bFadeOut;
      ngrs::Edit		*m_fadeInTime;
      ngrs::Edit		*m_fadeOutTime;
      ngrs::Label		*m_destVolText;
      ngrs::Label		*m_srcVolText;
    public:
      float srcVol;
      float destVol;
      bool bFadeIn;
      bool bFadeOut;
      float fadeInTime;
      float fadeOutTime;
    protected:
      void onOkClicked( ngrs::ButtonEvent *ev );
      void onCancelClicked( ngrs::ButtonEvent *ev );
    public:
      void onDestSliderMoved( ngrs::Slider* slider );
      void onSrcSliderMoved( ngrs::Slider* slider );
      void OnBnClickedFadeoutcheck( ngrs::ButtonEvent *ev );
      void OnBnClickedFadeincheck( ngrs::ButtonEvent *ev );
    };

  }
}
