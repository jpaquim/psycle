/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ngroupbox.h"
#include "nfontmetrics.h"

NGroupBox::NGroupBox()
 : NPanel()
{
//  gBorder = new GBorder(this);
//  gBorder->setHeaderText("GroupBox");
//  setBorder(gBorder,true);
}


NGroupBox::~NGroupBox()
{

}


// the class factories

extern "C" NObject* createGroupBox() {
    return new NGroupBox();
}

extern "C" void destroyGroupBox(NObject* p) {
    delete p;
}


/// GBorder Class

NBorder* NGroupBox::GBorder::clone( ) const
{
  return new GBorder(*this);
}

NGroupBox::GBorder::GBorder( )
{
  pBox = 0;
  NFontMetrics metrics;
  metrics.setFont(pBox->font());
  setSpacing(NSize(5,metrics.textHeight(),5,5));
}

NGroupBox::GBorder::GBorder(NGroupBox* groupBox )
{
  pBox = groupBox;
  NFontMetrics metrics;
  metrics.setFont(pBox->font());
  setSpacing(NSize(5,metrics.textHeight(),5,5));
}

NGroupBox::GBorder::~ GBorder( )
{
}

void NGroupBox::GBorder::paint( NGraphics * g, const NShape & geometry )
{
  g->setForeground(NColor(220,220,220));
  NRect r = geometry.rectArea();
  g->drawRoundRect(r.left(),r.top(),r.width()-1,r.height()-1,5,5);
  g->drawText(r.left() +spacing().left() ,r.top() + g->textAscent(),headerText_);
}

void NGroupBox::setHeaderText( std::string text )
{
 // gBorder->setHeaderText(text);
}

void NGroupBox::GBorder::setHeaderText( std::string text )
{
  //headerText_ = text;
}
