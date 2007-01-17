/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "dialog.h"


/**
@author  Stefan Nattkemper
*/

namespace ngrs {

  class Button;
  class Label;
  class Image;

  const int nMsgCancelBtn = 0;
  const int nMsgOkBtn = 1;
  const int nMsgUseBtn = 2;

  class MessageBox : public Dialog
  {
  public:
    MessageBox();
    MessageBox(const std::string & msgText);

    ~MessageBox();

    void setButtons(int btnMask);
    void setButtonText(const std::string & ok,const std::string & use, const std::string & cancel);
    void setText(const std::string & msgText);

    Image* icon();

    signal1<Object*> use;

    virtual void setVisible(bool on);

  private:

    Button* okBtn;
    Button* cancelBtn;
    Button* useBtn;

    Image* iconImg;

    Label* textLbl;

    void onBtnPress(ButtonEvent* ev);

    void init();
  };

}

#endif
