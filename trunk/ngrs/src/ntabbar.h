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
#ifndef NTABBAR_H
#define NTABBAR_H

#include <npanel.h>
#include "nflowlayout.h"
#include "ntab.h"
#include "nnotebook.h"
#include "ntogglepanel.h"

class NNotebook;
class NFlowLayout;

/**
@author Stefan
*/

class NTabBar : public NTogglePanel
{
public:
    NTabBar();

    ~NTabBar();

   void addTab(NTab* tab, NPanel* page);
   void setNoteBook(NNoteBook* noteBook);
   NCustomButton* tab(NPanel* page);

   void setOrientation(int orientation);
   void setActiveTab(NPanel* page);
   void setActiveTab(unsigned int index);

private:

   int orientation_;

   NFlowLayout* fl;
   std::map<NObject*,NPanel*> pageMap_;
   NNoteBook* noteBook_;

   void onTabClick(NButtonEvent* sender);

};

#endif
