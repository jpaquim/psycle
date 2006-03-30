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
#ifndef NFLOWLAYOUT_H
#define NFLOWLAYOUT_H

#include <nlayout.h>
#include "nvisualcomponent.h"
#include "nvisualcomponent.h"

const int nAlHeight = 10;

/**
@author Stefan
*/
class NFlowLayout : public NLayout
{
public:
    NFlowLayout();
    NFlowLayout(int align);
    NFlowLayout(int align, int hgap, int vgap);

    ~NFlowLayout();

    virtual void align(NVisualComponent* parent);

    virtual int preferredWidth(const NVisualComponent* target) const;
    virtual int preferredHeight(const NVisualComponent* target) const;

    void setAlign(int align);
    void setHgap(int hgap);
    void setVgap(int vgap);
    void setLineBreak(bool on);
    void setBaseLine(int line);

    int align();
    int hgap();
    int vgap();

    virtual void drawComponents(class NVisualComponent* target, NGraphics* g , const NRect & repaintArea);

private:

   int maxX, maxY;
   int align_;
   int hgap_,vgap_;
   int baseLine_;

   bool lineBrk_;

   void init();

};

#endif
