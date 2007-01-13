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

namespace ngrs {
  class NSlider;
  class NCheckBox;
  class NEdit;
  class NLabel;
  class NButtonEvent;
}
namespace psycle
{
  namespace host
  {
    class WaveEdMixDialog : public ngrs::NDialog
    {
    public:
      WaveEdMixDialog();
      ngrs::NSlider		*m_srcVol;
      ngrs::NSlider		*m_destVol;
      ngrs::NCheckBox	*m_bFadeIn;
      ngrs::NCheckBox	*m_bFadeOut;
      ngrs::NEdit		*m_fadeInTime;
      ngrs::NEdit		*m_fadeOutTime;
      ngrs::NLabel		*m_destVolText;
      ngrs::NLabel		*m_srcVolText;
    public:
      float srcVol;
      float destVol;
      bool bFadeIn;
      bool bFadeOut;
      float fadeInTime;
      float fadeOutTime;
    protected:
      void onOkClicked( ngrs::NButtonEvent *ev );
      void onCancelClicked( ngrs::NButtonEvent *ev );
    public:
      void onDestSliderMoved( ngrs::NSlider* slider );
      void onSrcSliderMoved( ngrs::NSlider* slider );
      void OnBnClickedFadeoutcheck( ngrs::NButtonEvent *ev );
      void OnBnClickedFadeincheck( ngrs::NButtonEvent *ev );
    };

  }
}
