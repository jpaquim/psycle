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
#include "global.h"
#include "song.h"
#include <ngrs/nlistlayout.h>
#include <ngrs/nmemo.h>

SongpDlg::SongpDlg()
 : NWindow()
{
  init();
}


SongpDlg::~SongpDlg()
{
}

void SongpDlg::init( )
{
  setTitle("Song properties");

  pane()->setLayout(NListLayout());

  NLabel* songTitleLb_ = new NLabel("Song Title");
  pane()->add(songTitleLb_);
  songTitle_    = new NEdit();
  songTitle_->setWidth(200);
  pane()->add(songTitle_);
  NLabel* songCreditsLb_ = new NLabel("Song Credits");
  pane()->add(songCreditsLb_);
  songCredits_  = new NEdit();
  songCredits_->setWidth(200);
  pane()->add(songCredits_);
  NLabel* songCommentsLb_ = new NLabel("Song comments");
  pane()->add(songCommentsLb_);
  songComments_ = new NMemo();
    songComments_->setPreferredSize(200,200);
    songComments_->setWordWrap(true);
  pane()->add(songComments_);
  okBtn_ = new NButton("OK");
  pane()->add(okBtn_);
  okBtn_->setWidth(200);

  pane()->updateAlign();
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
      Song* _pSong = Global::pSong();
      songTitle_->setText(_pSong->Name);
      songCredits_->setText(_pSong->Author);
      songComments_->setText(_pSong->Comment);
  }
  NWindow::setVisible(on);
}


