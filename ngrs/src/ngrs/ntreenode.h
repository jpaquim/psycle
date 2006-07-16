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
#ifndef NTREENODE_H
#define NTREENODE_H

#include "nflipbox.h"

/**
@author Stefan Nattkemper
*/

class NCustomItem;

class NTreeNode : public NFlipBox
{


public:
    NTreeNode();

    ~NTreeNode();

    signal2<NTreeNode*, NCustomItem*> itemSelected;

    virtual void setHeader(NCustomItem* entry);
    virtual void addEntry(NCustomItem* entry);
    virtual void addNode(NTreeNode* node);


    NCustomItem* headerItem();

private:

    NPanel* entries_;
    NPanel* subNodes;

   NCustomItem* headerItem_;

    void onItemPress(NButtonEvent * ev);

};

#endif
