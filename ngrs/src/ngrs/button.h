/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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

#include "custombutton.h"
#include "image.h"
#include "bevelborder.h"
#include "timer.h"

namespace ngrs {

  class Window;
  class NHint;

  /**
  @author  Stefan
  */
  class Button : public CustomButton
  {
  public:
    Button();
    Button(Image* icon,int minWidth, int minHeight);
    Button(Image* icon);
    Button(const std::string & text);
    Button(const std::string & text, bool flat);
    Button(const std::string & text, int minWidth, int minHeight);

    ~Button();


    void setFlat(bool on);
    void setHint(const std::string & text);

    virtual void onMousePress (int x, int y, int button);
    virtual void onMousePressed (int x, int y, int button);


    virtual void onMouseExit();
    virtual void onMouseEnter();
    virtual void setDown(bool on);

    virtual void resize();

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    void setRepeatMode( bool on);
    void setRepeatPolicy( int interval, int startLatency = 100 );

    void setSkin( const Skin & up, const Skin & down, const Skin & over, const Skin & flat );

  private:

    bool flat_;

    Skin btnUp_;
    Skin btnOver_;
    Skin btnDown_;
    Skin btnFlat_;

    Image* icon_;

    void init();

    NHint* hint;

    bool repeatMode_;
    Timer repeatTimer;
    Timer startLatencyTimer;

    int button_;

    void onStartTimer();
    void onRepeatTimer();
  };

}

#endif
