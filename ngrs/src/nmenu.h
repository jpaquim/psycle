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
#ifndef NMENU_H
#define NMENU_H

#include <ncustombutton.h>
#include <npopupmenu.h>
#include "nmenuseperator.h"

/**
@author Stefan
*/
class NMenu : public NCustomButton
{
public:
    NMenu();
    NMenu(std::string text);
    NMenu(std::string text, char mnemonic);
    NMenu(std::string text, char mnemonic, std::string create);

    ~NMenu();

   virtual void add(std::string create);
   virtual void add(NRuntime* component);
   virtual void add(NCustomMenuItem* item);

   void addSeperator();

   signal2<NEvent*,NButtonEvent*> itemClicked;


   signal1<NObject*> menuEntered;
   signal1<NObject*> menuHideRequest;
   signal1<NObject*> leftPress;
   signal1<NObject*> rightPress;


   NPopupMenu* popupMenu();

   virtual void onKeyAcceleratorNotify(NKeyAccelerator accelerator);
   virtual void onMouseEnter();
   virtual void onMouseExit();

   NObject* selectedItem();

private:

    NSkin btnOver_;
    NSkin btnNone_; 

    char del;
    NObject* selectedItem_;
    NPopupMenu* popupMenu_;
    NBorder* border_;

    void init();
    void onHideRequest(NObject* sender);
    void onLeftPress (NObject* sender);
    void onRightPress(NObject* sender);

    void onItemSelected(NButtonEvent* ev);

};

#endif
