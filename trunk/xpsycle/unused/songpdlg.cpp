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
#include "songpdlg.h"
#include "song.h"
#include <ngrs/memo.h>
#include <ngrs/alignlayout.h>

namespace psycle { namespace host {

SongpDlg::SongpDlg( ) 
  : ngrs::Window(), pSong_(0)
{
  init();
}


SongpDlg::~SongpDlg()
{
}

void SongpDlg::setSong( Song * pSong )
{
  pSong_ = pSong;
}

void SongpDlg::init( )
{
  setTitle("Song properties");

  pane()->setLayout( ngrs::AlignLayout( 5, 5) );

  ngrs::Label* songTitleLb_ = new ngrs::Label("Song Title");
  pane()->add(songTitleLb_,ngrs::nAlTop);
  songTitle_    = new ngrs::Edit();
    pane()->add(songTitle_, ngrs::nAlTop);
  ngrs::Label* songCreditsLb_ = new ngrs::Label("Song Credits");
  pane()->add(songCreditsLb_,ngrs::nAlTop);
    songCredits_  = new ngrs::Edit();
  pane()->add(songCredits_, ngrs::nAlTop);
  ngrs::Label* songCommentsLb_ = new ngrs::Label("Song comments");
  pane()->add(songCommentsLb_,ngrs::nAlTop);

  ngrs::Panel* buttonPanel = new ngrs::Panel();
    buttonPanel->setLayout( ngrs::AlignLayout( 5, 5) );
    cnclBtn_ = new ngrs::Button("Cancel");
    cnclBtn_->setFlat(false);
    cnclBtn_->clicked.connect(this,&SongpDlg::onCancelBtn);
    okBtn_ = new ngrs::Button("OK");
    okBtn_->setFlat(false);
    okBtn_->clicked.connect(this,&SongpDlg::onOkBtn);
    buttonPanel->add(okBtn_, ngrs::nAlRight);
    buttonPanel->add(cnclBtn_, ngrs::nAlRight);
  pane()->add(buttonPanel,ngrs::nAlBottom);

  songComments_ = new ngrs::Memo();
    songComments_->setPreferredSize(200,200);
    songComments_->setWordWrap(true);
  pane()->add(songComments_, ngrs::nAlClient);

  pack();

}

int SongpDlg::onClose( )
{
  setVisible(false);
  return ngrs::nHideWindow;
}

void SongpDlg::setVisible( bool on )
{
  if (pSong_ && on) {
      songTitle_->setText(pSong_->name() );
      songCredits_->setText(pSong_->author() );
      songComments_->setText(pSong_->comment() );
      setPositionToScreenCenter();
  }
  ngrs::Window::setVisible(on);
}

void SongpDlg::onOkBtn( ngrs::ButtonEvent * ev )
{
  if (pSong_) {
   pSong_->setName( songTitle_->text() );
   pSong_->setAuthor( songCredits_->text() );
   pSong_->setComment( songComments_->text() );
   setVisible(false);
  }
}

void SongpDlg::onCancelBtn( ngrs::ButtonEvent * ev )
{
   setVisible(false);
}

}
}
