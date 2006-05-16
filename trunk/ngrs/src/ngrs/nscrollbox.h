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
#ifndef NSCROLLBOX_H
#define NSCROLLBOX_H

#include "ngrs/npanel.h"
#include "ngrs/nscrollbar.h"

const int nAlwaysVisible = 1;
const int nNoneVisible   = 2;

/**
@author Stefan
*/
class NScrollBox : public NPanel
{
public:
    NScrollBox();

    ~NScrollBox();

    virtual void resize();
    void setScrollPane(NVisualComponent* scrollPane);
    NVisualComponent* scrollPane();

    void setHScrollBarPolicy(int policy);

private:

   NScrollBar* horBar;
   NScrollBar* verBar;
   NVisualComponent* scrollPane_;

   void init();
   void onVPosChange(NObject* sender, int pos);
   void onHPosChange(NObject* sender, int pos);

};

#endif
