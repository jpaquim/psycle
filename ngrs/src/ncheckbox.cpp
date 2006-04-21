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
#include "ncheckbox.h"
#include "nlabel.h"

NCheckBox::NCheckBox()
 : NPanel()
{
  init();
}


NCheckBox::NCheckBox( const std::string & text ) : NPanel()
{
  init();
  label_->setText(text);
}

void NCheckBox::init( )
{
  label_ = new NLabel();
  add(label_);

  dx=dy=10;
  setWidth(100);
  setHeight(20);
  checked_=false;
}


NCheckBox::~NCheckBox()
{
}

void NCheckBox::paint( NGraphics * g )
{
  dx = 5;
  dy = 3;
  g->setForeground(NColor(255,255,255));
  g->fillRect(0+dx,0+dy,10,10);
  g->setForeground(NColor(0,0,0));
  g->drawRect(0+dx,0+dy,10,10);

  if (checked_) drawCheck(g);
}

void NCheckBox::onMousePress( int x, int y, int button )
{
  dx = 5;
  dy = 3;
  if (NRect(0+dx,0+dy,10,10).intersects(NPoint(x,y))) {
    checked_ = !checked_;
    repaint();
  }
}

void NCheckBox::drawCheck(NGraphics* g)
{
  dx = 5;
  dy = 3; // (spacingHeight()-10)/2;

  g->setForeground(NColor(0,0,0));
  g->drawLine(dx+1,dy+1,dx+9,dy+9);
  g->drawLine(dx+1,dy+9,dx+9,dy+1);

}


void NCheckBox::setCheck( bool on )
{
  checked_ = on;
}

bool NCheckBox::checked( ) const
{
  return checked_;
}

void NCheckBox::setText( const std::string & text )
{
  label_->setText(text);
}

const std::string &  NCheckBox::text( ) const
{
  return label_->text();
}

int NCheckBox::preferredWidth( ) const
{
  return 20 + label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
}

int NCheckBox::preferredHeight( ) const
{
  return label_->preferredHeight() + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
}

void NCheckBox::resize( )
{
  label_->setPosition(20,0,spacingWidth()-20,spacingHeight());
}

void NCheckBox::onMousePressed( int x, int y, int button )
{
  NButtonEvent ev(this,x,y,button);
  clicked.emit(&ev);
}










