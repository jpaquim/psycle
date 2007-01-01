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
#include "ncoloritem.h"
#include "nfontmetrics.h"

NColorItem::NColorItem()
 : NCustomItem()
{
}

NColorItem::NColorItem( const NColor & color, const std::string & text )
 : NCustomItem(), color_(color), text_(text)
{
}

NColorItem::~NColorItem()
{
}

void NColorItem::paint( NGraphics * g )
{
  int cw = clientWidth();
  int ch = clientHeight();
  int ident = 4;

  g->setForeground(color_);
  g->fillRect(ident,ident,20,ch-2*ident);

  g->setForeground(NColor(220,220,220));
  g->drawRect(ident,ident,20,ch-2*ident);

  int yp = (clientHeight() + g->textHeight() /2 ) / 2;
  g->drawText(2*ident+20,yp , text_);
}

void NColorItem::setColor( const NColor & color )
{
  color_ = color;
}

const NColor & NColorItem::color( ) const
{
  return color_;
}

int NColorItem::preferredHeight( ) const
{
  NFontMetrics metrics( font() );
  return metrics.textHeight() + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
}




