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
#ifndef NCUSTOMBUTTON_H
#define NCUSTOMBUTTON_H

#include "npanel.h"

class NLabel;
class NBevelBorder;
class NGradient;

/**
@author Stefan
*/
class NCustomButton : public NPanel
{
public:
   NCustomButton();
   NCustomButton(const std::string & text);

   ~NCustomButton();

   void setText(std::string text);
   std::string text() const;

   void setTextHAlign(int align);
   void setTextVAlign(int align);

   virtual void setMnemonic(char c);
   char mnemonic();

   virtual void resize();
   virtual int preferredWidth() const;
   virtual int preferredHeight() const;

   virtual void onMousePress (int x, int y, int button);
   virtual void onMousePressed (int x, int y, int button);

   signal1<NButtonEvent*> click;
   signal1<NButtonEvent*> clicked;

   void setToggle(bool on);
   bool toggle() const;
   bool down() const;

   virtual void setDown(bool on);
   virtual void onMessage(NEvent* ev);

private:

   NLabel* label_;
   NGradient* gradient_;
   void init();

   bool down_;
   bool toggle_;

};

#endif
