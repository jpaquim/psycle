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
#include "songpdlg.h"
#include "song.h"
#include <ngrs/nmemo.h>
#include <ngrs/nalignlayout.h>

namespace psycle { namespace host {

SongpDlg::SongpDlg(Song* pSong) 
  : NWindow(), pSong_(pSong)
{
  init();
}


SongpDlg::~SongpDlg()
{
}

void SongpDlg::init( )
{
  setTitle("Song properties");

  pane()->setLayout(NAlignLayout(5,5));

  NLabel* songTitleLb_ = new NLabel("Song Title");
  pane()->add(songTitleLb_,nAlTop);
  songTitle_    = new NEdit();
    pane()->add(songTitle_, nAlTop);
  NLabel* songCreditsLb_ = new NLabel("Song Credits");
  pane()->add(songCreditsLb_,nAlTop);
    songCredits_  = new NEdit();
  pane()->add(songCredits_, nAlTop);
  NLabel* songCommentsLb_ = new NLabel("Song comments");
  pane()->add(songCommentsLb_,nAlTop);

  NPanel* okPanel = new NPanel();
    okPanel->setLayout(NAlignLayout(5,5));
    okBtn_ = new NButton("OK");
    okBtn_->setFlat(false);
    okBtn_->clicked.connect(this,&SongpDlg::onOkBtn);
    okPanel->add(okBtn_, nAlRight);
  pane()->add(okPanel,nAlBottom);

  songComments_ = new NMemo();
    songComments_->setPreferredSize(200,200);
    songComments_->setWordWrap(true);
  pane()->add(songComments_, nAlClient);

  pack();

}

int SongpDlg::onClose( )
{
  setVisible(false);
  return nHideWindow;
}

void SongpDlg::setVisible( bool on )
{
  if (on) {
      songTitle_->setText(pSong_->name() );
      songCredits_->setText(pSong_->author() );
      songComments_->setText(pSong_->comment() );
      setPositionToScreenCenter();
  }
  NWindow::setVisible(on);
}

void SongpDlg::onOkBtn( NButtonEvent * ev )
{
   pSong_->setName( songTitle_->text() );
   pSong_->setAuthor( songCredits_->text() );
   pSong_->setComment( songComments_->text() );
   setVisible(false);
}

}} // end of psycle::host namespace

