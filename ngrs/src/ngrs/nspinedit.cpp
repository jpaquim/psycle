/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "nspinedit.h"

NSpinEdit::NSpinEdit()
 : NPanel()
{
  add( edit_ = new NEdit() );
  add( spinBtn_  = new NSpinButton() );

  spinBtn_->decClicked.connect(this,&NSpinEdit::onDecBtnClicked);
  spinBtn_->incClicked.connect(this,&NSpinEdit::onIncBtnClicked);
}


NSpinEdit::~NSpinEdit()
{
}

void NSpinEdit::resize( )
{
  int cw = clientWidth();
  int ch = clientHeight();

  int prefBtnW = spinBtn_->preferredWidth();

  edit_->setPosition(0,0,cw-prefBtnW,ch);
  spinBtn_->setPosition(cw-prefBtnW,0,prefBtnW,ch);
}

void NSpinEdit::setText( const std::string & text )
{
  edit_->setText(text);
}

std::string NSpinEdit::text( ) const
{
  return edit_->text();
}

void NSpinEdit::onIncBtnClicked( NButtonEvent * ev )
{
  incClicked.emit(ev);
}

void NSpinEdit::onDecBtnClicked( NButtonEvent * ev )
{
  decClicked.emit(ev);
}


int NSpinEdit::preferredHeight( ) const
{
  NFontMetrics metrics;
  metrics.setFont(font());
  return metrics.textHeight();// + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
}


int NSpinEdit::preferredWidth( ) const
{ 
  NFontMetrics metrics;
  metrics.setFont(font());
  return width() + spinBtn_->preferredWidth();
}
