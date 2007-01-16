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
#include "textstatusitem.h"
#include "label.h"
#include "bevelborder.h"

namespace ngrs {

  TextStatusItem::TextStatusItem()
    : CustomStatusItem()
  {
    init();
  }

  TextStatusItem::TextStatusItem( unsigned int modelIndex )
    : CustomStatusItem( modelIndex )
  {
    init();
  }


  TextStatusItem::TextStatusItem( const std::string & text )
  {
    init();
    label_->setText(text);
  }

  void TextStatusItem::init( )
  {
    label_ = new Label();
    add(label_);
    label_->setBorder(BevelBorder(nRaised,nLowered));
    label_->setSpacing(Size(2,2,2,2));
  }

  TextStatusItem::~TextStatusItem()
  {
  }

  void TextStatusItem::setText( const std::string & text )
  {
    label_->setText(text);
  }

  const std::string & TextStatusItem::text( ) const
  {
    return label_->text();
  }

  int TextStatusItem::preferredWidth( ) const
  {
    return label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
  }

  int TextStatusItem::preferredHeight( ) const
  {
    return label_->preferredHeight() + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
  }

  void TextStatusItem::resize( )
  {
    label_->setPosition(0,0,spacingWidth(), spacingHeight());
  }

}
