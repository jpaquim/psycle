/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "spinedit.h"

namespace ngrs {

  SpinEdit::SpinEdit()
    : Panel()
  {
    add( edit_ = new Edit() );
    add( spinBtn_  = new SpinButton() );

    spinBtn_->decClick.connect(this,&SpinEdit::onDecBtnClick);
    spinBtn_->incClick.connect(this,&SpinEdit::onIncBtnClick);
  }


  SpinEdit::~SpinEdit()
  {
  }

  void SpinEdit::resize( )
  {
    int cw = clientWidth();
    int ch = clientHeight();

    int prefBtnW = spinBtn_->preferredWidth();

    edit_->setPosition(0,0,cw-prefBtnW,ch);
    spinBtn_->setPosition(cw-prefBtnW,0,prefBtnW,ch);
  }

  void SpinEdit::setText( const std::string & text )
  {
    edit_->setText(text);
  }

  std::string SpinEdit::text( ) const
  {
    return edit_->text();
  }

  void SpinEdit::onIncBtnClick( ButtonEvent * ev )
  {
    incClick.emit(ev);
  }

  void SpinEdit::onDecBtnClick( ButtonEvent * ev )
  {
    decClick.emit(ev);
  }


  int SpinEdit::preferredHeight( ) const
  {
    FontMetrics metrics;
    metrics.setFont(font());
    return metrics.textHeight();// + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
  }


  int SpinEdit::preferredWidth( ) const
  { 
    FontMetrics metrics;
    metrics.setFont(font());
    return width() + spinBtn_->preferredWidth();
  }

}
