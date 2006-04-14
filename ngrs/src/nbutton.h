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
#ifndef NBUTTON_H
#define NBUTTON_H

#include "ncustombutton.h"
#include "nimage.h"
#include "nbevelborder.h"

/**
@author Stefan
*/
class NButton : public NCustomButton
{
public:
    NButton();
    NButton(NImage* icon,int minWidth, int minHeight);
    NButton(NImage* icon);
    NButton(const std::string & text);
    NButton(const std::string & text, bool flat);
    NButton(const std::string & text, int minWidth, int minHeight);

    ~NButton();


    void setFlat(bool on);

    virtual void onMouseExit();
    virtual void onMouseEnter();
    virtual void setDown(bool on);

    virtual void resize();

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;


private:

    bool flat_;

    NSkin btnUp_;
    NSkin btnOver_;
    NSkin btnDown_;
    NSkin btnFlat_;

    NImage* icon_;

    void init();

};

#endif
