/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "nradiobutton.h"
#include "nlabel.h"

namespace ngrs {

  NRadioButton::NRadioButton()
    : NCustomButton()
  {
    label()->setHAlign(nAlLeft);
    label()->setVAlign(nAlTop);

    setToggle(true);
  }


  NRadioButton::~NRadioButton()
  {
  }

  void NRadioButton::paint( Graphics& g )
  {
    g.setForeground(NColor(255,255,255));
    g.fillArc(2,2,10,10,0,64*360);
    g.setForeground(NColor(0,0,0));
    g.drawArc(2,2,10,10,0,64*360);
    if ( down() ) {
      g.fillArc(4,4,6,6,0,64*360);
    }
  }

  void NRadioButton::resize( )
  {
    label()->setPosition(20,0,spacingWidth()-20,spacingHeight());
  }

  int NRadioButton::preferredWidth( ) const
  {
    if (ownerSize()) return NVisualComponent::preferredWidth();
    return 20 + label()->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
  }

  int NRadioButton::preferredHeight( ) const
  {
    if (ownerSize()) return NVisualComponent::preferredHeight();

    return label()->preferredHeight() + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
  }

  void NRadioButton::setDown( bool on )
  {
    NCustomButton::setDown(on);
    repaint();
  }

}
