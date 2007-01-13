/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NMESSAGEBOX_H
#define NMESSAGEBOX_H

#include "ndialog.h"


/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class NButton;
  class NLabel;
  class NImage;

  const int nMsgCancelBtn = 0;
  const int nMsgOkBtn = 1;
  const int nMsgUseBtn = 2;

  class NMessageBox : public NDialog
  {
  public:
    NMessageBox();
    NMessageBox(const std::string & msgText);

    ~NMessageBox();

    void setButtons(int btnMask);
    void setButtonText(const std::string & ok,const std::string & use, const std::string & cancel);
    void setText(const std::string & msgText);

    NImage* icon();

    signal1<NObject*> use;

    virtual void setVisible(bool on);

  private:

    NButton* okBtn;
    NButton* cancelBtn;
    NButton* useBtn;

    NImage* iconImg;

    NLabel* textLbl;

    void onBtnPress(NButtonEvent* ev);

    void init();
  };

}

#endif
