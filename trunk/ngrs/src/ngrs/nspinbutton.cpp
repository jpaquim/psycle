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
#include "nspinbutton.h"
#include "nbutton.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif

/* XPM */
const char * arrow_inc_xpm[] = {
"12 6 2 1",
" 	c None",
".	c black",
"            ",
"     .      ",
"    ...     ",
"   .....    ",
"  .......   ",
"            "};

/* XPM */
const char * arrow_dec_xpm[] = {
"12 6 2 1",
" 	c None",
".	c black",
"            ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            "};



NSpinButton::NSpinButton()
 : NPanel()
{
  incImg_    = new NImage();
  decImg_    = new NImage();

  incBit_.createFromXpmData(arrow_inc_xpm);
  decBit_.createFromXpmData(arrow_dec_xpm);

  incImg_->setBitmap(incBit_);
  decImg_->setBitmap(decBit_);

  add( decBtn_   = new NButton(decImg_) );
  add( incBtn_   = new NButton(incImg_) );

  decBtn_->setFlat(false);
  incBtn_->setFlat(false);

  decBtn_->click.connect(this,&NSpinButton::onDecBtnClick);
  incBtn_->click.connect(this,&NSpinButton::onIncBtnClick);

  decBtn_->setRepeatMode(true);
  incBtn_->setRepeatMode(true);
}


NSpinButton::~NSpinButton()
{
}

void NSpinButton::resize( )
{
  int cw = clientWidth();
  int ch = clientHeight();

  decBtn_->setPosition(0,ch/2,cw,ch/2);
  incBtn_->setPosition(0,0,cw,ch/2);
}

int NSpinButton::preferredWidth( ) const
{
  return std::max(decBtn_->preferredWidth(),incBtn_->preferredWidth()) + spacing().left() + spacing().right() ;
}

int NSpinButton::preferredHeight( ) const
{
  return decBtn_->preferredHeight(),incBtn_->preferredHeight() + spacing().top() + spacing().bottom() + 5;
}

void NSpinButton::onIncBtnClick( NButtonEvent * ev )
{
  incClick.emit(ev);
}

void NSpinButton::onDecBtnClick( NButtonEvent * ev )
{
  decClick.emit(ev);
}


