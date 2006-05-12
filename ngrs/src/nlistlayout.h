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
#ifndef NLISTLAYOUT_H
#define NLISTLAYOUT_H

#include <nlayout.h>
#include "nvisualcomponent.h"

/**
@author Stefan
*/
class NListLayout : public NLayout
{
public:
    NListLayout();
    virtual NListLayout* clone()  const;   // Uses the copy constructor


    ~NListLayout();

    virtual void align(NVisualComponent* parent);
    virtual int preferredWidth(const NVisualComponent* target) const;
    virtual int preferredHeight(const NVisualComponent* target) const;

    virtual void drawComponents( NVisualComponent* target, NGraphics* g , const NRegion & repaintArea, NVisualComponent* sender);

    void setAlign(int align);


private:

    int align_;
    int findVerticalStart( long comparator , NVisualComponent* owner);
    int maxX_, maxY_;

};

#endif
