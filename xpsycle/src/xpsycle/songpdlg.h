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
#ifndef SONGPDLG_H
#define SONGPDLG_H

#include <ngrs/window.h>
#include <ngrs/label.h>
#include <ngrs/edit.h>
#include <ngrs/button.h>

namespace ngrs {
  class Memo;
}

namespace psycle { 
  namespace host {

    class Song;

    /**
    @author  Stefan
    */
    class SongpDlg : public ngrs::Window
    {
    public:
      SongpDlg( );

      ~SongpDlg();

      virtual int onClose();
      virtual void setVisible( bool on );

      void setSong( Song* pSong );

    private:

      ngrs::Edit* songTitle_;
      ngrs::Edit* songCredits_;
      ngrs::Memo* songComments_;
      ngrs::Button* okBtn_;
      ngrs::Button* cnclBtn_;

      Song* pSong_;

      void init();
      void onOkBtn( ngrs::ButtonEvent* ev );
      void onCancelBtn( ngrs::ButtonEvent* ev );
    };

  }
}
#endif
