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
#include "spinbutton.h"
#include "button.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace ngrs {

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
    "            "}
  ;

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
    "            "}
  ;

  SpinButton::SpinButton()
    : Panel()
  {
    incImg_    = new Image();
    decImg_    = new Image();

    incBit_.createFromXpmData(arrow_inc_xpm);
    decBit_.createFromXpmData(arrow_dec_xpm);

    incImg_->setBitmap(incBit_);
    decImg_->setBitmap(decBit_);

    add( decBtn_   = new Button(decImg_) );
    add( incBtn_   = new Button(incImg_) );

    decBtn_->setFlat(false);
    incBtn_->setFlat(false);

    decBtn_->click.connect(this,&SpinButton::onDecBtnClick);
    incBtn_->click.connect(this,&SpinButton::onIncBtnClick);

    decBtn_->setRepeatMode(true);
    incBtn_->setRepeatMode(true);
  }


  SpinButton::~SpinButton()
  {
  }

  void SpinButton::resize( )
  {
    int cw = clientWidth();
    int ch = clientHeight();

    decBtn_->setPosition(0,ch/2,cw,ch/2);
    incBtn_->setPosition(0,0,cw,ch/2);
  }

  int SpinButton::preferredWidth( ) const
  {
    return std::max(decBtn_->preferredWidth(),incBtn_->preferredWidth()) + spacing().left() + spacing().right() ;
  }

  int SpinButton::preferredHeight( ) const
  {
    return decBtn_->preferredHeight(),incBtn_->preferredHeight() + spacing().top() + spacing().bottom() + 5;
  }

  void SpinButton::onIncBtnClick( ButtonEvent * ev )
  {
    incClick.emit(ev);
  }

  void SpinButton::onDecBtnClick( ButtonEvent * ev )
  {
    decClick.emit(ev);
  }

}
