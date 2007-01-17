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
#ifndef NVISUAL_H
#define NVISUAL_H

#include "runtime.h"
#include "shape.h"
#include "keyevent.h"
#include "buttonevent.h"

/**
@author  Stefan
*/

namespace ngrs {

  class Visual : public Runtime
  {
  public:
    Visual();

    ~Visual();

    virtual void setVisible( bool on );
    bool visible() const;

    virtual void setPosition(int x, int y, int width, int height);
    virtual void setPreferredSize(int width, int height);
    virtual void setPosition(const Rect & pos);
    virtual void setLeft(int left);
    virtual void setTop(int top);
    virtual void setWidth(int width);
    virtual void setHeight(int height);
    virtual void setSize(int width, int height);

    virtual void setMinimumWidth (int minWidth);
    virtual void setMinimumHeight(int minHeight);

    virtual int top()  const;
    virtual int left() const;

    // events ..

    signal1<ButtonEvent* > mousePress;
    signal1<ButtonEvent* > mousePressed;
    signal1<ButtonEvent* > mouseDoublePress;
    signal1<Event* > mouseEnter;
    signal1<Event* > mouseExit;

    virtual void onMousePress  (int x, int y, int button);
    virtual void onMousePressed(int x, int y, int button);
    virtual void onMouseDoublePress(int x, int y, int button);
    virtual void onMouseOver(int x, int y);
    virtual void onMouseExit();
    virtual void onMouseEnter();

    virtual void setFocus();
    virtual bool focus() const;

    virtual void onEnter(); // got focus
    virtual void onExit();  // lost focus

    virtual void onKeyPress(const KeyEvent & event);
    virtual void onKeyRelease(const KeyEvent & event);

    void setCursor( int crIdentifier );
    int cursor() const;

    Shape* geometry();
    Shape* geometry() const;
    void setGeometry(Shape* geometry);

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual int minimumWidth() const;
    virtual int minimumHeight() const;

    virtual int width() const;
    virtual int height() const;

    virtual void resize();

    virtual int absoluteLeft() const;
    virtual int absoluteTop() const;

    virtual Rect clipBox() const;


  private:

    Shape* geometry_;
    bool visible_;
    int minWidth_, minHeight_;
    int cursorId_;

  };

}


#endif
