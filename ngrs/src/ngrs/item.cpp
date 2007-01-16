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
#include "item.h"
#include "label.h"

namespace ngrs {

  Item::Item()
    : CustomItem()
  {
    init();
  }

  Item::Item( const std::string & text )
  {
    init();
    label_->setText(text);
  }

  void Item::init( )
  {
    CustomItem::add( label_ = new Label() );
    icon_ = 0;
    iconWidth_ = 20;
  }


  Item::~Item()
  {
  }

  void Item::resize( )
  {
    if (icon_==0)
      label_->setPosition(0,0,spacingWidth(),spacingHeight());
    else {
      icon_->setPosition(0,0,iconWidth_,spacingHeight());
      label_->setPosition(iconWidth_,0,spacingWidth()-iconWidth_,spacingHeight());
    }
  }

  int Item::preferredWidth( ) const
  {
    return label_->preferredWidth() + ( (icon_ != 0) ? iconWidth_ : 0 );
  }

  int Item::preferredHeight( ) const
  {
    return label_->preferredHeight();
  }

  std::string Item::text( ) const
  {
    return label_->text();
  }

  void Item::setText( const std::string & text )
  {
    label_->setText(text);
  }

  void Item::add( Image * icon )
  {
    icon_ = icon;
    icon_->setVAlign(nAlCenter);
    CustomItem::add(icon_);
  }

}
