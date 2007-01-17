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
#include "pen.h"

namespace ngrs {

  Pen::Pen()
  {
    lineWidth_ = 1;
    lineStyle_ = nLineSolid;
    capStyle_  = nCapButt;
    joinStyle_ = nJoinMiter;
    fillStyle_ = nFillSolid;
    function_  = nCopy;
  }


  Pen::~Pen()
  {
  }

  void Pen::setLineWidth( int width )
  {
    lineWidth_ = width;
  }

  int Pen::lineWidth( ) const
  {
    return lineWidth_;
  }

  void Pen::setLineStyle( LineStyle style )
  {
    lineStyle_ = style;
  }

  LineStyle Pen::lineStyle( ) const
  {
    return lineStyle_;
  }

  void Pen::setCapStyle( CapStyle style )
  {
    capStyle_ = style;
  }

  CapStyle Pen::capStyle( ) const
  {
    return capStyle_;
  }

  void Pen::setJoinStyle( JoinStyle style )
  {
    joinStyle_ = style;
  }

  JoinStyle Pen::joinStyle( ) const
  {
    return joinStyle_;
  }

  void Pen::setFillStyle( FillStyle style )
  {
    fillStyle_ = style;
  }

  FillStyle Pen::fillStyle( ) const
  {
    return fillStyle_;
  }

  void Pen::setFunction( int function )
  {
    function_ = function;
  }

  int Pen::function( ) const
  {
    return function_;
  }

}
