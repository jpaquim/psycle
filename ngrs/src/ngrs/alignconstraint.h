/***************************************************************************
 *   Copyright (C) 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef NALIGNCONSTRAINT_H
#define NALIGNCONSTRAINT_H

/**
@author  Stefan Nattkemper
*/

namespace ngrs {

  class AlignConstraint{
  public:
    AlignConstraint();
    AlignConstraint(int align, int col, int row);

    ~AlignConstraint();

    void setAlign(int align);
    int align() const;

    void setPosition(int col, int row);
    int col()  const;
    int row() const;

  private:

    int align_ , col_, row_;

  };

}

#endif
