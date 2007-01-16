/***************************************************************************
 *   Copyright (C) 2006, 2007 by Stefan Nattkemper   *
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
#include "alignconstraint.h"

namespace ngrs {

  AlignConstraint::AlignConstraint() : align_(0), col_(0), row_(0)
  {
  }

  AlignConstraint::AlignConstraint( int align, int col, int row ) : align_(align), col_(col), row_(row)
  {
  }

  AlignConstraint::~AlignConstraint()
  {
  }

  void AlignConstraint::setAlign( int align )
  {
    align_ = align;
  }

  int AlignConstraint::align( ) const
  {
    return align_;
  }

  void AlignConstraint::setPosition( int col, int row )
  {
    col_ = col;
    row_ = row;
  }

  int AlignConstraint::col( ) const
  {
    return col_;
  }

  int AlignConstraint::row( ) const
  {
    return row_;
  }

}
