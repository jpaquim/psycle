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
#include "nhint.h"
#include "nlabel.h"
#include "nframeborder.h"

NHint::NHint()
 : NWindow()
{
  setDecoration(false);

  pane()->setBorder(NFrameBorder());

  label_ = new NLabel();
	label_->setSpacing(NSize(2,2,2,2));
	pane()->add(label_, nAlClient);

  pane()->setBackground(NColor(0xFF,0xFF,0xD0));
}


NHint::~NHint()
{
}

void NHint::setText( const std::string & text )
{
  label_->setText( text );
}

const std::string & NHint::text( ) const
{
  return label_->text();
}


