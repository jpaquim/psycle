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
#include "nitem.h"
#include "nlabel.h"

NItem::NItem()
 : NCustomItem()
{
  init();
}

NItem::NItem( const std::string & text )
{
  init();
  label_->setText(text);
}

void NItem::init( )
{
  NCustomItem::add( label_ = new NLabel() );
  icon_ = 0;
  iconWidth_ = 20;
}


NItem::~NItem()
{
}

void NItem::resize( )
{
 if (icon_==0)
   label_->setPosition(0,0,spacingWidth(),spacingHeight());
 else {
   icon_->setPosition(0,0,iconWidth_,spacingHeight());
   label_->setPosition(iconWidth_,0,spacingWidth()-iconWidth_,spacingHeight());
 }
}

int NItem::preferredWidth( ) const
{
  return label_->preferredWidth() + ( (icon_ != 0) ? iconWidth_ : 0 );
}

int NItem::preferredHeight( ) const
{
  return label_->preferredHeight();
}

std::string NItem::text( )
{
  return label_->text();
}

void NItem::setText( const std::string text )
{
  label_->setText(text);
}

void NItem::add( NImage * icon )
{
  icon_ = icon;
  icon_->setVAlign(nAlCenter);
  NCustomItem::add(icon_);
}




