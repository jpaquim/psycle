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
#include "npen.h"

NPen::NPen()
{
  lineWidth_ = 1;
  lineStyle_ = nLineSolid;
  capStyle_  = nCapButt;
  joinStyle_ = nJoinMiter;
  fillStyle_ = nFillSolid;
  function_  = nCopy;
}


NPen::~NPen()
{
}

void NPen::setLineWidth( int width )
{
  lineWidth_ = width;
}

int NPen::lineWidth( ) const
{
  return lineWidth_;
}

void NPen::setLineStyle( LineStyle style )
{
  lineStyle_ = style;
}

LineStyle NPen::lineStyle( ) const
{
  return lineStyle_;
}

void NPen::setCapStyle( CapStyle style )
{
  capStyle_ = style;
}

CapStyle NPen::capStyle( ) const
{
  return capStyle_;
}

void NPen::setJoinStyle( JoinStyle style )
{
  joinStyle_ = style;
}

JoinStyle NPen::joinStyle( ) const
{
  return joinStyle_;
}

void NPen::setFillStyle( FillStyle style )
{
  fillStyle_ = style;
}

FillStyle NPen::fillStyle( ) const
{
  return fillStyle_;
}

void NPen::setFunction( int function )
{
  function_ = function;
}

int NPen::function( ) const
{
  return function_;
}
