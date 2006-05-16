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
#ifndef NAUTOSCROLLLAYOUT_H
#define NAUTOSCROLLLAYOUT_H

#include <ngrs/nlayout.h>
#include "ngrs/nvisualcomponent.h"

/**
@author Stefan
*/
class NAutoScrollLayout : public NLayout
{
public:
    NAutoScrollLayout();
    virtual NLayout* clone()  const;   // Uses the copy constructor

    ~NAutoScrollLayout();

    virtual void align(NVisualComponent* parent);
    virtual int preferredWidth(const NVisualComponent* target) const;
    virtual int preferredHeight(const NVisualComponent* target) const;



private:

    int maxX_, maxY_;

};

#endif
