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

namespace ngrs {
  class Edit;
  class NCheckBox;
  class ButtonEvent;
}

namespace psycle
{
  namespace host
  {
    class WaveEdInsertSilenceDialog : public ngrs::Dialog
    {
    public:
      WaveEdInsertSilenceDialog();
      ngrs::Edit	*m_time;
      ngrs::NCheckBox *m_atStart;
      ngrs::NCheckBox *m_atEnd;
      ngrs::NCheckBox *m_atCursor;
    public:
      enum insertPosition
      {
        at_start=0,
        at_end,
        at_cursor
      };
      float timeInSecs;
      insertPosition insertPos;
    protected:
      void onOkClicked( ngrs::ButtonEvent* ev );
      void onCancelClicked( ngrs::ButtonEvent* ev );
      void onInsStartClicked( ngrs::ButtonEvent* ev );
      void onInsEndClicked( ngrs::ButtonEvent* ev );
      void onInsCursorClicked( ngrs::ButtonEvent* ev );
    };

  }
}
