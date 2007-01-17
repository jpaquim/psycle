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
#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include "panel.h"

namespace ngrs {

  class Label;
  class BevelBorder;
  class Gradient;

  /**
  @author  Stefan
  */
  class CustomButton : public Panel
  {
  public:
    CustomButton();
    CustomButton(const std::string & text);

    ~CustomButton();

    void setText(const std::string & text);
    const std::string & text() const;

    void setTextHAlign(int align);
    void setTextVAlign(int align);

    virtual void setMnemonic(char c);
    char mnemonic();

    virtual void resize();
    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void onMousePress (int x, int y, int button);
    virtual void onMousePressed (int x, int y, int button);

    signal1<ButtonEvent*> click;
    signal1<ButtonEvent*> clicked;

    void setToggle(bool on);
    bool toggle() const;
    bool down() const;

    virtual void setDown(bool on);
    virtual void onMessage(Event* ev);

  protected:

    Label* label();
    Label* label() const;

  private:

    Label* label_;

    void init();

    bool down_;
    bool toggle_;

  };

}

#endif
