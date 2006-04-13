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
#ifndef NLAYOUT_H
#define NLAYOUT_H

#include "ngraphics.h"
#include "nregion.h"

const short nAlNone   = 0;
const short nAlLeft   = 1;
const short nAlTop    = 2;
const short nAlRight  = 3;
const short nAlBottom = 4;
const short nAlClient = 5;
const short nAlCenter = 6;

/**
@author Stefan
*/

class NLayout{
public:
    NLayout();

    virtual ~NLayout() = 0; //{};

    virtual void align(class NVisualComponent* parent);
    virtual int preferredWidth(const class NVisualComponent* target) const = 0;
    virtual int preferredHeight(const class NVisualComponent* target) const = 0;

    void setParent(class NVisualComponent* parent);
    class NVisualComponent* parent() const;

    virtual void drawComponents(class NVisualComponent* target, NGraphics* g , const NRegion & repaintArea, NVisualComponent* sender);

private:

   class NVisualComponent* parent_;

};

#endif
