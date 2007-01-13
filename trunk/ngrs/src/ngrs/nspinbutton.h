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
#ifndef NSPINBUTTON_H
#define NSPINBUTTON_H

#include "npanel.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class NButton;
  class NImage;

  class NSpinButton : public NPanel
  {
  public:
    NSpinButton();

    ~NSpinButton();

    virtual void resize();

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    signal1<NButtonEvent*> incClick;
    signal1<NButtonEvent*> decClick;

  private:

    NButton* incBtn_;
    NButton* decBtn_;

    NImage* incImg_;
    NImage* decImg_;

    NBitmap incBit_;
    NBitmap decBit_;

    void onIncBtnClick(NButtonEvent* ev);
    void onDecBtnClick(NButtonEvent* ev);

  };

}

#endif
