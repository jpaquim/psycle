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
#include "ntextstatusitem.h"
#include "nlabel.h"
#include "nbevelborder.h"

NTextStatusItem::NTextStatusItem()
 : NCustomStatusItem()
{
  init();
}

NTextStatusItem::NTextStatusItem( const std::string & text )
{
  init();
  label_->setText(text);
}


void NTextStatusItem::init( )
{
  label_ = new NLabel();
  add(label_);
  label_->setBorder(NBevelBorder(nRaised,nLowered));
  label_->setSpacing(NSize(2,2,2,2));
}

NTextStatusItem::~NTextStatusItem()
{
}

void NTextStatusItem::setText( const std::string & text )
{
  label_->setText(text);
}

const std::string & NTextStatusItem::text( ) const
{
  return label_->text();
}

int NTextStatusItem::preferredWidth( ) const
{
  return label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
}

int NTextStatusItem::preferredHeight( ) const
{
  return label_->preferredHeight() + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
}

void NTextStatusItem::resize( )
{
  label_->setPosition(0,0,spacingWidth(), spacingHeight());
}






