/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
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
#ifndef MACPROP_H
#define MACPROP_H

#include <ngrs/nwindow.h>
#include <ngrs/nedit.h>
#include "machinegui.h"

/**
@author Stefan
*/
namespace psycle {
namespace host {

class MacProp : public NWindow
{
public:
    MacProp(MachineGUI* mGUI);

    ~MacProp();

    virtual int onClose();
    virtual void setVisible(bool on);

private:

    MachineGUI* pMGUI_;
    NEdit* nameEdit_;

    void init();
    void onOKBtn(NButtonEvent *ev);

};
}}

#endif
